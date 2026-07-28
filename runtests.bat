@echo off
rem Configure, build, and run the winflexbison test suite.
rem
rem Usage:  runtests.bat [--with-autotest] ["<CMake generator>"]
rem   default generator: "Visual Studio 17 2022"
rem   e.g. runtests.bat "Visual Studio 16 2019"
rem        runtests.bat --with-autotest
rem
rem By default runs only the Windows CTest gate (flex + bison), no MSYS2 needed.
rem --with-autotest additionally runs the full bison GNU Autotest suite under
rem MSYS2 (tests/bison-autotest/run.sh); install its deps once with
rem tests/bison-autotest/install-msys2-deps.sh.
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

set MSYS2_BASH=C:\msys64\usr\bin\bash.exe
if not exist "%MSYS2_BASH%" (
    echo.
    echo --with-autotest: MSYS2 not found at %MSYS2_BASH%, skipping bison autotest.
    goto :ok
)
echo.
echo === Running bison GNU Autotest under MSYS2 ===
rem MSYSTEM picks the subsystem whose bin dir a login shell puts on PATH; MINGW64
rem is the one carrying the gcc the compile tiers use. -l is what applies it, and
rem a login shell starts in the MSYS2 home -- so run.sh is named by absolute path
rem (cygpath translates this checkout), not reached with a cd.
set MSYSTEM=MINGW64
"%MSYS2_BASH%" -lc "$(cygpath -u '%CD%')/tests/bison-autotest/run.sh"
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
