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

#ifndef DESURA_JAVASCRIPTEXTENDERPROXY_H
#define DESURA_JAVASCRIPTEXTENDERPROXY_H

#include "include/cef_v8.h"
#include "include/internal/cef_types.h"

#include "libjson.h"
#include "tinythread.h"

class ProcessApp;

class JavaScriptExtenderProxy : public CefV8Handler
{
public:
	static const cef_thread_id_t TaskThread = TID_RENDERER;

	JavaScriptExtenderProxy(const std::string &strId, const CefRefPtr<CefV8Value> &funct, const CefRefPtr<CefV8Context> &context);
	JavaScriptExtenderProxy(const std::string &strName);

	const char* getName();

	//! V8 is trying to call c++
	//!
	bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) OVERRIDE;

	//! c++ calling into v8
	//! 
	JSONNode execute(const std::string &strFunction, JSONNode object, JSONNode argumets);

	virtual JSONNode objectRequest(const std::string &strObjectId, const std::string &strFunction, JSONNode argumets);


	CefRefPtr<CefV8Value> getV8Value()
	{
		return m_Function;
	}

protected:
	JSONNode convertV8ToJson(CefRefPtr<CefV8Value> &object, const CefV8ValueList& arguments);

	CefRefPtr<CefV8Context> m_Context;

private:
	const std::string m_strName;
	CefRefPtr<CefV8Value> m_Function;
	
	IMPLEMENT_REFCOUNTING(V8ProxyHandler);
};


class JavaScriptGlobalObjectProxy : public JavaScriptExtenderProxy
{
public:
	JavaScriptGlobalObjectProxy(const std::string &strId, const CefRefPtr<CefV8Value> &object, const CefRefPtr<CefV8Context> &context)
		: JavaScriptExtenderProxy(strId, object, context)
		, m_Object(object)
	{
	}

	~JavaScriptGlobalObjectProxy();

	JSONNode objectRequest(const std::string &strObjectId, const std::string &strFunction, JSONNode argumets);

private:
	CefRefPtr<CefV8Value> m_Object;
	std::vector<std::string> m_vReleated;
};


#endif