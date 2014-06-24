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

#ifndef THIRDPARTY_CEF3_SCHEMEREQUEST_H
#define THIRDPARTY_CEF3_SCHEMEREQUEST_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "RefCount.h"
//#include "include/cef.h"
#include "include/internal/cef_ptr.h"
#include "include/cef_request.h"

class SchemeRequest : public ChromiumDLL::SchemeRequestI
{
public:
	SchemeRequest();
	SchemeRequest(CefRefPtr<CefRequest> request);

	virtual void getURL(char *buff, size_t buffsize);
	virtual void setURL(const char* url);

	virtual void getMethod(char *buff, size_t buffsize);
	virtual void setMethod(const char* method);

	virtual ChromiumDLL::RefPtr<ChromiumDLL::PostDataI> getPostData();
	virtual void setPostData(const ChromiumDLL::RefPtr<ChromiumDLL::PostDataI>& postData);

	virtual size_t getHeaderCount();

	virtual void getHeaderItem(size_t index, char *key, size_t keysize, char* data, size_t datasize);
	virtual void setHeaderItem(const char* key, const char* data);

	virtual void set(const char* url, const char* method, const ChromiumDLL::RefPtr<ChromiumDLL::PostDataI>& postData);

	CefRefPtr<CefRequest> getHandle()
	{
		return m_rRequest;
	}

private:
	CefRefPtr<CefRequest> m_rRequest;

	CEF3_IMPLEMENTREF_COUNTING(SchemeRequest);
};


#endif //THIRDPARTY_CEF3_SCHEMEREQUEST_H
