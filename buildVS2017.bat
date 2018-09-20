mkdir CMakeBuildVS2017
cd CMakeBuildVS2017
cmake .. -G "Visual Studio 15 2017"
cmake --build . --config "Release" --target package
cd ..