# run_parser_test.cmake — CTest launcher for a compiled bison parser.
# Runs the parser exe, optionally feeding a file on stdin, and uses the exit
# code as pass/fail (the parser returns 0 on a clean parse). Exit 77 = skip.
#
# Invoked as: cmake -DTEST_EXE=<path> [-DINPUT=<path>] -P run_parser_test.cmake

if(NOT DEFINED TEST_EXE)
    message(FATAL_ERROR "run_parser_test.cmake: TEST_EXE not set")
endif()

if(DEFINED INPUT AND NOT "${INPUT}" STREQUAL "" AND EXISTS "${INPUT}")
    execute_process(COMMAND "${TEST_EXE}" INPUT_FILE "${INPUT}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
else()
    execute_process(COMMAND "${TEST_EXE}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
endif()

if(NOT rc EQUAL 0)
    message(FATAL_ERROR
        "parser exited with ${rc}\n--- stdout ---\n${out}\n--- stderr ---\n${err}")
endif()
