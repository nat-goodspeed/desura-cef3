if NOT EXIST cef3 (
	IF NOT EXIST build_cef3 (
		mkdir build_cef3
	)	
	cd build_cef3
	cmake ..\cef3_dl -G "NMake Makefiles"
	nmake
	cd ..
)

IF NOT EXIST build_vis (
  mkdir build_vis
)

cd build_vis
cmake .. -G "Visual Studio 12" -DDEBUG=ON
cd ..