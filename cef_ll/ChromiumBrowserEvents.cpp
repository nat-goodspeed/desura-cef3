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


#include "ChromiumBrowserEvents.h"
#include "ChromiumBrowser.h"
#include "MenuInfo.h"

#include "Controller.h"

#include <sstream>
//#include "JavaScriptObject.h"
//#include "JavaScriptFactory.h"

extern int g_nApiVersion;

std::map<int, std::string> g_mErrorMsgMap;

class FillMap
{
public:
	FillMap()
	{
		g_mErrorMsgMap[ERR_FAILED] = "A generic failure occurred";
		g_mErrorMsgMap[ERR_ABORTED] = "An operation was aborted (due to user action)";
		g_mErrorMsgMap[ERR_INVALID_ARGUMENT] = "An argument to the function is incorrect";
		g_mErrorMsgMap[ERR_INVALID_HANDLE] = "The handle or file descriptor is invalid";
		g_mErrorMsgMap[ERR_FILE_NOT_FOUND] = "The file or directory cannot be found";
		g_mErrorMsgMap[ERR_TIMED_OUT] = "An operation timed out";
		g_mErrorMsgMap[ERR_FILE_TOO_BIG] = "The file is too large";
		g_mErrorMsgMap[ERR_UNEXPECTED] = "An unexpected error.  This may be caused by a programming mistake or an invalid assumption";
		g_mErrorMsgMap[ERR_ACCESS_DENIED] = "Permission to access a resource was denied";
		g_mErrorMsgMap[ERR_NOT_IMPLEMENTED] = "The operation failed because of unimplemented functionality";
		g_mErrorMsgMap[ERR_CONNECTION_CLOSED] = "A connection was closed (corresponding to a TCP FIN)";
		g_mErrorMsgMap[ERR_CONNECTION_RESET] = "A connection was reset (corresponding to a TCP RST)";
		g_mErrorMsgMap[ERR_CONNECTION_REFUSED] = "A connection attempt was refused";
		g_mErrorMsgMap[ERR_CONNECTION_ABORTED] = "A connection timed out as a result of not receiving an ACK for data sent";
		g_mErrorMsgMap[ERR_CONNECTION_FAILED] = "A connection attempt failed";
		g_mErrorMsgMap[ERR_NAME_NOT_RESOLVED] = "The host name could not be resolved";
		g_mErrorMsgMap[ERR_INTERNET_DISCONNECTED] = "The Internet connection has been lost";
		g_mErrorMsgMap[ERR_SSL_PROTOCOL_ERROR] = "An SSL protocol error occurred";
		g_mErrorMsgMap[ERR_ADDRESS_INVALID] = "The IP address or port number is invalid";
		g_mErrorMsgMap[ERR_ADDRESS_UNREACHABLE] = "The IP address is unreachable";
		g_mErrorMsgMap[ERR_SSL_CLIENT_AUTH_CERT_NEEDED] = "The server requested a client certificate for SSL client authentication";
		g_mErrorMsgMap[ERR_TUNNEL_CONNECTION_FAILED] = "A tunnel connection through the proxy could not be established";
		g_mErrorMsgMap[ERR_NO_SSL_VERSIONS_ENABLED] = "No SSL protocol versions are enabled";
		g_mErrorMsgMap[ERR_SSL_VERSION_OR_CIPHER_MISMATCH] = "The client and server don't support a common SSL protocol version or cipher suite";
		g_mErrorMsgMap[ERR_SSL_RENEGOTIATION_REQUESTED] = "The server requested a renegotiation (rehandshake)";
		g_mErrorMsgMap[ERR_CERT_COMMON_NAME_INVALID] = "The server responded with a certificate whose common name did not match the host name";
		g_mErrorMsgMap[ERR_CERT_DATE_INVALID] = "The server responded with a certificate that, by our clock, appears to either not yet be valid or to have expired";
		g_mErrorMsgMap[ERR_CERT_AUTHORITY_INVALID] = "The server responded with a certificate that is signed by an authority we don't trust";
		g_mErrorMsgMap[ERR_CERT_CONTAINS_ERRORS] = "The server responded with a certificate that contains errors";
		g_mErrorMsgMap[ERR_CERT_NO_REVOCATION_MECHANISM] = "The certificate has no mechanism for determining if it is revoked";
		g_mErrorMsgMap[ERR_CERT_UNABLE_TO_CHECK_REVOCATION] = "Revocation information for the security certificate for this site is not avaliable";
		g_mErrorMsgMap[ERR_CERT_REVOKED] = "The server responded with a certificate has been revoked";
		g_mErrorMsgMap[ERR_CERT_INVALID] = "The server responded with a certificate that is invalid";
		g_mErrorMsgMap[ERR_CERT_END] = "The value immediately past the last certificate error code";
		g_mErrorMsgMap[ERR_INVALID_URL] = "The URL is invalid";
		g_mErrorMsgMap[ERR_DISALLOWED_URL_SCHEME] = "The scheme of the URL is disallowed";
		g_mErrorMsgMap[ERR_UNKNOWN_URL_SCHEME] = "The scheme of the URL is unknown";
		g_mErrorMsgMap[ERR_TOO_MANY_REDIRECTS] = "Attempting to load an URL resulted in too many redirects";
		g_mErrorMsgMap[ERR_UNSAFE_REDIRECT] = "Attempting to load an URL resulted in an unsafe redirect";
		g_mErrorMsgMap[ERR_UNSAFE_PORT] = "Attempting to load an URL with an unsafe port number";
		g_mErrorMsgMap[ERR_INVALID_RESPONSE] = "The server's response was invalid";
		g_mErrorMsgMap[ERR_INVALID_CHUNKED_ENCODING] = "Error in chunked transfer encoding";
		g_mErrorMsgMap[ERR_METHOD_NOT_SUPPORTED] = "The server did not support the request method";
		g_mErrorMsgMap[ERR_UNEXPECTED_PROXY_AUTH] = "The response was 407 (Proxy Authentication Required), yet we did not send the request to a proxy";
		g_mErrorMsgMap[ERR_EMPTY_RESPONSE] = "The server closed the connection without sending any data";
		g_mErrorMsgMap[ERR_RESPONSE_HEADERS_TOO_BIG] = "The headers section of the response is too large";
		g_mErrorMsgMap[ERR_CACHE_MISS] = "The cache does not have the requested entry";
		g_mErrorMsgMap[ERR_INSECURE_RESPONSE] = "The server's response was insecure (e.g. there was a cert error)";
	}
};

