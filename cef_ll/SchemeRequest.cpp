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

#include "SchemeRequest.h"
#include "SchemePost.h"

#define _CRT_SECURE_NO_WARNINGS

int mystrncpy_s(char* dest, size_t destSize, const char* src, size_t srcSize);

SchemeRequest::SchemeRequest()
{

}

SchemeRequest::SchemeRequest(CefRefPtr<CefRequest> request)
{
	m_rRequest = request;
}


void SchemeRequest::getURL(char *buff, size_t buffsize)
{
	std::string url = m_rRequest->GetURL();

	if (buff)
		mystrncpy_s(buff, buffsize, url.c_str(), url.size());
}

void SchemeRequest::setURL(const char* url)
{
	m_rRequest->SetURL(url);
}


void SchemeRequest::getMethod(char *buff, size_t buffsize)
{
	std::string method = m_rRequest->GetMethod();

	if (buff)
		mystrncpy_s(buff, buffsize, method.c_str(), method.size());
}

void SchemeRequest::setMethod(const char* method)
{
	m_rRequest->SetMethod(method);
}


ChromiumDLL::RefPtr<ChromiumDLL::PostDataI> SchemeRequest::getPostData()
{
	if (!m_rRequest->GetPostData())
		return NULL;

	if (!m_pPostData)
		m_pPostData = new PostData(m_rRequest->GetPostData());

	return m_pPostData;
}

void SchemeRequest::setPostData(const ChromiumDLL::RefPtr<ChromiumDLL::PostDataI>& postData)
{
	m_pPostData = postData;

	if (postData)
	{
		PostData* pd = (PostData*)postData.get();

		if (pd)
			m_rRequest->SetPostData(pd->getHandle());
	}
}

size_t SchemeRequest::getHeaderCount()
{
	CefRequest::HeaderMap map;
	m_rRequest->GetHeaderMap(map);

	return map.size();
}


void SchemeRequest::getHeaderItem(size_t index, char *key, size_t keysize, char* data, size_t datasize)
{
	CefRequest::HeaderMap map;
	m_rRequest->GetHeaderMap(map);

	if (map.size() >= index)
		return;


	CefRequest::HeaderMap::iterator it = map.begin();

	for (size_t x=0; x<index; x++)
		;

	if (it == map.end())
		return;

	if (key)
	{
		cef_string_utf8_t tmp;
		cef_string_to_utf8(it->first.c_str(), it->first.size(), &tmp);
		CefStringUTF8 t(&tmp);

		mystrncpy_s(key, keysize, t.c_str(), t.size());
	}
		

	if (data)
	{
		cef_string_utf8_t tmp;
		cef_string_to_utf8(it->first.c_str(), it->first.size(), &tmp);
		CefStringUTF8 t(&tmp);

		mystrncpy_s(data, datasize, t.c_str(), t.size());
	}
}

void SchemeRequest::setHeaderItem(const char* key, const char* data)
{
	CefRequest::HeaderMap map;
	m_rRequest->GetHeaderMap(map);

	// CefRequest::HeaderMap is now a std::multimap. multimap has no
	// operator[]. But if the specified key already exists, we want to replace
	// it.
	CefRequest::HeaderMap::iterator found = map.find(key);
	if (found != map.end())
	{
		// we already have an entry for this key; update it
		found->second = data;
	}
	else
	{
		// we don't recognize this key, insert a whole new entry
		map.insert(CefRequest::HeaderMap::value_type(key, data));
	}

	m_rRequest->SetHeaderMap(map);
}

void SchemeRequest::set(const char* url, const char* method, const ChromiumDLL::RefPtr<ChromiumDLL::PostDataI>& postData)
{
	setURL(url);
	setMethod(method);
	setPostData(postData);
}
