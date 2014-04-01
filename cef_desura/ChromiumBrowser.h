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

#ifndef DESURA_CHROMIUMBROWSER_H
#define DESURA_CHROMIUMBROWSER_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
//#include "include/cef.h"
#include "include/cef_browser.h"
#include "include/cef_task.h"
#include <string>

class ChromiumBrowserEvents;


#ifdef OS_WIN
	typedef HWND WIN_HANDLE;
#else
	typedef void* WIN_HANDLE;
#endif

class ChromiumBrowser : public ChromiumDLL::ChromiumBrowserI
{
public:
	ChromiumBrowser(WIN_HANDLE handle, const char* defaultUrl);
	ChromiumBrowser(WIN_HANDLE handle);

	virtual ~ChromiumBrowser();

	void init(const char *defaultUrl, bool offScreen = false, int width = -1, int height = -1);
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

	virtual void setEventCallback(ChromiumDLL::ChromiumBrowserEventI* cbe);
	virtual void executeJScript(const char* code, const char* scripturl = 0, int startline = 0);

	virtual void showInspector();
	virtual void hideInspector();
	virtual void inspectElement(int x, int y);

	virtual void scroll(int x, int y, int delta, unsigned int flags);

	virtual int* getBrowserHandle();

	virtual void destroy()
	{
		delete this;
	}

	virtual ChromiumDLL::JavaScriptContextI* getJSContext();




	virtual void setBrowser(CefBrowser* browser);
	
	void setContext(CefRefPtr<CefV8Context> context);

protected:
	CefBrowserSettings getBrowserDefaults();

	CefRefPtr<CefV8Context> m_rContext;
	CefRefPtr<CefClient> m_rEventHandler;
	CefBrowser* m_pBrowser;

	CefRefPtr<CefBrowser> m_Inspector;


	WIN_HANDLE m_hFormHandle;
	std::string m_szBuffer;
	int m_iLastTask;
};


class ChromiumRenderer : public ChromiumBrowser, public ChromiumDLL::ChromiumRendererI
{
public:
	ChromiumRenderer(WIN_HANDLE handle, const char* defaultUrl, int width, int height);

	virtual void setWindowSize(int width, int height);
	virtual void getWindowSize(int &width, int &height);

	virtual void renderRectToBuffer(void *pBuffer, unsigned int x, unsigned int y, unsigned int w, unsigned h);
	virtual void renderToBuffer(void* pBuffer, unsigned int width, unsigned int height);

	virtual void onMouseClick(int x, int y, ChromiumDLL::MouseButtonType type, bool mouseUp, int clickCount);
	virtual void onMouseMove(int x, int y, bool mouseLeave);
	virtual void onKeyPress(ChromiumDLL::KeyType type, int key, int modifiers, bool sysChar, bool imeChar);

	virtual void onFocus(bool setFocus);
	virtual void onCaptureLost();

	virtual ChromiumBrowserI* getBrowser()
	{
		return this;
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void setBrowser(CefBrowser* browser);
	virtual void setEventCallback(ChromiumDLL::ChromiumRendererEventI* cbe);

private:
	int m_nDefaultWidth;
	int m_nDefaultHeight;
};


class TaskWrapper : public CefTask
{
public:
	TaskWrapper(ChromiumDLL::CallbackI* callback)
	{
		m_pCallback = callback;
	}

	~TaskWrapper()
	{
		if (m_pCallback)
			m_pCallback->destroy();
	}

	virtual void Execute()
	{
		if (m_pCallback)
			m_pCallback->run();
	}

	IMPLEMENT_REFCOUNTING(TaskWrapper)

private:
	ChromiumDLL::CallbackI* m_pCallback;
};




#endif //DESURA_CHROMIUMBROWSER_H





