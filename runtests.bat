@echo off
rem Configure, build, and run the winflexbison test suite (CTest).
rem Usage:  runtests.bat ["<CMake generator>"]
rem   default generator: "Visual Studio 17 2022"
rem   e.g. runtests.bat "Visual Studio 16 2019"
rem
rem Build dir CMakeBuildTests is gitignored (CMakeBuild*/).

setlocal
set GENERATOR=%~1
if "%GENERATOR%"=="" set GENERATOR=Visual Studio 17 2022

cmake -B CMakeBuildTests -S . -G "%GENERATOR%" -A x64
if errorlevel 1 goto :fail

cmake --build CMakeBuildTests --config Release
if errorlevel 1 goto :fail

ctest --test-dir CMakeBuildTests -C Release --output-on-failure
if errorlevel 1 goto :fail

echo.
echo All tests passed.
endlocal
exit /b 0

:fail
echo.
echo runtests: FAILED (see output above).
endlocal
exit /b 1
