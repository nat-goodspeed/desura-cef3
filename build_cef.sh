#!/bin/sh

set -e

if [ ! -d "cef3" ]; then
	if [ ! -d "build_cef3" ]; then
		mkdir build_cef3
	fi
	cd build_cef3
	cmake ../cef3_dl
	make
	cd ..
fi

if [ ! -d "build_nix" ]; then
	mkdir build_nix
fi
cd build_nix
# cmake .. -G Xcode    #Uncomment this line to generate an xcode project.
cmake .. 
make $@
cd ..
