Desura CEF3
=======================
 * Download cef3 binaries from http://cefbuilds.com/ and extract to cef3 folder
 * Run build_cef.bat or build_cef.sh script depending on platform
 
Chromium Source Code
=======================
 * download depot_tools.zip and extract/add to path
 * find src url in CHROMIUM_BUILD_COMPATIBILITY.txt file
 * run command "gclient config [src url]"
 * run command "gclient sync --force"

Recommened to add to .gclient file:

    "custom_deps" : {
      "src/chrome/tools/test/reference_build/chrome": None,
      "src/chrome/tools/test/reference_build/chrome_linux": None,
      "src/chrome/tools/test/reference_build/chrome_mac": None,
      "src/chrome_frame/tools/test/reference_build/chrome": None,
      "src/third_party/gles2_book": None,
      "src/third_party/hunspell_dictionaries": None,
      "src/third_party/WebKit/LayoutTests": None,
    },

ZMQ Messages
=======================

JSON Object format:
 * Object/Array Proxy
```javascript
 {
   __object_proxy__: <string>, // Object id
   __object_type__: <string>   // ["array"|"object"]
 }
```
 * Function Proxy - Renderer
```javascript
 {
   __function__: <string>,            // Function name
   __renderer_function_id__: <string> // Function id
 }
```
 * Function Proxy - Host
```javascript
 {
   __function__: <string>,           // Function name
   __browser_function_id__: <string> // Function id
 }
```
 * Object/Array non proxy: JSON
 * Undefined: JSON string "__undefined__"
 * Null: JSON null
 * Int: JSON int
 * Double: JSON double
 * String: JSON string
 * Bool: JSON bool



Host <--> Renderer
 * Javascript Request
```javascript
 {
  name: "JSE-Request",
  request: {
   command: <string>, // ["FunctionCall"|"ObjectRequest"]
   function: <string>,
   object: <JSON>,
   arguments: [<JSON>]
  }
 }
```
 * Javascript Request
```javascript
 {
  name: "JSE-Response",
  response: {
   command: <string>,  // ["ObjectReturn"|"FunctionReturn"|"FunctionException"]
   result: <JSON>,     // only for object/function return
   exception: <string> // only for exception
  }
 }
```


Render -> Host
 * Browser Created
```javascript
 {
  name: "Browser-Created",
  id: <int> // browser id << 16 + pid & 0xFFFF
 }
```
 * Browser Destroyed
```javascript
 {
  name: "Browser-Destroyed",
  id: <int> // browser id << 16 + pid & 0xFFFF
 }
```
 * Browser Javascript Context Created
```javascript
 {
  name: "Browser-JSContextCreated",
  id: <int>,         // browser id << 16 + pid & 0xFFFF
  browser: <int>,    // browser id
  extender: <string> // global object extender name
 }
```
 * Browser Javascript Context Destroyed
```javascript
 {
  name: "Browser-JSContextReleased",
  id: <int>,         // browser id << 16 + pid & 0xFFFF
  browser: <int>,    // browser id
  extender: <string> // global object extender name
 }
```



