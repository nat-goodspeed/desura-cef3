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
#include "JavaScriptExtender.h"
#include "JavaScriptFactory.h"
#include "JavaScriptContext.h"

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
}

JavaScriptObject::~JavaScriptObject()
{
}

void JavaScriptObject::addRef()
{
	m_iRefCount++;
}

void JavaScriptObject::delRef()
{
	m_iRefCount--;

	if (!m_iRefCount)
		delete this;
}

void JavaScriptObject::destory()
{
	delete this;
}

ChromiumDLL::JavaScriptObjectI* JavaScriptObject::clone()
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
	return m_JsonNode.type() == JSON_NODE;
}

bool JavaScriptObject::isArray()
{
	return m_JsonNode.type() == JSON_ARRAY;
}

bool JavaScriptObject::isFunction()
{
	//Todo: Implement
	return false;
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
	mystrncpy_s(buff, buffsize, strKey.c_str(), strKey.size());
}

int JavaScriptObject::getArrayLength()
{
	return getNumberOfKeys();
}

void JavaScriptObject::getFunctionName(char* buff, size_t buffsize)
{
	//TODO Implement
}

ChromiumDLL::JavaScriptExtenderI* JavaScriptObject::getFunctionHandler()
{
	//TODO Implement
	return NULL;
}

ChromiumDLL::JSObjHandle JavaScriptObject::executeFunction(ChromiumDLL::JavaScriptFunctionArgs *args)
{
	//TODO Implement
	return NULL;
}

void* JavaScriptObject::getUserObject()
{
	if (!hasValue("__user_data__"))
		return NULL;

	return (void*)m_JsonNode["__user_data__"].as_int();
}

void JavaScriptObject::setException()
{
	m_bIsException = true;
}

std::string JavaScriptObject::getJsonString()
{
	return m_JsonNode.write();
}
