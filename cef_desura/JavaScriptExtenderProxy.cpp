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

#include "JavaScriptExtenderProxy.h"
#include "ProcessApp.h"

#include "libjson.h"


extern CefStringUTF8 ConvertToUtf8(const CefString& str);

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




JavaScriptExtenderProxy::JavaScriptExtenderProxy(CefRefPtr<ProcessApp> &app, const std::string &strName)
	: m_App(app)
	, m_strName(strName)
{
}

bool JavaScriptExtenderProxy::Execute(const CefString& function, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
	JSONNode a = convertV8ToJson(object, arguments);
	a.set_name("arguments");

	CefStringUTF8 strFunction = ConvertToUtf8(function);

	JSONNode r(JSON_NODE);
	r.set_name("request");
	r.push_back(JSONNode("extender", m_strName));
	r.push_back(JSONNode("command", "FunctionCall"));
	r.push_back(JSONNode("function", strFunction.c_str()));
	r.push_back(a);

	JSONNode msg(JSON_NODE);
	msg.push_back(JSONNode("name", "JSE-Request"));
	msg.push_back(r);

	m_App->sendMessage(msg.write());

	tthread::lock_guard<tthread::mutex> guard(m_WaitLock);

#ifdef DEBUG
	int nWaitTimeout = 999; //allow for debugging
#else
	int nWaitTimeout = 1;
#endif

	if (!m_WaitCond.wait_timed(guard, nWaitTimeout))
	{
		exception = "Timed out waiting for response from browser";
	}
	else
	{
		tthread::lock_guard<tthread::mutex> guard(m_ReturnLock);

		if (m_bReturnIsException)
			exception = m_jsonFunctionReturn.as_string();
		else
			retval = ConvertJsonToV8(m_jsonFunctionReturn);
	}

	return true;
}

const char* JavaScriptExtenderProxy::getName()
{
		return m_strName.c_str();
}

void JavaScriptExtenderProxy::onMessageReceived(const std::string &strAction, JSONNode ret)
{
	tthread::lock_guard<tthread::mutex> guard(m_ReturnLock);

	m_jsonFunctionReturn = ret;
	m_bReturnIsException = (strAction != "FunctionReturn");

	m_WaitCond.notify_one();
}


JSONNode JavaScriptExtenderProxy::convertV8ToJson(CefRefPtr<CefV8Value> &object, const CefV8ValueList& arguments)
{
	JSONNode o(JSON_NULL); // = ConvertV8ToJson(object);
	o.set_name("object");

	JSONNode a(JSON_ARRAY);
	a.set_name("args");

	for (size_t x = 0; x < arguments.size(); ++x)
		a.push_back(ConvertV8ToJson(arguments[x]));

	JSONNode r;
	r.push_back(o);
	r.push_back(a);

	return r;
}
