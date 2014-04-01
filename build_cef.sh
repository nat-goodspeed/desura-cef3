#!/bin/sh
if [ ! -d "build_nix" ]; then
	mkdir build_nix
fi
cd build_nix
cmake .. || exit 1
make $@
cd ..