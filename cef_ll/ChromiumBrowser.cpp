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

#if defined(_WIN32)
#include "windows.h"
#endif

#include "ChromiumBrowser.h"
#include "Controller.h"
#include "SchemeExtender.h"

#include "JavaScriptContext.h"
#include "ChromiumBrowserEvents.h"

#include <map>

#ifdef OS_LINUX
#include <gtk/gtk.h>
#endif

#if defined __x86_64 || defined __amd64 || defined __x86_64__
	#define NIX64 1
#endif

CefStringUTF8 ConvertToUtf8(const CefString& str)
{
	cef_string_utf8_t *tmp = new cef_string_utf8_t();
	cef_string_to_utf8(str.c_str(), str.size(), tmp);

	CefStringUTF8 t;
	t.Attach(tmp, true);
	return t;
}





#ifdef OS_LINUX
static void gtkFocus(GtkWidget *widget, GdkEvent *event, ChromiumBrowser *data)
{
	if (data)
		data->onFocus();
}
#endif

ChromiumController* g_Controller = NULL;

extern "C"
{
	DLLINTERFACE ChromiumDLL::ChromiumControllerI* CEF_InitEx(bool threaded, const char* cachePath, const char* logPath, const char* userAgent)
	{
		if (!g_Controller)
			g_Controller = new ChromiumController(true);

		if (g_Controller->Init(threaded, cachePath, logPath, userAgent))
			return g_Controller;

		return NULL;
	}

#ifdef WIN32
	DLLINTERFACE int CEF_ExecuteProcessWin(HINSTANCE instance)
	{
		if (!g_Controller)
			g_Controller = new ChromiumController(false);

		return g_Controller->ExecuteProcess(instance);
	}
#else
	DLLINTERFACE int CEF_ExecuteProcess(int argc, char** argv)
	{
		if (!g_Controller)
			g_Controller = new ChromiumController(false);

		return g_Controller->ExecuteProcess(argc, argv);
	}
#endif
}

enum ACTION
{
	A_STOPLOAD,
	A_REFRESH,
	A_BACK,
	A_FORWARD,
	A_ZOOMIN,
	A_ZOOMOUT,
	A_ZOOMNORMAL,
	A_PRINT,
	A_VIEWSOURCE,
	A_UNDO,
	A_REDO,
	A_CUT,
	A_COPY,
	A_PASTE,
	A_DEL,
	A_SELECTALL,
};

class BrowserTask : public CefTask
{
public:
	BrowserTask(CefBrowser* browser, ACTION action)
	{
		m_pBrowser = browser;
		m_iRef = 1;
		m_Action = action;
	}

	virtual void Execute()
	{
		if (!m_pBrowser)
			return;

		bool handled = true;

		switch (m_Action)
		{
			case A_STOPLOAD:	m_pBrowser->StopLoad();						break;
			case A_REFRESH:		m_pBrowser->ReloadIgnoreCache();			break;
			case A_BACK:		m_pBrowser->GoBack();						break;
			case A_FORWARD:		m_pBrowser->GoForward();					break;
			default:
				handled = false;
				break;
		};

		if (handled)
			return;

		CefRefPtr<CefFrame> frame = m_pBrowser->GetFocusedFrame();

		if (!frame.get())
			return;

		switch (m_Action)
		{
		case A_ZOOMIN:		m_pBrowser->GetHost()->SetZoomLevel(m_pBrowser->GetHost()->GetZoomLevel() + 1);	break;
		case A_ZOOMOUT:		m_pBrowser->GetHost()->SetZoomLevel(m_pBrowser->GetHost()->GetZoomLevel() - 1);	break;
		case A_ZOOMNORMAL:	m_pBrowser->GetHost()->SetZoomLevel(0.0); break;
		case A_PRINT:		m_pBrowser->GetHost()->Print();	break;
		case A_VIEWSOURCE:	frame->ViewSource(); break;
		case A_UNDO:		frame->Undo();		break;
		case A_REDO:		frame->Redo();		break;
		case A_CUT:			frame->Cut();		break;
		case A_COPY:		frame->Copy();		break;
		case A_PASTE:		frame->Paste();		break;
		case A_DEL:			frame->Delete();	break;
		case A_SELECTALL:	frame->SelectAll(); break;
		default:
			handled = false;
			break;
		};
	}

