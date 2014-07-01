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

#include "JavaScriptObject.h"
#include "JavaScriptFactory.h"
#include "JavaScriptContext.h"

#include "ChromiumApp.h"
#include "FunctionHolder.h"
#include "Controller.h"

int mystrncpy_s(char* dest, size_t destSize, const char* src, size_t srcSize)
{
	size_t size = srcSize;

	if (size > destSize-1)
		size = destSize-1;

	if (!dest)
		return 0;

	strncpy(dest, src, size);
	dest[size]=0;

	return size;
}

static FunctionHolder<CefRefPtr<JavaScriptExtenderRef>, 'B'> g_V8FunctionHolder;

CefRefPtr<JavaScriptExtenderRef> LookupExtenderFromFunctionHolder_Browser(const std::string &strId)
{
	return g_V8FunctionHolder.find(strId);
}


class JsonJavaScriptExtender : public ChromiumDLL::JavaScriptExtenderI
{
public:
	JsonJavaScriptExtender(const std::string &strId)
		: m_strId(strId)
	{
	}

	void destroy() OVERRIDE
	{
		delete this;
	}

	ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI> clone() OVERRIDE
	{
		return NULL;
	}

	ChromiumDLL::JSObjHandle execute(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFunctionArgs>& args) OVERRIDE
	{
		cef3Trace("Function: %s, argc: %d", args->function, args->argc);

		if (args->context)
			args->context->enter();

		JavaScriptObject* pObj = dynamic_cast<JavaScriptObject*>(args->object.get());

		JSONNode object(JSON_NULL);
		if (pObj)
			object = pObj->getNode();

		std::vector<JSONNode> vArgs;

		for (int x = 0; x < args->argc; ++x)
		{
			JavaScriptObject* pA = dynamic_cast<JavaScriptObject*>(args->argv[x].get());

			JSONNode a(JSON_NULL);
			if (pA)
				a = pA->getNode();

			vArgs.push_back(a);
		}

		std::string strFunct = "";

		if (args->function)
			strFunct = args->function;

		JSONNode ret = JavaScriptContextHelper<JavaScriptExtenderRef>::Self.invokeFunction(m_strId, strFunct, object, vArgs);

		if (args->context)
			args->context->exit();

		return new JavaScriptObject(ret);
	}

	const char* getName()
	{
		return m_strId.c_str();
	}

	const char* getRegistrationCode()
	{
		return "";
	}

private:
	std::string m_strId;

	CEF3_IMPLEMENTREF_COUNTING(JsonJavaScriptExtender);
};

template <>
std::string TraceClassInfo<JsonJavaScriptExtender>(JsonJavaScriptExtender *pClass)
{
	return pClass->getName();
}



JavaScriptObject::JavaScriptObject()
	: m_iRefCount(0)
	, m_bIsException(false)
{
}

JavaScriptObject::JavaScriptObject(JSONNode node, bool bIsException)
	: m_JsonNode(node)
	, m_iRefCount(0)
	, m_bIsException(bIsException)
{
	if (isFunction() && m_JsonNode.find("__renderer_function_id__") != node.end())
	{
		m_strId = node["__renderer_function_id__"].as_string();
		m_pJavaScriptExtender = new JavaScriptExtenderRef(new JsonJavaScriptExtender(m_strId));
		g_V8FunctionHolder.add(m_strId, m_pJavaScriptExtender);
	}
}

JavaScriptObject::JavaScriptObject(const char* name, const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& handler)
	: m_iRefCount(0)
	, m_bIsException(false)
{
	m_pJavaScriptExtender = new JavaScriptExtenderRef(handler);

	m_strId = g_V8FunctionHolder.newKey();
	g_V8FunctionHolder.add(m_strId, m_pJavaScriptExtender);

	m_JsonNode = JSONNode(JSON_NODE);
	m_JsonNode.push_back(JSONNode("__function__", name));
	m_JsonNode.push_back(JSONNode("__browser_function_id__", m_strId));
}

JavaScriptObject::~JavaScriptObject()
{
	g_V8FunctionHolder.del(m_strId);
}

void JavaScriptObject::destory()
{
	delete this;
}

ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptObjectI> JavaScriptObject::clone()
{
	return new JavaScriptObject(m_JsonNode, m_bIsException);
}

bool JavaScriptObject::isUndefined()
{
	return m_JsonNode.type() == JSON_NULL;
}

bool JavaScriptObject::isNull()
{
	return m_JsonNode.type() == JSON_NULL;
}

bool JavaScriptObject::isBool()
{
	return m_JsonNode.type() == JSON_BOOL;
}

bool JavaScriptObject::isInt()
{
	return m_JsonNode.type() == JSON_NUMBER;
}

bool JavaScriptObject::isDouble()
{
	return m_JsonNode.type() == JSON_NUMBER;
}

