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


#ifndef THIRDPARTY_CEF3_HEADER
#define THIRDPARTY_CEF3_HEADER
#ifdef _WIN32
#pragma once
#endif

#ifdef WIN32
#include <winsock2.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#endif

#include <stdlib.h>

#if defined(_WIN32)

#ifdef BUILDING_CEF_DESURA_SHARED 
#ifndef BUILDING_3P_CEF3_SHARED
#define BUILDING_3P_CEF3_SHARED
#endif
#endif

#ifdef BUILDING_3P_CEF3_SHARED
#define DLLINTERFACE __declspec(dllexport)
#else
#define DLLINTERFACE __declspec(dllimport)
#endif
#else
#define DLLINTERFACE __attribute__ ((visibility("default")))
#endif

#ifdef WIN32
#define OVERRIDE override
#else
#define OVERRIDE
#endif

#ifdef CHROMIUM_API_SUPPORTS_V2
#define USE_CHROMIUM_API_V2
#endif

namespace ChromiumDLL
{
	template <typename T>
	class RefPtr
	{
	public:
		RefPtr()
			: m_pT(NULL)
		{
		}

		RefPtr(T *pT)
			: m_pT(pT)
		{
			if (m_pT)
				m_pT->addRef();
		}

		RefPtr(const RefPtr<T>& r)
			: m_pT(r.m_pT)
		{
			if (m_pT)
				m_pT->addRef();
		}

		~RefPtr()
		{
			if (m_pT)
				m_pT->delRef();
		}

		T* operator ->() const
		{
			return m_pT;
		}

		operator bool() const
		{
			return !!m_pT;
		}

		template <typename U>
		operator RefPtr<U>() const
		{
			return m_pT;
		}

		RefPtr<T>& operator=(const RefPtr<T>& r)
		{
			if (m_pT)
				m_pT->delRef();

			m_pT = r.m_pT;

			if (m_pT)
				m_pT->addRef();

			return *this;
		}

		T* get() const
		{
			return m_pT;
		}

		void reset()
		{
			if (m_pT)
				m_pT->delRef();

			m_pT = NULL;
		}

	private:
		T *m_pT;
	};

	class IntrusiveRefPtrI
	{
	public:
		virtual void addRef() = 0;
		virtual void delRef() = 0;

	protected:
		virtual ~IntrusiveRefPtrI(){};

		virtual void destroy()
		{
			delete this;
		}
	};


	enum KeyEventType
	{
		KEYEVENT_RAWKEYDOWN = 0,
		KEYEVENT_KEYDOWN,
		KEYEVENT_KEYUP,
		KEYEVENT_CHAR
	};

	class JavaScriptObjectI;
	class ChromiumBrowserI;
	class JavaScriptExtenderI;
	class JavaScriptFunctionArgs;

	typedef RefPtr<JavaScriptObjectI> JSObjHandle;

	class JavaScriptFactoryI : public IntrusiveRefPtrI
	{
	public:
		virtual RefPtr<JavaScriptObjectI> CreateUndefined() = 0;
		virtual RefPtr<JavaScriptObjectI> CreateNull() = 0;
		virtual RefPtr<JavaScriptObjectI> CreateBool(bool value) = 0;
		virtual RefPtr<JavaScriptObjectI> CreateInt(int value) = 0;
		virtual RefPtr<JavaScriptObjectI> CreateDouble(double value) = 0;
		virtual RefPtr<JavaScriptObjectI> CreateString(const char* value) = 0;
		virtual RefPtr<JavaScriptObjectI> CreateObject() = 0;
		virtual RefPtr<JavaScriptObjectI> CreateObject(const RefPtr<IntrusiveRefPtrI>& userData) = 0;
		virtual RefPtr<JavaScriptObjectI> CreateException(const char* value) = 0;
		virtual RefPtr<JavaScriptObjectI> CreateArray() = 0;
		virtual RefPtr<JavaScriptObjectI> CreateFunction(const char* name, const RefPtr<ChromiumDLL::JavaScriptExtenderI>& handler) = 0;
	};

	class JavaScriptContextI : public IntrusiveRefPtrI
	{
	public:
		//! Clone the context
		//!
		virtual RefPtr<ChromiumDLL::JavaScriptContextI> clone() = 0;

		//! Enter context. Must be called on CEF UI thread
		//!
		virtual void enter() = 0;

