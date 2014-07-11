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

#include <Windows.h>
#include "ChromiumBrowserI.h"
#include "ChromiumRefCount.h"
#include "SharedObjectLoader.h"
#include <iostream>

#include "ChromiumCallback.h"

#include "ll_jsbridge_test.h"

TCHAR* szWindowClass = "CefDesuraTestWinClass";  // the main window class name

HINSTANCE hInst;   // current instance
ChromiumDLL::ChromiumControllerI* g_ChromiumController = NULL;
ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserI> g_Browser;
HWND m_MainWinHwnd;

class JSFunctArgsWin : public ChromiumDLL::ChromiumRefCount<ChromiumDLL::JavaScriptFunctionArgs>
{
public:
	JSFunctArgsWin(ChromiumDLL::JSObjHandle o, ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptContextI> c, const ChromiumDLL::JSObjHandle& ret)
		: m_Ret(ret)
	{
		argc = 1;
		argv = &m_Ret;
		context = c;
		factory = context->getFactory();
		object = o;
		function = "";
	}

	ChromiumDLL::JSObjHandle m_Ret;
};


class EventCallback : public ChromiumDLL::ChromiumRefCount<ChromiumDLL::ChromiumBrowserEventI>
{
public:
	virtual bool onNavigateUrl(const char* url, bool isMain)
	{
		std::cout << "onNavigateUrl: " << url << std::endl;
		SetWindowText(m_MainWinHwnd, url);
		return true;
	}

	virtual void onPageLoadStart()
	{
		std::cout << "onPageLoadStart" << std::endl;
	}

	virtual void onPageLoadEnd()
	{
		std::cout << "onPageLoadEnd" << std::endl;
	}

	virtual bool onJScriptAlert(const char* msg)
	{
		return false;
	}

	virtual bool onJScriptConfirm(const char* msg, bool* result)
	{
		return false;
	}

	virtual bool onJScriptPrompt(const char* msg, const char* defualtVal, bool* handled, char result[255])
	{
		return false;
	}

	virtual bool onKeyEvent(ChromiumDLL::KeyEventType type, int code, int modifiers, bool isSystemKey)
	{
		if (type == ChromiumDLL::KEYEVENT_RAWKEYDOWN && code == 'J')
		{
			g_ChromiumController->PostcallbackT(ChromiumDLL::TID_UI, [](){

				auto context = g_Browser->getJSContext();
				context->enter();

				auto go = context->getGlobalObject();
				auto callback = go->getValue("callback");

				callback->executeFunction(new JSFunctArgsWin(go, context, context->getFactory()->CreateString("J key pressed")));

				context->exit();
			});
		}
		else if (type == ChromiumDLL::KEYEVENT_RAWKEYDOWN && code == 'I')
		{
			g_Browser->showInspector();
		}

		return false;
	}

	virtual void onLogConsoleMsg(const char* message, const char* source, int line)
	{
	}

	virtual void launchLink(const char* url)
	{
	}

	virtual bool onLoadError(const char* errorMsg, const char* url, char* buff, size_t size)
	{
		return false;
	}

	virtual bool handlePopupMenu(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumMenuInfoI>& menuInfo)
	{
		return false;
	}
};

ChromiumDLL::RefPtr<EventCallback> g_EventCallback = new EventCallback();

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) 
	{
	case WM_CREATE: 
		std::cout << "WM_CREATE" << std::endl;
		g_Browser = g_ChromiumController->NewChromiumBrowser((int*)hWnd, "", "jsbridge://run");
		g_Browser->setEventCallback(g_EventCallback);
		return 0;

	case WM_PAINT:
		std::cout << "WM_PAINT" << std::endl;
		g_Browser->onPaint();
		break;

	case WM_SIZE:
		std::cout << "WM_SIZE" << std::endl;
		g_Browser->onResize();
		break;

	case WM_ERASEBKGND:
		g_Browser->onPaintBg();
		break;

	case WM_SETFOCUS:
		g_Browser->onFocus();
		break;

	case WM_CLOSE:
		std::cout << "WM_CLOSE" << std::endl;
		if (g_Browser) 
			g_Browser.reset();

		// Allow the close.
		break;

	case WM_DESTROY:
		g_ChromiumController->Stop();
		return 0;
	};

	return DefWindowProc(hWnd, message, wParam, lParam);
}


LRESULT CALLBACK MessageWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) 
{
	hInst = hInstance;  // Store instance handle in our global variable
	m_MainWinHwnd = CreateWindow(szWindowClass, "Cef Desura Test", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!m_MainWinHwnd)
		return FALSE;

	ShowWindow(m_MainWinHwnd, nCmdShow);
	UpdateWindow(m_MainWinHwnd);

	return TRUE;
}

ATOM MyRegisterClass(HINSTANCE hInstance) 
{
	WNDCLASSEX wcex;

	memset(&wcex, 0, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = szWindowClass;

	return RegisterClassEx(&wcex);
}


HWND CreateMessageWindow(HINSTANCE hInstance) 
{
	static const char kWndClass[] = "ClientMessageWindow";

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = MessageWndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = kWndClass;
	RegisterClassEx(&wc);

	return CreateWindow(kWndClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hInstance, 0);
}

typedef ChromiumDLL::ChromiumControllerI* (*CEF_InitFn)(bool, const char*, const char*, const char*);

extern ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI> NewExternalLoaderScheme();
extern ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI> NewDesuraJSExtender();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;

#ifdef DEBUG
	AllocConsole();
#endif

	SharedObjectLoader sol;

	if (!sol.load("3p_cef3.dll"))
		return -1;

	CEF_InitFn CEF_Init = sol.getFunction<CEF_InitFn>("CEF_InitEx");

	if (!CEF_Init)
		return -2;

	g_ChromiumController = CEF_Init(false, "cache", "log", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/13.0.782.220 Safari/535.1 Desura/500.123 (Windows 8.1 x64)");

	if (!g_ChromiumController)
		return -3;

	g_ChromiumController->RegisterSchemeExtender(NewExternalLoaderScheme());
	g_ChromiumController->RegisterJSExtender(NewDesuraJSExtender());

	g_ChromiumController->RegisterSchemeExtender(new JSBridgeTestScheme());
	g_ChromiumController->RegisterJSExtender(new JSBridgeTestExtender(g_ChromiumController));


	MyRegisterClass(hInstance);

	// Perform application initialization
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	g_ChromiumController->RunMsgLoop();
	return 0;
}