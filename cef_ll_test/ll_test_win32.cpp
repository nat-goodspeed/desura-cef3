/**
 * @file media_plugin_cef.cpp
 * @brief CEF (Chrome Embedding Framework) plugin for LLMedia API plugin system
 *
 * @cond
 * $LicenseInfo:firstyear=2008&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 * @endcond
 */

#include <xstring>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

static HINSTANCE gs_hModule;


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		gs_hModule = hinstDLL;

	return TRUE;
}

std::string getResourceText(int nResourceId)
{
	HRSRC hSrc = FindResource(gs_hModule, MAKEINTRESOURCE(nResourceId), RT_HTML);

	if (!hSrc)
		return "";

	int nSize = SizeofResource(gs_hModule, hSrc);

	if (nSize <= 0)
		return "";

	HGLOBAL hGlobal = LoadResource(gs_hModule, hSrc);

	if (!hGlobal)
	{

		return "";
	}

	void* pData = LockResource(hGlobal);
	return std::string((char*)pData, nSize);
}