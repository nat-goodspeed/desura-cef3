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

#include "JavaScriptObject.h"
#include "JavaScriptFactory.h"

#include "libjson.h"
#include <memory>

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
		_snprintf(szBuff, 16, "%d", m_SharedMemInfo.getSize());

		command_line->AppendSwitchWithValue("desura-jse-name", m_SharedMemInfo.getName());
		command_line->AppendSwitchWithValue("desura-jse-size", szBuff);
	}
}

bool ChromiumApp::initJSExtenderSharedMem()
{
	if (m_SharedMemInfo.getMem())
		return true;

	if (m_vJSExtenders.empty())
		return false;

	initIpc();


	class JSInfo
	{
	public:
		std::string strName;
		std::string strBinding;
	};

	std::vector<JSInfo> vInfo;

	for (size_t x = 0; x < m_vJSExtenders.size(); ++x)
	{
		JSInfo i;
		i.strBinding = m_vJSExtenders[x]->getRegistrationCode();
		i.strName = m_vJSExtenders[x]->getName();

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
	catch (std::exception &e)
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

	m_ZmqServer.bind("tcp://*:*");
	m_ZmqServer.getsockopt(ZMQ_LAST_ENDPOINT, &szPort, &nPortSize);

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
		std::vector<std::shared_ptr<zmq::message_t>> vMsgList;

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
}

void ChromiumApp::OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info)
{
}

void ChromiumApp::OnContextInitialized()
{
	for (size_t x = 0; x < m_vSchemeExtenders.size(); ++x)
		SchemeExtender::Register(m_vSchemeExtenders[x]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// CefApp
/////////////////////////////////////////////////////////////////////////////////////////////////

bool ChromiumApp::RegisterJSExtender(ChromiumDLL::JavaScriptExtenderI* extender)
{
	if (m_bInit)
		return false;

	m_vJSExtenders.push_back(extender);
	return true;
}

bool ChromiumApp::RegisterSchemeExtender(ChromiumDLL::SchemeExtenderI* extender)
{
	if (m_bInit)
		return false;

	m_vSchemeExtenders.push_back(extender);
	return true;
}

void ChromiumApp::processMessageReceived(const std::string &strFrom, const std::string &strJson)
{
	JSONNode msg;

	try
	{
		msg = JSONWorker::parse(strJson);
	}
	catch (std::exception &e)
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
	else if (msg["name"].as_string() == "JSE-Response")
	{


	}
	else if (msg["name"].as_string() == "JSE-Request")
	{
		if (msg.find("request") != msg.end())
			handled = processRequest(strFrom, msg["request"]);
	}

	if (!handled)
	{
		//TODO
	}
}

bool ChromiumApp::processRequest(const std::string &strFrom, JSONNode jsonReq)
{
	if (jsonReq.find("extender") == jsonReq.end() || jsonReq.find("command") == jsonReq.end())
	{
		//Handle better
		return false;
	}

	std::string strJsName = jsonReq["extender"].as_string();
	std::string strCommand = jsonReq["command"].as_string();

	ChromiumDLL::JavaScriptExtenderI* pExtender = NULL;

	for (size_t x = 0; x < m_vJSExtenders.size(); ++x)
	{
		if (strJsName != m_vJSExtenders[x]->getName())
			continue;

		pExtender = m_vJSExtenders[x];
		break;
	}

	if (!pExtender)
	{
		//TODO: handle this better
		return false;
	}

	if (strCommand == "FunctionCall")
	{
		if (jsonReq.find("function") == jsonReq.end() || jsonReq.find("arguments") == jsonReq.end())
		{
			//Handle better
			return false;
		}

		std::string strFunction = jsonReq["function"].as_string();
		JavaScriptFactory factory;

		ChromiumDLL::JavaScriptFunctionArgs jsFunctArgs;
		jsFunctArgs.function = strFunction.c_str();
		jsFunctArgs.factory = &factory;
		jsFunctArgs.argc = 0;
		jsFunctArgs.argv = NULL;
		jsFunctArgs.context = NULL;

		ChromiumDLL::JSObjHandle* jsArgv = NULL;
		
		JSONNode ret(JSON_NODE);
		ret.set_name("response");

		try
		{
			JSONNode node = jsonReq["arguments"];

			if (node.find("object") != node.end())
				jsFunctArgs.object = new JavaScriptObject(node["object"]);
			
			if (node.find("args") != node.end())
			{
				JSONNode a = node["args"];

				jsArgv = new ChromiumDLL::JSObjHandle[a.size()];

				for (size_t x = 0; x < a.size(); ++x)
					jsArgv[x] = new JavaScriptObject(a[x]);

				jsFunctArgs.argc = a.size();
				jsFunctArgs.argv = jsArgv;
			}

			ret.push_back(JSONNode("extender", pExtender->getName()));

			ChromiumDLL::JSObjHandle pRet = pExtender->execute(&jsFunctArgs);
			delete[] jsArgv;

			JavaScriptObject* pObj = dynamic_cast<JavaScriptObject*>(pRet.get());

			if (pObj && !pObj->isException())
			{
				JSONNode t = pObj->getNode().duplicate();
				t.set_name("result");

				ret.push_back(JSONNode("command", "FunctionReturn"));
				ret.push_back(t);
			}
			else if (pObj)
			{
				char szException[255] = { 0 };
				pObj->getStringValue(szException, 255);

				ret.push_back(JSONNode("command", "FunctionException"));
				ret.push_back(JSONNode("exception", szException));
			}
			else
			{
				JSONNode n(JSON_NULL);
				n.set_name("result");

				ret.push_back(JSONNode("command", "FunctionReturn"));
				ret.push_back(n);
			}
		}
		catch (std::exception &e)
		{
			ret.push_back(JSONNode("command", "FunctionException"));
			ret.push_back(JSONNode("exception", e.what()));

			delete [] jsArgv;
		}

		JSONNode msg(JSON_NODE);
		msg.push_back(JSONNode("name", "JSE-Response"));
		msg.push_back(ret);

		std::string strOut = msg.write();

		m_ZmqServer.send(strFrom.c_str(), strFrom.size(), ZMQ_SNDMORE);
		m_ZmqServer.send(strOut.c_str(), strOut.size(), 0);
		return true;
	}
	else
	{
		//TODO: handle this better
		return false;
	}
}