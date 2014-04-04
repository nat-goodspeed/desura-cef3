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

#include "ChromiumApp.h"
#include "SchemeExtender.h"

std::vector<std::string> ChromiumApp::getSchemeList()
{
	std::map<std::string, int> mSchemes;

	for (size_t x = 0; x < m_vSchemeExtenders.size(); ++x)
		mSchemes[m_vSchemeExtenders[x]->getSchemeName()]++;

	std::vector<std::string> ret;
	std::map<std::string, int>::iterator it = mSchemes.begin();

	for (; it != mSchemes.end(); ++it)
		ret.push_back(it->first);

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// CefBrowserProcessHandler
/////////////////////////////////////////////////////////////////////////////////////////////////

void ChromiumApp::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar)
{
	m_bInit = true;
	std::vector<std::string> vSchemes = getSchemeList();

	for (size_t x = 0; x < vSchemes.size(); ++x)
		registrar->AddCustomScheme(vSchemes[x], true, false, false);
}

void ChromiumApp::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line)
{
	std::vector<std::string> vSchemes = getSchemeList();

	std::string strSchemes;

	for (size_t x = 0; x < vSchemes.size(); ++x)
	{
		if (strSchemes.size()>0)
			strSchemes += "&";

		strSchemes += vSchemes[x];
	}

	command_line->AppendSwitchWithValue("desura-schemes", strSchemes);
}

void ChromiumApp::OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info)
{
}

void ChromiumApp::OnContextInitialized()
{
	for (size_t x = 0; x < m_vSchemeExtenders.size(); ++x)
		SchemeExtender::Register(m_vSchemeExtenders[x]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// CefApp
/////////////////////////////////////////////////////////////////////////////////////////////////

bool ChromiumApp::RegisterJSExtender(ChromiumDLL::JavaScriptExtenderI* extender)
{
	if (m_bInit)
		return false;

	m_vJSExtenders.push_back(extender);
	return true;
}

bool ChromiumApp::RegisterSchemeExtender(ChromiumDLL::SchemeExtenderI* extender)
{
	if (m_bInit)
		return false;

	m_vSchemeExtenders.push_back(extender);
	return true;
}