		//! Exit context. Must be called on CEF UI thread after enter
		//!
		virtual void exit() = 0;

		//! only valid after enter has been called
		//!
		virtual RefPtr<ChromiumDLL::JavaScriptFactoryI> getFactory() = 0;

		//! Gets the current global object for this context. Only valid after enter has been called
		//!
		virtual RefPtr<JavaScriptObjectI> getGlobalObject() = 0;
	};

	class JavaScriptObjectI : public IntrusiveRefPtrI
	{
	public:
		//! Clones this object. Must call destroy once done.
		//!
		//! @return JSObject Clone
		//!
		virtual RefPtr<ChromiumDLL::JavaScriptObjectI> clone() = 0;

		//! Undefined JSObject
		//!
		virtual bool isUndefined() = 0;

		//! Null JSObject
		//!
		virtual bool isNull() = 0;

		//! Bool JSObject
		//!
		virtual bool isBool() = 0;

		//! Int JSObject
		//!
		virtual bool isInt() = 0;

		//! Double JSObject
		//!
		virtual bool isDouble() = 0;

		//! String JSObject
		//!
		virtual bool isString() = 0;

		//! Object JSObject
		//!
		virtual bool isObject() = 0;

		//! Array JSObject
		//!
		virtual bool isArray() = 0;

		//! Function JSObject
		//!
		virtual bool isFunction() = 0;

		//! Is this an exception
		//!
		virtual bool isException() = 0;

		virtual bool getBoolValue() = 0;
		virtual int getIntValue() = 0;
		virtual double getDoubleValue() = 0;

		//! @return String proper size
		virtual int getStringValue(char* buff, size_t buffsize) = 0;

		// OBJECT METHODS - These methods are only available on objects. Arrays and
		// functions are also objects. String- and integer-based keys can be used
		// interchangably with the framework converting between them as necessary.
		// Keys beginning with "Cef::" and "v8::" are reserved by the system.

		// Returns true if the object has a value with the specified identifier.
		virtual bool hasValue(const char* key) = 0;
		virtual bool hasValue(int index) = 0;

		// Delete the value with the specified identifier.
		virtual bool deleteValue(const char* key) = 0;
		virtual bool deleteValue(int index) = 0;

		//! Returns the value with the specified identifier. 
		virtual RefPtr<JavaScriptObjectI> getValue(const char* key) = 0;
		virtual RefPtr<JavaScriptObjectI> getValue(int index) = 0;

		//! Associate value with the specified identifier.
		virtual bool setValue(const char* key, RefPtr<JavaScriptObjectI> value) = 0;
		virtual bool setValue(int index, RefPtr<JavaScriptObjectI> value) = 0;

		virtual int getNumberOfKeys() = 0;
		virtual void getKey(int index, char* buff, size_t buffsize) = 0;

		// ARRAY METHODS - These methods are only available on arrays.
		// Returns the number of elements in the array.
		virtual int getArrayLength() = 0;


		// FUNCTION METHODS - These methods are only available on functions.
		// Returns the function name.
		virtual void getFunctionName(char* buff, size_t buffsize) = 0;

		//! Returns the function handler or NULL if not a CEF-created function. 
		//! Must call destroy once done!
		virtual RefPtr<ChromiumDLL::JavaScriptExtenderI> getFunctionHandler() = 0;

		//! executes a function.
		//! Must call destroy once done!
		//! args doesnt use function name, or factory
		virtual RefPtr<JavaScriptObjectI> executeFunction(const RefPtr<ChromiumDLL::JavaScriptFunctionArgs>& args) = 0;

		virtual void addRef() = 0;
		virtual void delRef() = 0;

		virtual RefPtr<IntrusiveRefPtrI> getUserObject() = 0;

		template <typename T>
		RefPtr<T> getUserObject()
		{
			return (T*)getUserObject().get();
		}
	};

	class JavaScriptFunctionArgs : public IntrusiveRefPtrI
	{
	public:
		const char* function;
		int argc;

		ChromiumDLL::RefPtr<JavaScriptObjectI> object;
		ChromiumDLL::RefPtr<JavaScriptObjectI>* argv;	//<< array
		RefPtr<ChromiumDLL::JavaScriptFactoryI> factory;
		RefPtr<ChromiumDLL::JavaScriptContextI> context;
	};


