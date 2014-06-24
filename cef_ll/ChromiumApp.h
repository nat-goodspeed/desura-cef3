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


class ChromiumApp : public CefApp, protected CefBrowserProcessHandler
{
public:
	ChromiumApp();
	~ChromiumApp();

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// CefBrowserProcessHandler
	/////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar);
	virtual void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line);
	virtual void OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info);
	virtual void OnContextInitialized();

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// CefApp
	/////////////////////////////////////////////////////////////////////////////////////////////////

	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() 
	{
		return this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// CefApp
	/////////////////////////////////////////////////////////////////////////////////////////////////

	bool RegisterJSExtender(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& extender);
	bool RegisterSchemeExtender(const ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI>& extender);

protected:
	std::vector<std::string> getSchemeList();

private:
	std::vector<ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>> m_vJSExtenders;
	std::vector<ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI>> m_vSchemeExtenders;

	bool m_bInit;

	IMPLEMENT_REFCOUNTING(ChromiumApp);
};






