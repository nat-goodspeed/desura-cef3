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

#include "JavaScriptFactory.h"
#include "JavaScriptObject.h"

JavaScriptFactory::JavaScriptFactory()
{
}

JavaScriptFactory::~JavaScriptFactory()
{
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateUndefined()
{
	return new JavaScriptObject();
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateNull()
{
	return new JavaScriptObject(JSONNode(JSON_NULL));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateBool(bool value)
{
	return new JavaScriptObject(JSONNode("", value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateInt(int value)
{
	return new JavaScriptObject(JSONNode("", value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateDouble(double value)
{
	return new JavaScriptObject(JSONNode("", value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateString(const char* value)
{
	return new JavaScriptObject(JSONNode("", value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateArray()
{
	return new JavaScriptObject(JSONNode(JSON_ARRAY));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateObject()
{
	return new JavaScriptObject(JSONNode(JSON_NODE));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateObject(const ChromiumDLL::RefPtr<ChromiumDLL::IntrusiveRefPtrI>& userData)
{
	JSONNode j(JSON_NODE);
	j.push_back(JSONNode("__user_data__", (long long)userData));
	return new JavaScriptObject(j);
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateFunction(const char* name, const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI>& handler)
{
	return new JavaScriptObject(name, handler);
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateException(const char* value)
{
	JavaScriptObject *ret = new JavaScriptObject(JSONNode("", value));
	ret->setException();
	return ret;
}
