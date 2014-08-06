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
#include "ChromiumRefCount.h"



class JSFunctArgs : public ChromiumDLL::ChromiumRefCount<ChromiumDLL::JavaScriptFunctionArgs>
{
public:
	JSFunctArgs(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFunctionArgs>& args, const ChromiumDLL::JSObjHandle& ret)
		: m_Ret(ret)
	{
		argc = 1;
		argv = &m_Ret;
		context = args->context;
		factory = args->factory;
		object = args->object;
		function = "";
	}

	ChromiumDLL::JSObjHandle m_Ret;
};


class DesuraJSExtender : public ChromiumDLL::ChromiumRefCount<ChromiumDLL::JavaScriptExtenderI>
{
public:
	ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI> clone() override
	{
		return new DesuraJSExtender();
	}

	ChromiumDLL::JSObjHandle execute(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFunctionArgs>& args)
	{
		ChromiumDLL::JSObjHandle ret = args->factory->CreateString("Cef3 C++ says hello!");

		if (args->argc > 0 && args->argv[0]->isFunction())
			return args->argv[0]->executeFunction(new JSFunctArgs(args, ret));

		return ret;
	}

	const char* getName() override
	{
		return "desura/test";
	}

	const char* getRegistrationCode() override
	{
		return "var desura; if (!desura){ desura = {}; } (function(){ desura.getString = function(callback){ native function GetStringFromNative(); return GetStringFromNative(callback); }; })();";
	}
};



ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI> NewDesuraJSExtender()
{
	return new DesuraJSExtender();
}