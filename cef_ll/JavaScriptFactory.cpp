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

#include "JavaScriptFactory.h"
//#include "include/cef.h"

#include "JavaScriptExtender.h"
#include "JavaScriptObject.h"

ChromiumDLL::RefPtr<JavaScriptFactory> g_pJavaScriptFactory;

ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFactoryI> GetJSFactory()
{
	if (!g_pJavaScriptFactory)
		g_pJavaScriptFactory = new JavaScriptFactory();

	return g_pJavaScriptFactory.get();
}




JavaScriptFactory::JavaScriptFactory()
{
}

JavaScriptFactory::~JavaScriptFactory()
{
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateUndefined()
{
	return new JavaScriptObject(CefV8Value::CreateUndefined());
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateNull()
{
	return new JavaScriptObject(CefV8Value::CreateNull());
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateBool(bool value)
{
	return new JavaScriptObject(CefV8Value::CreateBool(value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateInt(int value)
{
	return new JavaScriptObject(CefV8Value::CreateInt(value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateDouble(double value)
{
	return new JavaScriptObject(CefV8Value::CreateDouble(value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateString(const char* value)
{
	return new JavaScriptObject(CefV8Value::CreateString(value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateArray()
{
	return new JavaScriptObject(CefV8Value::CreateArray(0));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateObject()
{
	return new JavaScriptObject(CefV8Value::CreateObject(NULL));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateObject(const ChromiumDLL::RefPtr<ChromiumDLL::IntrusiveRefPtrI>& userData)
{
	CefBase* base = new ObjectWrapper(userData);
	// per Mark 2014-03-10: first create an object with no accessor
	CefRefPtr<CefV8Value> object = CefV8Value::CreateObject(NULL);
	// then set its UserData
	object->SetUserData(CefRefPtr<CefBase>(base));
	return new JavaScriptObject(object);
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateFunction(const char* name, const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& handler)
{
	CefRefPtr<CefV8Handler> e = new JavaScriptExtender(handler);
	return new JavaScriptObject(CefV8Value::CreateFunction(name, e));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateException(const char* value)
{
	JavaScriptObject *ret = new JavaScriptObject(CefV8Value::CreateString(value));
	ret->setException();

	return ret;
}
