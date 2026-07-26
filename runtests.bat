@echo off
rem Configure, build, and run the winflexbison test suite.
rem
rem Usage:  runtests.bat [--with-autotest] ["<CMake generator>"]
rem   default generator: "Visual Studio 17 2022"
rem   e.g. runtests.bat "Visual Studio 16 2019"
rem        runtests.bat --with-autotest
rem
rem By default runs only the Windows CTest gate (flex + bison, no WSL).
rem --with-autotest additionally runs the full bison GNU Autotest suite under
rem WSL (tests/bison-autotest/run.sh); requires WSL with autoconf/m4.
rem
rem Build dir CMakeBuildTests is gitignored (CMakeBuild*/).

setlocal
set GENERATOR=
set WITH_AUTOTEST=0

:parseargs
if "%~1"=="" goto :doneargs
if /i "%~1"=="--with-autotest" (
    set WITH_AUTOTEST=1
) else (
    set GENERATOR=%~1
)
shift
goto :parseargs
:doneargs
if "%GENERATOR%"=="" set GENERATOR=Visual Studio 17 2022

cmake -B CMakeBuildTests -S . -G "%GENERATOR%" -A x64
if errorlevel 1 goto :fail

cmake --build CMakeBuildTests --config Release
if errorlevel 1 goto :fail

ctest --test-dir CMakeBuildTests -C Release --output-on-failure
if errorlevel 1 goto :fail

if not "%WITH_AUTOTEST%"=="1" goto :ok

where wsl >nul 2>&1
if errorlevel 1 (
    echo.
    echo --with-autotest: WSL not found, skipping bison autotest.
    goto :ok
)
echo.
echo === Running bison GNU Autotest under WSL ===
wsl -e bash ./tests/bison-autotest/run.sh
if errorlevel 1 goto :fail

:ok
echo.
echo All tests passed.
endlocal
exit /b 0

:fail
echo.
echo runtests: FAILED (see output above).
endlocal
exit /b 1
