# run_compare_test.cmake — CTest launcher for flex's comparison-mode tests
# (the lineno "*.one" tests). Mirrors testwrapper.sh -1: run the scanner twice
# on the same stdin — once with no args (prints flex's computed value) and once
# with an argument (prints the reference value) — and require the two outputs to
# be equal (and both runs to exit 0).
#
# Invoked as:
#   cmake -DTEST_EXE=<path> -DINPUT=<path> -P run_compare_test.cmake

if(NOT DEFINED TEST_EXE)
    message(FATAL_ERROR "run_compare_test.cmake: TEST_EXE not set")
endif()
if(NOT DEFINED INPUT OR NOT EXISTS "${INPUT}")
    message(FATAL_ERROR "run_compare_test.cmake: INPUT not set or missing")
endif()

execute_process(
    COMMAND "${TEST_EXE}"
    INPUT_FILE "${INPUT}"
    RESULT_VARIABLE rc1
    OUTPUT_VARIABLE out1
    ERROR_VARIABLE  err1)

execute_process(
    COMMAND "${TEST_EXE}" 1
    INPUT_FILE "${INPUT}"
    RESULT_VARIABLE rc2
    OUTPUT_VARIABLE out2
    ERROR_VARIABLE  err2)

if(NOT rc1 EQUAL 0 OR NOT rc2 EQUAL 0)
    message(FATAL_ERROR
        "non-zero exit (rc1=${rc1}, rc2=${rc2})\n--- run1 err ---\n${err1}\n--- run2 err ---\n${err2}")
endif()

string(STRIP "${out1}" s1)
string(STRIP "${out2}" s2)
if(NOT "${s1}" STREQUAL "${s2}")
    message(FATAL_ERROR
        "output mismatch:\n  flex value : '${s1}'\n  reference  : '${s2}'")
endif()
