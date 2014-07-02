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

#ifndef DESURA_PROCESSAPP_H
#define DESURA_PROCESSAPP_H

#include "include/cef_app.h"
#include "include/cef_render_process_handler.h"

#include "JavaScriptContext.h"

#include "SharedMem.h"
#include "libjson.h"
#include "tinythread.h"
#include "zmq.hpp"
#include "ZmqMonitor.h"

class JavaScriptExtenderProxy;

class ProcessApp
	: public CefApp
	, protected CefRenderProcessHandler
	, protected JsonSendTargetI
{
public:
	ProcessApp();
	~ProcessApp();

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// CefApp
	/////////////////////////////////////////////////////////////////////////////////////////////////

	void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE;
	CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE;

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// CefRenderProcessHandler
	/////////////////////////////////////////////////////////////////////////////////////////////////

	void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE;
	void OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info) OVERRIDE;
	void OnWebKitInitialized() OVERRIDE;

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// ProcessApp
	/////////////////////////////////////////////////////////////////////////////////////////////////

	bool send(int nBrowser, JSONNode msg) OVERRIDE;

	static int CreateBrowserId(const CefRefPtr<CefBrowser>& browser);

protected:
	static void runThread(void* pObj);
	void run();
	void processMessageReceived(const std::string &strJson);
	bool processResponse(JSONNode json);
	bool processRequest(JSONNode json);
	
	CefRefPtr<JavaScriptExtenderProxy> findExtender(JSONNode msg);
	int getBrowserId(JSONNode msg);

	static std::string generateZmqName();

private:
	IMPLEMENT_REFCOUNTING(ProcessApp);

	SharedMem m_SharedMemInfo;
	std::vector<CefRefPtr<JavaScriptExtenderProxy>> m_vJSExtenders;

	zmq::context_t m_ZmqContext;
	zmq::socket_t m_ZmqClient;
	ZmqMonitor m_ZmqMonitor;

	volatile bool m_bIsStopped;
	tthread::thread* m_pWorkerThread;

	const std::string m_strZmqIdentity;
	int m_nZmqPort;

	tthread::mutex m_CurLock;
	CefRefPtr<JavaScriptExtenderProxy> m_pCurrentExtender;

	tthread::mutex m_BrowserLock;
	std::map<int, CefRefPtr<CefBrowser>> m_mBrowsers;

	tthread::mutex m_StartLock;
	bool m_bConnected;
	std::vector<std::string> m_vPendingSend;
};




#endif