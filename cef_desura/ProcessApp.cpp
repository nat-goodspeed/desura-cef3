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

#include "ChromiumBrowserI.h"
#include "include/cef_app.h"
#include "include/cef_scheme.h"
#include "ChromiumBrowser.h"
#include "Controller.h"

#include "SharedMem.h"

#include "JSONOptions.h"
#include "libjson.h"

#include <string>
#include <sstream>

CefRefPtr<CefCommandLine> g_command_line;


class ProcessApp;

class WaitCondition
{
public:

	void notify()
	{

	}

	bool wait()
	{
		return false;
	}
};

CefRefPtr<CefV8Value> ConvertJsonToV8(const JSONNode &node)
{
	if (node.type() == JSON_NULL)
		return CefV8Value::CreateNull();
	else if (node.type() == JSON_STRING)
		return CefV8Value::CreateString(node.as_string());
	else if (node.type() == JSON_NUMBER)
		return CefV8Value::CreateDouble(node.as_float());
	else if (node.type() == JSON_BOOL)
		return CefV8Value::CreateBool(node.as_bool());
	else if (node.type() == JSON_ARRAY)
	{
		CefRefPtr<CefV8Value> ret = CefV8Value::CreateArray(node.size());

		for (size_t x = 0; x < node.size(); ++x)
			ret->SetValue(x, ConvertJsonToV8(node[x]));

		return ret;
	}
	else if (node.type() == JSON_NODE)
	{
		CefRefPtr<CefV8Value> ret = CefV8Value::CreateArray(node.size());

		for (size_t x = 0; x < node.size(); ++x)
			ret->SetValue(node[x].name(), ConvertJsonToV8(node[x]), V8_PROPERTY_ATTRIBUTE_NONE);

		return ret;
	}

	return CefV8Value::CreateUndefined();
}

JSONNode ConvertV8ToJson(const CefRefPtr<CefV8Value>& val)
{
	if (val->IsBool())
		return JSONNode("", val->GetBoolValue());
	else if (val->IsDouble())
		return JSONNode("", val->GetDoubleValue());
	else if (val->IsInt())
		return JSONNode("", val->GetIntValue());
	else if (val->IsUndefined() || val->IsNull())
		return JSONNode();
	else if (val->IsUInt())
		return JSONNode("", val->GetUIntValue());
	else if (val->IsString())
	{
		CefStringUTF8 strVal = ConvertToUtf8(val->GetStringValue());
		return JSONNode("", strVal.c_str());
	}
	else if (val->IsArray())
	{
		JSONNode ret(JSON_ARRAY);

		for (int x = 0; x < val->GetArrayLength(); ++x)
			ret[x] = ConvertV8ToJson(val->GetValue(x));

		return ret;
	}
	else if (val->IsObject())
	{
		JSONNode ret;

		std::vector<CefString> vKeys;
		val->GetKeys(vKeys);

		for (size_t x = 0; x < vKeys.size(); ++x)
		{
			CefStringUTF8 strKey = ConvertToUtf8(vKeys[x]);

			JSONNode k = ConvertV8ToJson(val->GetValue(vKeys[x]));
			k.set_name(strKey.c_str());

			ret.push_back(k);
		}

		return ret;
	}
	else if (val->IsFunction())
	{
		return JSONNode("", "__function__");
	}

	return JSONNode();
}


class V8ProxyHandler : public CefV8Handler
{
public:
	V8ProxyHandler(CefRefPtr<ProcessApp> &app, const std::string &strName)
		: m_App(app)
		, m_strName(strName)
	{
	}

	bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) OVERRIDE
	{
		CefRefPtr<CefBrowser> pBrowser = CefV8Context::GetCurrentContext()->GetBrowser();

		CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("JSE-Request");
		CefRefPtr<CefListValue> args = msg->GetArgumentList();

		args->SetString(0, m_strName);
		args->SetString(1, "FunctionCall");
		args->SetString(2, name);
		args->SetString(3, convertV8ToJson(object, arguments));

		pBrowser->SendProcessMessage(PID_BROWSER, msg);

		if (!m_WaitCond.wait())
			exception = "Timed out waiting for response from browser";
		else
		{
			try
			{
				retval = convertResponseToV8();
			}
			catch (std::exception &e)
			{
				exception = e.what();
			}
		}

		return true;
	}

	const char* getName()
	{
		return m_strName.c_str();
	}

	void onMessageReceived(CefRefPtr<CefListValue> &args)
	{
		CefString strAction = args->GetString(1);

		if (strAction == "FunctionReturn")
		{
			m_strFunctionReturn = args->GetString(2);
			m_WaitCond.notify();
		}
		else if (strAction == "FunctionException")
		{

		}
	}

protected:
	CefRefPtr<CefV8Value> convertResponseToV8()
	{
		if (m_strFunctionReturn.empty())
			return CefV8Value::CreateUndefined();

		JSONNode node = JSONWorker::parse(m_strFunctionReturn);
		return ConvertJsonToV8(node);
	}

	CefString convertV8ToJson(CefRefPtr<CefV8Value> &object, const CefV8ValueList& arguments)
	{
		JSONNode o = ConvertV8ToJson(object);
		o.set_name("object");
		
		JSONNode a(JSON_ARRAY);
		a.set_name("args");

		for (size_t x = 0; x < arguments.size(); ++x)
			a.push_back(ConvertV8ToJson(arguments[x]));

		JSONNode r;
		r.push_back(o);
		r.push_back(a);

		return r.write();
	}

private:
	CefRefPtr<ProcessApp> m_App;
	const std::string m_strName;

	CefString m_strFunctionReturn;
	WaitCondition m_WaitCond;

	IMPLEMENT_REFCOUNTING(V8ProxyHandler);
};

class ProcessApp 
	: public CefApp
	, protected CefRenderProcessHandler
{
public:
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// CefApp
	/////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE
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

	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE
	{
		return (CefRenderProcessHandler*)this;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////
	// CefRenderProcessHandler
	/////////////////////////////////////////////////////////////////////////////////////////////////


	virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE
	{
#ifdef DEBUG
		while (!IsDebuggerPresent())
			Sleep(1000);
#endif
	}

	virtual void OnWebKitInitialized()
	{
		CefStringUTF8 strJSEName = ConvertToUtf8(g_command_line->GetSwitchValue("desura-jse-name"));
		CefStringUTF8 strJSESize = ConvertToUtf8(g_command_line->GetSwitchValue("desura-jse-size"));

		if (!strJSEName.empty() && !strJSESize.empty())
		{
			int nSize = atoi(strJSESize.c_str());

			if (m_SharedMemInfo.init(strJSEName.c_str(), nSize))
			{
				char* pBuff = static_cast<char*>(m_SharedMemInfo.getMem());

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

					CefRefPtr<V8ProxyHandler> pJSExtender = new V8ProxyHandler(CefRefPtr<ProcessApp>(this), strName);
					CefRegisterExtension(strName, strBinding, CefRefPtr<CefV8Handler>(pJSExtender));
					m_vJSExtenders.push_back(pJSExtender);
				}
			}
		}
	}

	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE
	{

	}

	virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE
	{

	}

	virtual void OnUncaughtException(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Exception> exception, CefRefPtr<CefV8StackTrace> stackTrace)
	{

	}

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE
	{
		if (message->GetName() == "JSE-Request")
		{


			return true;
		}
		else if (message->GetName() == "JSE-Response")
		{
			CefRefPtr<CefListValue> args = message->GetArgumentList();
			CefString strName = args->GetString(0);

			for (size_t x = 0; x < m_vJSExtenders.size(); ++x)
			{
				if (strName != m_vJSExtenders[x]->getName())
					continue;

				m_vJSExtenders[x]->onMessageReceived(args);
				break;
			}

			return true;
		}

		return CefRenderProcessHandler::OnProcessMessageReceived(browser, source_process, message);
	}

	IMPLEMENT_REFCOUNTING(ProcessApp);
	SharedMem m_SharedMemInfo;
	std::vector<CefRefPtr<V8ProxyHandler>> m_vJSExtenders;
};




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