	class JavaScriptExtenderI : public IntrusiveRefPtrI
	{
	public:
		//! Clones this Extender. Must call destroy once done.
		//!
		//! @return JSExtender Clone
		//!
		virtual RefPtr<ChromiumDLL::JavaScriptExtenderI> clone() = 0;


		//! Called when a javascript function is called
		//! Can chuck std::exception
		//!
		//! @param factory Javascript object factory
		//! @parma function Function name
		//! @param object Javascript object that function was called on. Can be NULL
		//! @param argc Number of args
		//! @param argv Args
		//! @return Null if not handled, JavaScriptObjectI Undefined if no return or a JavaScriptObjectI
		//!
		virtual RefPtr<JavaScriptObjectI> execute(const RefPtr<ChromiumDLL::JavaScriptFunctionArgs>& args) = 0;

		//! Gets the name to register the extension. i.e. "v8/test"
		//!
		//! @return Extension name
		//!
		virtual const char* getName() = 0;


		// Gets the registration javascript
		// Functions implemented by the handler are prototyped using the
		// keyword 'native'. The calling of a native function is restricted to the scope
		// in which the prototype of the native function is defined.
		//
		// Example JavaScript extension code:
		//
		//   // create the 'example' global object if it doesn't already exist.
		//   if (!example)
		//     example = {};
		//   // create the 'example.test' global object if it doesn't already exist.
		//   if (!example.test)
		//     example.test = {};
		//   (function() {
		//     // Define the function 'example.test.myfunction'.
		//     example.test.myfunction = function() {
		//       // Call CefV8Handler::Execute() with the function name 'MyFunction'
		//       // and no arguments.
		//       native function MyFunction();
		//       return MyFunction();
		//     };
		//     // Define the getter function for parameter 'example.test.myparam'.
		//     example.test.__defineGetter__('myparam', function() {
		//       // Call CefV8Handler::Execute() with the function name 'GetMyParam'
		//       // and no arguments.
		//       native function GetMyParam();
		//       return GetMyParam();
		//     });
		//     // Define the setter function for parameter 'example.test.myparam'.
		//     example.test.__defineSetter__('myparam', function(b) {
		//       // Call CefV8Handler::Execute() with the function name 'SetMyParam'
		//       // and a single argument.
		//       native function SetMyParam();
		//       if(b) SetMyParam(b);
		//     });
		//
		//     // Extension definitions can also contain normal JavaScript variables
		//     // and functions.
		//     var myint = 0;
		//     example.test.increment = function() {
		//       myint += 1;
		//       return myint;
		//     };
		//   })();
		//
		// Example usage in the page:
		//
		//   // Call the function.
		//   example.test.myfunction();
		//   // Set the parameter.
		//   example.test.myparam = value;
		//   // Get the parameter.
		//   value = example.test.myparam;
		//   // Call another function.
		//   example.test.increment();
		//
		virtual const char* getRegistrationCode() = 0;
	};

	class CookieI : public IntrusiveRefPtrI
	{
	public:

		virtual void SetDomain(const char* domain) = 0;
		virtual void SetName(const char* name) = 0;
		virtual void SetData(const char* data) = 0;
		virtual void SetPath(const char* path) = 0;
	};

	class PostElementI : public IntrusiveRefPtrI
	{
	public:
		virtual bool isFile() = 0;
		virtual bool isBytes() = 0;

		virtual void setToEmpty() = 0;
		virtual void setToFile(const char* fileName) = 0;
		virtual void setToBytes(size_t size, const void* bytes) = 0;

		virtual void getFile(char *buff, size_t buffsize) = 0;

		virtual size_t getBytesCount() = 0;
		virtual size_t getBytes(size_t size, void* bytes) = 0;
	};

	class PostDataI : public IntrusiveRefPtrI
	{
	public:
		virtual size_t getElementCount() = 0;
		virtual RefPtr<PostElementI> getElement(size_t index) = 0;

		virtual bool removeElement(const RefPtr<PostElementI>& element) = 0;
		virtual bool addElement(const RefPtr<PostElementI>& element) = 0;

		virtual void removeElements() = 0;
	};

	class SchemeRequestI : public IntrusiveRefPtrI
	{
	public:
		virtual void getURL(char *buff, size_t buffsize) = 0;
		virtual void setURL(const char* url) = 0;

		virtual void getMethod(char *buff, size_t buffsize) = 0;
		virtual void setMethod(const char* method) = 0;

