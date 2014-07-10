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

#include "Cookie.h"

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

	Cookie(CefCookie cookie)
		: m_rCookie(cookie)
	{
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

	CEF3_IMPLEMENTREF_COUNTING(Cookie);
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


class DeleteCookiesVisitor : public CefCookieVisitor
{
public:
	bool Visit(const CefCookie& cookie, int count, int total,
		bool& deleteCookie) OVERRIDE
	{
		deleteCookie = true;
		return true;
	}

	IMPLEMENT_REFCOUNTING(DeleteCookiesVisitor);
};

class CookiesVisitor : public CefCookieVisitor
{
public:
	CookiesVisitor(const ChromiumDLL::RefPtr<ChromiumDLL::ChormiumCookieVistor>& visitor)
		: m_Visitor(visitor)
	{
	}

	bool Visit(const CefCookie& cookie, int count, int total,
		bool& deleteCookie) OVERRIDE
	{
		ChromiumDLL::RefPtr<Cookie> c = new Cookie(cookie);
		return m_Visitor->visit(c);
	}

	ChromiumDLL::RefPtr<ChromiumDLL::ChormiumCookieVistor>  m_Visitor;
	IMPLEMENT_REFCOUNTING(DeleteCookiesVisitor);
};


void CookieManager::purgeAll()
{
	CefCookieManager::GetGlobalManager()->VisitAllCookies(new DeleteCookiesVisitor());
}

void CookieManager::setCookie(const char* url, const ChromiumDLL::RefPtr<ChromiumDLL::CookieI>& cookie)
{
	Cookie* c = (Cookie*)cookie.get();

	if (!c)
		return;

	CefPostTask(TID_IO, new CookieTask(url, c->m_rCookie));
}

void CookieManager::delCookie(const char* url, const char* name)
{
	CefPostTask(TID_IO, new CookieTask(url, name));
}

void CookieManager::visitCookies(const ChromiumDLL::RefPtr<ChromiumDLL::ChormiumCookieVistor>& visitor, const char* szUrl)
{
	if (szUrl)
		CefCookieManager::GetGlobalManager()->VisitUrlCookies(szUrl, false, new CookiesVisitor(visitor));
	else
		CefCookieManager::GetGlobalManager()->VisitAllCookies(new CookiesVisitor(visitor));
}

void CookieManager::enableCookies()
{
	std::vector<CefString> schemes;

	schemes.push_back("http");
	schemes.push_back("https");

	CefCookieManager::GetGlobalManager()->SetSupportedSchemes(schemes);
}

void CookieManager::disableCookies()
{
	std::vector<CefString> schemes;
	schemes.push_back("fakecookiescheme");

	CefCookieManager::GetGlobalManager()->SetSupportedSchemes(schemes);

	purgeAll();
}

ChromiumDLL::RefPtr<ChromiumDLL::CookieI> CookieManager::createCookie()
{
	return new Cookie();
}

