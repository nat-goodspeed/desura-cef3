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


#ifndef MEDIA_PLUGIN_CEF_JSBRIDGE_TEST_H
#define MEDIA_PLUGIN_CEF_JSBRIDGE_TEST_H


#define CEF_IGNORE_FUNCTIONS 1
#include "ChromiumBrowserI.h"
#include "ChromiumRefCount.h"
#include <string>



class JSBridgeTestScheme : public ChromiumDLL::ChromiumRefCount<ChromiumDLL::SchemeExtenderI>
{
public:
	virtual ChromiumDLL::RefPtr<ChromiumDLL::SchemeExtenderI> clone(const char* schemeName) OVERRIDE;
	virtual const char* getSchemeName() OVERRIDE;
	virtual const char* getHostName() OVERRIDE;

	//! Processes the request. Call response ready when ready to reply
	//! Set redirect to true to redirect to another url (read from getRedirectUrl)
	//! 
	virtual bool processRequest(const ChromiumDLL::RefPtr<ChromiumDLL::SchemeRequestI>& request, bool* redirect) OVERRIDE;

	//! Called when response is ready
	//!
	virtual size_t getResponseSize() OVERRIDE;

	//! Return NULL to use default
	virtual const char* getResponseMimeType() OVERRIDE;

	//! Return NULL to cancel redirect
	virtual const char* getRedirectUrl() OVERRIDE;

	//! Return false to cancel read
	//! Set readSize to zero and return true to wait for callback
	//! 
	virtual bool read(char* buffer, size_t size, size_t* readSize) OVERRIDE;

	//! Cancel request
	//!
	virtual void cancel() OVERRIDE;

private:
	std::string m_strResponse;
};




class JSBridgeTestExtender : public ChromiumDLL::ChromiumRefCount<ChromiumDLL::JavaScriptExtenderI>
{
public:
	JSBridgeTestExtender(ChromiumDLL::ChromiumControllerI* &pController);

	virtual ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptExtenderI> clone() OVERRIDE;
	virtual ChromiumDLL::JSObjHandle execute(const ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFunctionArgs>& args) OVERRIDE;


	virtual const char* getName() OVERRIDE;
	virtual const char* getRegistrationCode() OVERRIDE;

private:
	ChromiumDLL::JSObjHandle m_fnV1;
	ChromiumDLL::JSObjHandle m_fnV2;
	ChromiumDLL::JSObjHandle m_fnV3;
	ChromiumDLL::JSObjHandle m_fnDT;

	std::string m_strBinding;
	ChromiumDLL::ChromiumControllerI* &m_pController;
};



#endif
