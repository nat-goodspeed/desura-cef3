/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

$LicenseInfo:firstyear=2014&license=lgpl$
Copyright (C) 2014, Linden Research, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation;
version 2.1 of the License only.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <http://www.gnu.org/licenses/>
or write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
$/LicenseInfo$
*/

#include "ChromiumApp.h"
#include "SchemeExtender.h"
#include "Controller.h"

#include "JavaScriptObject.h"
#include "JavaScriptFactory.h"
#include "JavaScriptContext.h"

#include "Exception.h"

#include "libjson.h"
#include <memory>
#include <vector>

#ifndef WIN32
#include "SharedPtr.h"
#endif


std::vector<std::string> ChromiumApp::getSchemeList()
{
	std::map<std::string, int> mSchemes;

	for (size_t x = 0; x < m_vSchemeExtenders.size(); ++x)
		mSchemes[m_vSchemeExtenders[x]->getSchemeName()]++;

	std::vector<std::string> ret;
	std::map<std::string, int>::iterator it = mSchemes.begin();

	for (; it != mSchemes.end(); ++it)
		ret.push_back(it->first);

	return ret;
}

template <>
JavaScriptContextHelper<JavaScriptExtenderRef> JavaScriptContextHelper<JavaScriptExtenderRef>::Self;


class InternalErrorScheme : public ChromiumDLL::SchemeExtenderI
{
public:
	InternalErrorScheme()
		: m_strResponse("Internal Error Page")
	{
	}

	ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI> clone(const char* schemeName)
	{
		return new InternalErrorScheme();
	}

	const char* getSchemeName()
	{
		return "internal";
	}

	const char* getHostName()
	{
		return "loaderror";
	}

	bool processRequest(const ChromiumDLL::RefPtr<ChromiumDLL::SchemeRequestI>& request, bool* redirect)
	{
		if (!request->getPostData() || request->getPostData()->getElementCount() == 0)
			return true;

		ChromiumDLL::RefPtr<ChromiumDLL::PostElementI> el = request->getPostData()->getElement(0);
		m_strResponse.resize(el->getBytesCount());
		el->getBytes(m_strResponse.size(), &m_strResponse[0]);
		return true;
	}

	size_t getResponseSize()
	{
		return m_strResponse.size();
	}

	const char* getResponseMimeType()
	{
		return "text/html";
	}

	const char* getRedirectUrl()
	{
		return NULL;
	}

	bool read(char* buffer, size_t size, size_t* readSize)
	{
		if (size < m_strResponse.size())
		{
			memcpy(buffer, m_strResponse.c_str(), size);
			*readSize = size;
			m_strResponse = m_strResponse.substr(size);
		}
		else
		{
			memcpy(buffer, m_strResponse.c_str(), m_strResponse.size());
			*readSize = m_strResponse.size();
			m_strResponse.clear();
		}

		return true;
	}

	void cancel()
	{

	}

	std::string m_strResponse;

	CEF3_IMPLEMENTREF_COUNTING(InternalErrorScheme);
};

ChromiumApp::ChromiumApp()
	: m_bInit(false)
	, m_bIsStopped(false)
	, m_bIpcInit(false)
	, m_pWorkerThread(NULL)
	, m_ZmqContext(1)
	, m_ZmqServer(m_ZmqContext, ZMQ_ROUTER)
	, m_ZmqMonitor(m_ZmqContext, m_ZmqServer)
{
	JavaScriptContextHelper<JavaScriptExtenderRef>::Self.setTarget(this);

	RegisterSchemeExtender(new InternalErrorScheme());
}

ChromiumApp::~ChromiumApp()
{
	JavaScriptContextHelper<JavaScriptExtenderRef>::Self.setTarget(NULL);

	m_ZmqMonitor.stop();
	m_bIsStopped = true;

	if (m_pWorkerThread && m_pWorkerThread->joinable())
		m_pWorkerThread->join();

	delete m_pWorkerThread;
	m_vSchemeExtenders.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// CefBrowserProcessHandler
/////////////////////////////////////////////////////////////////////////////////////////////////

void ChromiumApp::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar)
{
	m_bInit = true;
	std::vector<std::string> vSchemes = getSchemeList();

	for (size_t x = 0; x < vSchemes.size(); ++x)
		registrar->AddCustomScheme(vSchemes[x], true, false, false);
}

