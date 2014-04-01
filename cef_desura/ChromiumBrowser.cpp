///////////// Copyright 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : minimal
//   File        : ChromiumBrowser.cpp
//   Description :
//      [TODO: Write the purpose of ChromiumBrowser.cpp.]
//
//   Created On: 5/25/2010 5:40:26 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
#include "windows.h"
#endif

#include "ChromiumBrowser.h"
//#include "include/cef.h"
#include "include/cef_app.h"
#include "JavaScriptExtender.h"
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

int g_nApiVersion = 1;

ChromiumDLL::LogMessageHandlerFn g_pLogHandler = NULL;

bool logHandler(int level, const std::string& msg)
{
	if (g_pLogHandler)
		return g_pLogHandler(level, msg.c_str());

	return false;
}

#ifdef OS_LINUX
static void gtkFocus(GtkWidget *widget, GdkEvent *event, ChromiumBrowser *data)
{
	if (data)
		data->onFocus();
}
#endif

extern void CEF_DeleteCookie_Internal(const char* url, const char* name);
extern ChromiumDLL::CookieI* CEF_CreateCookie_Internal();
extern void CEF_SetCookie_Internal(const char* url, ChromiumDLL::CookieI* cookie);

class ChromiumController : public ChromiumDLL::ChromiumControllerI
{
public:
	ChromiumController()
		: m_bInit(false)
	{
	}

	virtual int GetMaxApiVersion()
	{
		return 2;
	}

	virtual void SetApiVersion(int version)
	{
		if (version <= 0)
			g_nApiVersion = 1;
		else
			g_nApiVersion = version;
	}

	virtual void DoMsgLoop()
	{
		CefDoMessageLoopWork();
	}

	virtual void Stop()
	{
		CefShutdown();
	}

	virtual bool RegisterJSExtender(ChromiumDLL::JavaScriptExtenderI* extender)
	{
		return false;
		//return JavaScriptExtender::Register(extender);
	}

	virtual bool RegisterSchemeExtender(ChromiumDLL::SchemeExtenderI* extender)
	{
		return false;
		//return SchemeExtender::Register(extender);
	}

	virtual ChromiumDLL::ChromiumBrowserI* NewChromiumBrowser(int* formHandle, const char* name, const char* defaultUrl)
	{
		return new ChromiumBrowser((WIN_HANDLE)formHandle, defaultUrl);
	}

	virtual ChromiumDLL::ChromiumRendererI* NewChromiumRenderer(int* formHandle, const char* defaultUrl, int width, int height)
	{
		return new ChromiumRenderer((WIN_HANDLE)formHandle, defaultUrl, width, height);
	}
	
	virtual void SetLogHandler(ChromiumDLL::LogMessageHandlerFn logFn)
	{
		g_pLogHandler = logFn;
	}

	virtual void PostCallback(ChromiumDLL::CallbackI* callback)
	{
		CefPostTask(TID_UI, CefRefPtr<CefTask>(new TaskWrapper(callback)));
	}

	virtual void PostCallbackEx(ChromiumDLL::ThreadID thread, ChromiumDLL::CallbackI* callback)
	{
		CefPostTask((CefThreadId)thread, CefRefPtr<CefTask>(new TaskWrapper(callback)));
	}

	virtual void DeleteCookie(const char* url, const char* name)
	{
		CEF_DeleteCookie_Internal(url, name);
	}

	virtual ChromiumDLL::CookieI* CreateCookie()
	{
		return CEF_CreateCookie_Internal();
	}

	virtual void SetCookie(const char* url, ChromiumDLL::CookieI* cookie)
	{
		CEF_SetCookie_Internal(url, cookie);
	}


