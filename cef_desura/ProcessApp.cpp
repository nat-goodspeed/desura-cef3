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

#include "ChromiumBrowserI.h"
#include "include/cef_app.h"

class ProcessApp 
	: public CefApp
	, protected CefRenderProcessHandler
{
public:
	virtual void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE
	{
		//TODO Register schemes here
	}

	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE
	{
		return (CefRenderProcessHandler*)this;
	}

	virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE
	{
		//TODO Register js extenders here
	}

	virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE
	{

	}

	virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE
	{

	}

	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE
	{

	}

	virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE
	{

	}

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE
	{
		return CefRenderProcessHandler::OnProcessMessageReceived(browser, source_process, message);
	}

	IMPLEMENT_REFCOUNTING(ProcessApp);
};


extern "C"
{
#ifdef WIN32
	DLLINTERFACE int CEF_ExecuteProcessWin(HINSTANCE instance)
	{
		CefMainArgs main_args(instance);
		CefRefPtr<ProcessApp> app(new ProcessApp);

		// Execute the secondary process, if any.
		return CefExecuteProcess(main_args, app.get(), NULL);
	}
#else
	DLLINTERFACE int CEF_ExecuteProcess(int argc, char** argv)
	{
		CefMainArgs main_args;
		CefRefPtr<ProcessApp> app(new ProcessApp);

		// Execute the secondary process, if any.
		return CefExecuteProcess(main_args, app.get(), NULL);
	}
#endif
}