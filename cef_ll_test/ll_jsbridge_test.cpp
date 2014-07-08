/**
* @file media_plugin_cef.cpp
* @brief CEF (Chrome Embedding Framework) plugin for LLMedia API plugin system
*
* @cond
* $LicenseInfo:firstyear=2008&license=viewerlgpl$
* Second Life Viewer Source Code
* Copyright (C) 2010, Linden Research, Inc.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation;
* version 2.1 of the License only.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*
* Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
* $/LicenseInfo$
* @endcond
*/

#include "resource.h"
#include "ll_jsbridge_test.h"
#include <vector>
#include <string>

extern std::string getResourceText(int nResourceId);



ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI> JSBridgeTestScheme::clone(const char* schemeName)
{
	return new JSBridgeTestScheme();
}

const char* JSBridgeTestScheme::getSchemeName()
{
	return "jsbridge";
}

const char* JSBridgeTestScheme::getHostName()
{
	return "";
}

bool JSBridgeTestScheme::processRequest(const ChromiumDLL::RefPtr<ChromiumDLL::SchemeRequestI>& request, bool* redirect)
{
	if (m_strResponse.empty())
		m_strResponse = getResourceText(IDR_JSHTML_BRIDGETEST);

	return !m_strResponse.empty();
}

size_t JSBridgeTestScheme::getResponseSize()
{
	return m_strResponse.size();
}

const char* JSBridgeTestScheme::getResponseMimeType()
{
	return "text/html";
}

const char* JSBridgeTestScheme::getRedirectUrl()
{
	return NULL;
}

bool JSBridgeTestScheme::read(char* buffer, int size, int* readSize)
{
	if (size < m_strResponse.size())
	{
		memcpy(buffer, m_strResponse.c_str(), size);
		*readSize = size;
		m_strResponse = m_strResponse.substr(size);
	}
	else
	{
		memcpy(buffer, m_strResponse.c_str(), m_strResponse.size());
		*readSize = m_strResponse.size();
		m_strResponse.clear();
	}

	return true;
}

void JSBridgeTestScheme::cancel()
{
	m_strResponse.clear();
}




class DataTypeArgs : public ChromiumDLL::ChromiumRefCount<ChromiumDLL::JavaScriptFunctionArgs>
{
public:
	DataTypeArgs(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFunctionArgs>& args, const std::string &strDataType, const ChromiumDLL::JSObjHandle& pVal)
	{
		m_Argv.push_back(args->factory->CreateString(strDataType.c_str()));
		m_Argv.push_back(pVal);
		m_Argv.push_back(args->factory->CreateString("in"));


		function = nullptr;
		context = args->context;
		argc = m_Argv.size();
		argv = &m_Argv[0];
		factory = args->factory;
		object = nullptr;
	}

private:
	std::vector<ChromiumDLL::JSObjHandle> m_Argv;
};



JSBridgeTestExtender::JSBridgeTestExtender(ChromiumDLL::ChromiumControllerI* &pController)
	: m_pController(pController)
{

}

ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI> JSBridgeTestExtender::clone()
{
	return new JSBridgeTestExtender(m_pController);
}

