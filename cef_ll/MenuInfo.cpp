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

#include "MenuInfo.h"

extern CefStringUTF8 ConvertToUtf8(const CefString& str);

ChromiumMenuInfo::ChromiumMenuInfo(CefRefPtr<CefContextMenuParams> &params, MenuHandle_t hwnd)
	: m_MenuInfo(params)
	, m_Hwnd(hwnd)
	, m_XOffset(0)
	, m_YOffset(0)
{
}

ChromiumDLL::ChromiumMenuInfoI::TypeFlags ChromiumMenuInfo::getTypeFlags()
{
	switch (m_MenuInfo->GetTypeFlags())
	{
	default:
	case CM_TYPEFLAG_NONE:
		return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_NONE;

	case CM_TYPEFLAG_PAGE:
		return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_PAGE;

	case CM_TYPEFLAG_FRAME:
		return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_FRAME;

	case CM_TYPEFLAG_LINK:
		return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_LINK;

	case CM_TYPEFLAG_MEDIA:
	{
		switch (m_MenuInfo->GetMediaType())
		{
		case CM_MEDIATYPE_IMAGE:
			return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_SELECTION;

		case CM_MEDIATYPE_VIDEO:
			return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_VIDEO;

		case CM_MEDIATYPE_AUDIO:
			return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_AUDIO;

		case CM_MEDIATYPE_FILE:
			return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_FILE;

		case CM_MEDIATYPE_PLUGIN:
			return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_PLUGIN;

		case CM_MEDIATYPE_NONE:
			return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_NONE;
		};
	}

	case CM_TYPEFLAG_SELECTION:
		return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_SELECTION;

	case CM_TYPEFLAG_EDITABLE:
		return ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_EDITABLE;
	};
}

ChromiumDLL::ChromiumMenuInfoI::EditFlags ChromiumMenuInfo::getEditFlags()
{
	return (ChromiumDLL::ChromiumMenuInfoI::EditFlags)m_MenuInfo->GetEditStateFlags();
}

void ChromiumMenuInfo::getMousePos(int* x, int* y)
{
	*x = m_MenuInfo->GetXCoord() + m_XOffset;
	*y = m_MenuInfo->GetYCoord() + m_YOffset;
}

const char* ChromiumMenuInfo::getLinkUrl()
{
	if (m_LinkUrl.empty())
		m_LinkUrl = ConvertToUtf8(m_MenuInfo->GetLinkUrl());
		

	return m_LinkUrl.c_str();
}

const char* ChromiumMenuInfo::getImageUrl()
{
	if (m_ImgUrl.empty())
		m_ImgUrl = ConvertToUtf8(m_MenuInfo->GetSourceUrl());

	return m_ImgUrl.c_str();
}

const char* ChromiumMenuInfo::getPageUrl()
{
	if (m_PageUrl.empty())
		m_PageUrl = ConvertToUtf8(m_MenuInfo->GetPageUrl());

	return m_PageUrl.c_str();
}

const char* ChromiumMenuInfo::getFrameUrl()
{
	if (m_FrameUrl.empty())
		m_FrameUrl = ConvertToUtf8(m_MenuInfo->GetFrameUrl());

	return m_FrameUrl.c_str();
}

const char* ChromiumMenuInfo::getSelectionText()
{
	if (m_SelText.empty())
		m_SelText = ConvertToUtf8(m_MenuInfo->GetSelectionText());

	return m_SelText.c_str();
}

const char* ChromiumMenuInfo::getMisSpelledWord()
{
	return "";
}

const char* ChromiumMenuInfo::getSecurityInfo()
{
	return "";
}

int* ChromiumMenuInfo::getHWND()
{
	return (int*)m_Hwnd;
}

int ChromiumMenuInfo::getCustomCount()
{
	return 0;
}

ChromiumDLL::ChromiumMenuItemI* ChromiumMenuInfo::getCustomItem(size_t index)
{
	return NULL;
}

