# winflexbison test suites. Wired in from the root CMakeLists via
# enable_testing() + add_subdirectory(tests). Run with `ctest` from the build
# tree. See docs/specs/03-test-adoption/spec.md.

# C++ is needed for the flex C++ scanner tests (.ll / .lll). The main product
# builds as C only; enable CXX here so it is scoped to the test tree.
enable_language(CXX)

add_subdirectory(flex)
add_subdirectory(bison)
add_subdirectory(winflexbison)

# Opt-in: run the full bison GNU Autotest (696 groups) under WSL as one ctest
# test. It is a POSIX-shell harness so it can't be a native ctest test, but this
# lets `ctest` drive it when explicitly enabled. Default OFF keeps the gate
# dependency-free. Passes iff run.sh reports 0 unexpected failures.
option(WFB_WSL_AUTOTEST "Run the WSL bison autotest as a ctest test" OFF)
if(WFB_WSL_AUTOTEST)
    find_program(WSL_EXE wsl)
    if(WSL_EXE)
        # Translate the Windows source path to a WSL /mnt path.
        string(SUBSTRING "${CMAKE_SOURCE_DIR}" 0 1 _drv)
        string(TOLOWER "${_drv}" _drv)
        string(REGEX REPLACE "^.:" "/mnt/${_drv}" _wslroot "${CMAKE_SOURCE_DIR}")
        add_test(NAME bison.autotest.wsl
            COMMAND ${WSL_EXE} -e bash "${_wslroot}/tests/bison-autotest/run.sh")
        set_tests_properties(bison.autotest.wsl PROPERTIES
            TIMEOUT 1800 LABELS "bison;wsl")
    else()
        message(WARNING "WFB_WSL_AUTOTEST=ON but 'wsl' not found; skipping bison.autotest.wsl")
    endif()
endif()