ChromiumDLL::JSObjHandle JSBridgeTestExtender::execute(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFunctionArgs>& args)
{
	std::string strFunction(args->function);

	if (strFunction == "functionCallback")
	{
		return args->factory->CreateBool(true);
	}
	else if (strFunction == "regCallbacks")
	{
		if (args->argc >= 1)
			m_fnV1 = args->argv[0];
		if (args->argc >= 2)
			m_fnV2 = args->argv[1];
		if (args->argc >= 3)
			m_fnV3 = args->argv[2];
		if (args->argc >= 4)
			m_fnDT = args->argv[3];

		assert(m_fnV1 && m_fnV1->isFunction());
		assert(m_fnV2 && m_fnV2->isFunction());
		assert(m_fnV3 && m_fnV3->isFunction());
		assert(m_fnDT && m_fnDT->isFunction());

		if (m_fnV1)
		{
			auto go = args->context->getGlobalObject();
			assert(go);

			if (go)
			{
				auto jsobject = go->getValue("jsobject");
				assert(jsobject && jsobject->isObject());

				if (jsobject && jsobject->isObject())
				{
					jsobject->setValue("count", args->factory->CreateInt(jsobject->getNumberOfKeys()));
					jsobject->setValue("second", jsobject->getValue("first"));
					jsobject->deleteValue("first");
					jsobject->setValue("third", args->factory->CreateInt(3));
				}


				auto jsarray = go->getValue("jsarray");
				assert(jsarray && jsarray->isArray());

				if (jsarray && jsarray->isArray())
				{
					jsarray->setValue(1, jsarray->getValue(0));
					jsarray->setValue(0, args->factory->CreateInt(jsarray->getNumberOfKeys()));
					jsarray->deleteValue(2);
					jsarray->setValue(3, args->factory->CreateInt(3));
				}
			}

			m_fnV1->executeFunction(args);
		}

		if (m_fnV2)
		{

			auto go = args->context->getGlobalObject();
			assert(go);

			if (go)
			{
				auto obj = args->factory->CreateObject();
				obj->setValue("a", args->factory->CreateInt(123));
				go->setValue("cppobject", obj);
			}

			m_fnV2->executeFunction(args);
		}

		if (m_fnV3)
		{
			auto go = args->context->getGlobalObject();
			assert(go);

			if (go)
			{
				auto jscallback = go->getValue("jscallback");
				assert(jscallback && jscallback->isFunction());

				if (jscallback && jscallback->isFunction())
					jscallback->executeFunction(args);
			}

			m_fnV3->executeFunction(args);
		}

		if (m_fnDT)
		{
			m_fnDT->executeFunction(new DataTypeArgs(args, "undefined", args->factory->CreateUndefined()));
			m_fnDT->executeFunction(new DataTypeArgs(args, "null", args->factory->CreateNull()));
			m_fnDT->executeFunction(new DataTypeArgs(args, "double", args->factory->CreateDouble(123.456)));
			m_fnDT->executeFunction(new DataTypeArgs(args, "bool", args->factory->CreateBool(true)));
			m_fnDT->executeFunction(new DataTypeArgs(args, "int", args->factory->CreateInt(789)));
			m_fnDT->executeFunction(new DataTypeArgs(args, "string", args->factory->CreateString("Im a string")));

			auto obj = args->factory->CreateObject();
			obj->setValue("a", args->factory->CreateInt(123));

			m_fnDT->executeFunction(new DataTypeArgs(args, "object", obj));


			auto arr = args->factory->CreateArray();
			arr->setValue(0, args->factory->CreateInt(190));
			arr->setValue(1, args->factory->CreateInt(1));
			arr->setValue(2, args->factory->CreateInt(84));

			m_fnDT->executeFunction(new DataTypeArgs(args, "array", arr));


			auto funct = args->factory->CreateFunction("functionCallback", this);
			m_fnDT->executeFunction(new DataTypeArgs(args, "function", funct));
		}
	}
	else if (strFunction == "dataTypes")
	{
		if (args->argc < 2 || !args->argv[0]->isString())
		{
			assert(false);
		}
		else
		{
			char szDataType[255] = { 0 };
			args->argv[0]->getStringValue(szDataType, 255);

			std::string strDataType(szDataType);

			auto val = args->argv[1];

			if (strDataType == "undefined") {
				assert(val->isUndefined());
			}
			else if (strDataType == "null") {
				assert(val->isNull());
			}
			else if (strDataType == "double") {
				assert(val->isDouble());
				assert((123.456 - val->getDoubleValue()) < 0.001);
			}
			else if (strDataType == "bool") {
				assert(val->isBool());
				assert(val->getBoolValue());
			}
			else if (strDataType == "int") {
				assert(val->isInt());
				assert(val->getIntValue() == 789);
			}
			else if (strDataType == "string") {
				assert(val->isString());

				char szVal[255] = { 0 };
				val->getStringValue(szVal, 255);

				assert(std::string("Im a string") == szVal);
			}
			else if (strDataType == "function") {
				assert(val->isFunction());
			}
			else if (strDataType == "object") {
				assert(val->isObject());

				auto a = val->getValue("a");
				assert(a->isInt() && a->getIntValue() == 123);
			}
			else if (strDataType == "array") {
				assert(val->isArray());
				assert(val->getArrayLength() == 3);

				auto a = val->getValue(0);
				assert(a->isInt() && a->getIntValue() == 190);

				auto b = val->getValue(1);
				assert(b->isInt() && b->getIntValue() == 1);

				auto c = val->getValue(2);
				assert(c->isInt() && c->getIntValue() == 84);
			}

			if (m_fnDT)
				m_fnDT->executeFunction(args);
		}
	}
	else if (strFunction == "assert")
	{
		assert(false);
	}

	return ChromiumDLL::JSObjHandle();
}

const char* JSBridgeTestExtender::getName()
{
	return "jsbridgetest/extender";
}

const char* JSBridgeTestExtender::getRegistrationCode()
{
	if (m_strBinding.empty())
		m_strBinding = getResourceText(IDR_JSBINDING_BRIDGETEST);

	return m_strBinding.c_str();
}