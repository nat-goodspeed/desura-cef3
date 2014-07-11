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


#ifndef THIRDPARTY_CEF3_COOKIE_HEADER
#define THIRDPARTY_CEF3_COOKIE_HEADER
#ifdef _WIN32
#pragma once
#endif


#include "ChromiumBrowserI.h"
#include "RefCount.h"

class CookieManager : public ChromiumDLL::ChromiumCookieManagerI
{
public:
	virtual void purgeAll();
	virtual void setCookie(const char* ulr, const ChromiumDLL::RefPtr<ChromiumDLL::CookieI>& cookie);

	virtual void delCookie(const char* url, const char* name);
	virtual void visitCookies(const ChromiumDLL::RefPtr<ChromiumDLL::ChormiumCookieVistor>& visitor, const char* szUrl = NULL);

	virtual void enableCookies();
	virtual void disableCookies();

	virtual ChromiumDLL::RefPtr<ChromiumDLL::CookieI> createCookie();

private:
	CEF3_IMPLEMENTREF_COUNTING(CookieManager);
};




#endif