void ChromiumApp::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line)
{
	TraceS("Thread::BaseThread::start", "", "ChromiumApp::OnBeforeChildProcessLaunch");

	std::vector<std::string> vSchemes = getSchemeList();

	std::string strSchemes;

	for (size_t x = 0; x < vSchemes.size(); ++x)
	{
		if (strSchemes.size()>0)
			strSchemes += "&";

		strSchemes += vSchemes[x];
	}

	if (!strSchemes.empty())
		command_line->AppendSwitchWithValue("desura-schemes", strSchemes);

	if (initJSExtenderSharedMem())
	{
		char szBuff[16] = { 0 };
#ifdef WIN32
		_snprintf(szBuff, 16, "%d", m_SharedMemInfo.getSize());
#else
		snprintf(szBuff, 16, "%lu", m_SharedMemInfo.getSize());
#endif

		command_line->AppendSwitchWithValue("desura-jse-name", m_SharedMemInfo.getName());
		command_line->AppendSwitchWithValue("desura-jse-size", szBuff);
	}
}

class JSInfo
{
public:
	std::string strName;
	std::string strBinding;
};

bool ChromiumApp::initJSExtenderSharedMem()
{
	if (m_SharedMemInfo.getMem())
		return true;

	if (m_vJSExtenders.empty())
		return false;

	initIpc();

	std::vector<JSInfo> vInfo;

	for (size_t x = 0; x < m_vJSExtenders.size(); ++x)
	{
		JSInfo i;
		i.strBinding = (*m_vJSExtenders[x])->getRegistrationCode();
		i.strName = (*m_vJSExtenders[x])->getName();

		vInfo.push_back(i);
	}

	size_t nSize = 8; //4 bytes for ipc port, 4 bytes for number of entries

	for (size_t x = 0; x < vInfo.size(); ++x)
		nSize += 8 + vInfo[x].strBinding.size() + vInfo[x].strName.size();

#ifdef WIN32
	char strShared[255] = { 0 };
	_snprintf(strShared, 255, "DESURA_JSEXTENDER_%d", GetCurrentProcessId());

	if (!m_SharedMemInfo.init(strShared, nSize, false))
		return false;
#else
	assert(false);
	return false;
#endif

	char* pBuff = (char*)m_SharedMemInfo.getMem();

	writeInt(pBuff, m_nZmqPort);
	pBuff += 4;

	writeInt(pBuff, vInfo.size());
	pBuff += 4;

	for (size_t x = 0; x < vInfo.size(); ++x)
	{
		writeInt(pBuff, vInfo[x].strName.size());
		pBuff += 4;

		memcpy(pBuff, vInfo[x].strName.c_str(), vInfo[x].strName.size());
		pBuff += vInfo[x].strName.size();

		writeInt(pBuff, vInfo[x].strBinding.size());
		pBuff += 4;

		memcpy(pBuff, vInfo[x].strBinding.c_str(), vInfo[x].strBinding.size());
		pBuff += vInfo[x].strBinding.size();
	}

	return true;
}

void ChromiumApp::runThread(void* pObj)
{
	try
	{
		static_cast<ChromiumApp*>(pObj)->run();
	}
	catch (const std::exception& /*e*/)
	{
		int a = 1;
	}
}

void ChromiumApp::initIpc()
{
	if (m_bIpcInit)
		return;

	if (m_bIsStopped || m_pWorkerThread)
		return;

	char szPort[255] = { 0 };
	size_t nPortSize = sizeof(szPort);

	m_ZmqMonitor.start();
	m_ZmqServer.bind("tcp://*:*");
	m_ZmqServer.getsockopt(ZMQ_LAST_ENDPOINT, &szPort, &nPortSize);

	cef3Trace("Zmq Address: %s", szPort);

	std::string t(szPort, nPortSize);
	t = t.substr(t.find_last_of(":") + 1);
	m_nZmqPort = atoi(t.c_str());

	m_bIpcInit = true;
	m_pWorkerThread = new tthread::thread(&ChromiumApp::runThread, this);
}

