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

extern void CEF_DeleteCookie_Internal(const char* url, const char* name);
extern ChromiumDLL::CookieI* CEF_CreateCookie_Internal();
extern void CEF_SetCookie_Internal(const char* url, ChromiumDLL::CookieI* cookie);

int g_nApiVersion = 1;

ChromiumDLL::LogMessageHandlerFn g_pLogHandler = NULL;

bool logHandler(int level, const std::string& msg)
{
	if (g_pLogHandler)
		return g_pLogHandler(level, msg.c_str());

	return false;
}




ChromiumController::ChromiumController()
	: m_App(new ChromiumApp())
	, m_bInit(false)
	, m_bPendingInit(false)
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

bool ChromiumController::RegisterJSExtender(ChromiumDLL::JavaScriptExtenderI* extender)
{
	return m_App->RegisterJSExtender(extender);
}

bool ChromiumController::RegisterSchemeExtender(ChromiumDLL::SchemeExtenderI* extender)
{
	return m_App->RegisterSchemeExtender(extender);
}

ChromiumDLL::ChromiumBrowserI* ChromiumController::NewChromiumBrowser(int* formHandle, const char* name, const char* defaultUrl)
{
	if (m_bInit && m_bPendingInit)
		DoInit(m_bThreaded, m_strCachePath.c_str(), m_strLogPath.c_str(), m_strUserAgent.c_str());

	if (!m_bInit)
		return NULL;

	return new ChromiumBrowser((WIN_HANDLE)formHandle, defaultUrl);
}

ChromiumDLL::ChromiumRendererI* ChromiumController::NewChromiumRenderer(int* formHandle, const char* defaultUrl, int width, int height)
{
	if (m_bInit && m_bPendingInit)
		DoInit(m_bThreaded, m_strCachePath.c_str(), m_strLogPath.c_str(), m_strUserAgent.c_str());

	if (!m_bInit)
		return NULL;

	return new ChromiumRenderer((WIN_HANDLE)formHandle, defaultUrl, width, height);
}

void ChromiumController::SetLogHandler(ChromiumDLL::LogMessageHandlerFn logFn)
{
	g_pLogHandler = logFn;
}

void ChromiumController::PostCallback(ChromiumDLL::CallbackI* callback)
{
	CefPostTask(TID_UI, CefRefPtr<CefTask>(new TaskWrapper(callback)));
}

void ChromiumController::PostCallbackEx(ChromiumDLL::ThreadID thread, ChromiumDLL::CallbackI* callback)
{
	CefPostTask((CefThreadId)thread, CefRefPtr<CefTask>(new TaskWrapper(callback)));
}

void ChromiumController::DeleteCookie(const char* url, const char* name)
{
	CEF_DeleteCookie_Internal(url, name);
}

ChromiumDLL::CookieI* ChromiumController::CreateCookie()
{
	return CEF_CreateCookie_Internal();
}

void ChromiumController::SetCookie(const char* url, ChromiumDLL::CookieI* cookie)
{
	CEF_SetCookie_Internal(url, cookie);
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

		setCefString(settings.product_version, temp);
	}

	setCefString(settings.browser_subprocess_path, "cef_desura_host");
	setCefString(settings.cache_path, cachePath);

	settings.multi_threaded_message_loop = threaded;
	settings.remote_debugging_port = 2323;

#ifdef WIN32_SANDBOX_ENABLED
	void* sandbox_info = m_pSandbox.sandbox_info();
#else
	settings.no_sandbox = true;
	void* sandbox_info = NULL;
#endif

#if defined(__APPLE__)
	// The Mac buildbot packages a Resources directory containing en.lproj and
	// en_GB.lproj -- but despite the fact that the hardcoded default locale
	// string is en-US, there is no en-US.lproj.
	setCefString(settings.locale, "en");

	// give up in disgust?!
	settings.pack_loading_disabled = true;
#endif

	if (!CefInitialize(args, settings, m_App.get(), sandbox_info))
	{
		m_bInit = false;
		m_bPendingInit = false;
		return false;
	}

#if defined(_WIN32)
	CefAddWebPluginPath("gcswf32.dll");
#else
	CefAddWebPluginPath("libdesura_flashwrapper.so");
#endif

	m_bPendingInit = false;
	return true;
}