FillMap fm;


/////////////////////////////////////////////////////////////////////////////////////////////
/// LifeSpanHandler
/////////////////////////////////////////////////////////////////////////////////////////////

void LifeSpanHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	if (browser->IsPopup())
		return;

	SetBrowser(browser);
}

void LifeSpanHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	if (GetBrowser() && GetBrowser()->GetHost()->GetWindowHandle() == 
		browser->GetHost()->GetWindowHandle())
		SetBrowser(NULL);
}

bool LifeSpanHandler::OnBeforePopup(CefRefPtr<CefBrowser> parentBrowser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& target_url,
                                    const CefString& target_frame_name,
                                    const CefPopupFeatures& popupFeatures,
                                    CefWindowInfo& windowInfo,
                                    CefRefPtr<CefClient>& client,
                                    CefBrowserSettings& settings,
                                    bool* no_javascript_access)
{
	//dont show popups unless its the inspector
	CefStringUTF8 t(ConvertToUtf8(target_url));

	cef3Trace("Url: %s", t.c_str());

	if (!t.empty() && std::string(t.c_str()).find("resources/inspector/devtools.") != std::string::npos)
		return false;

	if (GetCallbackV2())
	{
		if (GetCallbackV2()->onNewWindowUrl(t.c_str()))
			GetBrowser()->GetMainFrame()->LoadURL(target_url);
	}
	else
	{
		if (GetCallback()->onNavigateUrl(t.c_str(), true))
			GetBrowser()->GetMainFrame()->LoadURL(target_url);
	}
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// LifeSpanHandler
/////////////////////////////////////////////////////////////////////////////////////////////

void LoadHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
	if (GetCallback() && frame->IsMain())
		GetCallback()->onPageLoadStart();
}

void LoadHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	if (GetCallback() && frame->IsMain())
		GetCallback()->onPageLoadEnd();
}

void LoadHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
	if (errorCode == ERR_ABORTED)
		return;

	std::string errorMsg;
	std::map<int, std::string>::iterator it = g_mErrorMsgMap.find(errorCode);
	
	CefStringUTF8 strUrl = ConvertToUtf8(failedUrl);
	CefStringUTF8 et = ConvertToUtf8(errorText);
	cef3Trace("ErrId: %d, ErrMsg: [%s], Url: [%s]", errorCode, et.c_str(), strUrl.c_str());

	if (it != g_mErrorMsgMap.end())
	{
		std::stringstream stream;
		stream << g_mErrorMsgMap[errorCode] << " [" << errorCode << "]";
		errorMsg = stream.str();
	}
	else
	{
		std::stringstream stream;
		stream << "Error Code " << errorCode;
		errorMsg = stream.str();
	}

	std::string out;

	//if no frame its the whole page
	if (GetCallback())
	{
		const size_t size = 100*1024;
		char buff[size];
		buff[0] = 0;

		if (GetCallback()->onLoadError(errorMsg.c_str(), strUrl.c_str(), buff, size) && buff[0])
		{
			size_t nSize = strlen(buff);

			if (nSize > size)
				nSize = size;

			out = std::string(buff, nSize);
		}
	}

	if (out.empty())
	{
		// All other messages.
		std::stringstream ss;
		ss << "<html><head><title>Load Failed</title></head>"
			"<body><h1>Load Failed</h1>"
			"<h2>Load of URL " << strUrl.c_str() <<
			" failed: " << errorMsg <<
			".</h2></body>"
			"</html>";

		out = ss.str();
	}

	CefRefPtr<CefPostData> postData = CefPostData::Create();

	CefRefPtr<CefPostDataElement> el = CefPostDataElement::Create();
	el->SetToBytes(out.size(), out.c_str());

	postData->AddElement(el);

	CefRefPtr<CefRequest> req = CefRequest::Create();
	req->SetMethod("POST");
	req->SetURL("internal://loaderror");
	req->SetPostData(postData);
	frame->LoadRequest(req);
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// RequestHandler
/////////////////////////////////////////////////////////////////////////////////////////////


class ChromiumAuthCredentials : public ChromiumDLL::ChromiumAuthCredentialsI
{
public:
	ChromiumAuthCredentials(bool isProxy,
		const CefString& host,
		int port,
		const CefString& realm,
		const CefString& scheme,
		CefRefPtr<CefAuthCallback> callback)
		: m_bProxy(isProxy)
		, m_nPort(port)
		, m_strRealm(realm)
		, m_strScheme(scheme)
		, m_strHost(host)
		, m_Callback(callback)
	{
	}

	void cancel()
	{
		m_Callback->Cancel();
	}

	void procecced(const char* szUsername, const char* szPassword)
	{
		m_Callback->Continue(szUsername, szPassword);
	}

	int getPort()
	{
		return m_nPort;
	}

	const char* getRealm()
	{
		return m_strRealm.c_str();
	}

	const char* getScheme()
	{
		return m_strScheme.c_str();
	}

	const char* getHost()
	{
		return m_strHost.c_str();
	}

	bool isProxy()
	{
		return m_bProxy;
	}

private:
	bool m_bProxy;
	int m_nPort;

	const std::string m_strRealm;
	const std::string m_strScheme;
	const std::string m_strHost;

	CefRefPtr<CefAuthCallback> m_Callback;
	CEF3_IMPLEMENTREF_COUNTING(ChromiumAuthCredentials);
};

bool RequestHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool isRedirect)
{
	std::string url = request->GetURL();
	cef3Trace("Url: %s", url.c_str());

	if (url.find("internal://") == 0)
		return false;

	if (!GetCallback())
		return false;
	
	return !GetCallback()->onNavigateUrl(url.c_str(), frame->IsMain());
}

