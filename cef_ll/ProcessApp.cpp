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

#include "ProcessApp.h"
#include "JavaScriptExtenderProxy.h"

#include "ChromiumBrowserI.h"
#include "JavaScriptContext.h"
#include "Controller.h"

#include <string>
#include <sstream>
#include <memory>
#include <assert.h>

#ifndef WIN32
#include "SharedPtr.h"
#endif

extern CefStringUTF8 ConvertToUtf8(const CefString& str);
CefRefPtr<CefCommandLine> g_command_line;

class JsonRequestTask : public CefTask
{
public:
	JsonRequestTask(CefRefPtr<JavaScriptExtenderProxy> &proxy, CefRefPtr<CefBrowser> &browser, JSONNode json)
		: m_Proxy(proxy)
		, m_Browser(browser)
		, m_Json(json)
	{
	}

	void Execute() OVERRIDE
	{
		CefRefPtr<CefV8Context> context = m_Browser->GetMainFrame()->GetV8Context();

		context->Enter();

		assert(false);
		//TODO
		int a = 1;

		context->Exit();
	}

	IMPLEMENT_REFCOUNTING(JsonRequestTask);

	JSONNode m_Json;
	CefRefPtr<JavaScriptExtenderProxy> m_Proxy;
	CefRefPtr<CefBrowser> m_Browser;
};

template <>
JavaScriptContextHelper<JavaScriptExtenderProxy> JavaScriptContextHelper<JavaScriptExtenderProxy>::Self;


ProcessApp::ProcessApp()
	: m_ZmqContext(1)
	, m_ZmqClient(m_ZmqContext, ZMQ_DEALER)
	, m_ZmqMonitor(m_ZmqContext, m_ZmqClient)
	, m_pWorkerThread(NULL)
	, m_bIsStopped(false)
	, m_strZmqIdentity(generateZmqName())
	, m_nZmqPort(0)
	, m_bConnected(false)
{
	JavaScriptContextHelper<JavaScriptExtenderProxy>::Self.setTarget(this);
}

ProcessApp::~ProcessApp()
{
	JavaScriptContextHelper<JavaScriptExtenderProxy>::Self.setTarget(NULL);
	
	m_bIsStopped = true;

	if (m_pWorkerThread && m_pWorkerThread->joinable())
		m_pWorkerThread->join();

	delete m_pWorkerThread;

	m_ZmqMonitor.stop();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// CefApp
/////////////////////////////////////////////////////////////////////////////////////////////////

void ProcessApp::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar)
{
	CefStringUTF8 strSchemes = ConvertToUtf8(g_command_line->GetSwitchValue("desura-schemes"));
		
	if (!strSchemes.empty())
	{
		std::stringstream ss(strSchemes.c_str());
		std::string s;

		while (std::getline(ss, s, '&'))
			registrar->AddCustomScheme(s, true, false, false);
	}
}

CefRefPtr<CefRenderProcessHandler> ProcessApp::GetRenderProcessHandler()
{
	return (CefRenderProcessHandler*)this;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// CefRenderProcessHandler
/////////////////////////////////////////////////////////////////////////////////////////////////

int ProcessApp::CreateBrowserId(const CefRefPtr<CefBrowser>& browser)
{
#ifdef WIN32
	return (browser->GetIdentifier() << 16) + (GetCurrentProcessId() & 0xFFFF);
#else
	return (browser->GetIdentifier() << 16) + (pthread_self() & 0xFFFF);
#endif
}

std::string ProcessApp::CreateBrowserExtenderId(const CefRefPtr<CefBrowser>& browser)
{
	std::ostringstream ss;
	ss << "__browser_" << browser->GetIdentifier() << "_global__";
	return ss.str();
}

void ProcessApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
	int bid = CreateBrowserId(browser);
	cef3Trace("Id: %d", bid);

	JSONNode msg(JSON_NODE);
	msg.push_back(JSONNode("name", "Browser-Created"));
	msg.push_back(JSONNode("id", bid));

	send(-1, msg);

	tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);
	m_mBrowsers[bid] = browser;
}

void ProcessApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser)
{
	int bid = CreateBrowserId(browser);
	cef3Trace("Id: %d", bid);

	JSONNode msg(JSON_NODE);
	msg.push_back(JSONNode("name", "Browser-Destroyed"));
	msg.push_back(JSONNode("id", bid));

	send(-1, msg);


	tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);
	std::map<int, CefRefPtr<CefBrowser> >::iterator it = m_mBrowsers.find(bid);

	if (it != m_mBrowsers.end())
		m_mBrowsers.erase(it);
}

