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


#ifndef THIRDPARTY_CEF3_WIN32KEYPRESS_HEADER
#define THIRDPARTY_CEF3_WIN32KEYPRESS_HEADER
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "ChromiumRefCount.h"

namespace ChromiumDLL
{

#ifdef WIN32
	class Win32ChromiumKeyPress : public ChromiumRefCount<ChromiumKeyPressI>
	{
	public:
		Win32ChromiumKeyPress(int message, WPARAM wParam, LPARAM lParam)
			: m_Message(message)
			, m_wParam(wParam)
			, m_lParam(lParam)
		{
		}

		KeyType getType() override
		{
			if (m_Message == WM_KEYDOWN || m_Message == WM_SYSKEYDOWN)
				return KT_RAWKEYDOWN;
			else if (m_Message == WM_KEYUP || m_Message == WM_SYSKEYUP)
				return KT_KEYUP;
			else
				return KT_CHAR;
		}

		int getNativeCode()
		{
			return m_lParam;
		}

		int getCharacter()
		{
			return 0;
		}

		int getUnModCharacter()
		{
			return 0;
		}

		KeyModifiers getModifiers()
		{
			int modifiers = KM_NONE;

			if (isKeyDown(VK_SHIFT))
				modifiers |= KM_SHIFT_DOWN;
			if (isKeyDown(VK_CONTROL))
				modifiers |= KM_CONTROL_DOWN;
			if (isKeyDown(VK_MENU))
				modifiers |= KM_ALT_DOWN;

			// Low bit set from GetKeyState indicates "toggled".
			if (::GetKeyState(VK_NUMLOCK) & 1)
				modifiers |= KM_NUM_LOCK_ON;
			if (::GetKeyState(VK_CAPITAL) & 1)
				modifiers |= KM_CAPS_LOCK_ON;

			switch (m_wParam)
			{
			case VK_RETURN:
				if ((m_lParam >> 16) & KF_EXTENDED)
					modifiers |= KM_IS_KEY_PAD;
				break;
			case VK_INSERT:
			case VK_DELETE:
			case VK_HOME:
			case VK_END:
			case VK_PRIOR:
			case VK_NEXT:
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
				if (!((m_lParam >> 16) & KF_EXTENDED))
					modifiers |= KM_IS_KEY_PAD;
				break;
			case VK_NUMLOCK:
			case VK_NUMPAD0:
			case VK_NUMPAD1:
			case VK_NUMPAD2:
			case VK_NUMPAD3:
			case VK_NUMPAD4:
			case VK_NUMPAD5:
			case VK_NUMPAD6:
			case VK_NUMPAD7:
			case VK_NUMPAD8:
			case VK_NUMPAD9:
			case VK_DIVIDE:
			case VK_MULTIPLY:
			case VK_SUBTRACT:
			case VK_ADD:
			case VK_DECIMAL:
			case VK_CLEAR:
				modifiers |= KM_IS_KEY_PAD;
				break;
			case VK_SHIFT:
				if (isKeyDown(VK_LSHIFT))
					modifiers |= KM_IS_LEFT;
				else if (isKeyDown(VK_RSHIFT))
					modifiers |= KM_IS_RIGHT;
				break;
			case VK_CONTROL:
				if (isKeyDown(VK_LCONTROL))
					modifiers |= KM_IS_LEFT;
				else if (isKeyDown(VK_RCONTROL))
					modifiers |= KM_IS_RIGHT;
				break;
			case VK_MENU:
				if (isKeyDown(VK_LMENU))
					modifiers |= KM_IS_LEFT;
				else if (isKeyDown(VK_RMENU))
					modifiers |= KM_IS_RIGHT;
				break;
			case VK_LWIN:
				modifiers |= KM_IS_LEFT;
				break;
			case VK_RWIN:
				modifiers |= KM_IS_RIGHT;
				break;
			}

			return (KeyModifiers)modifiers;
		}

		int getWinKeyCode()
		{
			return m_wParam;
		}

		bool isSystemKey()
		{
			return m_Message == WM_SYSCHAR || m_Message == WM_SYSKEYDOWN || m_Message == WM_SYSKEYUP;
		}

	protected:
		bool isKeyDown(WPARAM wparam)
		{
			return (GetKeyState(wparam) & 0x8000) != 0;
		}

	private:
		int m_Message;
		WPARAM m_wParam;
		LPARAM m_lParam;
	};
#endif

}

#endif