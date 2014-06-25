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

#ifndef THIRDPARTY_CEF3_JAVASCRIPTOBJECT_H
#define THIRDPARTY_CEF3_JAVASCRIPTOBJECT_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "RefCount.h"
#include "include/cef_base.h"
#include "libjson.h"

class JavaScriptExtenderRef;

class JavaScriptObject : public ChromiumDLL::JavaScriptObjectI
{
public:
	JavaScriptObject();
	JavaScriptObject(JSONNode node, bool bIsException = false);
	JavaScriptObject(const char* name, const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& handler);
	~JavaScriptObject();

	virtual void destory();
	virtual ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptObjectI> clone();


	virtual bool isUndefined();
	virtual bool isNull();
	virtual bool isBool();
	virtual bool isInt();
	virtual bool isDouble();
	virtual bool isString();
	virtual bool isObject();
	virtual bool isArray();
	virtual bool isFunction();
	virtual bool isException();

	virtual bool getBoolValue();
	virtual int getIntValue();
	virtual double getDoubleValue();
	virtual int getStringValue(char* buff, size_t buffsize);

	virtual bool hasValue(const char* key);
	virtual bool hasValue(int index);

	virtual bool deleteValue(const char* key);
	virtual bool deleteValue(int index);

	virtual ChromiumDLL::JSObjHandle getValue(const char* key);
	virtual ChromiumDLL::JSObjHandle getValue(int index);

	virtual bool setValue(const char* key, ChromiumDLL::JSObjHandle value);
	virtual bool setValue(int index, ChromiumDLL::JSObjHandle value);

	virtual int getNumberOfKeys();
	virtual void getKey(int index, char* buff, size_t buffsize);

	virtual int getArrayLength();
	virtual void getFunctionName(char* buff, size_t buffsize);

	virtual ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI> getFunctionHandler();
	virtual ChromiumDLL::JSObjHandle executeFunction(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFunctionArgs>& args);

	virtual ChromiumDLL::RefPtr<ChromiumDLL::IntrusiveRefPtrI> getUserObject();

	void setException();
	void setFunctionHandler(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& pExtender);

	JSONNode getNode()
	{
		return m_JsonNode;
	}

private:
	int m_iRefCount;
	bool m_bIsException;

	std::string m_strId;

	CefRefPtr<JavaScriptExtenderRef> m_pJavaScriptExtender;
	JSONNode m_JsonNode;

	CEF3_IMPLEMENTREF_COUNTING(JavaScriptObject);
};





#endif //THIRDPARTY_CEF3_JAVASCRIPTOBJECT_H