		virtual RefPtr<PostDataI> getPostData() = 0;
		virtual void setPostData(const RefPtr<PostDataI>& postData) = 0;

		virtual size_t getHeaderCount() = 0;

		virtual void getHeaderItem(size_t index, char *key, size_t keysize, char* data, size_t datasize) = 0;
		virtual void setHeaderItem(const char* key, const char* data) = 0;

		virtual void set(const char* url, const char* method, const RefPtr<PostDataI>& postData) = 0;
	};

	class SchemeExtenderI : public IntrusiveRefPtrI
	{
	public:
		virtual RefPtr<SchemeExtenderI> clone(const char* schemeName) = 0;

		virtual const char* getSchemeName() = 0;
		virtual const char* getHostName() = 0;

		//! Processes the request. Call response ready when ready to reply
		//! Set redirect to true to redirect to another url (read from getRedirectUrl)
		//! 
		virtual bool processRequest(const RefPtr<SchemeRequestI>& request, bool* redirect) = 0;

		//! Called when response is ready
		//!
		virtual size_t getResponseSize() = 0;

		//! Return NULL to use default
		virtual const char* getResponseMimeType() = 0;

		//! Return NULL to cancel redirect
		virtual const char* getRedirectUrl() = 0;

		//! Return false to cancel read
		//! Set readSize to zero and return true to wait for callback
		//! 
		virtual bool read(char* buffer, int size, int* readSize) = 0;

		//! Cancel request
		//!
		virtual void cancel() = 0;
	};


	class FunctionArgI : public IntrusiveRefPtrI
	{
	public:
		virtual void setBool(bool value) = 0;
		virtual void setInt(int value) = 0;
		virtual void setDouble(double value) = 0;
		virtual void setString(const char* value) = 0;
		virtual void setNull() = 0;
		virtual void setVoid() = 0;

		virtual bool getBool() = 0;
		virtual int getInt() = 0;
		virtual double getDouble() = 0;
		virtual void getString(char* buff, size_t buffsize) = 0;

		virtual bool isBool() = 0;
		virtual bool isInt() = 0;
		virtual bool isDouble() = 0;
		virtual bool isString() = 0;
		virtual bool isNull() = 0;
		virtual bool isVoid() = 0;
	};


	class FunctionArgsI : public IntrusiveRefPtrI
	{
	public:
		virtual size_t getCount() = 0;
		virtual RefPtr<FunctionArgI> getArg(size_t index) = 0;
	};



	class FunctionDelegateI : public IntrusiveRefPtrI
	{
	public:
		virtual void operator()(const RefPtr<FunctionArgsI>& args, const RefPtr<FunctionArgI>& result) = 0;
	};

	template <class T>
	class FunctionDelegate : public FunctionDelegateI
	{
	public:
		typedef void (T::*JSItemFunction)(const RefPtr<FunctionArgsI>&, const RefPtr<FunctionArgsI>&);

		FunctionDelegate(const RefPtr<T> &t, JSItemFunction function)
		{
			m_pItem = t;
			m_pFunction = function;
		}

		void operator()(const RefPtr<FunctionArgsI>& args, const RefPtr<FunctionArgI>& result)
		{
			return (*m_pItem.get().*m_pFunction)(args, result);
		}

	private:
		RefPtr<T> m_pItem;
		JSItemFunction m_pFunction;
	};

	template <class TObj>
	RefPtr<FunctionDelegateI> newFunctionDelegate(const RefPtr<TObj>& pObj, void (TObj::*function)(const RefPtr<FunctionArgsI>&, const RefPtr<FunctionArgsI>&))
	{
		return new FunctionDelegate<TObj>(pObj, function);
	}


	class FunctionRegisterI : public IntrusiveRefPtrI
	{
	public:
		virtual void registerFunction(const char* name, const RefPtr<FunctionDelegateI>& delegate) = 0;
	};

	class ChromiumMenuItemI : public IntrusiveRefPtrI
	{
	public:
		enum TypeFlags
		{
			MENUITEM_SEPERATOR = 1,
			MENUITEM_OPTION,
			MENUITEM_CHECKABLEOPTION,
			MENUITEM_GROUP,
		};

		virtual int getAction() = 0;
		virtual int getType() = 0;
		virtual const char* getLabel() = 0;

		virtual bool isEnabled() = 0;
		virtual bool isChecked() = 0;
	};

