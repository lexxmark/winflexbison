mkdir CMakeBuildVS2022
cd CMakeBuildVS2022
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config "Release" --target package
cd ..