	virtual int AddRef()
	{
		m_iRef++;
		return m_iRef;
	}

	virtual int Release()
	{
		m_iRef--;

		if (m_iRef == 0)
			delete this;

		return m_iRef;
	}

	virtual int GetRefCt()
	{
		return m_iRef;
	}

	int m_iRef;
	ACTION m_Action;
	CefBrowser* m_pBrowser;
};

ChromiumBrowser::ChromiumBrowser(WIN_HANDLE handle, const char* defaultUrl, const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>& defaults)
{
	m_iLastTask = 0;
	m_hFormHandle = handle;
	m_pBrowser = NULL;

	m_rEventHandler = (CefClient*)new ChromiumBrowserEvents(this);
	init(defaultUrl, false, defaults);
}

ChromiumBrowser::ChromiumBrowser(WIN_HANDLE handle)
{
	m_iLastTask = 0;
	m_hFormHandle = handle;
	m_pBrowser = NULL;

	m_rEventHandler = (CefClient*)new ChromiumBrowserEvents(this);
}

ChromiumBrowser::~ChromiumBrowser()
{
	ChromiumBrowserEvents* cbe = (ChromiumBrowserEvents*)m_rEventHandler.get();

	if (cbe)
	{
		cbe->setParent(NULL);
		cbe->setCallBack((ChromiumDLL::ChromiumBrowserEventI*)NULL);
		cbe->setCallBack((ChromiumDLL::ChromiumRendererEventI*)NULL);
	}
}

CefBrowserSettings ChromiumBrowser::getBrowserDefaults(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>& defaults)
{
	CefBrowserSettings browserDefaults;
	browserDefaults.universal_access_from_file_urls = STATE_ENABLED;
	browserDefaults.file_access_from_file_urls = STATE_ENABLED;
	browserDefaults.java = (defaults && defaults->enableJava()) ? STATE_ENABLED : STATE_DISABLED;
	browserDefaults.plugins = (!defaults || defaults->enablePlugins()) ? STATE_ENABLED : STATE_DISABLED;
	browserDefaults.javascript = (!defaults || defaults->enableJavascript()) ? STATE_ENABLED : STATE_DISABLED;
	browserDefaults.javascript_close_windows = STATE_DISABLED;
	browserDefaults.javascript_open_windows = STATE_DISABLED;

	return browserDefaults;
}

#ifdef OS_WIN
void ChromiumBrowser::init(const char *defaultUrl, bool offScreen, int width, int height, const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>& defaults)
{
	if (width <= 0)
		width = 500;

	if (height <= 0)
		height = 500;

	CefWindowInfo winInfo;
	winInfo.parent_window = m_hFormHandle;
	winInfo.height = width;
	winInfo.width = height;

	if (offScreen)
		winInfo.window_rendering_disabled = true;
	else
		winInfo.style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP;

	const char* name = "DesuraCEFBrowser";
	cef_string_utf8_to_utf16(name, strlen(name), &winInfo.window_name);

	CefBrowserHost::CreateBrowser(winInfo, m_rEventHandler, defaultUrl, getBrowserDefaults(defaults), CefRefPtr<CefRequestContext>());
}

#else

class CreateTask : public CefTask
{
public:
	CreateTask(ChromiumBrowser* browser, const std::string& defaultUrl)
	{
		m_pBrowser = browser;
		m_szDefaultUrl = defaultUrl;
	}

	void Execute()
	{
		m_pBrowser->initCallback(m_szDefaultUrl);
	}

	virtual int AddRef(){return 1;}
	virtual int Release(){return 1;}
	virtual int GetRefCt(){return 1;}

