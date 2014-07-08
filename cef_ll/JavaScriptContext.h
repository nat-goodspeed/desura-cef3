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

#ifndef DESURA_JAVASCRIPTCONTEXT_H
#define DESURA_JAVASCRIPTCONTEXT_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "JavaScriptFactory.h"
#include "include\cef_base.h"

#include "libjson.h"
#include "tinythread.h"

#include "Controller.h"

#include <xstring>
#include <vector>
#include <assert.h>

template <typename T>
class JavaScriptContextHandle
{
public:
	JavaScriptContextHandle(int nBrowser)
		: m_nBrowser(nBrowser)
	{
		JavaScriptContextHelper<T>::Self.push(m_nBrowser);
	}

	~JavaScriptContextHandle()
	{
		JavaScriptContextHelper<T>::Self.pop(m_nBrowser);
	}

private:
	int m_nBrowser;
};



class JavaScriptContext : public ChromiumDLL::JavaScriptContextI
{
public:
	JavaScriptContext(int nBrowserId);

	void destroy() OVERRIDE;
	ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptContextI> clone() OVERRIDE;

	void enter() OVERRIDE;
	void exit() OVERRIDE;

	ChromiumDLL::RefPtr<ChromiumDLL::JavaScriptFactoryI> getFactory() OVERRIDE;
	ChromiumDLL::JSObjHandle getGlobalObject() OVERRIDE;


private:
	ChromiumDLL::RefPtr<JavaScriptFactory> m_JSFactory;
	unsigned int m_uiCount;

	int m_nBrowserId;

	CEF3_IMPLEMENTREF_COUNTING(JavaScriptContext);
};



class JsonSendTargetI
{
public:
	virtual bool send(int nBrowser, JSONNode msg) = 0;
};


template <typename T>
class JavaScriptContextHelper
{
public:
	JavaScriptContextHelper()
		: m_nActiveRequests(0)
	{
	}

	//! Used to keep track of what browser we are current representing while processing received messages
	//!
	void push(int nBrowser)
	{
		cef3Trace("nBrowser: %d", nBrowser);
		m_vBrowserContext.push_back(nBrowser);
	}

	void pop(int nBrowser)
	{
		cef3Trace("nBrowser: %d", nBrowser);
		assert(nBrowser == m_vBrowserContext.back());
		m_vBrowserContext.pop_back();
	}

	int peek()
	{
		return m_vBrowserContext.back();
	}

	bool processResponse(CefRefPtr<T> &pExtender, int nBrowser, JSONNode jsonRes);
	bool processRequest(CefRefPtr<T> &pExtender, int nBrowser, JSONNode jsonReq);

	void newRequest(CefRefPtr<T> &pExtender, int nBrowser, JSONNode jsonReq);

	void setTarget(JsonSendTargetI* pTarget)
	{
		m_pTarget = pTarget;
	}

	JSONNode invokeFunction(const std::string &strJSNameOrFunctId, const std::string &strFunction, JSONNode object, const std::vector<JSONNode> &args);
	JSONNode invokeObjectRequest(const std::string &strJSNameOrFunctId, const std::string &strObjectId, const std::string &strRequestType, JSONNode args = JSONNode());

	static JavaScriptContextHelper<T> Self;

protected:
	class RequestInfo
	{
	public:
		RequestInfo()
			: m_nBrowser(-1)
		{
		}

		RequestInfo(CefRefPtr<T> &pExtender, int nBrowser, JSONNode jsonReq)
			: m_pExtender(pExtender)
			, m_nBrowser(nBrowser)
			, m_jsonReq(jsonReq)
		{
		}

		CefRefPtr<T> m_pExtender;
		int m_nBrowser;
		JSONNode m_jsonReq;
	};

	class JsonRequestTask : public CefTask
	{
	public:
		JsonRequestTask(RequestInfo info)
			: m_Info(info)
		{
		}

