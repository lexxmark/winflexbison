# winflexbison test suites. Wired in from the root CMakeLists via
# enable_testing() + add_subdirectory(tests). Run with `ctest` from the build
# tree. See docs/specs/03-test-adoption/spec.md.

# C++ is needed for the flex C++ scanner tests (.ll / .lll). The language is
# enabled in the root CMakeLists rather than here: enabling it at this point is
# too late for the root's USE_STATIC_RUNTIME flag rewrite, which then leaves
# C++ targets on the wrong CRT. See the comment on project() there.

# Keep test executables out of bin/{Debug,Release}: the root CMakeLists points
# the per-config runtime output there and CPack installs that whole directory,
# so anything landing in it is shipped in the release zip. Redirecting here
# (directory scope) covers every target added below. Tests locate each other
# through $<TARGET_FILE:...>, so nothing else needs to change.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/bin")

# Warnings are errors in the test targets, and only there. Every executable
# below compiles output that win_flex or win_bison just generated, so a new
# warning here means our tools started emitting worse code -- which is exactly
# how #73 (C4244), #95 (C4244 via a 32-bit YYPTRDIFF_T) and #29 (C4005) each
# reached users. Those were all found by hand, months late; this makes the
# build say it.
#
# NOT on win_flex, win_bison, winflexbison_common, fl or y. Those are vendored
# upstream sources, so a newer MSVC or a re-vendor would break the build on
# upgrade day instead of printing a line to read. See docs/build-warnings-plan.md.
#
# Targets that need an exception already carry a narrow /wd (C4065 on
# bisontest_many_tokens, C4311 on the two flex mem tests); /WX does not
# override those. If an upstream upgrade brings a warning that needs thought
# rather than a fix, configure with -DWFB_TESTS_WERROR=OFF to get the old
# behaviour back while you deal with it.
option(WFB_TESTS_WERROR "Treat warnings as errors in the test targets" ON)
if(MSVC AND WFB_TESTS_WERROR)
    add_compile_options(/WX)
endif()

# win_bison reads its skeletons from bison/data/ at run time, so a generated
# parser depends on those files as surely as on the grammar it came from --
# but nothing in the build said so, and an incremental build after a skeleton
# edit silently recompiled the previous run's output. That cost a debugging
# detour on issue #95 (the c.m4 fix looked like it had not taken, then like it
# had broken a test it had already been changed to avoid). Every custom command
# below that runs win_bison lists this alongside its grammar.
#
# Plain GLOB, no CONFIGURE_DEPENDS: content changes are what matter here, and
# they are caught. Adding or removing a skeleton file needs a re-run of cmake.
file(GLOB_RECURSE WFB_BISON_SKELETONS "${CMAKE_SOURCE_DIR}/bison/data/*")

# "The generated file must (not) contain this" assertion, used by both the flex
# and the bison suite -- the only way to test output we cannot compile here (a
# C++-only construct, a D parser on a worker without a D compiler). It lived in
# tests/flex/ while flex was its only user. See run_content_check.cmake.
set(WFB_CONTENT_CHECK "${CMAKE_CURRENT_SOURCE_DIR}/run_content_check.cmake")

add_subdirectory(flex)
add_subdirectory(bison)
add_subdirectory(winflexbison)

# Opt-in: run the full bison GNU Autotest (776 groups) under MSYS2 as one ctest
# test. It is a POSIX-shell harness so it can't be a native ctest test, but this
# lets `ctest` drive it when explicitly enabled. Default OFF keeps the gate
# dependency-free. Passes iff run.sh reports 0 unexpected failures.
option(WFB_MSYS2_AUTOTEST "Run the MSYS2 bison autotest as a ctest test" OFF)
if(WFB_MSYS2_AUTOTEST)
    find_program(MSYS2_BASH bash
        HINTS "C:/msys64/usr/bin" "$ENV{MSYS2_ROOT}/usr/bin"
        NO_SYSTEM_ENVIRONMENT_PATH NO_DEFAULT_PATH)
    if(MSYS2_BASH)
        # MSYS2 paths are /c/... , not the C:/... CMake hands us. Autotest splits
        # $PATH on ':', so a drive letter anywhere in it makes every program in
        # that entry invisible -- hence the translation rather than passing the
        # native path through.
        string(SUBSTRING "${CMAKE_SOURCE_DIR}" 0 1 _drv)
        string(TOLOWER "${_drv}" _drv)
        string(REGEX REPLACE "^.:" "/${_drv}" _msysroot "${CMAKE_SOURCE_DIR}")
        # -l so MSYSTEM=MINGW64 puts the compilers on PATH; a login shell starts
        # in the MSYS2 home, so run.sh is named absolutely (it derives the rest
        # from its own location).
        add_test(NAME bison.autotest.msys2
            COMMAND ${MSYS2_BASH} -lc "${_msysroot}/tests/bison-autotest/run.sh")
        set_tests_properties(bison.autotest.msys2 PROPERTIES
            TIMEOUT 3600 LABELS "bison;msys2"
            ENVIRONMENT "MSYSTEM=MINGW64")
    else()
        message(WARNING "WFB_MSYS2_AUTOTEST=ON but MSYS2 bash not found; skipping bison.autotest.msys2")
    endif()
endif()
