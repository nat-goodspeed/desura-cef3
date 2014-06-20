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

#include "ChromiumBrowserI.h"
//#include "include/cef.h"
#include "include/cef_task.h"
#include "include/cef_cookie.h"
#include "include/internal/cef_types_wrappers.h" // CefCookie

class Cookie : public ChromiumDLL::CookieI
{
public:
	Cookie()
	{
		m_rCookie.secure = false;
		m_rCookie.httponly = false;
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void SetDomain(const char* domain)
	{
		cef_string_utf8_to_utf16(domain, strlen(domain), &m_rCookie.domain);
	}

	virtual void SetName(const char* name)
	{
		cef_string_utf8_to_utf16(name, strlen(name), &m_rCookie.name);
	}

	virtual void SetData(const char* data)
	{
		cef_string_utf8_to_utf16(data, strlen(data), &m_rCookie.value);
	}

	virtual void SetPath(const char* path)
	{
		cef_string_utf8_to_utf16(path, strlen(path), &m_rCookie.path);
	}

	CefCookie m_rCookie;
};

class CookieTask : public CefTask
{
public:
	CookieTask(const char* url, CefCookie &cookie)
	{
		m_szCookie = cookie;

		if (url)
			m_szUrl = url;

		m_bDel = false;
	}

	CookieTask(const char* url, const char* name)
	{
		if (url)
			m_szUrl = url;

		if (name)
			m_szName = name;
	
		m_bDel = true;
	}

	virtual void Execute()
	{
		CefRefPtr<CefCookieManager> cookiemgr = CefCookieManager::GetGlobalManager();
		if (m_szName.size())
			cookiemgr->DeleteCookies(m_szUrl.c_str(), m_szName.c_str());
		else
			cookiemgr->SetCookie(m_szUrl.c_str(), m_szCookie);
	}

	bool m_bDel;

	std::string m_szUrl;
	std::string m_szName;

	CefCookie m_szCookie;
	IMPLEMENT_REFCOUNTING(CookieTask);
};


void CEF_DeleteCookie_Internal(const char* url, const char* name)
{
	CefPostTask(TID_IO, new CookieTask(url, name));
}

ChromiumDLL::CookieI* CEF_CreateCookie_Internal()
{
	return new Cookie();
}

void CEF_SetCookie_Internal(const char* url, ChromiumDLL::CookieI* cookie)
{
	Cookie* c = (Cookie*)cookie;

	if (!c)
		return;

	CefPostTask(TID_IO, new CookieTask(url, c->m_rCookie));
}