	bool Init(bool threaded, const char* cachePath, const char* logPath, const char* userAgent)
	{
		if (m_bInit)
			return true;

		CefSettings settings;
		CefMainArgs args;
		CefRefPtr<CefApp> app;

		// It turns out to be unreasonably difficult to discover an internal
		// cross-platform API to obtain the pathname of the current executable.
		// Otherwise I would concatenate its directory name with this filename.
		static const char cefclient[] = "cefclient";
		cef_string_utf8_to_utf16(cefclient, strlen(cefclient), &settings.browser_subprocess_path);

		cef_string_utf8_to_utf16(cachePath, strlen(cachePath), &settings.cache_path);
		cef_string_utf8_to_utf16(userAgent, strlen(userAgent), &settings.user_agent);

		settings.multi_threaded_message_loop = threaded;
		settings.remote_debugging_port = 2323;

#ifdef _DEBUG
		settings.single_process = true;
#endif

		if (!CefInitialize(args, settings, app, NULL))
			return false;

#if defined(_WIN32)
		CefAddWebPluginPath("gcswf32.dll");
#else
		CefAddWebPluginPath("libdesura_flashwrapper.so");
#endif

		m_bInit = true;
		return true;
	}

	bool m_bInit;
};

ChromiumController* g_Controller = new ChromiumController();