bool RequestHandler::GetAuthCredentials(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	bool isProxy,
	const CefString& host,
	int port,
	const CefString& realm,
	const CefString& scheme,
	CefRefPtr<CefAuthCallback> callback)
{
	if (!GetCallbackV2())
		return false;

	ChromiumDLL::RefPtr<ChromiumAuthCredentials> cred = new ChromiumAuthCredentials(isProxy, host, port, realm, scheme, callback);
	return GetCallbackV2()->getAuthCredentials(cred);
}

void RequestHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
	const CefString& url,
	bool& allow_os_execution)
{
	if (!GetCallbackV2())
		return;

	std::string u = url;
	allow_os_execution = GetCallbackV2()->onProtocolExecution(u.c_str());
}


/////////////////////////////////////////////////////////////////////////////////////////////
/// DownloadHandler
/////////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>

void DownloadHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
{
	std::string strUrl = download_item->GetURL();

	if (GetCallbackV2())
	{
		std::string strMimeType = download_item->GetMimeType();
		int64 contentLength = download_item->GetTotalBytes();
		
		GetCallbackV2()->onDownloadFile(strUrl.c_str(), strMimeType.c_str(), contentLength);
	}
		
	std::transform(strUrl.begin(), strUrl.end(), strUrl.begin(), ::tolower);

	if (strUrl.find_last_of(".swf") == strUrl.size() - 1)
		callback->Continue("", false);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/// DisplayHandler
/////////////////////////////////////////////////////////////////////////////////////////////

bool DisplayHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
	if (GetCallback())
	{
		CefStringUTF8 m(ConvertToUtf8(message));
		CefStringUTF8 s(ConvertToUtf8(source));
		GetCallback()->onLogConsoleMsg(m.c_str(), s.c_str(), line);
	}
		
	return true;
}

void DisplayHandler::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
{
	if (!GetCallbackV2())
		return;

	std::string strText = value;
	GetCallbackV2()->onStatus(strText.c_str(), ChromiumDLL::STATUSTYPE_TEXT);
}

void DisplayHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
	if (!GetCallbackV2())
		return;

	std::string strText = title;
	GetCallbackV2()->onTitle(strText.c_str());
}

bool DisplayHandler::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text)
{
	if (!GetCallbackV2())
		return false;

	std::string strText = text;
	return GetCallbackV2()->onToolTip(strText.c_str());
}


/////////////////////////////////////////////////////////////////////////////////////////////
/// KeyboardHandler
/////////////////////////////////////////////////////////////////////////////////////////////

// cef_key_event_type_t is NOT THE SAME as ChromiumDLL::KeyType
class KeyTypeFinder
{
public:
	KeyTypeFinder()
	{
		mMap[KEYEVENT_RAWKEYDOWN] = ChromiumDLL::KEYEVENT_RAWKEYDOWN;
		mMap[KEYEVENT_KEYDOWN]    = ChromiumDLL::KEYEVENT_KEYDOWN;
		mMap[KEYEVENT_KEYUP]      = ChromiumDLL::KEYEVENT_KEYUP;
		mMap[KEYEVENT_CHAR]       = ChromiumDLL::KEYEVENT_CHAR;
	}

	ChromiumDLL::KeyEventType find(cef_key_event_type_t type) const
	{
		KeyTypeMap::const_iterator found = mMap.find(type);
		if (found != mMap.end())
			return found->second;

		return ChromiumDLL::KeyEventType(); // invalid
	}

private:
	typedef std::map<cef_key_event_type_t, ChromiumDLL::KeyEventType> KeyTypeMap;
	KeyTypeMap mMap;
};
static const KeyTypeFinder keyfinder;

