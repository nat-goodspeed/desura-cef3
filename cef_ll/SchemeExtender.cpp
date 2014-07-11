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

#include "ChromiumBrowser.h"

#include "SchemeExtender.h"
#include "SchemeRequest.h"
#include "SchemePost.h"

#include "include/cef_scheme.h"

#include <map>
#include <algorithm>

class SchemeHandlerFactory;
class CefResourceHandler;

std::map<std::string, SchemeHandlerFactory*> g_mSchemeExtenders;

class SchemeHandlerFactory : public CefSchemeHandlerFactory
{
public:
	SchemeHandlerFactory()
	{
	}

	~SchemeHandlerFactory()
	{
		if (m_mSchemeMap.size() > 0)
			g_mSchemeExtenders[m_mSchemeMap.begin()->second->getSchemeName()] = 0;

		m_mSchemeMap.clear();
	}

	CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>,
										 const CefString& scheme_name,
										 CefRefPtr<CefRequest> request)
	{
		std::string url = request->GetURL();
		std::vector<size_t> slashes;

		for (size_t x=0; x<url.size(); x++)
		{
			if (url[x] == '/')
				slashes.push_back(x);
		}

		if (slashes.size() < 3)
			return NULL;

		CefStringUTF8 t(ConvertToUtf8(scheme_name));

		std::string scheme = t.c_str();
		std::string host = url.substr(slashes[1]+1, slashes[2]-slashes[1]-1);

		std::string strKey = scheme + ":" + host;
		std::map<std::string, ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI> >::iterator it = m_mSchemeMap.find(strKey);

		if (it == m_mSchemeMap.end())
		{
			strKey = scheme + ":__ALL__";
			it = m_mSchemeMap.find(strKey);
		}

		if (it == m_mSchemeMap.end())
			return NULL;

		return new SchemeExtender(it->second->clone(scheme.c_str()));
	}

	bool registerScheme(const ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI>& se)
	{
		std::string strHostname;
		std::string strScheme;

		if (se->getHostName())
			strHostname = se->getHostName();

		if (se->getSchemeName())
			strScheme = se->getSchemeName();
		
		if (strHostname.empty())
			strHostname = "__ALL__";

		std::string strKey = strScheme + ":" + strHostname;
		m_mSchemeMap[strKey] = se;

		return CefRegisterSchemeHandlerFactory(se->getSchemeName(), se->getHostName(), this);
	}

	IMPLEMENT_REFCOUNTING(SchemeHandlerFactory);

private:
	std::map<std::string, ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI> > m_mSchemeMap;
};

bool SchemeExtender::Register(const ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI>& se)
{
	if (!se)
		return false;

	if (!g_mSchemeExtenders[se->getSchemeName()])
		g_mSchemeExtenders[se->getSchemeName()] = new SchemeHandlerFactory();

	return g_mSchemeExtenders[se->getSchemeName()]->registerScheme(se);
}

SchemeExtender::SchemeExtender(const ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI>& se) 
	: m_redirect(false)
{
	m_pSchemeExtender = se;
}

SchemeExtender::~SchemeExtender()
{
}

bool SchemeExtender::ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback)
{
	if (!m_pSchemeExtender)
		return false;

	ChromiumDLL::RefPtr<ChromiumDLL::SchemeRequestI> r = new SchemeRequest(request);

	// capture redirect flag in bool member for GetResponseHeaders()
	bool res = m_pSchemeExtender->processRequest(r, &m_redirect);

	if (res)
		callback->Continue();

	return res;
}

void SchemeExtender::Cancel()
{
	if (!m_pSchemeExtender)
		return;

	m_pSchemeExtender->cancel();
}

void SchemeExtender::GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl)
{
	if (!m_pSchemeExtender)
		return;

	// nat: moved this stanza from ProcessRequest() when redirectUrl param
	// moved from base-class ProcessRequest() method to GetResponseHeaders()
	// method
	if (m_redirect)
	{
		const char *szRUrl = m_pSchemeExtender->getRedirectUrl();

		if (szRUrl)
			redirectUrl.FromASCII(szRUrl);
	}
	// end migrated code

	response_length = m_pSchemeExtender->getResponseSize();
	const char* mime = m_pSchemeExtender->getResponseMimeType();

	if (mime)
		response->SetMimeType(mime);


}

bool SchemeExtender::ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback)
{
	if (!m_pSchemeExtender)
		return false;

	bool res = m_pSchemeExtender->read((char*)data_out, bytes_to_read, &bytes_read);

	if (res)
		callback->Continue();

	return res;
}