	ChromiumBrowser *m_pBrowser;
	std::string m_szDefaultUrl;
};

void ChromiumBrowser::init(const char *defaultUrl,
						   bool /*offScreen*/, int /*width*/, int /*height*/)
{
	CefPostTask(TID_UI, new CreateTask(this, defaultUrl));
}

void ChromiumBrowser::initCallback(const std::string& defaultUrl)
{
	CefWindowInfo winInfo;
	winInfo.SetAsChild(GTK_WIDGET(m_hFormHandle));

	m_pBrowser = CefBrowserHost::CreateBrowserSync(winInfo, m_rEventHandler, defaultUrl.c_str(), getBrowserDefaults(),
                                                   // nat: CreateBrowserSync()
                                                   // now requires a
                                                   // CefRequestContext. I do
                                                   // not know what to use for
                                                   // that.
                                                   CefRefPtr<CefRequestContext>());
	g_signal_connect(GTK_WIDGET(m_hFormHandle), "button-press-event", G_CALLBACK(gtkFocus), this);
	gtk_widget_show_all(GTK_WIDGET(m_hFormHandle));
}
#endif






void ChromiumBrowser::loadString(const char* string)
{
	if (m_pBrowser && m_pBrowser->GetMainFrame())
	{
		m_pBrowser->GetMainFrame()->LoadString(string, "http://local");
	}
	else
	{
		if (string)
			m_szBuffer = string;
		m_iLastTask = 1;
	}
}

void ChromiumBrowser::loadUrl(const char* url)
{
	if (m_pBrowser)
	{
		m_pBrowser->GetMainFrame()->LoadURL(url);
	}
	else
	{
		m_szBuffer = url;
		m_iLastTask = 2;
	}
}


void ChromiumBrowser::stop()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_STOPLOAD));
}

void ChromiumBrowser::refresh(bool ignoreCache)
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_REFRESH));
}

void ChromiumBrowser::back()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_BACK));
}

void ChromiumBrowser::forward()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_FORWARD));
}

void ChromiumBrowser::zoomIn()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_ZOOMIN));
}

void ChromiumBrowser::zoomOut()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_ZOOMOUT));	
}

void ChromiumBrowser::zoomNormal()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_ZOOMNORMAL));
}

void ChromiumBrowser::print()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_PRINT));
}

void ChromiumBrowser::viewSource()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_VIEWSOURCE));	
}

void ChromiumBrowser::undo()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_UNDO));
}

void ChromiumBrowser::redo()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_REDO));
}

void ChromiumBrowser::cut()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_CUT));
}


void ChromiumBrowser::copy()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_COPY));	
}

void ChromiumBrowser::paste()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_PASTE));	
}

void ChromiumBrowser::del()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_DEL));
}

void ChromiumBrowser::selectall()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_SELECTALL));
}



void ChromiumBrowser::setEventCallback(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserEventI>& cbeI)
{
	ChromiumBrowserEvents* cbe = (ChromiumBrowserEvents*)m_rEventHandler.get();

	if (cbe)
		cbe->setCallBack(cbeI);
}

void ChromiumBrowser::executeJScript(const char* code, const char* scripturl, int startline)
{
	if (!m_pBrowser || !m_pBrowser->GetMainFrame() || !code)
		return;

	m_pBrowser->GetMainFrame()->ExecuteJavaScript(code, scripturl?scripturl:"", startline);
}

void ChromiumBrowser::onFocus()
{
	if (m_pBrowser && m_pBrowser->GetHost())
		m_pBrowser->GetHost()->SetFocus(true);
}

#if defined(_WIN32)
void ChromiumBrowser::onPaintBg()
{
	// Dont erase the background if the browser window has been loaded
	// (this avoids flashing)
}


void ChromiumBrowser::onPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hFormHandle, &ps);
	EndPaint(m_hFormHandle, &ps);
}