		void Execute() OVERRIDE
		{
			JavaScriptContextHelper<T>::Self.newRequest(m_Info.m_pExtender, m_Info.m_nBrowser, m_Info.m_jsonReq);
		}

	private:
		RequestInfo m_Info;
		IMPLEMENT_REFCOUNTING(JsonRequestTask);
	};

	JSONNode waitForResponse();

private:
	tthread::mutex m_WaitLock;
	tthread::condition_variable m_WaitCond;

	tthread::mutex m_ResponseLock;
	std::vector<RequestInfo> m_vResponseList;

	JsonSendTargetI *m_pTarget;
	std::vector<int> m_vBrowserContext;

	tthread::mutex m_ActiveRequestLock;
	int m_nActiveRequests;
};



template <typename T>
JSONNode JavaScriptContextHelper<T>::invokeFunction(const std::string &strJSNameOrFunctId, const std::string &strFunction, JSONNode object, const std::vector<JSONNode> &args)
{
	cef3Trace("id: %s, fun: %s, argc: %d", strJSNameOrFunctId.c_str(), strFunction.c_str(), args.size());

	int nBrowserId = -1;

	if (!m_vBrowserContext.empty())
		nBrowserId = m_vBrowserContext.back();

	JSONNode a(JSON_ARRAY);
	a.set_name("arguments");

	for (size_t x = 0; x < args.size(); ++x)
		a.push_back(args[x]);

	object.set_name("object");

	JSONNode r(JSON_NODE);
	r.set_name("request");
	
	r.push_back(JSONNode("command", "FunctionCall"));
	r.push_back(JSONNode("function", strFunction));
	r.push_back(object);
	r.push_back(a);

	JSONNode msg(JSON_NODE);
	msg.push_back(JSONNode("name", "JSE-Request"));
	msg.push_back(JSONNode("extender", strJSNameOrFunctId));
	msg.push_back(JSONNode("browser", nBrowserId));
	msg.push_back(r);

	JSONNode res;

	try
	{
		{
			tthread::lock_guard<tthread::mutex> arguard(m_ActiveRequestLock);
			m_nActiveRequests++;

			cef3Trace("Increased Active Req: %d", m_nActiveRequests);
		}


		if (!m_pTarget->send(nBrowserId, msg))
			throw std::exception("Failed to send request");

		res = waitForResponse();
	}
	catch (...)
	{
		{
			tthread::lock_guard<tthread::mutex> arguard(m_ActiveRequestLock);
			m_nActiveRequests--;

			cef3Trace("Dec Active Req: %d", m_nActiveRequests);
		}

		throw;
	}

	{
		tthread::lock_guard<tthread::mutex> arguard(m_ActiveRequestLock);
		m_nActiveRequests--;

		cef3Trace("Dec Active Req: %d", m_nActiveRequests);
	}

	return res;
}



template <typename T>
JSONNode JavaScriptContextHelper<T>::invokeObjectRequest(const std::string &strJSNameOrFunctId, const std::string &strObjectId, const std::string &strRequestType, JSONNode args = JSONNode())
{
	cef3Trace("Id: %s Type: %s", strObjectId.c_str(), strRequestType.c_str());

	int nBrowserId = -1;

	if (!m_vBrowserContext.empty())
		nBrowserId = m_vBrowserContext.back();

	args.set_name("arguments");

	JSONNode r(JSON_NODE);
	r.set_name("request");

	r.push_back(JSONNode("command", "ObjectRequest"));
	r.push_back(JSONNode("object", strObjectId));
	r.push_back(JSONNode("function", strRequestType));
	r.push_back(args);

	JSONNode msg(JSON_NODE);
	msg.push_back(JSONNode("name", "JSE-Request"));
	msg.push_back(JSONNode("extender", strJSNameOrFunctId));
	msg.push_back(JSONNode("browser", nBrowserId));
	msg.push_back(r);

	JSONNode res;

	try
	{
		{
			tthread::lock_guard<tthread::mutex> arguard(m_ActiveRequestLock);
			m_nActiveRequests++;

			cef3Trace("Increased Active Req: %d", m_nActiveRequests);
		}


		if (!m_pTarget->send(nBrowserId, msg))
			throw std::exception("Failed to send request");

		res = waitForResponse();
	}
	catch (...)
	{
		{
			tthread::lock_guard<tthread::mutex> arguard(m_ActiveRequestLock);
			m_nActiveRequests--;

			cef3Trace("Dec Active Req: %d", m_nActiveRequests);
		}

		throw;
	}

	{
		tthread::lock_guard<tthread::mutex> arguard(m_ActiveRequestLock);
		m_nActiveRequests--;

		cef3Trace("Dec Active Req: %d", m_nActiveRequests);
	}

	return res;
}


