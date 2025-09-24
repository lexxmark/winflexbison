mkdir CMakeBuildClangCl
cd CMakeBuildClangCl
cmake .. -G"Ninja Multi-Config" -DCMAKE_C_COMPILER=clang-cl.exe
cmake --build . --config "Release" --target package
cd ..