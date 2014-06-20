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

#include "JavaScriptContext.h"
#include "JavaScriptFactory.h"
#include "JavaScriptObject.h"

JavaScriptContext::JavaScriptContext()
{
	m_pContext = CefV8Context::GetCurrentContext();
	m_uiCount = 0;
}

JavaScriptContext::JavaScriptContext(CefRefPtr<CefV8Context> context)
{
	m_pContext = context;
	m_uiCount = 0;
}

void JavaScriptContext::destroy()
{
	delete this;
}

ChromiumDLL::JavaScriptContextI* JavaScriptContext::clone()
{
	return new JavaScriptContext(m_pContext);
}

void JavaScriptContext::enter()
{
	if (m_pContext.get())
	{
		m_pContext->Enter();
		m_uiCount++;
	}
}

void JavaScriptContext::exit()
{
	if (m_pContext.get())
	{
		m_pContext->Exit();
		m_uiCount--;
	}
}

ChromiumDLL::JavaScriptFactoryI* JavaScriptContext::getFactory()
{
	if (!m_pContext.get() || m_uiCount == 0)
		return NULL;

	return GetJSFactory();
}

CefRefPtr<CefV8Context> JavaScriptContext::getCefV8()
{
	return m_pContext;
}

ChromiumDLL::JSObjHandle JavaScriptContext::getGlobalObject()
{
	if (m_pContext.get())
		return new JavaScriptObject(m_pContext->GetGlobal());

	return NULL;
}