void ChromiumApp::run()
{
	while (!m_bIsStopped)
	{
		std::vector<std::shared_ptr<zmq::message_t> > vMsgList;

		int64_t more = 0;
		size_t more_size = sizeof(more);

		do 
		{
			std::shared_ptr<zmq::message_t> message(new zmq::message_t);
			m_ZmqServer.recv(message.get(), 0);
			vMsgList.push_back(message);

			m_ZmqServer.getsockopt(ZMQ_RCVMORE, &more, &more_size);
		} 
		while (more);

		if (vMsgList.size() > 1)
		{
			std::string strFrom(static_cast<char*>(vMsgList[0]->data()), vMsgList[0]->size());
			std::string strData(static_cast<char*>(vMsgList[1]->data()), vMsgList[1]->size());

			processMessageReceived(strFrom, strData);
		}
	}

	m_ZmqMonitor.stop();
}

void ChromiumApp::OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info)
{
	cef3Trace("");
}

void ChromiumApp::OnContextInitialized()
{
	for (size_t x = 0; x < m_vSchemeExtenders.size(); ++x)
		SchemeExtender::Register(m_vSchemeExtenders[x].get());
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// CefApp
/////////////////////////////////////////////////////////////////////////////////////////////////

bool ChromiumApp::RegisterJSExtender(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& extender)
{
	if (m_bInit)
		return false;

	m_vJSExtenders.push_back(new JavaScriptExtenderRef(extender));
	return true;
}

bool ChromiumApp::RegisterSchemeExtender(const ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI>& extender)
{
	if (m_bInit)
		return false;

	m_vSchemeExtenders.push_back(extender);
	return true;
}

void ChromiumApp::processMessageReceived(const std::string &strFrom, const std::string &strJson)
{
	cef3Trace("From: %s, Json: %s", strFrom.c_str(), strJson.c_str());

	JSONNode msg;

	try
	{
		msg = JSONWorker::parse(strJson);
	}
	catch (const std::exception& /*e*/)
	{
		return;
	}

	if (msg.find("name") == msg.end())
		return;

	bool handled = false;

	if (msg["name"].as_string() == "Browser-Created")
	{
		handled = true;

		if (msg.find("id") != msg.end())
		{
			tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);
			m_mBrowserIdentity[msg["id"].as_int()] = strFrom;
		}
	}
	else if (msg["name"].as_string() == "Browser-Destroyed")
	{
		handled = true;

		if (msg.find("id") != msg.end())
		{
			tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);

			std::map<int, std::string>::iterator it = m_mBrowserIdentity.find(msg["id"].as_int());

			if (it != m_mBrowserIdentity.end())
				m_mBrowserIdentity.erase(it);
		}
	}
	else if (msg["name"].as_string() == "Browser-JSContextCreated")
	{
		handled = true;

		if (msg.find("browser") != msg.end())
		{
			tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);
			m_mBrowserContext[msg["browser"].as_int()] = new JavaScriptProxyObject(msg["extender"].as_string(), "__global__", msg["id"].as_int());
		}
	}
	else if (msg["name"].as_string() == "Browser-JSContextReleased")
	{
		handled = true;

		if (msg.find("browser") != msg.end())
		{
			tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);
			std::map<int, ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptObjectI> >::iterator it = m_mBrowserContext.find(msg["browser"].as_int());

			if (it != m_mBrowserContext.end())
				m_mBrowserContext.erase(it);
		}
	}
	else
	{
		CefRefPtr<JavaScriptExtenderRef> pExtender = findExtender(msg);
		int nBrowser = getBrowserId(msg);

		if (msg["name"].as_string() == "JSE-Response")
		{
			if (msg.find("response") != msg.end())
				handled = JavaScriptContextHelper<JavaScriptExtenderRef>::Self.processResponse(pExtender, nBrowser, msg["response"]);
		}
		else if (msg["name"].as_string() == "JSE-Request")
		{
			if (msg.find("request") != msg.end())
				handled = JavaScriptContextHelper<JavaScriptExtenderRef>::Self.processRequest(pExtender, nBrowser, msg["request"]);
		}
	}

	if (!handled)
	{
		cef3Trace("Request not handled (From: %s, Json: %s)", strFrom.c_str(), strJson.c_str());
		//TODO
	}
}

ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptObjectI> ChromiumApp::GetGlobalObject(int nBrowserIdentifier)
{
	if (nBrowserIdentifier & 0xFFFF0000)
		nBrowserIdentifier >>= 16;

	tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);
	std::map<int, ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptObjectI> >::iterator it = m_mBrowserContext.find(nBrowserIdentifier);

	if (it != m_mBrowserContext.end())
		return it->second;

	return NULL;
}

extern CefRefPtr<JavaScriptExtenderRef> LookupExtenderFromFunctionHolder_Browser(const std::string &strId);

