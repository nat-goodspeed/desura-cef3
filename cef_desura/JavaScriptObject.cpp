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
{
	m_iRefCount = 0;
	m_bIsException = false;
}

JavaScriptObject::JavaScriptObject(CefRefPtr<CefV8Value> obj)
{
	m_pObject = obj;
	m_iRefCount = 0;
	m_bIsException = false;
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
	return new JavaScriptObject(m_pObject);
}

bool JavaScriptObject::isUndefined()
{
	return m_pObject->IsUndefined();
}

bool JavaScriptObject::isNull()
{
	if (!m_pObject)
		return true;

	return m_pObject->IsNull();
}

bool JavaScriptObject::isBool()
{
	return m_pObject->IsBool();
}

bool JavaScriptObject::isInt()
{
	return m_pObject->IsInt();
}

bool JavaScriptObject::isDouble()
{
	return m_pObject->IsDouble();
}

bool JavaScriptObject::isString()
{
	return m_pObject->IsString();
}

bool JavaScriptObject::isObject()
{
	return m_pObject->IsObject();
}

bool JavaScriptObject::isArray()
{
	return m_pObject->IsArray();
}

bool JavaScriptObject::isFunction()
{
	return m_pObject->IsFunction();
}

bool JavaScriptObject::isException()
{
	return m_bIsException;
}

bool JavaScriptObject::getBoolValue()
{
	return m_pObject->GetBoolValue();
}

int JavaScriptObject::getIntValue()
{
	return m_pObject->GetIntValue();
}

double JavaScriptObject::getDoubleValue()
{
	return m_pObject->GetDoubleValue();
}

int JavaScriptObject::getStringValue(char* buff, size_t buffsize)
{
	std::string str = m_pObject->GetStringValue();

	if (buff)
		mystrncpy_s(buff, buffsize, str.c_str(), str.size());

	return str.size();
}

bool JavaScriptObject::hasValue(const char* key)
{
	return m_pObject->HasValue(key);
}

bool JavaScriptObject::hasValue(int index)
{
	return m_pObject->HasValue(index);
}

bool JavaScriptObject::deleteValue(const char* key)
{
	return m_pObject->DeleteValue(key);
}

bool JavaScriptObject::deleteValue(int index)
{
	return m_pObject->DeleteValue(index);
}

ChromiumDLL::JSObjHandle JavaScriptObject::getValue(const char* key)
{
	CefRefPtr<CefV8Value> val = m_pObject->GetValue(key);

	if (!val)
		return NULL;

	return ChromiumDLL::JSObjHandle(new JavaScriptObject(val));
}

ChromiumDLL::JSObjHandle JavaScriptObject::getValue(int index)
{
	CefRefPtr<CefV8Value> val = m_pObject->GetValue(index);

	if (!val)
		return NULL;

	return ChromiumDLL::JSObjHandle(new JavaScriptObject(val));
}

bool JavaScriptObject::setValue(const char* key, ChromiumDLL::JSObjHandle value)
{
	JavaScriptObject* jso = (JavaScriptObject*)value.get();

	if (!jso)
		return false;

	// nat: V8_PROPERTY_ATTRIBUTE_NONE is a guess; CefV8Value::SetValue()
	// didn't used to require a PropertyAttribute.
	return m_pObject->SetValue(key, jso->getCefV8(), V8_PROPERTY_ATTRIBUTE_NONE);
}

bool JavaScriptObject::setValue(int index, ChromiumDLL::JSObjHandle value)
{
	JavaScriptObject* jso = (JavaScriptObject*)value.get();

	if (!jso)
		return false;

	return m_pObject->SetValue(index, jso->getCefV8());
}

int JavaScriptObject::getNumberOfKeys()
{
	std::vector<CefString> keys;
	m_pObject->GetKeys(keys);

	return keys.size();
}

void JavaScriptObject::getKey(int index, char* buff, size_t buffsize)
{
	std::vector<CefString> keys;
	m_pObject->GetKeys(keys);

	if (index >= 0 && index < (int)keys.size())
	{
		cef_string_utf8_t tmp;
		cef_string_to_utf8(keys[index].c_str(), keys[index].size(), &tmp);
		CefStringUTF8 t(&tmp);

		mystrncpy_s(buff, buffsize, t.c_str(), t.size());
	}
}

int JavaScriptObject::getArrayLength()
{
	return m_pObject->GetArrayLength();
}

void JavaScriptObject::getFunctionName(char* buff, size_t buffsize)
{
	std::string name = m_pObject->GetFunctionName();
	mystrncpy_s(buff, buffsize, name.c_str(), name.size());
}

ChromiumDLL::JavaScriptExtenderI* JavaScriptObject::getFunctionHandler()
{
	return new JavaScriptWrapper(m_pObject->GetFunctionHandler());
}

ChromiumDLL::JSObjHandle JavaScriptObject::executeFunction(ChromiumDLL::JavaScriptFunctionArgs *args)
{
	if (!isFunction())
		return GetJSFactory()->CreateException("Not a function!");

	if (!args)
		return GetJSFactory()->CreateException("Args are null for function call");

	JavaScriptContext* context = (JavaScriptContext*)args->context;
	JavaScriptObject* jso = (JavaScriptObject*)args->object.get();

	CefV8ValueList argList;

	for (int x=0; x<args->argc; x++)
	{
		JavaScriptObject* jsoa = (JavaScriptObject*)args->argv[x].get();

		if (jsoa)
			argList.push_back(jsoa->getCefV8());
		else
			argList.push_back(NULL);
	}

	CefRefPtr<CefV8Value> retval = m_pObject->ExecuteFunctionWithContext(context->getCefV8(), jso?jso->getCefV8():NULL, argList);

	if (!retval)
	{
		CefRefPtr<CefV8Exception> exception = m_pObject->GetException();
		if (exception)
		{
			cef_string_utf8_t tmp;
			cef_string_to_utf8(exception->GetMessage().c_str(), exception->GetMessage().size(), &tmp);
			CefStringUTF8 t(&tmp);

			return GetJSFactory()->CreateException(t.c_str());
		}
			
		return GetJSFactory()->CreateException("failed to run function");
	}

	return new JavaScriptObject(retval);
}

void* JavaScriptObject::getUserObject()
{
	CefRefPtr<CefBase> data = m_pObject->GetUserData();

	ObjectWrapper* ow = (ObjectWrapper*)data.get();
	
	if (ow)
		return ow->getData();

	return NULL;
}

CefRefPtr<CefV8Value> JavaScriptObject::getCefV8()
{
	return m_pObject;
}

CefRefPtr<CefBase> JavaScriptObject::getCefBase()
{
	CefRefPtr<CefBase> base(new V8ValueBaseWrapper(m_pObject));
	return base;
}

void JavaScriptObject::setException()
{
	m_bIsException = true;
}


