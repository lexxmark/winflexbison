# run_update_test.cmake — CTest launcher for `win_bison --update`.
#
# Issue #62: --update must rewrite a grammar using deprecated syntax in
# place and leave a `<file>~` backup, without failing with "cannot backup:
# Permission denied" (a Windows rename()-semantics bug: POSIX rename()
# replaces an existing target atomically, MSVCRT's doesn't).
#
# Invoked as:
#   cmake -DTEST_EXE=<win_bison path> -DFIXTURE=<file.y> -DSCRATCH=<dir>
#         -P run_update_test.cmake

foreach(v TEST_EXE FIXTURE SCRATCH)
    if(NOT DEFINED ${v})
        message(FATAL_ERROR "run_update_test.cmake: ${v} not set")
    endif()
endforeach()

# Recreate the scratch dir fresh each run (don't assume it's pristine from a
# previous ctest invocation).
if(EXISTS "${SCRATCH}")
    file(REMOVE_RECURSE "${SCRATCH}")
endif()
file(MAKE_DIRECTORY "${SCRATCH}")

get_filename_component(fixture_name "${FIXTURE}" NAME)
set(copy "${SCRATCH}/${fixture_name}")
file(COPY "${FIXTURE}" DESTINATION "${SCRATCH}")

file(READ "${copy}" before)

# --update rewrites in place from the grammar's own directory; run bison
# there so the backup lands next to the copy, not in some unrelated cwd.
execute_process(
    COMMAND "${TEST_EXE}" --update "${fixture_name}"
    WORKING_DIRECTORY "${SCRATCH}"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err)

if(NOT rc EQUAL 0)
    message(FATAL_ERROR
        "win_bison --update exited with ${rc}\n--- stdout ---\n${out}\n--- stderr ---\n${err}")
endif()

if(NOT EXISTS "${copy}")
    message(FATAL_ERROR "run_update_test.cmake: ${copy} vanished after --update")
endif()

file(READ "${copy}" after)
if(after STREQUAL before)
    message(FATAL_ERROR "run_update_test.cmake: ${copy} was not rewritten by --update")
endif()

set(backup "${copy}~")
if(NOT EXISTS "${backup}")
    message(FATAL_ERROR "run_update_test.cmake: expected backup file missing: ${backup}")
endif()
