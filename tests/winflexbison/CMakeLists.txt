# winflexbison "our own" tests — port-specific behavior upstream doesn't cover.
# See docs/specs/03-test-adoption.md ("our own tests"). Windows-native (CTest),
# no WSL.

find_program(POWERSHELL_EXE NAMES pwsh powershell)

if(POWERSHELL_EXE)
    set(RUN_PARALLEL "${CMAKE_CURRENT_SOURCE_DIR}/run_parallel.ps1")

    # Resistance: sustained concurrent invocations must not collide on %TEMP%
    # files (per-process unique temp names + delete-on-close). The launcher
    # self-verifies correctness, real overlap, and temp cleanup. --header-file
    # makes win_flex use the header-split filter chain (mkstempFILE path).
    #
    # 4x10 rather than 8x25: these two tests were 50.8s of a 56.1s CI run, and
    # 40 invocations exercise the same code paths as 200. The launcher FAILS if
    # peak concurrency < MinConcurrency (default 2), so a setting too low to
    # overlap is a loud failure, not silent loss of coverage -- but it would be
    # a flaky one, so the values were chosen with margin. Measured over 10 runs
    # each: flex peak concurrency min 4 (was min 2 at 4x5 -- on the threshold,
    # rejected), bison min 6. Re-measure before lowering further.
    add_test(NAME winflexbison.flex_parallel
        COMMAND ${POWERSHELL_EXE} -NoProfile -ExecutionPolicy Bypass -File "${RUN_PARALLEL}"
            -Tool       $<TARGET_FILE:win_flex>
            -Grammar    "${CMAKE_CURRENT_SOURCE_DIR}/../flex/cases/basic_nr.l"
            -ToolArgs   "--header-file=out.h"
            -Jobs 4 -Iterations 10)

    add_test(NAME winflexbison.bison_parallel
        COMMAND ${POWERSHELL_EXE} -NoProfile -ExecutionPolicy Bypass -File "${RUN_PARALLEL}"
            -Tool    $<TARGET_FILE:win_bison>
            -Grammar "${CMAKE_CURRENT_SOURCE_DIR}/../bison/cases/calc.y"
            -Jobs 4 -Iterations 10)

    set_tests_properties(winflexbison.flex_parallel winflexbison.bison_parallel
        PROPERTIES LABELS "winflexbison")
else()
    message(STATUS "PowerShell not found; skipping winflexbison resistance tests")
endif()

# ---------------------------------------------------------------------------
# Issue #39: a CMake build with USE_STATIC_RUNTIME=ON must not pull in the
# dynamic-CRT DLL (vcruntime140.dll) the way a plain /MD build does. Only
# meaningful (and only registered) when that option is actually on, mirroring
# the WFB_WSL_AUTOTEST opt-in-test precedent above.
if(USE_STATIC_RUNTIME)
    find_program(DUMPBIN_EXE dumpbin)
    if(NOT DUMPBIN_EXE)
        # dumpbin isn't on PATH outside a VS Developer Prompt; it lives next
        # to the compiler in every VS layout, so fall back to that.
        get_filename_component(_cl_dir "${CMAKE_C_COMPILER}" DIRECTORY)
        find_program(DUMPBIN_EXE dumpbin PATHS "${_cl_dir}" NO_DEFAULT_PATH)
    endif()

    if(DUMPBIN_EXE)
        add_test(NAME winflexbison.static_runtime_no_vcruntime
            COMMAND ${CMAKE_COMMAND}
                -DDUMPBIN_EXE=${DUMPBIN_EXE}
                -DEXE1=$<TARGET_FILE:win_bison>
                -DEXE2=$<TARGET_FILE:win_flex>
                -P "${CMAKE_CURRENT_SOURCE_DIR}/run_no_vcruntime_check.cmake")
        set_tests_properties(winflexbison.static_runtime_no_vcruntime
            PROPERTIES LABELS "winflexbison")
    else()
        message(WARNING "USE_STATIC_RUNTIME=ON but dumpbin not found; skipping winflexbison.static_runtime_no_vcruntime")
    endif()
endif()
