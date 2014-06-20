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

#ifndef DESURA_JAVASCRIPTEXTENDER_H
#define DESURA_JAVASCRIPTEXTENDER_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
//#include "include/cef.h"
#include "include/cef_v8.h"

class V8HandleBaseWrapper : public CefBase
{
public:
	V8HandleBaseWrapper(CefRefPtr<CefV8Handler> object)
	{
		m_pObject = object;
		m_iNumRef = 1;
	}

	virtual int AddRef()
	{
		m_iNumRef++;
		return m_iNumRef;
	}

	virtual int Release()
	{
		m_iNumRef--;

		if (m_iNumRef == 0)
			delete this;

		return m_iNumRef;
	}

	virtual int GetRefCt()
	{
		return m_iNumRef;
	}

	int m_iNumRef;
	CefRefPtr<CefV8Handler> m_pObject;
};


class JavaScriptExtender : public CefV8Handler
{
public:
	static bool Register(ChromiumDLL::JavaScriptExtenderI* jse);

	JavaScriptExtender(ChromiumDLL::JavaScriptExtenderI* jse);
	~JavaScriptExtender();

	virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception);

	IMPLEMENT_REFCOUNTING(JavaScriptExtender)

private:
	ChromiumDLL::JavaScriptExtenderI* m_pJSExtender;
};



class JavaScriptWrapper : public ChromiumDLL::JavaScriptExtenderI
{
public:
	JavaScriptWrapper();
	JavaScriptWrapper(CefRefPtr<CefV8Handler> obj);

	virtual void destroy();
	virtual ChromiumDLL::JavaScriptExtenderI* clone();
	virtual ChromiumDLL::JSObjHandle execute(ChromiumDLL::JavaScriptFunctionArgs *args);

	virtual const char* getName();
	virtual const char* getRegistrationCode();

	CefRefPtr<CefV8Handler> getCefV8Handler();
	CefRefPtr<CefBase> getCefBase();

private:
	CefRefPtr<CefV8Handler> m_pObject;
};

#endif //DESURA_JAVASCRIPTEXTENDER_H
