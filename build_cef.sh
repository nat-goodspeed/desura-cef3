#!/bin/sh

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
cmake .. || exit 1
make $@
cd ..
