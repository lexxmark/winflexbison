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
    add_test(NAME winflexbison.flex_parallel
        COMMAND ${POWERSHELL_EXE} -NoProfile -ExecutionPolicy Bypass -File "${RUN_PARALLEL}"
            -Tool       $<TARGET_FILE:win_flex>
            -Grammar    "${CMAKE_CURRENT_SOURCE_DIR}/../flex/cases/basic_nr.l"
            -ToolArgs   "--header-file=out.h"
            -Jobs 8 -Iterations 25)

    add_test(NAME winflexbison.bison_parallel
        COMMAND ${POWERSHELL_EXE} -NoProfile -ExecutionPolicy Bypass -File "${RUN_PARALLEL}"
            -Tool    $<TARGET_FILE:win_bison>
            -Grammar "${CMAKE_CURRENT_SOURCE_DIR}/../bison/cases/calc.y"
            -Jobs 8 -Iterations 25)

    set_tests_properties(winflexbison.flex_parallel winflexbison.bison_parallel
        PROPERTIES LABELS "winflexbison")
else()
    message(STATUS "PowerShell not found; skipping winflexbison resistance tests")
endif()
