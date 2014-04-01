IF NOT EXIST build_vis (
  mkdir build_vis
)

cd build_vis
cmake .. -G "Visual Studio 12" -T "v120_xp" -DDEBUG=ON
cd ..