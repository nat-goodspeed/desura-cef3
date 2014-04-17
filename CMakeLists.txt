#
#
#
#

project(cef_desura)

if(APPLE) 
  # see #418
  cmake_minimum_required(VERSION 2.8.10)
elseif(WIN32)
  #needed for x120_xp target
  cmake_minimum_required(VERSION 2.8.12)
else()
  cmake_minimum_required(VERSION 2.8.5)
endif()

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

if(DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
else()
  if(OFFICIAL_BUILD)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
  else()
    set(CMAKE_BUILD_TYPE Release)
  endif()
endif()

if(NOT WIN32)
	set(CMAKE_OUTPUT "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_OUTPUT})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_OUTPUT})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_OUTPUT})
endif()

if(NOT 32BITBUILD AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(64BIT TRUE)
  message("-- detected 64bit")
else()
  set(64BIT FALSE)
  message("-- detected 32bit")
endif()


###############################################################################
# macros
###############################################################################


MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
  SET(DEFAULT_ARGS)
  FOREACH(arg_name ${arg_names})    
    SET(${prefix}_${arg_name})
  ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option} FALSE)
  ENDFOREACH(option)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})            
    SET(larg_names ${arg_names})    
    LIST(FIND larg_names "${arg}" is_arg_name)                   
    IF (is_arg_name GREATER -1)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name GREATER -1)
      SET(loption_names ${option_names})    
      LIST(FIND loption_names "${arg}" is_option)            
      IF (is_option GREATER -1)
         SET(${prefix}_${arg} TRUE)
      ELSE (is_option GREATER -1)
         SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option GREATER -1)
    ENDIF (is_arg_name GREATER -1)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)

macro(add_linker_flags)
  foreach(flag ${ARGN})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${flag}")
  endforeach()
endmacro()


macro(add_compiler_flags)
  set(flags_list "")
  parse_arguments(ARG "" "C;CXX;DEBUG;RELEASE" ${ARGN})

  if(NOT ARG_DEBUG AND NOT ARG_RELEASE)
    foreach(flag ${ARG_DEFAULT_ARGS})
      set(flags_list "${flags_list} ${flag}")
    endforeach()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_C)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flags_list}")
    endif()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_CXX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags_list}")
    endif()
  elseif(ARG_DEBUG)
    foreach(flag ${ARG_DEFAULT_ARGS})
      set(flags_list "${flags_list} ${flag}")
    endforeach()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_C)
      set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${flags_list}")
    endif()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_CXX)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${flags_list}")
    endif()
  elseif(ARG_RELEASE)
    foreach(flag ${ARG_DEFAULT_ARGS})
      set(flags_list "${flags_list} ${flag}")
    endforeach()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_C)
      set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${flags_list}")
      set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${flags_list}")
    endif()
    if(NOT ARG_C AND NOT ARG_CXX OR ARG_CXX)
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${flags_list}")
      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${flags_list}")
    endif()
  endif()
endmacro()

###############################################################################
# Setup
###############################################################################

if(WIN32)
	if( MSVC_VERSION LESS 1800 )       # VC10-/VS2010- 
		message(FATAL_ERROR "The project requires C++11 features. " 
		  "You need at least Visual Studio 12 (Microsoft Visual Studio 2013)") 
	endif()

	if(DEBUG)
		add_definitions(-DDEBUG -D_DEBUG)
	else()
		add_definitions(-DNDEBUG -D_NDEBUG)
	endif()

	# ignore some warnings
	add_linker_flags(/ignore:4006
					 /ignore:4099
					 /ignore:4221)
	add_compiler_flags(/wd4996 /MP)

	# jom needs /FS msvc12 for parallel builds
	if(MSVC12)
	  add_compiler_flags(/FS)
	endif()

	set(CMAKE_GENERATOR_TOOLSET "v120_xp")
	set(WIN_TARGET 0x0502) # Windows XP SP2
	set(WIN_SDK_MIN 0x0600) # Windows Vista
	set(WIN_IE_VERSION 0x0603) # IE 6 SP2
	add_definitions(-DWINVER=${WIN_TARGET}
					-D_WIN32_WINNT=${WIN_SDK_MIN}
					-D_WIN32_IE=${WIN_IE_VERSION}
					-D_USING_V120_SDK71_)
									
	macro(SetSharedRuntime target)
	  if(DEBUG)
		set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/MDd")
	  else()
		set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/MD")
	  endif()
	endmacro()

	macro(SetStaticRuntime target)
	  if(DEBUG)
		set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/MTd")
	  else()
		set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/MT")
	  endif()
	endmacro()