void ProcessApp::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info)
{
	TraceS("Thread::BaseThread::start::", "", "ProcessApp::OnRenderThreadCreated Starting thread");

//#ifdef DEBUG
//	while (!IsDebuggerPresent())
//		Sleep(1000);
//#endif
}

void ProcessApp::OnWebKitInitialized()
{
	cef3Trace("");

	if (m_bIsStopped || m_pWorkerThread)
		return;

	CefStringUTF8 strJSEName = ConvertToUtf8(g_command_line->GetSwitchValue("desura-jse-name"));
	CefStringUTF8 strJSESize = ConvertToUtf8(g_command_line->GetSwitchValue("desura-jse-size"));
	
	if (!strJSEName.empty() && !strJSESize.empty())
	{
		int nSize = atoi(strJSESize.c_str());

		if (m_SharedMemInfo.init(strJSEName.c_str(), nSize))
		{
			char* pBuff = static_cast<char*>(m_SharedMemInfo.getMem());

			m_nZmqPort = readInt(pBuff);
			pBuff += 4;

			int nCount = readInt(pBuff);
			pBuff += 4;

			for (int x = 0; x < nCount; ++x)
			{
				int nNameSize = readInt(pBuff);
				pBuff += 4;

				std::string strName(pBuff, nNameSize);
				pBuff += nNameSize;

				int nBindingSize = readInt(pBuff);
				pBuff += 4;

				std::string strBinding(pBuff, nBindingSize);
				pBuff += nBindingSize;

				CefRefPtr<JavaScriptExtenderProxy> pJSExtender = new JavaScriptExtenderProxy(strName);
				CefRegisterExtension(strName, strBinding, CefRefPtr<CefV8Handler>(pJSExtender));
				m_vJSExtenders.push_back(pJSExtender);
			}
		}
	}

	m_pWorkerThread = new tthread::thread(&ProcessApp::runThread, this);
}

void ProcessApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	int bid = CreateBrowserId(browser);
	cef3Trace("Id: %d", bid);

	std::string strNameOrId = CreateBrowserExtenderId(browser);

	{
		tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);
		m_mBrowserProxies[strNameOrId] = new JavaScriptGlobalObjectProxy(strNameOrId, context->GetGlobal(), context);
	}

	JSONNode msg(JSON_NODE);
	msg.push_back(JSONNode("name", "Browser-JSContextCreated"));
	msg.push_back(JSONNode("id", bid));
	msg.push_back(JSONNode("browser", browser->GetIdentifier()));
	msg.push_back(JSONNode("extender", strNameOrId));

	send(-1, msg);
}

extern void ContextedRelease_Renderer(int nBrowserId);

void ProcessApp::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	int bid = CreateBrowserId(browser);
	cef3Trace("Id: %d", bid);

	std::string strNameOrId = CreateBrowserExtenderId(browser);

	{
		tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);

		std::map<std::string, CefRefPtr<JavaScriptExtenderProxy> >::iterator it = m_mBrowserProxies.find(strNameOrId);

		if (it != m_mBrowserProxies.end())
			m_mBrowserProxies.erase(it);
	}

	JSONNode msg(JSON_NODE);
	msg.push_back(JSONNode("name", "Browser-JSContextReleased"));
	msg.push_back(JSONNode("id", bid));
	msg.push_back(JSONNode("browser", browser->GetIdentifier()));
	msg.push_back(JSONNode("extender", strNameOrId));

	send(-1, msg);

	ContextedRelease_Renderer(bid);
}


void ProcessApp::runThread(void* pObj)
{
	try
	{
		static_cast<ProcessApp*>(pObj)->run();
	}
	catch (std::exception &e)
	{
		int a = 1;
	}
}

void ProcessApp::run()
{
	char szHost[255] = { 0 };

#ifdef WIN32
	_snprintf(szHost, 255, "tcp://localhost:%d", m_nZmqPort);
#else
	assert(false);
#endif

	int nTimeout = 1000;

	m_ZmqMonitor.start();
	m_ZmqClient.setsockopt(ZMQ_IDENTITY, m_strZmqIdentity.c_str(), m_strZmqIdentity.size());
	m_ZmqClient.setsockopt(ZMQ_RCVTIMEO, &nTimeout, sizeof(int));
	m_ZmqClient.connect(szHost);

	{
		tthread::lock_guard<tthread::mutex> guard(m_StartLock);
		m_bConnected = true;

		for (size_t x = 0; x < m_vPendingSend.size(); x++)
		{
			cef3Trace("PendingSend Json: %s", m_vPendingSend[x].c_str());
			m_ZmqClient.send(m_vPendingSend[x].c_str(), m_vPendingSend[x].size(), 0);
		}

		m_vPendingSend.clear();
	}

	while (!m_bIsStopped)
	{
		std::vector<std::shared_ptr<zmq::message_t> > vMsgList;

		int64_t more = 0;
		size_t more_size = sizeof(more);

		do
		{
			std::shared_ptr<zmq::message_t> message(new zmq::message_t);
			
			if (m_ZmqClient.recv(message.get(), 0) == 0)
				break;

			vMsgList.push_back(message);
			m_ZmqClient.getsockopt(ZMQ_RCVMORE, &more, &more_size);
		} 
		while (more);

		if (vMsgList.size() > 0)
		{
			std::string strData(static_cast<char*>(vMsgList[0]->data()), vMsgList[0]->size());
			processMessageReceived(strData);
		}
	}

	m_ZmqMonitor.stop();
}

