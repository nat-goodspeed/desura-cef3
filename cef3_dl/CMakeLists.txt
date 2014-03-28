

project(cef3)

cmake_minimum_required(VERSION 2.8.5)
include(ExternalProject)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(64BIT TRUE)
  message("-- detected 64bit")
else()
  set(64BIT FALSE)
  message("-- detected 32bit")
endif()


###############################################################################
# SVN, HTTP stuff
###############################################################################

set(DOWNLOAD_URL "https://s3-us-west-2.amazonaws.com/lecs.desura.lindenlab.com/" CACHE STRING "Download mirror for third party source code and binaries")

macro(SetupDownloadUrlCustom name url md5)
  set(${name}_URL_DEFAULT "${url}")
  set(${name}_URL ${${name}_URL_DEFAULT} CACHE STRING "URL to get ${name} archive")
  if(${name}_URL STREQUAL ${name}_URL_DEFAULT)
    set(${name}_MD5 ${md5} CACHE STRING "Md5 for ${name} archive")
  else()
    unset(${name}_MD5)
  endif()
endmacro()

macro(SetupDownloadUrl name url md5)
  SetupDownloadUrlCustom(${name} "${DOWNLOAD_URL}${url}" ${md5})
endmacro()

if(APPLE)
	SetupDownloadUrl(CEF_RELEASE "cef_mac32_1750.zip" faaa4fbda59c4d1fdd9449accaf12bf9)
elseif(WIN32)
	SetupDownloadUrl(CEF_RELEASE "cef_win32_1750.zip" 20806ca5a97892f19389431ce69b2952)
else(UNIX)
	if (64BIT)
		SetupDownloadUrl(CEF_RELEASE "cef_lin64_1750.zip" bcbfdcdc8ad8f111fbeb0e55664fda0c)
	else()
		SetupDownloadUrl(CEF_RELEASE "cef_lin32_1750.zip" 439fb6e30761034854bc3d2e672d2f39)
	endif()
endif()

ExternalProject_Add(
	cef_bin_dl
	URL "${CEF_RELEASE_URL}"
	URL_MD5 ${CEF_RELEASE_MD5}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE 1
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
  
ExternalProject_Add_Step(
	cef_bin_dl
	custom_build
	DEPENDEES configure
	DEPENDERS build
	COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> "${CMAKE_BINARY_DIR}/../cef3"
)