extern "C"
{
	DLLINTERFACE ChromiumDLL::ChromiumControllerI* CEF_InitEx(bool threaded, const char* cachePath, const char* logPath, const char* userAgent)
	{
		if (g_Controller->Init(threaded, cachePath, logPath, userAgent))
			return g_Controller;

		return NULL;
	}
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

ChromiumBrowser::ChromiumBrowser(WIN_HANDLE handle, const char* defaultUrl)
{
	m_iLastTask = 0;
	m_hFormHandle = handle;
	m_pBrowser = NULL;

	m_rEventHandler = (CefClient*)new ChromiumBrowserEvents(this);
	init(defaultUrl, false);
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

CefBrowserSettings ChromiumBrowser::getBrowserDefaults()
{
	CefBrowserSettings browserDefaults;

//	browserDefaults.developer_tools_disabled = false;
	browserDefaults.webgl = STATE_DISABLED;
	browserDefaults.universal_access_from_file_urls = STATE_ENABLED;
	browserDefaults.file_access_from_file_urls = STATE_ENABLED;
	browserDefaults.java = STATE_DISABLED;
	browserDefaults.javascript_close_windows = STATE_DISABLED;
	browserDefaults.javascript_open_windows = STATE_DISABLED;
//	browserDefaults.drag_drop = STATE_DISABLED;

	browserDefaults.accelerated_compositing = STATE_DISABLED;

	return browserDefaults;
}

#ifdef OS_WIN
void ChromiumBrowser::init(const char *defaultUrl, bool offScreen, int width, int height)
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

	CefBrowserHost::CreateBrowser(winInfo, m_rEventHandler, defaultUrl, getBrowserDefaults(), CefRefPtr<CefRequestContext>());
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



void ChromiumBrowser::setEventCallback(ChromiumDLL::ChromiumBrowserEventI* cbeI)
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
	if (m_pBrowser)
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

void ChromiumBrowser::setBrowser(CefBrowser* browser)
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

template <typename T>
CefRefPtr<CefTask> NewCallbackT(T t)
{
	return CefRefPtr<CefTask>(new TaskWrapper(new ChromiumDLL::CallbackT<T>(t)));
}

void ChromiumBrowser::showInspector()
{
	CefPostTask(TID_UI, NewCallbackT([&](){

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
		cef_string_copy(name, strlen(name), &winInfo.window_name);

		CefString devUrl = m_pBrowser->GetHost()->GetDevToolsURL(true);
		m_Inspector = CefBrowserHost::CreateBrowserSync(winInfo, CefRefPtr<CefClient>(), devUrl, getBrowserDefaults(), CefRefPtr<CefRequestContext>());
#else
		CefWindowInfo info;
		m_pBrowser->GetHost()->ShowDevTools(info, CefRefPtr<CefClient>(), getBrowserDefaults());
#endif
	}));
}

void ChromiumBrowser::hideInspector()
{
	CefPostTask(TID_UI, NewCallbackT([&](){

#ifdef CEF_NEW_DEVTOOL_API
		if (!m_Inspector)
			return;

		m_Inspector->GetHost()->CloseBrowser(true);
		m_Inspector = CefRefPtr<CefBrowser>();
#else
		m_pBrowser->GetHost()->CloseDevTools();
#endif
	}));
}

void ChromiumBrowser::inspectElement(int x, int y)
{
	showInspector();
}

void ChromiumBrowser::scroll(int x, int y, int delta, unsigned int flags)
{
	CefMouseEvent e;

	e.x = x;
	e.y = y;

	if (m_pBrowser && m_pBrowser->GetHost())
		m_pBrowser->GetHost()->SendMouseWheelEvent(e, delta, 0);
}

int* ChromiumBrowser::getBrowserHandle()
{
	if (m_pBrowser)
		return (int*)m_pBrowser->GetHost()->GetWindowHandle();

	return 0;
}

ChromiumDLL::JavaScriptContextI* ChromiumBrowser::getJSContext()
{
	if (m_pBrowser)
		return new JavaScriptContext(m_rContext);

	return NULL;
}

void ChromiumBrowser::setContext(CefRefPtr<CefV8Context> context)
{
	m_rContext = context;
}




ChromiumRenderer::ChromiumRenderer(WIN_HANDLE handle, const char* defaultUrl, int width, int height)
	: ChromiumBrowser(handle)
	, m_nDefaultWidth(width)
	, m_nDefaultHeight(height)
{
	init(defaultUrl, true, width, height);
}

void ChromiumRenderer::setWindowSize(int width, int height)
{
/*==========================================================================*|
	// nat: no such method
	if (m_pBrowser)
		m_pBrowser->SetSize(PET_VIEW, width, height);
|*==========================================================================*/
}

void ChromiumRenderer::getWindowSize(int &width, int &height)
{
/*==========================================================================*|
	// nat: no such method
	if (m_pBrowser)
		m_pBrowser->GetSize(PET_VIEW, width, height);
|*==========================================================================*/
}

void ChromiumRenderer::renderRectToBuffer(void *pBuffer, unsigned int x, unsigned int y, unsigned int w, unsigned h)
{

}

void ChromiumRenderer::renderToBuffer(void* pBuffer, unsigned int width, unsigned int height)
{
/*==========================================================================*|
	// nat: no such method
	if (m_pBrowser)
		m_pBrowser->GetImage(PET_VIEW, width, height, pBuffer);
|*==========================================================================*/
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

// cef_key_event_type_t is NOT THE SAME as ChromiumDLL::KeyType
class KeyTypeFinder
{
public:
	KeyTypeFinder()
	{
		mMap[ChromiumDLL::KT_KEYUP]   = KEYEVENT_KEYUP;
		mMap[ChromiumDLL::KT_KEYDOWN] = KEYEVENT_KEYDOWN;
		mMap[ChromiumDLL::KT_CHAR]    = KEYEVENT_CHAR;
	}

	cef_key_event_type_t find(ChromiumDLL::KeyType type) const
	{
		KeyTypeMap::const_iterator found = mMap.find(type);
		if (found != mMap.end())
			return found->second;

		return cef_key_event_type_t(); // invalid
	}

private:
	typedef std::map<ChromiumDLL::KeyType, cef_key_event_type_t> KeyTypeMap;
	KeyTypeMap mMap;
};
static const KeyTypeFinder keyfinder;

void ChromiumRenderer::onKeyPress(ChromiumDLL::KeyType type, int key, int modifiers, bool sysChar, bool imeChar)
{
	if (m_pBrowser)
	{
		CefKeyEvent event;
		event.type = keyfinder.find(type);
		// BUG: we probably also need to translate modifier bits, but I find
		// no definition for the ChromiumDLL modifier bits.
		event.modifiers = modifiers;
		// BUG: no idea what to set for windows_key_code
		event.native_key_code = key;
		event.is_system_key = sysChar;
		// BUG: no idea how to set character or unmodified_character
		// BUG: no idea what to do about imeChar
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

void ChromiumRenderer::setBrowser(CefBrowser* browser)
{
	ChromiumBrowser::setBrowser(browser);
	setWindowSize(m_nDefaultWidth, m_nDefaultHeight);
}

void ChromiumRenderer::setEventCallback(ChromiumDLL::ChromiumRendererEventI* cbeI)
{
	ChromiumBrowserEvents* cbe = (ChromiumBrowserEvents*)m_rEventHandler.get();

	if (cbe)
		cbe->setCallBack(cbeI);
}