	class ChromiumMenuInfoI : public IntrusiveRefPtrI
	{
	public:
		enum TypeFlags
		{
			// No node is selected
			MENUTYPE_NONE = 0x0,
			// The top page is selected
			MENUTYPE_PAGE = 0x1,
			// A subframe page is selected
			MENUTYPE_FRAME = 0x2,
			// A link is selected
			MENUTYPE_LINK = 0x4,
			// An image is selected
			MENUTYPE_IMAGE = 0x8,
			// There is a textual or mixed selection that is selected
			MENUTYPE_SELECTION = 0x10,
			// An editable element is selected
			MENUTYPE_EDITABLE = 0x20,
			// A misspelled word is selected
			MENUTYPE_MISSPELLED_WORD = 0x40,
			// A video node is selected
			MENUTYPE_VIDEO = 0x80,
			// A video node is selected
			MENUTYPE_AUDIO = 0x100,
			MENUTYPE_FILE = 0x200,
			MENUTYPE_PLUGIN = 0x400,
		};

		enum EditFlags
		{
			MENU_CAN_DO_NONE = 0x0,
			MENU_CAN_UNDO = 0x1,
			MENU_CAN_REDO = 0x2,
			MENU_CAN_CUT = 0x4,
			MENU_CAN_COPY = 0x8,
			MENU_CAN_PASTE = 0x10,
			MENU_CAN_DELETE = 0x20,
			MENU_CAN_SELECT_ALL = 0x40,
			MENU_CAN_TRANSLATE = 0x80,
			MENU_CAN_GO_FORWARD = 0x10000000,
			MENU_CAN_GO_BACK = 0x20000000,
		};

		virtual TypeFlags getTypeFlags() = 0;
		virtual EditFlags getEditFlags() = 0;

		virtual void getMousePos(int* x, int* y) = 0;

		virtual const char* getLinkUrl() = 0;
		virtual const char* getImageUrl() = 0;
		virtual const char* getPageUrl() = 0;
		virtual const char* getFrameUrl() = 0;
		virtual const char* getSelectionText() = 0;
		virtual const char* getMisSpelledWord() = 0;
		virtual const char* getSecurityInfo() = 0;

		virtual int getCustomCount() = 0;
		virtual RefPtr<ChromiumMenuItemI> getCustomItem(size_t index) = 0;

		virtual int* getHWND() = 0;
	};

	class ChromiumBrowserEventI : public IntrusiveRefPtrI
	{
	public:
		//! Before browser loads a new url. Return false to stop
		//!
		//! @param url Url of new page 
		//! @param isMain Is this loading on the main page or loading in an iframe/resource
		//! @return true to continue, false to stop
		//!
		virtual bool onNavigateUrl(const char* url, bool isMain) = 0;

		//! When a page starts loading
		//!
		virtual void onPageLoadStart() = 0;

		//! When a page finishes loading
		//!
		virtual void onPageLoadEnd() = 0;

		//! Show a javascript Alert Box
		//!
		//! @param msg Message to show
		//! @return True if handled, false to show default
		//!
		virtual bool onJScriptAlert(const char* msg) = 0;

		//! Show a javascript Confirm Box
		//!
		//! @param msg Message to show
		//! @param result Result of confirm
		//! @return True if handled, false to show default
		//!
		virtual bool onJScriptConfirm(const char* msg, bool* result) = 0;

		//! Show a javascript Alert
		//!
		//! @param msg Message to show
		//! @return True if handled, false to show default
		//!
		virtual bool onJScriptPrompt(const char* msg, const char* defualtVal, bool* handled, char result[255]) = 0;

		//! When a key is pressed
		//!
		//! @param type is the type of keyboard event.
		//! @param code is the windows scan-code for the event.
		//! @param modifiers is a set of bit-flags describing any pressed modifier keys.
		//! @param isSystemKey is set if Windows considers this a 'system key' message;
		//   (see http://msdn.microsoft.com/en-us/library/ms646286(VS.85).aspx)
		//! @return True if handled, false for default handling
		//!
		virtual bool onKeyEvent(KeyEventType type, int code, int modifiers, bool isSystemKey) = 0;


		//Logs a console message
		virtual void onLogConsoleMsg(const char* message, const char* source, int line) = 0;

		virtual void launchLink(const char* url) = 0;

