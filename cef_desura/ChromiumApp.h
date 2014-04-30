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

#include "SharedMem.h"
#include "tinythread.h"

#include <list>

class ChromiumApp : public CefApp, protected CefBrowserProcessHandler
{
public:
	ChromiumApp()
		: m_bInit(false)
		, m_bIsStopped(false)
		, m_pWorkerThread(NULL)
	{
	}

	~ChromiumApp()
	{
		m_bIsStopped = true;

		m_WaitCond.notify_all();

		if (m_pWorkerThread && m_pWorkerThread->joinable())
			m_pWorkerThread->join();

		delete m_pWorkerThread;

		for (size_t x = 0; x < m_vJSExtenders.size(); ++x)
			m_vJSExtenders[x]->destroy();

		for (size_t x = 0; x < m_vSchemeExtenders.size(); ++x)
			m_vSchemeExtenders[x]->destroy();
	}

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

	bool RegisterJSExtender(ChromiumDLL::JavaScriptExtenderI* extender);
	bool RegisterSchemeExtender(ChromiumDLL::SchemeExtenderI* extender);


	bool processMessageReceived(CefRefPtr<CefBrowser> &browser, CefRefPtr<CefProcessMessage> &message);

protected:
	std::vector<std::string> getSchemeList();

	bool initJSExtenderSharedMem();

	void initThread();
	void run();
	static void runThread(void* pObj);
	void processRequest(CefRefPtr<CefBrowser> &browser, CefRefPtr<CefListValue> &request);

private:
	std::vector<ChromiumDLL::JavaScriptExtenderI*> m_vJSExtenders;
	std::vector<ChromiumDLL::SchemeExtenderI*> m_vSchemeExtenders;

	SharedMem m_SharedMemInfo;

	bool m_bInit;

	volatile bool m_bIsStopped;

	tthread::thread* m_pWorkerThread;
	tthread::mutex m_WaitLock;
	tthread::condition_variable m_WaitCond;
	tthread::mutex m_QueueLock;
	std::list<std::pair<CefRefPtr<CefBrowser>, CefRefPtr<CefListValue>>> m_WorkQueue;

	IMPLEMENT_REFCOUNTING(ChromiumApp);
};