elseif(APPLE)
	# we download the mac32 buildbot package
	add_compiler_flags(-std=c++11 -m32)

elseif(UNIX)
	if(DEBUG)
		add_definitions(-DDEBUG)
	else()
		add_definitions(-DNDEBUG)
	endif()
	
	add_compiler_flags(-std=c++11)
	add_linker_flags(-std=c++11)

	add_compiler_flags(-fPIC)
	add_linker_flags(-fPIC)

	# some mingw things
	if(MINGW)
	  set(WIN_TARGET 0x0502) # Windows XP SP2
	  set(WIN_SDK_MIN 0x0600) # Windows Vista
	  set(WIN_IE_VERSION 0x0603) # IE 6 SP2
	  add_definitions(-U__STRICT_ANSI__
					  -DWINVER=${WIN_TARGET}
					  -D_WIN32_WINNT=${WIN_SDK_MIN}
					  -D_WIN32_IE=${WIN_IE_VERSION}
					  -DDONT_INCLUDE_AFXWIN
					  -DMINGW_HAS_SECURE_API
					  )
	  # link libgcc staticly
	  add_compiler_flags(-static-libgcc)
	endif()	

	if(32BITBUILD)
		add_compiler_flags(-m32)
		add_linker_flags(-m32)
	endif()
endif()


function(add_copy_target_file_step target target_loc)
  get_filename_component(target_name "${target_loc}" NAME)
  add_custom_command(TARGET copy_bin_files POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${target_name}")
endfunction()

function(add_copy_target_dir_step target target_loc)
	get_filename_component(target_name "${target_loc}" NAME)
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${target_loc}" "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${target_name}")
endfunction()

###############################################################################
# Projects
###############################################################################

if(UNIX OR APPLE)
	if(UNIX)
		FIND_PACKAGE(GTK2 COMPONENTS gtk)
		include_directories(
			${GTK2_INCLUDE_DIRS}
		)
	endif()
	FIND_PACKAGE(OpenGL)
	FIND_PACKAGE(GLUT)
	include_directories(
		${OPENGL_INCLUDE_DIRS}
		${GLUT_INCLUDE_DIR}
	)
endif()


set(CEF_DL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cef3)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/cef_desura_includes
  ${CMAKE_CURRENT_SOURCE_DIR}/cef_desura
  ${CEF_DL_DIR}
  ${CEF_DL_DIR}/libcef_dll
)

set(CEF_INCLUDE_PATH ${CEF_DL_DIR}/include)
set(CEF_DLLSOURCE_PATH ${CEF_DL_DIR}/libcef_dll)

if(DEBUG)
	set(CEF_BIN_PATH ${CEF_DL_DIR}/Debug)
else()
	set(CEF_BIN_PATH ${CEF_DL_DIR}/Release)
endif()

add_definitions(
  -DUSING_CEF_SHARED
  -DNOMINMAX
  -DCHROMIUM_API_SUPPORTS_V2
)

