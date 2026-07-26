# run_files_exist.cmake — assert one or more files exist.
#
# Invoked as: cmake -DFILE1=<path> [-DFILE2=<path> ...] -P run_files_exist.cmake
#
# Accepts FILE1..FILE9 (only the ones that are defined are checked), so a
# single generic runner covers any test that needs to confirm N output files
# were actually produced (e.g. a CLI flag that's supposed to emit a second
# file alongside the primary one).

set(_missing "")
foreach(n RANGE 1 9)
    set(v "FILE${n}")
    if(DEFINED ${v})
        if(NOT EXISTS "${${v}}")
            list(APPEND _missing "${${v}}")
        endif()
    endif()
endforeach()

if(_missing)
    string(REPLACE ";" "\n  " _missing_str "${_missing}")
    message(FATAL_ERROR "run_files_exist.cmake: missing expected file(s):\n  ${_missing_str}")
endif()