CefRefPtr<JavaScriptExtenderRef> ChromiumApp::findExtender(JSONNode msg)
{
	if (msg.find("extender") == msg.end())
		return NULL;

	std::string strNameOrId = msg["extender"].as_string();

	for (size_t x = 0; x < m_vJSExtenders.size(); ++x)
	{
		if (strNameOrId == m_vJSExtenders[x]->getName())
			return m_vJSExtenders[x];
	}

	return LookupExtenderFromFunctionHolder_Browser(strNameOrId);
}

int ChromiumApp::getBrowserId(JSONNode msg)
{
	if (msg.find("browser") == msg.end())
		return -1;

	return msg["browser"].as_int();
}

bool ChromiumApp::send(int nBrowser, JSONNode msg)
{
	cef3Trace("Browser: %d", nBrowser);

	std::string strTo;

	{
		tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);
		strTo = m_mBrowserIdentity[nBrowser];
	}

	if (strTo.empty())
	{
		cef3Trace("Browser not found for id: %d", nBrowser);
		//TODO Handle
		return false;
	}

	std::string strOut = msg.write();

	cef3Trace("To: %s, Json: %s", strTo.c_str(), strOut.c_str());

	m_ZmqServer.send(strTo.c_str(), strTo.size(), ZMQ_SNDMORE);
	m_ZmqServer.send(strOut.c_str(), strOut.size(), 0);

	return true;
}



class JSContext : public ChromiumDLL::JavaScriptContextI
{
public:
	JSContext(int nBrowserId, const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFactoryI> &factory, const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptObjectI> &object)
		: m_Factory(factory)
		, m_Object(object)
		, m_nBrowserId(nBrowserId)
	{
	}

	ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptContextI> clone() OVERRIDE
	{
		return new JSContext(m_nBrowserId, m_Factory, m_Object);
	}

	void enter() OVERRIDE
	{
		cef3Trace("");
		JavaScriptContextHelper<JavaScriptExtenderRef>::Self.push(m_nBrowserId);
	}

	void exit() OVERRIDE
	{
		cef3Trace("");
		JavaScriptContextHelper<JavaScriptExtenderRef>::Self.pop(m_nBrowserId);
	}

	ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFactoryI> getFactory() OVERRIDE
	{
		return m_Factory;
	}

	ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptObjectI> getGlobalObject() OVERRIDE
	{
		return m_Object;
	}

	ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptObjectI> m_Object;
	ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFactoryI> m_Factory;
	int m_nBrowserId;
	CEF3_IMPLEMENTREF_COUNTING(JSContext);
};


class JSFunctArgs : public ChromiumDLL::JavaScriptFunctionArgs
{
public:
	JSFunctArgs(const std::string &strExtenderId, const std::string &strFunction, JSONNode o, JSONNode argumets)
		: m_Factory(new JavaScriptFactory())
	{
		int nBid = JavaScriptContextHelper<JavaScriptExtenderRef>::Self.peek();

		function = strFunction.c_str();
		factory = m_Factory;
		argc = argumets.size();
		argv = new ChromiumDLL::JSObjHandle[argumets.size()];
		object = JavaScriptObjectFactory::Create(o, strExtenderId);
		context = new JSContext(nBid, m_Factory, m_Factory->getGlobalObject(nBid));
	
		for (int x = 0; x < argc; ++x)
			argv[x] = JavaScriptObjectFactory::Create(argumets[x], strExtenderId);
	}

	~JSFunctArgs()
	{
		delete[] argv;
	}

	ChromiumDLL::RefPtr<JavaScriptFactory> m_Factory;
	CEF3_IMPLEMENTREF_COUNTING(JSFunctArgs);
};


JSONNode JavaScriptExtenderRef::execute(const std::string &strFunction, JSONNode object, JSONNode argumets)
{
	cef3Trace("Function: %s, argc: %d", strFunction.c_str(), argumets.size());

	ChromiumDLL::JSObjHandle pRet = m_pExtender->execute(new JSFunctArgs(getName(), strFunction, object, argumets));

	if (!pRet || pRet->isNull())
		return JSONNode(JSON_NULL);

	if (pRet->isException())
	{
		char szException[255] = { 0 };
		pRet->getStringValue(szException, 255);
		throw exception(szException);
	}

	return JavaScriptObjectFactory::GetNode(pRet, true);
}
