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

#ifndef DESURA_SCHEMEEXTENDER_H
#define DESURA_SCHEMEEXTENDER_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
//#include "include/cef.h"
#include "include/cef_response.h"
#include "include/cef_resource_handler.h"

class SchemeExtender : public CefResourceHandler, public ChromiumDLL::SchemeCallbackI
{
public:
	static bool Register(ChromiumDLL::SchemeExtenderI* se);

	SchemeExtender(ChromiumDLL::SchemeExtenderI* se);
	~SchemeExtender();

	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback);
	virtual void Cancel();

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response,
                                    int64& response_length,
                                    CefString& redirectUrl);
	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback);

	virtual void responseReady();
	virtual void dataReady();
	virtual void cancel();

	IMPLEMENT_REFCOUNTING(SchemeExtender);

private:
	ChromiumDLL::SchemeExtenderI* m_pSchemeExtender;
	CefRefPtr<CefCallback> m_Callback;
	bool m_redirect;
};


#endif //DESURA_SCHEMEEXTENDER_H
