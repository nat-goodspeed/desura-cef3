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
#include "ChromiumApp.h"

#ifdef WIN32_SANDBOX_ENABLED
#include "include/cef_sandbox_win.h"
#endif

class ChromiumController : public ChromiumDLL::ChromiumControllerI
{
public:
	ChromiumController();

	virtual int GetMaxApiVersion();

	virtual void SetApiVersion(int version);

	virtual void DoMsgLoop();
	virtual void RunMsgLoop();

	virtual void Stop();

	virtual void SetDefaults(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>& defaults) OVERRIDE;

	virtual bool RegisterJSExtender(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& extender);
	virtual bool RegisterSchemeExtender(const ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI>& extender);

	virtual ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserI> NewChromiumBrowser(int* formHandle, const char* name, const char* defaultUrl);
	virtual ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererI> NewChromiumRenderer(int* formHandle, const char* defaultUrl, int width, int height);

	virtual void SetLogHandler(ChromiumDLL::LogMessageHandlerFn logFn);

	virtual void PostCallback(const ChromiumDLL::RefPtr<ChromiumDLL::CallbackI>& callback);
	virtual void PostCallbackEx(ChromiumDLL::ThreadID thread, const ChromiumDLL::RefPtr<ChromiumDLL::CallbackI>& callback);

	virtual ChromiumDLL::RefPtr<ChromiumDLL::ChromiumCookieManagerI> GetCookieManager() OVERRIDE;

	bool Init(bool threaded, const char* cachePath, const char* logPath, const char* userAgent);

#ifdef WIN32
	int ExecuteProcess(HINSTANCE instance);
#else
	int ExecuteProcess(int argc, char** argv);
#endif

protected:
	bool DoInit(bool threaded, const char* cachePath, const char* logPath, const char* userAgent);

private:
	CefRefPtr<ChromiumApp> m_App;

	bool m_bInit;
	bool m_bPendingInit;
	bool m_bThreaded;

	std::string m_strCachePath;
	std::string m_strLogPath;
	std::string m_strUserAgent;

#ifdef WIN32_SANDBOX_ENABLED
	CefScopedSandboxInfo m_pSandbox;
#endif

	ChromiumDLL::RefPtr<ChromiumDLL::ChromiumCookieManagerI> m_pCookieManager;
	ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI> m_pDefaults;
};
