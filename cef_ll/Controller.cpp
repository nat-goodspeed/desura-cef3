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

#include "Controller.h"
#include "ChromiumBrowser.h"
#include "include/cef_web_plugin.h"
#include "Cookie.h"

int g_nApiVersion = 1;

ChromiumDLL::LogMessageHandlerFn g_pLogHandler = NULL;

bool logHandler(int level, const std::string& msg)
{
	if (g_pLogHandler)
		return g_pLogHandler(level, msg.c_str());

	return false;
}





ChromiumController::ChromiumController()
	: m_bInit(false)
	, m_bPendingInit(false)
	, m_App(new ChromiumApp())
{
}

int ChromiumController::GetMaxApiVersion()
{
	return 2;
}

void ChromiumController::SetApiVersion(int version)
{
	if (version <= 0)
		g_nApiVersion = 1;
	else
		g_nApiVersion = version;
}

void ChromiumController::DoMsgLoop()
{
	CefDoMessageLoopWork();
}

void ChromiumController::RunMsgLoop()
{
	CefRunMessageLoop();
}

void ChromiumController::Stop()
{
	CefShutdown();
}

bool ChromiumController::RegisterJSExtender(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& extender)
{
	return m_App->RegisterJSExtender(extender);
}

bool ChromiumController::RegisterSchemeExtender(const ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI>& extender)
{
	return m_App->RegisterSchemeExtender(extender);
}

ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserI> ChromiumController::NewChromiumBrowser(int* formHandle, const char* name, const char* defaultUrl)
{
	if (m_bInit && m_bPendingInit)
		DoInit(m_bThreaded, m_strCachePath.c_str(), m_strLogPath.c_str(), m_strUserAgent.c_str());

	if (!m_bInit)
		return NULL;

	return new ChromiumBrowser((WIN_HANDLE)formHandle, defaultUrl, m_pDefaults);
}

ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererI> ChromiumController::NewChromiumRenderer(int* formHandle, const char* defaultUrl, int width, int height)
{
	if (m_bInit && m_bPendingInit)
		DoInit(m_bThreaded, m_strCachePath.c_str(), m_strLogPath.c_str(), m_strUserAgent.c_str());

	if (!m_bInit)
		return NULL;

	return new ChromiumRenderer((WIN_HANDLE)formHandle, defaultUrl, width, height, m_pDefaults);
}

void ChromiumController::SetLogHandler(ChromiumDLL::LogMessageHandlerFn logFn)
{
	g_pLogHandler = logFn;
}

void ChromiumController::PostCallback(const ChromiumDLL::RefPtr<ChromiumDLL::CallbackI>& callback)
{
	CefPostTask(TID_UI, CefRefPtr<CefTask>(new TaskWrapper(callback)));
}

void ChromiumController::PostCallbackEx(ChromiumDLL::ThreadID thread, const ChromiumDLL::RefPtr<ChromiumDLL::CallbackI>& callback)
{
	CefPostTask((CefThreadId)thread, CefRefPtr<CefTask>(new TaskWrapper(callback)));
}


bool ChromiumController::Init(bool threaded, const char* cachePath, const char* logPath, const char* userAgent)
{
	if (m_bInit || m_bPendingInit)
		return true;

	m_bThreaded = threaded;

	if (cachePath)
		m_strCachePath = cachePath;

	if (logPath)
		m_strLogPath = logPath;

	if (userAgent)
		m_strUserAgent = userAgent;

	m_bPendingInit = true;
	m_bInit = true;
	return true;
}

bool ChromiumController::DoInit(bool threaded, const char* cachePath, const char* logPath, const char* userAgent)
{
	CefSettings settings;
	CefMainArgs args;

	if (userAgent)
	{
		std::string temp(userAgent);
		size_t pos = temp.find("Desura/");

		if (pos != std::string::npos)
			temp = temp.substr(pos);

		cef_string_utf8_to_utf16(temp.c_str(), temp.size(), &settings.product_version);
	}

	static const char browser_subprocess_path[] = "3p_cef3_host";
	cef_string_utf8_to_utf16(browser_subprocess_path, strlen(browser_subprocess_path), &settings.browser_subprocess_path);
	cef_string_utf8_to_utf16(cachePath, strlen(cachePath), &settings.cache_path);
	

	settings.multi_threaded_message_loop = threaded;
	settings.remote_debugging_port = 2323;

#ifdef WIN32_SANDBOX_ENABLED
	void* sandbox_info = m_pSandbox.sandbox_info();
#else
	settings.no_sandbox = true;
	void* sandbox_info = NULL;
#endif

	if (!CefInitialize(args, settings, m_App.get(), sandbox_info))
	{
		m_bInit = false;
		m_bPendingInit = false;
		return false;
	}

	if (!m_pDefaults || m_pDefaults->enableFlash())
	{
#if defined(_WIN32)
		CefAddWebPluginPath("gcswf32.dll");
#else
		CefAddWebPluginPath("libdesura_flashwrapper.so");
#endif

		CefAddWebPluginDirectory("PepperFlash");
		CefRefreshWebPlugins();
	}

	m_bPendingInit = false;
	return true;
}

void ChromiumController::SetDefaults(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>& defaults)
{
	m_pDefaults = defaults;
}

ChromiumDLL::RefPtr<ChromiumDLL::ChromiumCookieManagerI> ChromiumController::GetCookieManager()
{
	if (!m_pCookieManager)
		m_pCookieManager = new CookieManager();

	return m_pCookieManager;
}