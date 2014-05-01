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

#include <string>
#include <sstream>
#include <memory>

extern CefStringUTF8 ConvertToUtf8(const CefString& str);
CefRefPtr<CefCommandLine> g_command_line;



ProcessApp::ProcessApp()
	: m_ZmqContext(1)
	, m_ZmqClient(m_ZmqContext, ZMQ_DEALER)
	, m_pWorkerThread(NULL)
	, m_bIsStopped(false)
	, m_strZmqIdentity(generateZmqName())
	, m_nZmqPort(0)
{
}

ProcessApp::~ProcessApp()
{
	m_bIsStopped = true;

	if (m_pWorkerThread && m_pWorkerThread->joinable())
		m_pWorkerThread->join();

	delete m_pWorkerThread;
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

void ProcessApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
	JSONNode msg(JSON_NODE);
	msg.push_back(JSONNode("name", "Browser-Created"));
	msg.push_back(JSONNode("id", browser->GetIdentifier()));

	sendMessage(msg.write());
}

void ProcessApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser)
{
	JSONNode msg(JSON_NODE);
	msg.push_back(JSONNode("name", "Browser-Destroyed"));
	msg.push_back(JSONNode("id", browser->GetIdentifier()));

	sendMessage(msg.write());
}

void ProcessApp::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info)
{
#ifdef DEBUG
	while (!IsDebuggerPresent())
		Sleep(1000);
#endif
}

void ProcessApp::OnWebKitInitialized()
{
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

				CefRefPtr<JavaScriptExtenderProxy> pJSExtender = new JavaScriptExtenderProxy(CefRefPtr<ProcessApp>(this), strName);
				CefRegisterExtension(strName, strBinding, CefRefPtr<CefV8Handler>(pJSExtender));
				m_vJSExtenders.push_back(pJSExtender);
			}
		}
	}

	m_pWorkerThread = new tthread::thread(&ProcessApp::runThread, this);
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

	m_ZmqClient.setsockopt(ZMQ_IDENTITY, m_strZmqIdentity.c_str(), m_strZmqIdentity.size());
	m_ZmqClient.connect(szHost);

	while (!m_bIsStopped)
	{
		std::vector<std::shared_ptr<zmq::message_t>> vMsgList;

		int64_t more = 0;
		size_t more_size = sizeof(more);

		do
		{
			std::shared_ptr<zmq::message_t> message(new zmq::message_t);
			m_ZmqClient.recv(message.get(), 0);
			vMsgList.push_back(message);

			m_ZmqClient.getsockopt(ZMQ_RCVMORE, &more, &more_size);
		} while (more);

		if (vMsgList.size() > 0)
		{
			std::string strData(static_cast<char*>(vMsgList[0]->data()), vMsgList[0]->size());
			processMessageReceived(strData);
		}
	}
}

void ProcessApp::processMessageReceived(const std::string &strJson)
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

	if (msg["name"].as_string() == "JSE-Request")
	{

	}
	else if (msg["name"].as_string() == "JSE-Response")
	{
		if (msg.find("response") != msg.end())
			handled = processResponse(msg["response"]);
	}

	if (!handled)
	{
		//TODO
	}
}

bool ProcessApp::processResponse(JSONNode json)
{
	if (json.find("extender") == json.end() || json.find("command") == json.end())
	{
		//Handle better
		return false;
	}

	std::string strName = json["extender"].as_string();

	CefRefPtr<JavaScriptExtenderProxy> pExtender;

	for (size_t x = 0; x < m_vJSExtenders.size(); ++x)
	{
		if (strName != m_vJSExtenders[x]->getName())
			continue;

		pExtender = m_vJSExtenders[x];
		break;
	}

	if (!pExtender)
		return false;

	std::string strAction = json["command"].as_string();

	if (strAction == "FunctionReturn")
	{
		if (json.find("result") != json.end())
			pExtender->onMessageReceived(strAction, json["result"]);
		else
			pExtender->onMessageReceived(strAction, JSONNode(JSON_NULL));
	}
	else if (strAction == "FunctionException")
	{
		if (json.find("exception") != json.end())
			pExtender->onMessageReceived(strAction, json["exception"]);
		else
			pExtender->onMessageReceived(strAction, JSONNode(JSON_NULL));
	}
	else
	{
		return false;
	}

	return true;
}

void ProcessApp::sendMessage(const std::string& strMsg)
{
	m_ZmqClient.send(strMsg.c_str(), strMsg.size(), 0);
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