bool KeyboardHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser,
                                 const CefKeyEvent& event,
                                 CefEventHandle os_event)
{
	if (!GetCallback())
		return false;

	return GetCallback()->onKeyEvent(keyfinder.find(event.type),
									 event.unmodified_character, event.modifiers, !!event.is_system_key);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/// MenuHandler
/////////////////////////////////////////////////////////////////////////////////////////////


void ContextMenuHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	CefRefPtr<CefMenuModel> model)
{
	if (!GetCallback())
		return;

	ChromiumDLL::RefPtr<ChromiumDLL::ChromiumMenuInfoI> cmi = new ChromiumMenuInfo(params, GetBrowser()->GetHost()->GetWindowHandle());

	if (GetCallback()->handlePopupMenu(cmi))
		model->Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// JSDialogHandler
/////////////////////////////////////////////////////////////////////////////////////////////

bool JSDialogHandler::OnJSDialog(CefRefPtr<CefBrowser> browser,
	const CefString& origin_url,
	const CefString& accept_lang,
	JSDialogType dialog_type,
	const CefString& message_text,
	const CefString& default_prompt_text,
	CefRefPtr<CefJSDialogCallback> callback,
	bool& suppress_message)
{
	if (!GetCallback())
		return false;

	char resultBuff[255] = { 0 };
	bool success = false;
	bool res = false;

	CefStringUTF8 m(ConvertToUtf8(message_text));

	if (dialog_type == JSDIALOGTYPE_ALERT)
	{
		res = GetCallback()->onJScriptAlert(m.c_str());
		success = true;
	}
	else if (dialog_type == JSDIALOGTYPE_CONFIRM)
	{
		res = GetCallback()->onJScriptConfirm(m.c_str(), &success);
	}
	else if (dialog_type == JSDIALOGTYPE_PROMPT)
	{
		CefStringUTF8 d(ConvertToUtf8(default_prompt_text));
		res = GetCallback()->onJScriptPrompt(m.c_str(), d.c_str(), &success, resultBuff);
	}

	if (res)
		callback->Continue(success, resultBuff);

	return res;
}



/////////////////////////////////////////////////////////////////////////////////////////////
/// RenderHandler
/////////////////////////////////////////////////////////////////////////////////////////////

class PaintInfo : public ChromiumDLL::ChromiumPaintInfo
{
public:
	PaintInfo(const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height)
	{
		windowRegion.x = 0;
		windowRegion.y = 0;
		windowRegion.w = width;
		windowRegion.h = height;

		this->buffer = buffer;

		if (dirtyRects.size() == 1 && dirtyRects[0].x == 0 && dirtyRects[0].y == 0 && dirtyRects[0].width == width && dirtyRects[0].height == height)
		{
			regionCount = 0;
			invalidatedRegions = NULL;
		}
		else
		{
			regionCount = dirtyRects.size();

			for (int x = 0; x < regionCount; ++x)
			{
				ChromiumDLL::ChromiumRegionInfo ri;
				ri.x = dirtyRects[x].x;
				ri.y = dirtyRects[x].y;
				ri.w = dirtyRects[x].width;
				ri.h = dirtyRects[x].height;

				m_vRegionInfo.push_back(ri);
			}

			invalidatedRegions = &m_vRegionInfo[0];
		}
	}

private:
	std::vector<ChromiumDLL::ChromiumRegionInfo> m_vRegionInfo;

	CEF3_IMPLEMENTREF_COUNTING(PaintInfo);
};

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
	PaintElementType type,
	const RectList& dirtyRects,
	const void* buffer,
	int width, int height)
{
	ChromiumDLL::RefPtr<ChromiumDLL::ChromiumPaintInfo> info = new PaintInfo(dirtyRects, buffer, width, height);

	if (type == PET_VIEW)
	{
		if (GetRenderCallback())
			GetRenderCallback()->onPaint(info);
	}
	else if (type == PET_POPUP)
	{
		if (GetRenderPopupCallback())
			GetRenderPopupCallback()->onPUPaint(info);
	}
}


void RenderHandler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
	if (!GetRenderPopupCallback())
		return;

	if (show)
		GetRenderPopupCallback()->onShow();
	else
		GetRenderPopupCallback()->onHide();
}

