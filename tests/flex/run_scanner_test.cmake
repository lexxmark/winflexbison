# run_scanner_test.cmake — CTest launcher for a generated flex scanner.
#
# Runs the built test scanner, optionally feeding a .txt file on stdin, and
# uses the scanner's own exit code as pass/fail (this mirrors upstream flex's
# testwrapper.sh: the catch-all rule calls exit(1) on unexpected input, and a
# clean EOF returns 0). Exit code 77 is reserved by CTest for "skipped".
#
# Invoked as:
#   cmake -DTEST_EXE=<path> [-DINPUT=<path>] [-DARG1=<a>] [-DARG2=<b>]
#         [-DARG3=<c>] [-DWORKDIR=<dir>] -P run_scanner_test.cmake
#
# INPUT     : file fed on stdin (stdin-style tests).
# ARG1..3   : positional argv passed to the scanner (external-tables tests take
#             <tables> <input>; the "direct" and yywrap tests take input-file
#             path(s) as arguments).
# WORKDIR   : working directory for the child (the "direct" tests must run from
#             the cases dir so relative include-file opens resolve).
# INPUT and ARGn are independent; a test uses whichever it needs.

if(NOT DEFINED TEST_EXE)
    message(FATAL_ERROR "run_scanner_test.cmake: TEST_EXE not set")
endif()

set(cmd "${TEST_EXE}")
foreach(a ARG1 ARG2 ARG3)
    if(DEFINED ${a} AND NOT "${${a}}" STREQUAL "")
        list(APPEND cmd "${${a}}")
    endif()
endforeach()

set(workdir_opt "")
if(DEFINED WORKDIR AND NOT "${WORKDIR}" STREQUAL "")
    set(workdir_opt WORKING_DIRECTORY "${WORKDIR}")
endif()

if(DEFINED INPUT AND NOT "${INPUT}" STREQUAL "" AND EXISTS "${INPUT}")
    execute_process(
        COMMAND ${cmd}
        INPUT_FILE "${INPUT}"
        ${workdir_opt}
        RESULT_VARIABLE rc
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err)
else()
    execute_process(
        COMMAND ${cmd}
        ${workdir_opt}
        RESULT_VARIABLE rc
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err)
endif()

if(NOT rc EQUAL 0)
    message(FATAL_ERROR
        "scanner exited with ${rc}\n--- stdout ---\n${out}\n--- stderr ---\n${err}")
endif()
