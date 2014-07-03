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

#ifndef THIRDPARTY_CEF3_CONTROLLER_H
#define THIRDPARTY_CEF3_CONTROLLER_H

#include "ChromiumBrowserI.h"
#include "Tracer.h"
#include "include\internal\cef_ptr.h"

#ifdef WIN32_SANDBOX_ENABLED
#include "include/cef_sandbox_win.h"
#endif

class ChromiumApp;

class ChromiumController : public ChromiumDLL::ChromiumControllerI
{
public:
	ChromiumController(bool bMainProcess);

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

	void trace(const std::string &strTrace, std::map<std::string, std::string> *mpArgs)
	{
		m_Tracer.trace(strTrace, mpArgs);
	}

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

	TracerStorage m_Tracer;


	ChromiumDLL::RefPtr<ChromiumDLL::ChromiumCookieManagerI> m_pCookieManager;
	ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI> m_pDefaults;
};


extern ChromiumController* g_Controller;



template <typename T>
std::string TraceClassInfo(T *pClass)
{
	return "";
}


#ifdef WIN32
#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif
#endif

#ifdef WIN32
inline std::string GetModule(HMODULE hModule, bool addPid = false)
{
	char szModuleName[255] = { 0 };
	DWORD nLen = GetModuleFileNameA(hModule, szModuleName, 255);

	auto t = std::string(szModuleName, nLen);
	t = t.substr(t.find_last_of('\\') + 1);

	if (addPid)
	{
		char szBuff[16] = { 0 };
		_snprintf(szBuff, 16, "-%d", GetCurrentProcessId());
		t += szBuff;
	}

	return t;
}
#endif

template <typename ... Args>
void TraceS(const char* szFunction, const char* szClassInfo, const char* szFormat, Args ... args)
{
	static auto getCurrentThreadId = []()
	{
#ifdef WIN32
		return ::GetCurrentThreadId();
#else
		return (uint64)pthread_self();
#endif
	};

	char szBuff[16] = { 0 };
	_snprintf(szBuff, 16, "%d", getCurrentThreadId());

	std::map<std::string, std::string> mArgs;

	mArgs["function"] = szFunction ? szFunction : "";
	mArgs["classinfo"] = szClassInfo ? szClassInfo : "";
	mArgs["thread"] = szBuff;


#ifdef WIN32
	LARGE_INTEGER pTime;
	pTime.QuadPart = 0;
	QueryPerformanceCounter(&pTime);

	char szTime[32] = { 0 };
	_snprintf(szTime, 32, "%I64u", pTime.QuadPart);

	mArgs["time"] = szTime;// gcTime().to_js_string();
	mArgs["module"] = GetModule(reinterpret_cast<HMODULE>(&__ImageBase));
	mArgs["app"] = GetModule(NULL, true);
#endif





	std::string strTrace; // = gcString(szFormat, args...);
	strTrace.resize(1024);

	int nSize = _snprintf(&strTrace[0], 1024, szFormat, args...);
	strTrace.resize(nSize);

	g_Controller->trace(strTrace, &mArgs);
}

template <typename T, typename ... Args>
void TraceT(const char* szFunction, T *pClass, const char* szFormat, Args ... args)
{
	auto ci = TraceClassInfo(pClass);
	TraceS(szFunction, ci.c_str(), szFormat, args...);
}

namespace
{
	class FakeTracerClass
	{
	};
}

#define cef3Trace( ... ) TraceT(__FUNCTION__, this, __VA_ARGS__)
#define cef3TraceS( ... ) TraceT(__FUNCTION__, (FakeTracerClass*)nullptr, __VA_ARGS__)




#endif