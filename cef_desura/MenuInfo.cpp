///////////// Copyright 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : MenuInfo.cpp
//   Description :
//      [TODO: Write the purpose of MenuInfo.cpp.]
//
//   Created On: 9/18/2010 10:01:52 AM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

#include "MenuInfo.h"


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
		m_LinkUrl = m_MenuInfo->GetLinkUrl().c_str();

	return m_LinkUrl.c_str();
}

const char* ChromiumMenuInfo::getImageUrl()
{
	if (m_ImgUrl.empty())
		m_ImgUrl = m_MenuInfo->GetSourceUrl().c_str();

	return m_ImgUrl.c_str();
}

const char* ChromiumMenuInfo::getPageUrl()
{
	if (m_PageUrl.empty())
		m_PageUrl = m_MenuInfo->GetPageUrl().c_str();

	return m_PageUrl.c_str();
}

const char* ChromiumMenuInfo::getFrameUrl()
{
	if (m_FrameUrl.empty())
		m_FrameUrl = m_MenuInfo->GetFrameUrl().c_str();

	return m_FrameUrl.c_str();
}

const char* ChromiumMenuInfo::getSelectionText()
{
	if (m_SelText.empty())
		m_SelText = m_MenuInfo->GetSelectionText().c_str();

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

