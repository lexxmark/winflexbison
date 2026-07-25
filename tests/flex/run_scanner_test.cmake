# run_scanner_test.cmake — CTest launcher for a generated flex scanner.
#
# Runs the built test scanner, optionally feeding a .txt file on stdin, and
# uses the scanner's own exit code as pass/fail (this mirrors upstream flex's
# testwrapper.sh: the catch-all rule calls exit(1) on unexpected input, and a
# clean EOF returns 0). Exit code 77 is reserved by CTest for "skipped".
#
# Invoked as:
#   cmake -DTEST_EXE=<path> [-DINPUT=<path>] -P run_scanner_test.cmake

if(NOT DEFINED TEST_EXE)
    message(FATAL_ERROR "run_scanner_test.cmake: TEST_EXE not set")
endif()

if(DEFINED INPUT AND NOT "${INPUT}" STREQUAL "" AND EXISTS "${INPUT}")
    execute_process(
        COMMAND "${TEST_EXE}"
        INPUT_FILE "${INPUT}"
        RESULT_VARIABLE rc
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err)
else()
    execute_process(
        COMMAND "${TEST_EXE}"
        RESULT_VARIABLE rc
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err)
endif()

if(NOT rc EQUAL 0)
    message(FATAL_ERROR
        "scanner exited with ${rc}\n--- stdout ---\n${out}\n--- stderr ---\n${err}")
endif()