		//! When a page fails to load you can return a custom error page into buff with max size size
		//! 
		//! @param errorCode Code of error
		//! @param url Url error happened on
		//! @param buff Buff to save response into
		//! @param size Max buffer size
		//! 
		//! @return true if handled, false if not
		//!
		virtual bool onLoadError(const char* errorMsg, const char* url, char* buff, size_t size) = 0;
		virtual bool handlePopupMenu(const RefPtr<ChromiumMenuInfoI>& menuInfo) = 0;

#ifdef CHROMIUM_API_SUPPORTS_V2
		virtual int ApiVersion()
		{
			return 1;
		}
#endif
	};

	enum StatusType
	{
		STATUSTYPE_TEXT = 0,
		STATUSTYPE_MOUSEOVER_URL,
		STATUSTYPE_KEYBOARD_FOCUS_URL,
	};

	class ChromiumAuthCredentialsI : public IntrusiveRefPtrI
	{
	public:
		virtual void cancel() = 0;
		virtual void procecced(const char* szUsername, const char* szPassword)=0;

		virtual int getPort() = 0;
		virtual const char* getRealm() = 0;
		virtual const char* getScheme() = 0;
		virtual const char* getHost() = 0;

		virtual bool isProxy() = 0;
	};

	class ChromiumBrowserEventI_V2 : public ChromiumBrowserEventI
	{
	public:
		virtual int ApiVersion()
		{
			return 2;
		}

		//! Callback when a file wants to be downloaded
		//!
		//! @param szUrl Full url to file
		//! @param szMimeType Mime Type of file provided by server
		//! @param ullFileSize Download file size, will be -1 if unknown
		//!
		virtual void onDownloadFile(const char* szUrl, const char* szMimeType, unsigned long long ullFileSize) = 0;

		virtual void onStatus(const char* szStatus, StatusType eType) = 0;

		virtual void onTitle(const char* szTitle) = 0;

		//! Return false to show default tool tip
		virtual bool onToolTip(const char* szToolTop) = 0;

		//! Called when a url wants to open in new window, return true to open in current browser, false to ignore
		virtual bool onNewWindowUrl(const char* url) = 0;

		//! Return false to cancel else call procecced/cancel when ready
		virtual bool getAuthCredentials(const RefPtr<ChromiumAuthCredentialsI>& info) = 0;

		//! return false to cancel, true to pass to os to handle
		virtual bool onProtocolExecution(const char* url) = 0;
	};

	enum KeyType
	{
		KT_RAWKEYDOWN = 0,
		KT_KEYDOWN,
		KT_KEYUP,
		KT_CHAR,
	};

	enum KeyModifiers
	{
		KM_NONE = 0,
		KM_CAPS_LOCK_ON = 1 << 0,
		KM_SHIFT_DOWN = 1 << 1,
		KM_CONTROL_DOWN = 1 << 2,
		KM_ALT_DOWN = 1 << 3,
		KM_LEFT_MOUSE_BUTTON = 1 << 4,
		KM_MIDDLE_MOUSE_BUTTON = 1 << 5,
		KM_RIGHT_MOUSE_BUTTON = 1 << 6,
		// Mac OS-X command key.
		KM_COMMAND_DOWN = 1 << 7,
		KM_NUM_LOCK_ON = 1 << 8,
		KM_IS_KEY_PAD = 1 << 9,
		KM_IS_LEFT = 1 << 10,
		KM_IS_RIGHT = 1 << 11,
	};

	enum MouseButtonType
	{
		MBT_LEFT = 0,
		MBT_MIDDLE,
		MBT_RIGHT,
	};

	class ChromiumBrowserI : public IntrusiveRefPtrI
	{
	public:
		virtual void onFocus() = 0;

#ifdef _WIN32
		virtual void onPaintBg() = 0;
		virtual void onPaint() = 0;
		virtual void onResize() = 0;
#else
		virtual void onResize(int x, int y, int width, int height) = 0;
#endif

		virtual void loadUrl(const char* url) = 0;
		virtual void loadString(const char* string) = 0;

		virtual void stop() = 0;
		virtual void refresh(bool ignoreCache = false) = 0;
		virtual void back() = 0;
		virtual void forward() = 0;

		virtual void zoomIn() = 0;
		virtual void zoomOut() = 0;
		virtual void zoomNormal() = 0;

		virtual void print() = 0;
		virtual void viewSource() = 0;

