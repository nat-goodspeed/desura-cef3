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
#include "include/cef_scheme.h"
#include "ChromiumBrowser.h"
#include "Controller.h"

#include <string>
#include <sstream>

CefRefPtr<CefCommandLine> g_command_line;

class ProcessApp 
	: public CefApp
	, protected CefRenderProcessHandler
{
public:
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// CefApp
	/////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE
	{
		CefStringUTF8 strSchemes = ConvertToUtf8(g_command_line->GetSwitchValue("desura-schemes"));
		
		if (strSchemes.empty())
			return;

		std::stringstream ss(strSchemes.c_str());
		std::string s;

		while (std::getline(ss, s, '&')) 
			registrar->AddCustomScheme(s, true, false, false);
	}

	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE
	{
		return (CefRenderProcessHandler*)this;
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////
	// CefRenderProcessHandler
	/////////////////////////////////////////////////////////////////////////////////////////////////


	virtual void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE
	{
		//TODO Register js extenders here
	}

	virtual void OnWebKitInitialized()
	{

	}

	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE
	{

	}

	virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE
	{

	}

	virtual void OnUncaughtException(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Exception> exception, CefRefPtr<CefV8StackTrace> stackTrace)
	{

	}

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE
	{
		return CefRenderProcessHandler::OnProcessMessageReceived(browser, source_process, message);
	}

	IMPLEMENT_REFCOUNTING(ProcessApp);
};




#ifdef WIN32
int ChromiumController::ExecuteProcess(HINSTANCE instance)
{
	g_command_line = CefCommandLine::CreateCommandLine();
	g_command_line->InitFromString(::GetCommandLineW());

	CefMainArgs main_args(instance);
	CefRefPtr<ProcessApp> app(new ProcessApp);

#ifdef WIN32_SANDBOX_ENABLED
	void* sandbox_info = m_pSandbox.sandbox_info();
#else
	void* sandbox_info = NULL;
#endif

	// Execute the secondary process, if any.
	return CefExecuteProcess(main_args, app.get(), sandbox_info);
}
#else
int ChromiumController::ExecuteProcess(int argc, char** argv)
{
	g_command_line = CefCommandLine::CreateCommandLine();
	g_command_line->InitFromArgv(argc, argv);

	CefMainArgs main_args;
	CefRefPtr<ProcessApp> app(new ProcessApp);

	// Execute the secondary process, if any.
	return CefExecuteProcess(main_args, app.get(), NULL);
}
#endif
