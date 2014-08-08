#!/bin/bash

cd "$(dirname "$0")"

# turn on verbose debugging output for parabuild logs.
set -x
# make errors fatal
set -e

CEF_VERSION="1750"
CEF_SOURCE_DIR="."

if [ -z "$AUTOBUILD" ] ; then 
    fail
fi

if [ "$OSTYPE" = "cygwin" ] ; then
    export AUTOBUILD="$(cygpath -u $AUTOBUILD)"
fi

# load autobuild provided shell functions and variables
set +x
# bug in autobuild 0.8 source_environment: on Windows, it replaces AUTOBUILD
# with a bogus path
SAVE_AUTOBUILD="$AUTOBUILD"
eval "$("$AUTOBUILD" source_environment)"
AUTOBUILD="$SAVE_AUTOBUILD"
unset SAVE_AUTOBUILD
set -x

# pick up third-party libraries
"$AUTOBUILD" install

stage="$(pwd)/stage"
pushd "$CEF_SOURCE_DIR"
    case "$AUTOBUILD_PLATFORM" in
        # ----------------------------- windows ------------------------------
        "windows")
            load_vsvars

            # catch directory for "build products" from CEF3 download
            mkdir build_cef3 || echo "build_cef3 exists"
            pushd build_cef3
            # use CMake to download CEF3 from buildbots
            cmake ../cef3_dl -G "NMake Makefiles"
            nmake
            popd            

            # real solution
            cmake . -G "Visual Studio 10"
            build_sln "3p_cef3.sln" "Release|Win32"
            mkdir -p "$stage/lib/release/cef/locales"
            mkdir -p "$stage/include/cef"
            cp -v Release/*.{dll,exe,pak} "$stage/lib/release/cef/"
            cp -v Release/locales/* "$stage/lib/release/cef/locales/"
            cp -v cef_ll_includes/*.h "$stage/include/cef/"
        ;;

        # ------------------------------ darwin ------------------------------
        "darwin")
            pushd test
            # Because of set -e, if this fails, we'll crump.
            make
            popd

            cmake . -GXcode \
                    -D'CMAKE_OSX_ARCHITECTURES:STRING=i386' \
                    -D'BUILD_SHARED_LIBS:bool=off' \
                    -DCMAKE_INSTALL_PREFIX="$stage"

            for cfg in Release Debug
            do
               xcodebuild -configuration $cfg -target cef -project cef.xcodeproj
            done

            mkdir -p "$stage/lib/debug"
            mkdir -p "$stage/lib/release"
            mkdir -p "$stage/include/cef"

            cp -v "source/build/Debug/libcef.a" "$stage/lib/debug/libcefd.a"
            cp -v "source/build/Release/libcef.a" "$stage/lib/release/"
            cp -v source/*.h "$stage/include/cef/"
        ;;            

        # ------------------------------ linux -------------------------------
        "linux")
            pushd test
            # Because of set -e, if this fails, we'll crump.
            make
            popd

            cmake -G"Unix Makefiles" \
                  -DCMAKE_INSTALL_PREFIX="$stage" \
                  -DBUILD_SHARED_LIBS:bool=off .
            # remove any pre-existing value for CXXFLAGS
            unset CXXFLAGS
            make

            mkdir -p "$stage/lib/release"
            mkdir -p "$stage/include/cef"

            cp -v source/build/libcef.a "$stage/lib/release/"
            cp -v source/*.h "$stage/include/cef/"
        ;;
    esac
    mkdir -p "$stage/LICENSES"
    cp cef_license.txt "$stage/LICENSES/cef.txt"
popd

pass
