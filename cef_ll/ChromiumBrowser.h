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

#ifndef THIRDPARTY_CEF3_CHROMIUMBROWSER_H
#define THIRDPARTY_CEF3_CHROMIUMBROWSER_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
//#include "include/cef.h"
#include "RefCount.h"
#include "include/cef_browser.h"
#include "include/cef_task.h"
#include <string>

CefStringUTF8 ConvertToUtf8(const CefString& str);

class ChromiumBrowserEvents;


#ifdef OS_WIN
	typedef HWND WIN_HANDLE;
#else
	typedef void* WIN_HANDLE;
#endif

class ChromiumBrowser : public ChromiumDLL::ChromiumBrowserI
{
public:
	ChromiumBrowser(WIN_HANDLE handle, const char* defaultUrl, const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>& defaults);
	ChromiumBrowser(WIN_HANDLE handle);

	virtual ~ChromiumBrowser();

	void init(const char *defaultUrl, bool offScreen = false, int width = -1, int height = -1, const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>& defaults = NULL);
	virtual void onFocus();

#ifdef OS_WIN
	virtual void onPaintBg();
	virtual void onPaint();
	virtual void onResize();
#else
	void initCallback(const std::string& defaultUrl);
	virtual void onResize(int x, int y, int width, int height);
#endif


	virtual void loadUrl(const char* url);
	virtual void loadString(const char* string);

	virtual void stop();
	virtual void refresh(bool ignoreCache = false);
	virtual void back();
	virtual void forward();

	virtual void zoomIn();
	virtual void zoomOut();
	virtual void zoomNormal();

	virtual void print();
	virtual void viewSource();

	virtual void undo();
	virtual void redo();
	virtual void cut();
	virtual void copy();
	virtual void paste();
	virtual void del();
	virtual void selectall();

	virtual void setEventCallback(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserEventI>& cbe);
	virtual void executeJScript(const char* code, const char* scripturl = 0, int startline = 0);

	virtual void showInspector();
	virtual void hideInspector();
	virtual void inspectElement(int x, int y);

	virtual void scroll(int x, int y, int deltaX, int deltaY);

	virtual int* getBrowserHandle();

	virtual ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptContextI> getJSContext();


	virtual void setBrowser(const CefRefPtr<CefBrowser>& browser);
	void setContext(const CefRefPtr<CefV8Context>& context);

protected:
	CefBrowserSettings getBrowserDefaults(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>& defaults);

	CefRefPtr<CefV8Context> m_rContext;
	CefRefPtr<CefClient> m_rEventHandler;
	CefRefPtr<CefBrowser> m_pBrowser;

	CefRefPtr<CefBrowser> m_Inspector;


	WIN_HANDLE m_hFormHandle;
	std::string m_szBuffer;
	int m_iLastTask;

	CEF3_IMPLEMENTREF_COUNTING(ChromiumBrowser);
};


class ChromiumRenderer : public ChromiumBrowser, public ChromiumDLL::ChromiumRendererI
{
public:
	ChromiumRenderer(WIN_HANDLE handle, const char* defaultUrl, int width, int height, const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserDefaultsI>& defaults);

	virtual void invalidateSize();

	virtual void onMouseClick(int x, int y, ChromiumDLL::MouseButtonType type, bool mouseUp, int clickCount);
	virtual void onMouseMove(int x, int y, bool mouseLeave);
	virtual void onKeyPress(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumKeyPressI>& pKeyPress);

	virtual void onFocus(bool setFocus);
	virtual void onCaptureLost();

	virtual ChromiumDLL::RefPtr<ChromiumBrowserI> getBrowser()
	{
		return this;
	}

	virtual void setBrowser(const CefRefPtr<CefBrowser>& browser);
	virtual void setEventCallback(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererEventI>& cbe);
	virtual void setEventCallback(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererPopupEventI>& cbe);

	void addRef()
	{
		ChromiumBrowser::addRef();
	}

	void delRef()
	{
		ChromiumBrowser::delRef();
	}

	void destroy()
	{
		ChromiumBrowser::destroy();
	}

private:
	int m_nDefaultWidth;
	int m_nDefaultHeight;
};


class TaskWrapper : public CefTask
{
public:
	TaskWrapper(const ChromiumDLL::RefPtr<ChromiumDLL::CallbackI>& callback)
		: m_pCallback(callback)
	{
	}

	virtual void Execute()
	{
		if (m_pCallback)
			m_pCallback->run();
	}

	IMPLEMENT_REFCOUNTING(TaskWrapper)

private:
	ChromiumDLL::RefPtr<ChromiumDLL::CallbackI> m_pCallback;
};




#endif //THIRDPARTY_CEF3_CHROMIUMBROWSER_H





