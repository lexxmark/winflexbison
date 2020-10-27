mkdir CMakeBuildVS2019
cd CMakeBuildVS2019
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config "Release" --target package
cd ..