bool JavaScriptObject::isString()
{
	return m_JsonNode.type() == JSON_STRING;
}

bool JavaScriptObject::isObject()
{
	return m_JsonNode.type() == JSON_NODE && m_JsonNode.find("__function__") == m_JsonNode.end();
}

bool JavaScriptObject::isArray()
{
	return m_JsonNode.type() == JSON_ARRAY;
}

bool JavaScriptObject::isFunction()
{
	return m_JsonNode.type() == JSON_NODE && m_JsonNode.find("__function__") != m_JsonNode.end();
}

bool JavaScriptObject::isException()
{
	return m_bIsException;
}

bool JavaScriptObject::getBoolValue()
{
	return m_JsonNode.as_bool();
}

int JavaScriptObject::getIntValue()
{
	return m_JsonNode.as_int();
}

double JavaScriptObject::getDoubleValue()
{
	return m_JsonNode.as_float();
}

int JavaScriptObject::getStringValue(char* buff, size_t buffsize)
{
	std::string str = m_JsonNode.as_string();

	if (buff)
		mystrncpy_s(buff, buffsize, str.c_str(), str.size());

	return str.size();
}

bool JavaScriptObject::hasValue(const char* key)
{
	return m_JsonNode.find(key) != m_JsonNode.end();
}

bool JavaScriptObject::hasValue(int index)
{
	return (index < getNumberOfKeys());
}

bool JavaScriptObject::deleteValue(const char* key)
{
	JSONNode::iterator it = m_JsonNode.find(key);

	if (it == m_JsonNode.end())
		return false;

	m_JsonNode.erase(it);
	return true;
}

bool JavaScriptObject::deleteValue(int index)
{
	if (index >= getNumberOfKeys())
		return false;

	m_JsonNode.erase(m_JsonNode.begin() + index);
	return true;
}

ChromiumDLL::JSObjHandle JavaScriptObject::getValue(const char* key)
{
	if (!hasValue(key))
		return NULL;

	return new JavaScriptObject(m_JsonNode[key]);
}

ChromiumDLL::JSObjHandle JavaScriptObject::getValue(int index)
{
	if (index >= getNumberOfKeys())
		return NULL;

	return new JavaScriptObject(m_JsonNode[index]);
}

bool JavaScriptObject::setValue(const char* key, ChromiumDLL::JSObjHandle value)
{
	JavaScriptObject* jso = (JavaScriptObject*)value.get();

	if (!jso)
		return false;

	JSONNode n = jso->getNode().duplicate();
	n.set_name(key);

	if (hasValue(key))
		m_JsonNode[key] = n;
	else
		m_JsonNode.push_back(n);

	return true;
}

bool JavaScriptObject::setValue(int index, ChromiumDLL::JSObjHandle value)
{
	JavaScriptObject* jso = (JavaScriptObject*)value.get();

	if (!jso)
		return false;

	while (m_JsonNode.size() < (size_t)index)
		m_JsonNode.push_back(JSONNode(JSON_NULL));

	JSONNode n = jso->getNode().duplicate();
	n.set_name("");

	m_JsonNode[index] = n;
	return true;
}

int JavaScriptObject::getNumberOfKeys()
{
	return m_JsonNode.size();
}

void JavaScriptObject::getKey(int index, char* buff, size_t buffsize)
{
	if (index >= getNumberOfKeys())
		return;

	std::string strKey = m_JsonNode[index].name();

	if (buff)
		mystrncpy_s(buff, buffsize, strKey.c_str(), strKey.size());
}

int JavaScriptObject::getArrayLength()
{
	return getNumberOfKeys();
}

void JavaScriptObject::getFunctionName(char* buff, size_t buffsize)
{
	if (!isFunction())
		return;

	std::string strFunctionName = m_JsonNode["__function__"].as_string();

	if (buff)
		mystrncpy_s(buff, buffsize, strFunctionName.c_str(), strFunctionName.size());
}

ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI> JavaScriptObject::getFunctionHandler()
{
	if (!isFunction())
		return NULL;

	return *m_pJavaScriptExtender;
}

ChromiumDLL::JSObjHandle JavaScriptObject::executeFunction(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFunctionArgs>& args)
{
	if (!isFunction() || !getFunctionHandler())
		return NULL;

	args->function = m_strId.c_str();
	return getFunctionHandler()->execute(args);
}

ChromiumDLL::RefPtr<ChromiumDLL::IntrusiveRefPtrI> JavaScriptObject::getUserObject()
{
	if (!hasValue("__user_data__"))
		return NULL;

	return (ChromiumDLL::IntrusiveRefPtrI*)m_JsonNode["__user_data__"].as_int();
}

void JavaScriptObject::setException()
{
	m_bIsException = true;
}

void JavaScriptObject::setFunctionHandler(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& pExtender)
{
	m_pJavaScriptExtender = new JavaScriptExtenderRef(pExtender);
}
