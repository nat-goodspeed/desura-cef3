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

	size_t nSize = 4; //number of entries

	for (size_t x = 0; x < vInfo.size(); ++x)
		nSize += 8 + vInfo[x].strBinding.size() + vInfo[x].strName.size();

#ifdef WIN32
	char strShared[255] = { 0 };
	_snprintf(strShared, 255, "DESURA_JSEXTENDER_%d", GetCurrentProcessId());

	if (!m_SharedMemInfo.init(strShared, nSize, false))
		return false;
#else
	return false;
#endif

	char* pBuff = (char*)m_SharedMemInfo.getMem();

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

bool ChromiumApp::processMessageReceived(CefRefPtr<CefBrowser> &browser, CefRefPtr<CefProcessMessage> &message)
{
	if (message->GetName() == "JSE-Response")
	{


	}
	else if (message->GetName() == "JSE-Request")
	{
		initThread();

		{
			tthread::lock_guard<tthread::mutex> guard(m_QueueLock);
			m_WorkQueue.push_back(std::make_pair(browser, message->GetArgumentList()));
		}
		
		m_WaitCond.notify_one();
	}
	else
	{
		return false;
	}

	return true;
}

void ChromiumApp::run()
{
	while (!m_bIsStopped)
	{
		CefRefPtr<CefListValue> request;
		CefRefPtr<CefBrowser> browser;

		{
			tthread::lock_guard<tthread::mutex> guard(m_QueueLock);

			if (!m_WorkQueue.empty())
			{
				request = m_WorkQueue.front().second;
				browser = m_WorkQueue.front().first;
				m_WorkQueue.pop_front();
			}
		}

		if (request && browser)
		{
			processRequest(browser, request);
		}
		else if (!m_bIsStopped)
		{
			m_WaitCond.wait(m_WaitLock);
		}
	}
}

CefStringUTF8 ConvertToUtf8(const CefString& str);

void ChromiumApp::runThread(void* pObj)
{
	static_cast<ChromiumApp*>(pObj)->run();
}

void ChromiumApp::initThread()
{
	if (m_bIsStopped || m_pWorkerThread)
		return;

	m_pWorkerThread = new tthread::thread(&ChromiumApp::runThread, this);
}

void ChromiumApp::processRequest(CefRefPtr<CefBrowser> &browser, CefRefPtr<CefListValue> &request)
{
	CefRefPtr<CefProcessMessage> res = CefProcessMessage::Create("JSE-Response");
	CefRefPtr<CefListValue> args = res->GetArgumentList();

	if (request->GetSize() < 4)
	{
		//TODO: handle this better
		return;
	}

	ChromiumDLL::JavaScriptExtenderI* pExtender = NULL;
	CefStringUTF8 strJsName = ConvertToUtf8(request->GetString(0));

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
		return;
	}

	args->SetString(0, request->GetString(0));

	CefStringUTF8 strCommand = ConvertToUtf8(request->GetString(1));

	if (strCommand == "FunctionCall")
	{
		CefStringUTF8 strFunction = ConvertToUtf8(request->GetString(2));
		CefStringUTF8 strArgs = ConvertToUtf8(request->GetString(3));

		JavaScriptFactory factory;

		ChromiumDLL::JavaScriptFunctionArgs jsFunctArgs;
		jsFunctArgs.function = strFunction.c_str();
		jsFunctArgs.factory = &factory;
		jsFunctArgs.argc = 0;
		jsFunctArgs.argv = NULL;
		jsFunctArgs.context = NULL;

		ChromiumDLL::JSObjHandle* jsArgv = NULL;
		
		try
		{
			JSONNode node = JSONWorker::parse(strArgs);

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

			ChromiumDLL::JSObjHandle pRet = pExtender->execute(&jsFunctArgs);
			delete[] jsArgv;

			JavaScriptObject* pObj = dynamic_cast<JavaScriptObject*>(pRet.get());

			if (pObj && !pObj->isException())
			{
				args->SetString(1, "FunctionReturn");
				args->SetString(2, pObj->getJsonString());
			}
			else if (pObj)
			{
				char szException[255] = { 0 };
				pObj->getStringValue(szException, 255);

				args->SetString(1, "FunctionException");
				args->SetString(2, szException);
			}
			else
			{
				args->SetString(1, "FunctionReturn");
				args->SetString(2, "");
			}
		}
		catch (std::exception &e)
		{
			args->SetString(1, "FunctionException");
			args->SetString(2, e.what());

			delete [] jsArgv;
		}

		browser->SendProcessMessage(PID_RENDERER, res);
	}
	else
	{
		//TODO: handle this better
		return;
	}
}