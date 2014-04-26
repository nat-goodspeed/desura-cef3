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

#ifndef DESURA_JAVASCRIPTFACTORY_H
#define DESURA_JAVASCRIPTFACTORY_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
//#include "include/cef.h"
#include "include/cef_base.h"

class ObjectWrapper : public CefBase
{
public:
	ObjectWrapper(void* data)
	{
		m_pData = data;
	}

	void* getData()
	{
		return m_pData;
	}

	IMPLEMENT_REFCOUNTING(ObjectWrapper);

private:
	void* m_pData;
};

class JavaScriptFactory : public ChromiumDLL::JavaScriptFactoryI
{
public:
	JavaScriptFactory();
	virtual ~JavaScriptFactory();

	virtual ChromiumDLL::JSObjHandle CreateUndefined();
	virtual ChromiumDLL::JSObjHandle CreateNull();
	virtual ChromiumDLL::JSObjHandle CreateBool(bool value);
	virtual ChromiumDLL::JSObjHandle CreateInt(int value);
	virtual ChromiumDLL::JSObjHandle CreateDouble(double value);
	virtual ChromiumDLL::JSObjHandle CreateString(const char* value);
	virtual ChromiumDLL::JSObjHandle CreateArray();
	virtual ChromiumDLL::JSObjHandle CreateObject();
	virtual ChromiumDLL::JSObjHandle CreateObject(void* userData);
	virtual ChromiumDLL::JSObjHandle CreateException(const char* value);
	virtual ChromiumDLL::JSObjHandle CreateFunction(const char* name, ChromiumDLL::JavaScriptExtenderI* handler);
};

ChromiumDLL::JavaScriptFactoryI* GetJSFactory();

#endif //DESURA_JAVASCRIPTFACTORY_H