void RenderHandler::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
	if (!GetRenderPopupCallback())
		return;

	GetRenderPopupCallback()->onResize(rect.x, rect.y, rect.width, rect.height);
}



void RenderHandler::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor)
{
#ifdef _WIN32
	static HCURSOR s_Hand = LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND));
	static HCURSOR s_IBeam = LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM));
	static HCURSOR s_SizeWestEast = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE));
	static HCURSOR s_SizeNorthSouth = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
	static HCURSOR s_SizeAll = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL));

	ChromiumDLL::eCursor ec = ChromiumDLL::CURSOR_NORMAL;

	if (cursor == s_Hand)
		ec = ChromiumDLL::CURSOR_HAND;
	else if (cursor == s_IBeam)
		ec = ChromiumDLL::CURSOR_IBEAM;
	else if (cursor == s_SizeWestEast)
		ec = ChromiumDLL::CURSOR_HAND;
	else if (cursor == s_SizeNorthSouth)
		ec = ChromiumDLL::CURSOR_IBEAM;
	else if (cursor == s_SizeAll)
		ec = ChromiumDLL::CURSOR_HAND;

	if (GetRenderCallback())
		GetRenderCallback()->onCursorChange(ec);
#endif
}

bool RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
	if (GetRenderCallback())
		return GetRenderCallback()->getViewRect(rect.x, rect.y, rect.width, rect.height);

	return false;
}



/////////////////////////////////////////////////////////////////////////////////////////////
/// RenderProcessHandler
/////////////////////////////////////////////////////////////////////////////////////////////

void RenderProcessHandler::OnRenderThreadCreated(CefRefPtr<CefListValue> extra_info)
{
	


}



/////////////////////////////////////////////////////////////////////////////////////////////
/// ChromiumBrowserEvents
/////////////////////////////////////////////////////////////////////////////////////////////

#include "Controller.h"

extern ChromiumController* g_Controller;

ChromiumBrowserEvents::ChromiumBrowserEvents(ChromiumBrowser* pParent)
{
	m_pParent = pParent;
	m_pEventCallBack = NULL;
	m_pRendereEventCallBack = NULL;
}

bool ChromiumBrowserEvents::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	return false;
}

void ChromiumBrowserEvents::setCallBack(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserEventI>& cbe)
{
	m_pEventCallBack = cbe;
}

void ChromiumBrowserEvents::setCallBack(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererEventI>& cbe)
{
	m_pRendereEventCallBack = cbe;
}

void ChromiumBrowserEvents::setCallBack(const ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererPopupEventI>& cbe)
{
	m_pRenderePopupEventCallBack = cbe;
}

void ChromiumBrowserEvents::setParent(ChromiumBrowser* parent)
{
	m_pParent = parent;
}

ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserEventI> ChromiumBrowserEvents::GetCallback()
{
	return m_pEventCallBack;
}

ChromiumDLL::RefPtr<ChromiumDLL::ChromiumBrowserEventI_V2> ChromiumBrowserEvents::GetCallbackV2()
{
	if (!GetCallback())
		return NULL;

	if (g_nApiVersion <= 1)
		return NULL;

	if (GetCallback()->ApiVersion() <= 1)
		return NULL;

	return static_cast<ChromiumDLL::ChromiumBrowserEventI_V2*>(GetCallback().get());
}

ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererEventI> ChromiumBrowserEvents::GetRenderCallback()
{
	return m_pRendereEventCallBack;
}

ChromiumDLL::RefPtr<ChromiumDLL::ChromiumRendererPopupEventI> ChromiumBrowserEvents::GetRenderPopupCallback()
{
	return m_pRenderePopupEventCallBack;
}


void ChromiumBrowserEvents::SetBrowser(CefRefPtr<CefBrowser> browser)
{
	m_Browser = browser;

	if (m_pParent)
		m_pParent->setBrowser(browser);
}

CefRefPtr<CefBrowser> ChromiumBrowserEvents::GetBrowser()
{
	return m_Browser;
}

void ChromiumBrowserEvents::setContext(CefRefPtr<CefV8Context> context)
{
	m_pParent->setContext(context);
}


