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
#include "FunctionHolder.h"

#include "JavaScriptContext.h"
#include "libjson.h"
#include "Controller.h"


extern CefStringUTF8 ConvertToUtf8(const CefString& str);

static FunctionHolder<CefRefPtr<JavaScriptExtenderProxy>, 'R'> g_V8FunctionHolder;

CefRefPtr<JavaScriptExtenderProxy> LookupExtenderFromFunctionHolder_Renderer(const std::string &strId)
{
	return g_V8FunctionHolder.find(strId);
}




CefRefPtr<CefV8Value> ConvertJsonToV8(const JSONNode &node, CefRefPtr<CefV8Context> &context)
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
			ret->SetValue(x, ConvertJsonToV8(node[x], context));

		return ret;
	}
	else if (node.type() == JSON_NODE)
	{
		if (node.find("__function__") != node.end())
		{
			CefRefPtr<JavaScriptExtenderProxy> pObject;

			if (node.find("__renderer_function_id__") != node.end())
				pObject = g_V8FunctionHolder.find(node["__renderer_function_id__"].as_string());

			if (pObject)
				return pObject->getV8Value();

			if (node.find("__browser_function_id__") == node.end())
				return NULL;

			std::string strBrowserFunctId = node["__browser_function_id__"].as_string();
			pObject = g_V8FunctionHolder.find(strBrowserFunctId);

			if (pObject)
				return pObject->getV8Value();

			CefRefPtr<CefV8Value> ret = CefV8Value::CreateFunction(node["__function__"].as_string(), new JavaScriptExtenderProxy(strBrowserFunctId));
			g_V8FunctionHolder.add(strBrowserFunctId, new JavaScriptExtenderProxy(strBrowserFunctId, ret, context));
			return ret;
		}
		else
		{
			CefRefPtr<CefV8Value> ret = CefV8Value::CreateArray(node.size());

			for (size_t x = 0; x < node.size(); ++x)
				ret->SetValue(node[x].name(), ConvertJsonToV8(node[x], context), V8_PROPERTY_ATTRIBUTE_NONE);

			return ret;
		}
	}

	return CefV8Value::CreateUndefined();
}

JSONNode ConvertV8ToJson(const CefRefPtr<CefV8Value>& val, CefRefPtr<CefV8Context> &context)
{
	if (!val)
		return JSONNode(JSON_NULL);

	if (val->IsBool())
		return JSONNode("", val->GetBoolValue());
	else if (val->IsDouble())
		return JSONNode("", val->GetDoubleValue());
	else if (val->IsInt())
		return JSONNode("", val->GetIntValue());
	else if (val->IsUInt())
		return JSONNode("", val->GetUIntValue());
	else if (val->IsUndefined() || val->IsNull())
		return JSONNode(JSON_NULL);
	else if (val->IsString())
	{
		CefStringUTF8 strVal = ConvertToUtf8(val->GetStringValue());
		return JSONNode("", strVal.c_str());
	}
	else if (val->IsArray())
	{
		JSONNode ret(JSON_ARRAY);

		for (int x = 0; x < val->GetArrayLength(); ++x)
			ret[x] = ConvertV8ToJson(val->GetValue(x), context);

		return ret;
	}
	else if (val->IsFunction())
	{
		std::map<std::string, CefRefPtr<JavaScriptExtenderProxy>> holders = g_V8FunctionHolder.duplicate();
		std::map<std::string, CefRefPtr<JavaScriptExtenderProxy>>::iterator it = holders.begin();

		std::string strKey;

		for (; it != holders.end(); ++it)
		{
			if (it->second->getV8Value() != val)
				continue;

			strKey = it->first;
			break;
		}

		if (strKey.empty())
		{
			strKey = g_V8FunctionHolder.newKey();
			g_V8FunctionHolder.add(strKey, new JavaScriptExtenderProxy(strKey, val, context));
		}
			
		std::string strFuntionName = ConvertToUtf8(val->GetFunctionName());

		JSONNode ret(JSON_NODE);
		ret.push_back(JSONNode("__function__", strFuntionName));

		if (g_V8FunctionHolder.isRendererFunction(strKey))
			ret.push_back(JSONNode("__renderer_function_id__", strKey));
		else
			ret.push_back(JSONNode("__browser_function_id__", strKey));

		return ret;
	}
	else if (val->IsObject())
	{
		JSONNode ret(JSON_NODE);

		std::vector<CefString> vKeys;
		val->GetKeys(vKeys);

		for (size_t x = 0; x < vKeys.size(); ++x)
		{
			CefStringUTF8 strKey = ConvertToUtf8(vKeys[x]);

			JSONNode k = ConvertV8ToJson(val->GetValue(vKeys[x]), context);
			k.set_name(strKey.c_str());

			ret.push_back(k);
		}

		return ret;
	}

	return JSONNode(JSON_NULL);
}


template <>
std::string TraceClassInfo<JavaScriptExtenderProxy>(JavaScriptExtenderProxy *pClass)
{
	return pClass->getName();
}

JavaScriptExtenderProxy::JavaScriptExtenderProxy(const std::string &strId, const CefRefPtr<CefV8Value> &funct, const CefRefPtr<CefV8Context> &context)
	: m_strName(strId)
	, m_Function(funct)
	, m_Context(context)
{
}

JavaScriptExtenderProxy::JavaScriptExtenderProxy(const std::string &strName)
	: m_strName(strName)
{
}

const char* JavaScriptExtenderProxy::getName()
{
	return m_strName.c_str();
}

bool JavaScriptExtenderProxy::Execute(const CefString& function, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
	CefStringUTF8 strFunction = ConvertToUtf8(function);
	cef3Trace("Function: %s, argc: %d", strFunction.c_str(), arguments.size());

	auto v8Context = CefV8Context::GetCurrentContext();

	v8Context->Enter();

	int nBrowserId = ProcessApp::CreateBrowserId(v8Context->GetBrowser());
	JavaScriptContextHandle<JavaScriptExtenderProxy> context(nBrowserId);

	JSONNode o(JSON_NULL); // = ConvertV8ToJson(object, vObjectIdent);
	o.set_name("object");

	std::vector<JSONNode> a;

	for (size_t x = 0; x < arguments.size(); ++x)
		a.push_back(ConvertV8ToJson(arguments[x], CefV8Context::GetCurrentContext()));

	try
	{
		JSONNode ret = JavaScriptContextHelper<JavaScriptExtenderProxy>::Self.invokeFunction(m_strName, strFunction, o, a);
		retval = ConvertJsonToV8(ret, CefV8Context::GetCurrentContext());
	}
	catch (std::exception &e)
	{
		exception = e.what();
	}

	v8Context->Exit();

	return true;
}

JSONNode JavaScriptExtenderProxy::execute(const std::string &strFunction, JSONNode object, JSONNode argumets)
{
	cef3Trace("Function: %s, argc: %d", strFunction.c_str(), argumets.size());

	if (!m_Function || !m_Function->IsFunction())
		throw std::exception("No valid v8 function");

	CefRefPtr<CefV8Value> o = ConvertJsonToV8(object, m_Context);
	CefV8ValueList a;

	for (size_t x = 0; x < argumets.size(); ++x)
		a.push_back(ConvertJsonToV8(argumets[x], m_Context));

	if (!o->IsValid() || !o->IsObject())
		o = CefV8Value::CreateObject(NULL);

	CefRefPtr<CefV8Value> ret = m_Function->ExecuteFunctionWithContext(m_Context, o, a);
	return ConvertV8ToJson(ret, m_Context);
}