template <typename T>
JSONNode JavaScriptContextHelper<T>::waitForResponse()
{
#ifdef DEBUG
	int nWaitTimeout = 999; //allow for debugging
#else
	int nWaitTimeout = 1;
#endif

	int nEmptyRespone = 0;

	while (true)
	{
		cef3Trace("wait");

		{
			tthread::lock_guard<tthread::mutex> guard(m_WaitLock);

			if (!m_WaitCond.wait_timed(guard, nWaitTimeout))
			{
				cef3Trace("Timed out");
				throw std::exception("Timed out waiting for response from browser");
				break;
			}
		}

		RequestInfo info;

		{

			tthread::lock_guard<tthread::mutex> guard(m_ResponseLock);

			if (m_vResponseList.empty())
			{
				cef3Trace("Empty Response");

				nEmptyRespone++;

				if (nEmptyRespone == 3)
					throw std::exception("Received empty response too many times.");

				continue;
			}

			info = m_vResponseList.back();
			m_vResponseList.pop_back();
		}

		JSONNode jsonReq = info.m_jsonReq;

		if (jsonReq.find("command") == jsonReq.end())
		{
			cef3Trace("No Command in Response");

			nEmptyRespone++;

			if (nEmptyRespone == 3)
				throw std::exception("Received empty response too many times.");

			continue;
		}

		std::string strAction = jsonReq["command"].as_string();

		if (strAction == "FunctionReturn")
		{
			cef3Trace("function return - Extender: %s, nBrowser: %d", info.m_pExtender->getName(), info.m_nBrowser);

			if (jsonReq.find("result") != jsonReq.end())
				return jsonReq["result"];

			return JSONNode(JSON_NULL);
		}
		else if (strAction == "FunctionException")
		{
			cef3Trace("function exception - Extender: %s, nBrowser: %d", info.m_pExtender->getName(), info.m_nBrowser);

			std::string strExcpt = jsonReq["exception"].as_string();
			throw std::exception(strExcpt.c_str());
		}
		else if (strAction == "ObjectReturn")
		{
			cef3Trace("object result - nBrowser: %d", info.m_nBrowser);

			if (jsonReq.find("result") != jsonReq.end())
				return jsonReq["result"];

			return JSONNode(JSON_NULL);
		}
		else
		{
			cef3Trace("new request - Extender: %s, nBrowser: %d", info.m_pExtender ? info.m_pExtender->getName() : "NULL", info.m_nBrowser);
			newRequest(info.m_pExtender, info.m_nBrowser, info.m_jsonReq);
		}
	}
}





