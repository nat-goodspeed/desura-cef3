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

#ifndef DESURA_MENUINFO_H
#define DESURA_MENUINFO_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "include/cef_context_menu_handler.h"

#ifdef OS_WIN
	typedef HWND MenuHandle_t;
#else
	typedef void* MenuHandle_t;
#endif

class ChromiumMenuInfo : public ChromiumDLL::ChromiumMenuInfoI
{
public:
	ChromiumMenuInfo(CefRefPtr<CefContextMenuParams> &params, MenuHandle_t hwnd);

	void setMouseOffset(int x, int y)
	{
		m_XOffset = x;
		m_YOffset = y;
	}

	virtual ChromiumDLL::ChromiumMenuInfoI::TypeFlags getTypeFlags();
	virtual ChromiumDLL::ChromiumMenuInfoI::EditFlags getEditFlags();

	virtual void getMousePos(int* x, int* y);

	virtual const char* getLinkUrl();
	virtual const char* getImageUrl();
	virtual const char* getPageUrl();
	virtual const char* getFrameUrl();
	virtual const char* getSelectionText();
	virtual const char* getMisSpelledWord();
	virtual const char* getSecurityInfo();

	virtual int getCustomCount();
	virtual ChromiumDLL::ChromiumMenuItemI* getCustomItem(size_t index);

	virtual int* getHWND();

private:
	CefRefPtr<CefContextMenuParams> &m_MenuInfo;
	MenuHandle_t m_Hwnd;

	int m_XOffset;
	int m_YOffset;

	CefStringUTF8 m_ImgUrl;
	CefStringUTF8 m_LinkUrl;
	CefStringUTF8 m_SelText;
	CefStringUTF8 m_PageUrl;
	CefStringUTF8 m_FrameUrl;
};


#endif //DESURA_MENUINFO_H