		virtual void undo() = 0;
		virtual void redo() = 0;
		virtual void cut() = 0;
		virtual void copy() = 0;
		virtual void paste() = 0;
		virtual void del() = 0;
		virtual void selectall() = 0;

		virtual void setEventCallback(const RefPtr<ChromiumBrowserEventI>& cbe) = 0;
		virtual void executeJScript(const char* code, const char* scripturl = 0, int startline = 0) = 0;

		virtual void showInspector() = 0;
		virtual void hideInspector() = 0;
		virtual void inspectElement(int x, int y) = 0;

		virtual void scroll(int x, int y, int deltaX, int deltaY) = 0;

		virtual int* getBrowserHandle() = 0;
		virtual RefPtr<JavaScriptContextI> getJSContext() = 0;
	};

	enum eCursor
	{
		CURSOR_NORMAL,
		CURSOR_HAND,
		CURSOR_IBEAM,
		CURSOR_SIZE_WE,
		CURSOR_SIZE_NS,
		CURSOR_SIZE_ALL
	};


	class ChromiumRegionInfo
	{
	public:
		unsigned int x;
		unsigned int y;
		unsigned int w;
		unsigned int h;
	};

	class ChromiumPaintInfo : public IntrusiveRefPtrI
	{
	public:
		ChromiumRegionInfo windowRegion;
		const void* buffer;
		int regionCount;
		ChromiumRegionInfo* invalidatedRegions;
	};

	class ChromiumRendererEventI : public IntrusiveRefPtrI
	{
	public:
		virtual int apiVersion()
		{
			return 1;
		}

		virtual void onPaint(const RefPtr<ChromiumPaintInfo>& info) = 0;

		virtual void onCursorChange(eCursor cursor) = 0;

		virtual bool getViewRect(int &x, int &y, int &w, int &h) = 0;
	};

	class ChromiumRendererPopupEventI : public IntrusiveRefPtrI
	{
	public:
		virtual int apiVersion()
		{
			return 1;
		}

		virtual void onPUPaint(const RefPtr<ChromiumPaintInfo>& info) = 0;

		virtual void onShow() = 0;

		virtual void onHide() = 0;

		virtual void onResize(int x, int y, int w, int h) = 0;
	};

	class ChromiumKeyPressI : public IntrusiveRefPtrI
	{
	public:
		virtual KeyType getType() = 0;

		// The actual key code generated by the platform.
		virtual int getNativeCode() = 0;

		// The character generated by the keystroke.
		virtual int getCharacter() = 0;

		// Same as |character| but unmodified by any concurrently-held modifiers
		// (except shift). This is useful for working out shortcut keys.
		virtual int getUnModCharacter() = 0;

		virtual KeyModifiers getModifiers() = 0;

#ifdef WIN32
		// The Windows key code for the key event. This value is used by the DOM
		// specification. Sometimes it comes directly from the event (i.e. on
		// Windows) and sometimes it's determined using a mapping function. See
		// WebCore/platform/chromium/KeyboardCodes.h for the list of values.
		virtual int getWinKeyCode() = 0;

		// Indicates whether the event is considered a "system key" event (see
		// http://msdn.microsoft.com/en-us/library/ms646286(VS.85).aspx for details).
		// This value will always be false on non-Windows platforms.
		virtual bool isSystemKey() = 0;
#endif
	};

	class ChromiumRendererI : public IntrusiveRefPtrI
	{
	public:
		//Call when need to resize browser. Will call ChromiumRendererEventI::getViewRect to get new size
		virtual void invalidateSize() = 0;

		virtual void onMouseClick(int x, int y, MouseButtonType type, bool mouseUp, int clickCount) = 0;
		virtual void onMouseMove(int x, int y, bool mouseLeave) = 0;
		virtual void onKeyPress(const RefPtr<ChromiumKeyPressI>& pKeyPress) = 0;

		virtual void onFocus(bool setFocus) = 0;
		virtual void onCaptureLost() = 0;

		virtual RefPtr<ChromiumBrowserI> getBrowser() = 0;

		virtual void setEventCallback(const RefPtr<ChromiumRendererEventI>& cbe) = 0;
		virtual void setEventCallback(const RefPtr<ChromiumRendererPopupEventI>& cbe) = 0;
	};


	typedef bool(*LogMessageHandlerFn)(int severity, const char* str);

	class CallbackI : public IntrusiveRefPtrI
	{
	public:
		virtual void run() = 0;
	};

