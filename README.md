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
