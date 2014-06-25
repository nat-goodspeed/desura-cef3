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

#ifndef THIRDPARTY_CEF3_JAVASCRIPTFACTORY_H
#define THIRDPARTY_CEF3_JAVASCRIPTFACTORY_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "RefCount.h"
#include "libjson.h"

class JavaScriptFactory : public ChromiumDLL::JavaScriptFactoryI
{
public:
	JavaScriptFactory();
	~JavaScriptFactory();

	ChromiumDLL::JSObjHandle CreateUndefined() OVERRIDE;
	ChromiumDLL::JSObjHandle CreateNull() OVERRIDE;
	ChromiumDLL::JSObjHandle CreateBool(bool value) OVERRIDE;
	ChromiumDLL::JSObjHandle CreateInt(int value) OVERRIDE;
	ChromiumDLL::JSObjHandle CreateDouble(double value) OVERRIDE;
	ChromiumDLL::JSObjHandle CreateString(const char* value) OVERRIDE;
	ChromiumDLL::JSObjHandle CreateArray() OVERRIDE;
	ChromiumDLL::JSObjHandle CreateObject() OVERRIDE;
	virtual ChromiumDLL::JSObjHandle CreateObject(const ChromiumDLL::RefPtr<ChromiumDLL::IntrusiveRefPtrI>& userData) OVERRIDE;
	ChromiumDLL::JSObjHandle CreateException(const char* value) OVERRIDE;
	virtual ChromiumDLL::JSObjHandle CreateFunction(const char* name, const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& handler) OVERRIDE;

	CEF3_IMPLEMENTREF_COUNTING(JavaScriptFactory);
};

#endif //DESURA_JAVASCRIPTFACTORY_H