void ChromiumBrowser::onResize()
{
	HWND hWnd = m_hFormHandle;

	if(m_pBrowser && m_pBrowser->GetHost()->GetWindowHandle())
	{
		// Resize the browser window and address bar to match the new frame
		// window size
		RECT rect;
		::GetClientRect(hWnd, &rect);

		HDWP hdwp = BeginDeferWindowPos(1);
		hdwp = DeferWindowPos(hdwp, m_pBrowser->GetHost()->GetWindowHandle(), NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		EndDeferWindowPos(hdwp);
	}
}

#else
void ChromiumBrowser::onResize(int x, int y, int width, int height)
{
	GtkAllocation a;
	a.x = x;
	a.y = y;
	a.width = width;
	a.height = height;

	gtk_widget_size_allocate(GTK_WIDGET(m_hFormHandle), &a);
	gtk_widget_set_size_request(GTK_WIDGET(m_hFormHandle), width, height);
}
#endif

void ChromiumBrowser::setBrowser(const CefRefPtr<CefBrowser>& browser)
{
	m_pBrowser = browser;

	if (m_iLastTask == 1)
	{
		loadString(m_szBuffer.c_str());
	}
	else if (m_iLastTask == 2)
	{
		loadUrl(m_szBuffer.c_str());
	}

	m_szBuffer = "";

#ifdef WIN32
	onResize();
#endif
}

class BrowserCallback : public CefTask
{
public:
	typedef void(ChromiumBrowser::*CallbackFn)();

	BrowserCallback(ChromiumBrowser* pBrowser, CallbackFn callback)
		: m_pBrowser(pBrowser)
		, m_fnCallback(callback)
	{
	}

	void Execute() OVERRIDE
	{
		(*m_pBrowser.*m_fnCallback)();
	}

	IMPLEMENT_REFCOUNTING(BrowserCallback);

private:
	CallbackFn m_fnCallback;
	ChromiumBrowser *m_pBrowser;
};

class DevToolsClient : public CefClient
{
public:
	IMPLEMENT_REFCOUNTING(DevToolsClient);
};


void ChromiumBrowser::showInspector()
{
	if (!CefCurrentlyOn(TID_UI)) {
		CefPostTask(TID_UI, new BrowserCallback(this, &ChromiumBrowser::showInspector));
	}
	else
	{
#ifdef CEF_NEW_DEVTOOL_API
		if (m_Inspector)
		{
			m_Inspector->GetHost()->SetFocus(true);
			return;
		}

		CefWindowInfo winInfo;
		winInfo.height = 500;
		winInfo.width = 500;

	#if defined(OS_WIN)
		winInfo.style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP;
		winInfo.SetAsPopup(NULL, "Webkit Inspector");
	#endif

		const char* name = "WebkitInspector";
		cef_string_utf8_to_utf16(name, strlen(name), &winInfo.window_name);

		CefString devUrl = m_pBrowser->GetHost()->GetDevToolsURL(true);
		m_Inspector = CefBrowserHost::CreateBrowserSync(winInfo, CefRefPtr<CefClient>(), devUrl, getBrowserDefaults(NULL), CefRefPtr<CefRequestContext>());
#else
		CefWindowInfo windowInfo;
		CefBrowserSettings settings;

#if defined(OS_WIN)
		windowInfo.SetAsPopup(NULL, "DevTools");
#endif

		CefRefPtr<CefClient> client = new DevToolsClient();
		m_pBrowser->GetHost()->ShowDevTools(windowInfo, client, settings);
#endif
	}
}

void ChromiumBrowser::hideInspector()
{
	if (!CefCurrentlyOn(TID_UI)) {
		CefPostTask(TID_UI, new BrowserCallback(this, &ChromiumBrowser::hideInspector));
	}
	else
	{
#ifdef CEF_NEW_DEVTOOL_API
		if (!m_Inspector)
			return;

		m_Inspector->GetHost()->CloseBrowser(true);
		m_Inspector = CefRefPtr<CefBrowser>();
#else
		m_pBrowser->GetHost()->CloseDevTools();
#endif
	}
}

void ChromiumBrowser::inspectElement(int x, int y)
{
	showInspector();
}

void ChromiumBrowser::scroll(int x, int y, int deltaX, int deltaY)
{
	CefMouseEvent e;

	e.x = x;
	e.y = y;

	if (m_pBrowser && m_pBrowser->GetHost())
		m_pBrowser->GetHost()->SendMouseWheelEvent(e, deltaX, deltaY);
}

int* ChromiumBrowser::getBrowserHandle()
{
	if (m_pBrowser)
		return (int*)m_pBrowser->GetHost()->GetWindowHandle();

	return 0;
}

ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptContextI> ChromiumBrowser::getJSContext()
{
	if (m_pBrowser)
		return new JavaScriptContext(m_pBrowser->GetIdentifier());

	return NULL;
}

void ChromiumBrowser::setContext(const CefRefPtr<CefV8Context>& context)
{
}




ChromiumRenderer::ChromiumRenderer(WIN_HANDLE handle, const char* defaultUrl, int width, int height, const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>& defaults)
	: ChromiumBrowser(handle)
	, m_nDefaultWidth(width)
	, m_nDefaultHeight(height)
{
	init(defaultUrl, true, width, height, defaults);
}

void ChromiumRenderer::invalidateSize()
{
	if (m_pBrowser)
		m_pBrowser->GetHost()->WasResized();
}

void ChromiumRenderer::onMouseClick(int x, int y, ChromiumDLL::MouseButtonType type, bool mouseUp, int clickCount)
{
	if (m_pBrowser)
	{
		CefMouseEvent event;
		event.x = x;
		event.y = y;
		m_pBrowser->GetHost()->SendMouseClickEvent(event, (CefBrowserHost::MouseButtonType)type,
												   mouseUp, clickCount);
	}
}

void ChromiumRenderer::onMouseMove(int x, int y, bool mouseLeave)
{
	if (m_pBrowser)
	{
		CefMouseEvent event;
		event.x = x;
		event.y = y;
		m_pBrowser->GetHost()->SendMouseMoveEvent(event, mouseLeave);
	}
}

void ChromiumRenderer::onKeyPress(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumKeyPressI>& pKeyPress)
{
	if (!pKeyPress)
		return;

	if (m_pBrowser)
	{
		CefKeyEvent event;
		event.type = (cef_key_event_type_t)pKeyPress->getType();
		event.modifiers = pKeyPress->getModifiers();
		event.native_key_code = pKeyPress->getNativeCode();
		event.character = pKeyPress->getCharacter();
		event.unmodified_character = pKeyPress->getUnModCharacter();

#ifdef WIN32
		event.windows_key_code = pKeyPress->getWinKeyCode();
		event.is_system_key = pKeyPress->isSystemKey();
#endif
		m_pBrowser->GetHost()->SendKeyEvent(event);
	}
}

void ChromiumRenderer::onFocus(bool setFocus)
{
	if (m_pBrowser)
		m_pBrowser->GetHost()->SendFocusEvent(setFocus);
}

void ChromiumRenderer::onCaptureLost()
{
	if (m_pBrowser)
		m_pBrowser->GetHost()->SendCaptureLostEvent();
}

void ChromiumRenderer::setBrowser(const CefRefPtr<CefBrowser>& browser)
{
	ChromiumBrowser::setBrowser(browser);
	invalidateSize();
}

void ChromiumRenderer::setEventCallback(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererEventI>& cbeI)
{
	ChromiumBrowserEvents* cbe = (ChromiumBrowserEvents*)m_rEventHandler.get();

	if (cbe)
		cbe->setCallBack(cbeI);
}

void ChromiumRenderer::setEventCallback(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererPopupEventI>& cbeI)
{
	ChromiumBrowserEvents* cbe = (ChromiumBrowserEvents*)m_rEventHandler.get();

	if (cbe)
		cbe->setCallBack(cbeI);
}