template <typename T>
bool JavaScriptContextHelper<T>::processRequest(CefRefPtr<T> &pExtender, int nBrowser, JSONNode jsonReq)
{
	RequestInfo info(pExtender, nBrowser, jsonReq);
	tthread::lock_guard<tthread::mutex> arguard(m_ActiveRequestLock);

	//if we have a pending active request let it handle the new request
	if (m_nActiveRequests == 0)
	{
		cef3Trace("PostTask - Extender: %s, nBrowser: %d (AD: %d)", pExtender->getName(), nBrowser, m_nActiveRequests);
		CefPostTask(T::TaskThread, new JsonRequestTask(info));
	}
	else
	{
		cef3Trace("Waiting Task - Extender: %s, nBrowser: %d (AD: %d)", pExtender->getName(), nBrowser, m_nActiveRequests);

		tthread::lock_guard<tthread::mutex> guard(m_ResponseLock);
		m_vResponseList.push_back(info);
		m_WaitCond.notify_all();
	}

	return true;
}

template <typename T>
bool JavaScriptContextHelper<T>::processResponse(CefRefPtr<T> &pExtender, int nBrowser, JSONNode jsonReq)
{
	RequestInfo info(pExtender, nBrowser, jsonReq);

	tthread::lock_guard<tthread::mutex> arguard(m_ActiveRequestLock);
	cef3Trace("Extender: %s, nBrowser: %d (AR: %d)", pExtender?pExtender->getName():"NULL", nBrowser, m_nActiveRequests);

	if (m_nActiveRequests == 0)
	{
		cef3Trace("No Active Requests");
		return false; //TODO: Handle better
	}

	tthread::lock_guard<tthread::mutex> guard(m_ResponseLock);
	m_vResponseList.push_back(info);

	m_WaitCond.notify_all();
	return true;
}

template <typename T>
void JavaScriptContextHelper<T>::newRequest(CefRefPtr<T> &pExtender, int nBrowserId, JSONNode jsonReq)
{
	if (jsonReq.find("command") == jsonReq.end())
	{
		cef3Trace("No Command in request");
		return; //TODO: Handle better
	}
		
	std::string strAction = jsonReq["command"].as_string();

	cef3Trace("Action: %s, Extender: %s, nBrowser: %d", strAction.c_str(), pExtender->getName(), nBrowserId);

	JSONNode res(JSON_NODE);
	res.set_name("response");

	if (strAction == "FunctionCall")
	{
		if (jsonReq.find("function") == jsonReq.end() || jsonReq.find("object") == jsonReq.end() || jsonReq.find("arguments") == jsonReq.end())
		{
			cef3Trace("Missing funciton || object || arguments for FunctionCall");
			return; //TODO: Handle better
		}
			
		std::string strFunction = jsonReq["function"].as_string();

		try
		{
			JavaScriptContextHandle<T> handle(nBrowserId);
			JSONNode ret = pExtender->execute(strFunction, jsonReq["object"], jsonReq["arguments"]);
			ret.set_name("result");

			res.push_back(JSONNode("command", "FunctionReturn"));
			res.push_back(ret);
		}
		catch (std::exception &e)
		{
			res.push_back(JSONNode("command", "FunctionException"));
			res.push_back(JSONNode("exception", e.what()));
		}
	
	}
	else if (strAction == "ObjectRequest")
	{
		if (jsonReq.find("function") == jsonReq.end() || jsonReq.find("object") == jsonReq.end() || jsonReq.find("arguments") == jsonReq.end())
		{
			cef3Trace("Missing funciton || object || arguments for ObjectRequest");
			return; //TODO: Handle better
		}

		JavaScriptContextHandle<T> handle(nBrowserId);
		JSONNode ret = pExtender->objectRequest(jsonReq["object"].as_string(), jsonReq["function"].as_string(), jsonReq["arguments"]);
		ret.set_name("result");

		res.push_back(JSONNode("command", "ObjectReturn"));
		res.push_back(ret);
	}

	JSONNode msg(JSON_NODE);
	msg.push_back(JSONNode("name", "JSE-Response"));
	msg.push_back(JSONNode("extender", pExtender->getName()));
	msg.push_back(JSONNode("browser", nBrowserId));
	msg.push_back(res);
	m_pTarget->send(nBrowserId, msg);
}


#endif //DESURA_JAVASCRIPTCONTEXT_H
