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

#include "JavaScriptContext.h"

#include "SharedMem.h"
#include "tinythread.h"

#include "zmq.hpp"
#include "libjson.h"

#include <list>

class JavaScriptExtenderRef : public CefBase
{
public:
	static const cef_thread_id_t TaskThread = TID_UI;

	JavaScriptExtenderRef(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& pExtender)
		: m_pExtender(pExtender)
	{
	}

	ChromiumDLL::JavaScriptExtenderI* operator->()
	{
		return m_pExtender.get();
	}

	operator ChromiumDLL::JavaScriptExtenderI*()
	{
		return m_pExtender.get();
	}

	JSONNode execute(const std::string &strFunction, JSONNode object, JSONNode argumets);

	const char* getName()
	{
		return m_pExtender->getName();
	}

private:
	ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI> m_pExtender;

	IMPLEMENT_REFCOUNTING(JavaScriptExtenderRef);
};

class ChromiumApp : public CefApp, protected CefBrowserProcessHandler, protected JsonSendTargetI
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

	bool send(int nBrowser, JSONNode msg) OVERRIDE;

protected:
	std::vector<std::string> getSchemeList();

	bool initJSExtenderSharedMem();

	void initThread();
	void run();
	static void runThread(void* pObj);

	void initIpc();

	CefRefPtr<JavaScriptExtenderRef> findExtender(JSONNode msg);
	int getBrowserId(JSONNode msg);

	void processMessageReceived(const std::string &strFrom, const std::string &strJson);

private:
	zmq::context_t m_ZmqContext;
	zmq::socket_t m_ZmqServer;

	std::vector<CefRefPtr<JavaScriptExtenderRef>> m_vJSExtenders;
	std::vector<ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI>> m_vSchemeExtenders;

	SharedMem m_SharedMemInfo;

	bool m_bInit;
	bool m_bIpcInit;

	volatile bool m_bIsStopped;
	tthread::thread* m_pWorkerThread;

	tthread::mutex m_BrowserLock;
	std::map<int, std::string> m_mBrowserIdentity;

	int m_nZmqPort;

	IMPLEMENT_REFCOUNTING(ChromiumApp);
};






