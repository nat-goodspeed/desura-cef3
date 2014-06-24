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

#ifndef THIRDPARTY_CEF3_JAVASCRIPTFACTORY_H
#define THIRDPARTY_CEF3_JAVASCRIPTFACTORY_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "RefCount.h"
//#include "include/cef.h"
#include "include/cef_base.h"

class ObjectWrapper : public CefBase
{
public:
	ObjectWrapper(const ChromiumDLL::RefPtr<ChromiumDLL::IntrusiveRefPtrI>& data)
		: m_pData(data)
	{
	}

	ChromiumDLL::RefPtr<ChromiumDLL::IntrusiveRefPtrI> getData()
	{
		return m_pData;
	}

	IMPLEMENT_REFCOUNTING(ObjectWrapper);

private:
	ChromiumDLL::RefPtr<ChromiumDLL::IntrusiveRefPtrI> m_pData;
};

class JavaScriptFactory : public ChromiumDLL::JavaScriptFactoryI
{
public:
	JavaScriptFactory();
	~JavaScriptFactory();

	virtual ChromiumDLL::JSObjHandle CreateUndefined();
	virtual ChromiumDLL::JSObjHandle CreateNull();
	virtual ChromiumDLL::JSObjHandle CreateBool(bool value);
	virtual ChromiumDLL::JSObjHandle CreateInt(int value);
	virtual ChromiumDLL::JSObjHandle CreateDouble(double value);
	virtual ChromiumDLL::JSObjHandle CreateString(const char* value);
	virtual ChromiumDLL::JSObjHandle CreateArray();
	virtual ChromiumDLL::JSObjHandle CreateObject();
	virtual ChromiumDLL::JSObjHandle CreateObject(const ChromiumDLL::RefPtr<ChromiumDLL::IntrusiveRefPtrI>& userData);
	virtual ChromiumDLL::JSObjHandle CreateException(const char* value);
	virtual ChromiumDLL::JSObjHandle CreateFunction(const char* name, const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& handler);

	CEF3_IMPLEMENTREF_COUNTING(JavaScriptFactory);
};

ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFactoryI> GetJSFactory();

#endif //THIRDPARTY_CEF3_JAVASCRIPTFACTORY_H