file(GLOB cef_desura_sources 
	cef_desura/*.cpp 
	cef_desura/*.h 
)

file(GLOB cef_desura_host_sources 
	cef_desura_host/*.cpp 
	cef_desura_host/*.h 
)

file(GLOB libcef_dll_wrapper_sources 
	${CEF_INCLUDE_PATH}/*.h 
	${CEF_INCLUDE_PATH}/internal/*.h
	${CEF_INCLUDE_PATH}/capi/cef_base_capi.h
	${CEF_INCLUDE_PATH}/wrapper/*.h
	${CEF_DLLSOURCE_PATH}/cpptoc/*.h
	${CEF_DLLSOURCE_PATH}/cpptoc/*.cc
	${CEF_DLLSOURCE_PATH}/ctocpp/*.h
	${CEF_DLLSOURCE_PATH}/ctocpp/*.cc
	${CEF_DLLSOURCE_PATH}/transfer_util.cpp
	${CEF_DLLSOURCE_PATH}/transfer_util.h
	${CEF_DLLSOURCE_PATH}/wrapper/*.h
	${CEF_DLLSOURCE_PATH}/wrapper/*.cc
)

add_library(cef_desura SHARED ${cef_desura_sources})
add_library(libcef_dll_wrapper STATIC ${libcef_dll_wrapper_sources})

if(WIN32)
	add_executable(cef_desura_host WIN32 ${cef_desura_host_sources})
elseif(APPLE)
	add_executable(cef_desura_host MACOSX_BUNDLE ${cef_desura_host_sources})
else()
	add_executable(cef_desura_host ${cef_desura_host_sources})
endif()

set_property(TARGET cef_desura PROPERTY COMPILE_DEFINITIONS BUILDING_CEF_DESURA_SHARED)

target_link_libraries(cef_desura
  libcef_dll_wrapper
)

target_link_libraries(cef_desura_host
)

add_executable(cef_desura_test_ogl cef_desura_test/main_opengl.cpp)

add_dependencies(cef_desura cef_desura_host)
add_dependencies(cef_desura_test_ogl cef_desura)

if(WIN32)
	add_executable(cef_desura_test_msw WIN32 cef_desura_test/main_msw.cpp cef_desura_test/desura_scheme.cpp)

	target_link_libraries(cef_desura_test_ogl
		${CMAKE_CURRENT_SOURCE_DIR}/cef_desura_test/freeglut/freeglut.lib
	)

	target_link_libraries(cef_desura
		${CEF_BIN_PATH}/libcef.lib
	)

	add_copy_target_file_step(cef_desura_test_ogl cef_desura_test/freeglut.dll)

	SetStaticRuntime(cef_desura_host)
	SetStaticRuntime(cef_desura)
	SetStaticRuntime(libcef_dll_wrapper)
	SetStaticRuntime(cef_desura_test_msw)
	SetStaticRunTime(cef_desura_test_ogl)

	add_dependencies(cef_desura_test_msw cef_desura)

elseif(APPLE)
	add_definitions(-DNIX)

	target_link_libraries(cef_desura_test_ogl
		${OPENGL_LIBRARIES}
		${GLUT_glut_LIBRARY}
		${CMAKE_DL_LIBS}
	)

	find_library(LIBCEF NAMES "Chromium Embedded Framework" PATHS "${CEF_BIN_PATH}")

	target_link_libraries(cef_desura
		${LIBCEF}
	)
	add_executable(cef_desura_test_mac MACOSX_BUNDLE cef_desura_test/main_mac.mm)
	
	set_target_properties(cef_desura_test_mac PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cef_desura_test/mac/Info.plist")
	set_target_properties(cef_desura_host PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cef_desura_test/mac/helper-Info.plist")
	
	target_link_libraries(cef_desura_test_mac
		${CMAKE_DL_LIBS}
	)	
elseif(UNIX)
	add_definitions(-DNIX)

	add_executable(cef_desura_test_gtk cef_desura_test/main_gtk.cpp)

	target_link_libraries(cef_desura_test_ogl
		${OPENGL_LIBRARIES}
		${GLUT_glut_LIBRARY}
		${CMAKE_DL_LIBS}
	)

	target_link_libraries(libcef_dll_wrapper
		${GTK2_LIBRARIES}
	)

	target_link_libraries(cef_desura_test_gtk
		${GTK2_LIBRARIES}
		${CMAKE_DL_LIBS}
	)

	target_link_libraries(cef_desura
		${CEF_BIN_PATH}/libcef.so
	)

	target_link_libraries(cef_desura_host
		${GTK2_LIBRARIES}
		${CMAKE_DL_LIBS}
	)
endif()


###############################################################################
# output files
###############################################################################

file(GLOB cef_bin_files ${CEF_BIN_PATH}/*.*)
file(GLOB cef_res_files ${CEF_DL_DIR}/Resources/*.*)

add_custom_target(copy_bin_files ALL)

foreach(file ${cef_bin_files})
	if(IS_DIRECTORY "${file}")
	  add_copy_target_dir_step(copy_bin_files "${file}")
	else()
	  add_copy_target_file_step(copy_bin_files "${file}")
	endif()
endforeach()

foreach(file ${cef_res_files})
	add_copy_target_file_step(copy_bin_files ${file})
endforeach()

add_copy_target_dir_step(copy_bin_files ${CEF_DL_DIR}/Resources/locales)