	enum ThreadID
	{
		///
		// The main thread in the browser. This will be the same as the main
		// application thread if CefInitialize() is called with a
		// CefSettings.multi_threaded_message_loop value of false.
		///
		TID_UI,

		///
		// Used to interact with the database.
		///
		TID_DB,

		///
		// Used to interact with the file system.
		///
		TID_FILE,

		///
		// Used for file system operations that block user interactions.
		// Responsiveness of this thread affects users.
		///
		TID_FILE_USER_BLOCKING,

		///
		// Used to launch and terminate browser processes.
		///
		TID_PROCESS_LAUNCHER,

		///
		// Used to handle slow HTTP cache operations.
		///
		TID_CACHE,

		///
		// Used to process IPC and network messages.
		///
		TID_IO,

		// RENDER PROCESS THREADS -- Only available in the render process.

		///
		// The main thread in the renderer. Used for all WebKit and V8 interaction.
		///
		TID_RENDERER,
	};

	class ChromiumBrowserDefaultsI : public IntrusiveRefPtrI
	{
	public:
		virtual bool enablePlugins() = 0;
		virtual bool enableJavascript() = 0;
		virtual bool enableJava() = 0;
		virtual bool enableFlash() = 0;
	};


	class ChormiumCookieVistor : public IntrusiveRefPtrI
	{
	public:
		//return false to stop
		virtual bool visit(const RefPtr<CookieI>& cookie) = 0;
	};

	class ChromiumCookieManagerI : public IntrusiveRefPtrI
	{
	public:
        virtual void purge(const char* domain) = 0;
        virtual void purgeAll() = 0;

		virtual void setCookie(const char* ulr, const RefPtr<CookieI>& cookie) = 0;
		virtual void delCookie(const char* url, const char* name) = 0;

		virtual void visitCookies(const RefPtr<ChormiumCookieVistor>& visitor, const char* szUrl = NULL) = 0;

		virtual void enableCookies() = 0;
		virtual void disableCookies() = 0;

		virtual RefPtr<CookieI> createCookie() = 0;
	};


	template <typename T>
	class CallbackT;

	class ChromiumControllerI
	{
	public:
		//Gets the max api version this dll supports
		virtual int GetMaxApiVersion() = 0;

		//Sets the api version the client supports
		virtual void SetApiVersion(int nVersion) = 0;

		virtual void DoMsgLoop() = 0;
		virtual void RunMsgLoop() = 0;
		virtual void Stop() = 0;

		virtual bool RegisterJSExtender(const RefPtr<JavaScriptExtenderI>& extender) = 0;
		virtual bool RegisterSchemeExtender(const RefPtr<SchemeExtenderI>& extender) = 0;

		//! Make sure to call before calling NewChromium*
		virtual void SetDefaults(const RefPtr<ChromiumBrowserDefaultsI>& defaults) = 0;

		// Form handle as HWND
		virtual RefPtr<ChromiumBrowserI> NewChromiumBrowser(int* formHandle, const char *name, const char* defaultUrl) = 0;

		//Creates a offscreen browser renderer
		virtual RefPtr<ChromiumRendererI> NewChromiumRenderer(int* formHandle, const char* defaultUrl, int width, int height) = 0;

		// Return true to handle msg
		virtual void SetLogHandler(LogMessageHandlerFn logFn) = 0;
		virtual void PostCallback(const RefPtr<CallbackI>& callback) = 0;

		virtual void PostCallbackEx(ChromiumDLL::ThreadID thread, const RefPtr<CallbackI>& callback) = 0;

		template <typename T>
		void PostcallbackT(ChromiumDLL::ThreadID thread, T t)
		{
			PostCallbackEx(thread, new CallbackT<T>(t));
		}

		virtual RefPtr<ChromiumCookieManagerI> GetCookieManager() = 0;
	};
}

#ifndef CEF_IGNORE_FUNCTIONS 
#ifdef WIN32

#include <windows.h>

extern "C"
{
	DLLINTERFACE ChromiumDLL::ChromiumControllerI* CEF_InitEx(bool threaded, const char* cachePath, const char* logPath, const char* userAgent);
	DLLINTERFACE int CEF_ExecuteProcessWin(HINSTANCE instance);
}
#endif // TODO LINUX
#endif

#endif //THIRDPARTY_CEF3_HEADER