bool ProcessApp::send(int nBrowser, JSONNode msg)
{
	std::string strMsg = msg.write();

	{
		tthread::lock_guard<tthread::mutex> guard(m_StartLock);

		if (!m_bConnected)
		{
			m_vPendingSend.push_back(strMsg);
			return true;
		}
	}

	cef3Trace("Json: %s", strMsg.c_str());

	if (!m_ZmqClient.connected())
		return false;

	m_ZmqClient.send(strMsg.c_str(), strMsg.size(), 0);
	return true;
}

void ProcessApp::processMessageReceived(const std::string &strJson)
{
	cef3Trace("Json: %s", strJson.c_str());

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

	CefRefPtr<JavaScriptExtenderProxy> pExtender = findExtender(msg);
	int nBrowser = getBrowserId(msg);

	if (msg["name"].as_string() == "JSE-Response")
	{
		if (msg.find("response") != msg.end())
			handled = JavaScriptContextHelper<JavaScriptExtenderProxy>::Self.processResponse(pExtender, nBrowser, msg["response"]);
	}
	else if (msg["name"].as_string() == "JSE-Request")
	{
		if (msg.find("request") != msg.end())
			handled = JavaScriptContextHelper<JavaScriptExtenderProxy>::Self.processRequest(pExtender, nBrowser, msg["request"]);
	}

	if (!handled)
	{
		//TODO
	}
}

std::string ProcessApp::generateZmqName()
{
	char szBuff[255] = { 0 };

#ifdef WIN32
	_snprintf(szBuff, 255, "Cef3Renderer-%d", GetCurrentProcessId());
#else
	assert(false);
#endif

	return szBuff;
}

extern CefRefPtr<JavaScriptExtenderProxy> LookupExtenderFromFunctionHolder_Renderer(const std::string &strId);

CefRefPtr<JavaScriptExtenderProxy> ProcessApp::findExtender(JSONNode msg)
{
	if (msg.find("extender") == msg.end())
		return NULL;

	std::string strNameOrId = msg["extender"].as_string();

	{
		tthread::lock_guard<tthread::mutex> guard(m_BrowserLock);

		std::map<std::string, CefRefPtr<JavaScriptExtenderProxy> >::const_iterator it = m_mBrowserProxies.find(strNameOrId);

		if (it != m_mBrowserProxies.end())
			return it->second;
	}

	for (size_t x = 0; x < m_vJSExtenders.size(); ++x)
	{
		if (strNameOrId == m_vJSExtenders[x]->getName())
			return m_vJSExtenders[x];
	}

	return LookupExtenderFromFunctionHolder_Renderer(strNameOrId);
}

int ProcessApp::getBrowserId(JSONNode msg)
{
	if (msg.find("browser") == msg.end())
		return -1;

	return msg["browser"].as_int();
}


#include "Controller.h"

#ifdef WIN32
int ChromiumController::ExecuteProcess(HINSTANCE instance)
{
	g_command_line = CefCommandLine::CreateCommandLine();
	g_command_line->InitFromString(::GetCommandLineW());

	CefMainArgs main_args(instance);
	CefRefPtr<ProcessApp> app(new ProcessApp);

#ifdef WIN32_SANDBOX_ENABLED
	void* sandbox_info = m_pSandbox.sandbox_info();
#else
	void* sandbox_info = NULL;
#endif

	// Execute the secondary process, if any.
	return CefExecuteProcess(main_args, app.get(), sandbox_info);
}
#else
int ChromiumController::ExecuteProcess(int argc, char** argv)
{
	g_command_line = CefCommandLine::CreateCommandLine();
	g_command_line->InitFromArgv(argc, argv);

	CefMainArgs main_args(argc, argv);
	CefRefPtr<ProcessApp> app(new ProcessApp);

	// Execute the secondary process, if any.
	return CefExecuteProcess(main_args, app.get(), NULL);
}
#endif
