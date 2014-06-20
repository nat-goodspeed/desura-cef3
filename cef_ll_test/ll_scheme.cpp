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
#include <string>


template <class T>
class DesuraSchemeBase : public ChromiumDLL::SchemeExtenderI
{
public:
	DesuraSchemeBase(const char* schemename, const char* hostname)
	{
		m_szHostname = hostname;
		m_szSchemename = schemename;
		m_uiResponseSize = 0;
	}

	virtual SchemeExtenderI* clone(const char* scheme)
	{
		return new T();
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual const char* getSchemeName()
	{
		return m_szSchemename.c_str(); //L"desura";
	}

	virtual const char* getHostName()
	{
		return m_szHostname.c_str(); //L"image";
	}

	virtual size_t getResponseSize()
	{
		return m_uiResponseSize;
	}

	virtual const char* getResponseMimeType()
	{
		if (m_szMimeType.size() == 0)
			return nullptr;

		return m_szMimeType.c_str();
	}

	virtual const char* getRedirectUrl()
	{
		if (m_szRedirectUrl.size() == 0)
			return nullptr;

		return m_szRedirectUrl.c_str();
	}

	virtual ~DesuraSchemeBase(){}

protected:
	size_t m_uiResponseSize;
	std::string m_szMimeType;
	std::string m_szRedirectUrl;

private:
	std::string m_szHostname;
	std::string m_szSchemename;
};


class ExternalLoaderScheme : public DesuraSchemeBase<ExternalLoaderScheme>
{
public:
	ExternalLoaderScheme();

	virtual bool processRequest(ChromiumDLL::SchemeRequestI* request, bool* redirect);
	virtual void cancel();
	virtual bool read(char* buffer, int size, int* readSize);

private:
	const std::string m_Js;
};


ExternalLoaderScheme::ExternalLoaderScheme() 
	: DesuraSchemeBase<ExternalLoaderScheme>("desura", "")
	, m_Js("alert('Hi From scheme!');")
{
}

bool ExternalLoaderScheme::processRequest(ChromiumDLL::SchemeRequestI* request, bool* redirect)
{
	m_uiResponseSize = m_Js.size();
	m_szMimeType = "application/javascript";

	return true;
}

void ExternalLoaderScheme::cancel()
{
}

bool ExternalLoaderScheme::read(char* buffer, int size, int* readSize)
{
	strcpy(buffer, m_Js.c_str());
	*readSize = m_Js.size();
	return true;
}

ChromiumDLL::SchemeExtenderI* NewExternalLoaderScheme()
{
	return new ExternalLoaderScheme();
}