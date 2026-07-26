# run_no_vcruntime_check.cmake — issue #39: a CMake build with
# USE_STATIC_RUNTIME=ON must not pull in the dynamic-CRT DLL (vcruntime140).
#
# Invoked as:
#   cmake -DDUMPBIN_EXE=<path> -DEXE1=<path> [-DEXE2=<path>]
#         -P run_no_vcruntime_check.cmake

if(NOT DEFINED DUMPBIN_EXE)
    message(FATAL_ERROR "run_no_vcruntime_check.cmake: DUMPBIN_EXE not set")
endif()

foreach(v EXE1 EXE2)
    if(DEFINED ${v} AND NOT "${${v}}" STREQUAL "")
        execute_process(
            COMMAND "${DUMPBIN_EXE}" /dependents "${${v}}"
            RESULT_VARIABLE rc
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err)
        if(NOT rc EQUAL 0)
            message(FATAL_ERROR "dumpbin /dependents ${${v}} exited with ${rc}\n${err}")
        endif()
        string(TOLOWER "${out}" out_lower)
        string(FIND "${out_lower}" "vcruntime140" idx)
        if(idx GREATER -1)
            message(FATAL_ERROR
                "${${v}} depends on a dynamic-CRT DLL despite USE_STATIC_RUNTIME=ON:\n${out}")
        endif()
    endif()
endforeach()
