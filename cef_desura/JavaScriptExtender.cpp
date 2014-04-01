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

#include "JavaScriptExtender.h"
#include "JavaScriptFactory.h"
#include "JavaScriptObject.h"
#include "JavaScriptContext.h"

bool JavaScriptExtender::Register(ChromiumDLL::JavaScriptExtenderI* jse)
{
	return CefRegisterExtension(jse->getName(), jse->getRegistrationCode(), new JavaScriptExtender(jse));
}

JavaScriptExtender::JavaScriptExtender(ChromiumDLL::JavaScriptExtenderI* jse)
{
	m_pJSExtender = jse;
}

JavaScriptExtender::~JavaScriptExtender()
{
	if (m_pJSExtender)
		m_pJSExtender->destroy();
}

bool JavaScriptExtender::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
	if (!m_pJSExtender)
		return false;

	size_t argc = arguments.size();
	ChromiumDLL::JSObjHandle *argv = new ChromiumDLL::JSObjHandle[argc];

	for (size_t x=0; x<argc; x++)
		argv[x] = new JavaScriptObject(arguments[x]);

	ChromiumDLL::JavaScriptFunctionArgs args;

	CefStringUTF8 t(name.c_str());

	args.function = t.c_str();
	args.argc = argc;
	args.argv = argv;
	args.object = new JavaScriptObject(object);
	args.factory = GetJSFactory();
	args.context = new JavaScriptContext();

	ChromiumDLL::JSObjHandle jsoRes = m_pJSExtender->execute(&args);

	delete [] argv;
	args.context->destroy();

	if (jsoRes.get() == NULL)
		return false;

	if (jsoRes->isException())
	{
		char except[255] = {0};
		jsoRes->getStringValue(except, 255);

		exception.FromASCII(except);
		return true;
	}

	if (!jsoRes->isUndefined())
	{
		JavaScriptObject* jsoRetProper = (JavaScriptObject*)jsoRes.get();

		if (jsoRetProper)
			retval = jsoRetProper->getCefV8();
	}

	return true;
}




JavaScriptWrapper::JavaScriptWrapper()
{
}

JavaScriptWrapper::JavaScriptWrapper(CefRefPtr<CefV8Handler> obj)
{
	m_pObject = obj;
}


void JavaScriptWrapper::destroy()
{
	delete this;
}

ChromiumDLL::JavaScriptExtenderI* JavaScriptWrapper::clone()
{
	return new JavaScriptWrapper(m_pObject);
}

ChromiumDLL::JSObjHandle JavaScriptWrapper::execute(ChromiumDLL::JavaScriptFunctionArgs *args)
{
	int argc = args->argc;
	const char* function = args->function;

	ChromiumDLL::JSObjHandle *argv = args->argv;
	ChromiumDLL::JavaScriptFactoryI *factory = args->factory;
	ChromiumDLL::JavaScriptObjectI* jso = args->object.get();
	
	CefRefPtr<CefV8Value> object;
	CefRefPtr<CefV8Value> ret;
	CefString exception;
	CefV8ValueList arguments;

	JavaScriptObject* jsoRetProper = (JavaScriptObject*)jso;

	if (jsoRetProper)
		object = jsoRetProper->getCefV8();

	for (int x=0; x<argc; x++)
	{
		JavaScriptObject* jsoProper = (JavaScriptObject*)argv[x].get();

		if (jsoProper)
			arguments.push_back(jsoRetProper->getCefV8());
		else
			arguments.push_back(CefV8Value::CreateUndefined());
	}

	m_pObject->Execute(function, object, arguments, ret, exception);

	CefStringUTF8 t(exception.c_str());

	if (t.size() > 0)
		return factory->CreateException(t.c_str());

	return ChromiumDLL::JSObjHandle(new JavaScriptObject(ret));
}



const char* JavaScriptWrapper::getName()
{
	return NULL;
}

const char* JavaScriptWrapper::getRegistrationCode()
{
	return NULL;
}

CefRefPtr<CefV8Handler> JavaScriptWrapper::getCefV8Handler()
{
	return m_pObject;
}

CefRefPtr<CefBase> JavaScriptWrapper::getCefBase()
{
	CefRefPtr<CefBase> base(new V8HandleBaseWrapper(m_pObject));
	return base;
}















