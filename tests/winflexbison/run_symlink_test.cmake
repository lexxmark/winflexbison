# run_symlink_test.cmake — CTest launcher for "tool invoked through a symlink".
#
# Issue #97: WinGet installs the package under .../WinGet/Packages/ and only
# symlinks the executables into .../WinGet/Links/. GetModuleFileName reports
# the link, so win_bison looked for data/ next to the link and died with
# "data/m4sugar/m4sugar.m4: cannot open". The link directory here deliberately
# holds nothing but the link, so the tool can only succeed by resolving it.
#
# Invoked as:
#   cmake -DTEST_EXE=<tool> -DSCRATCH=<dir> -DFIXTURE=<grammar> -DMODE=bison|flex
#         -P run_symlink_test.cmake
#
# Creating a symbolic link on Windows needs Developer Mode or elevation, so a
# machine without either skips rather than fails: the script prints SKIP: and
# the test carries SKIP_REGULAR_EXPRESSION.

foreach(v TEST_EXE SCRATCH FIXTURE MODE)
    if(NOT DEFINED ${v})
        message(FATAL_ERROR "run_symlink_test.cmake: ${v} not set")
    endif()
endforeach()

if(CMAKE_VERSION VERSION_LESS 3.14)
    message("SKIP: file(CREATE_LINK) needs CMake 3.14, have ${CMAKE_VERSION}")
    return()
endif()

# Recreate the scratch dir fresh each run, and keep it empty apart from the
# link: a stray data/ here would mask the very bug under test.
if(EXISTS "${SCRATCH}")
    file(REMOVE_RECURSE "${SCRATCH}")
endif()
file(MAKE_DIRECTORY "${SCRATCH}")

get_filename_component(exe_name "${TEST_EXE}" NAME)
set(link "${SCRATCH}/${exe_name}")

file(CREATE_LINK "${TEST_EXE}" "${link}" SYMBOLIC RESULT link_result)
if(NOT link_result STREQUAL "0")
    message("SKIP: cannot create a symbolic link (${link_result}); "
            "Windows requires Developer Mode or elevation")
    return()
endif()

set(out "${SCRATCH}/symlink_out.c")

if(MODE STREQUAL "bison")
    # The datadir the tool settled on must be the real one, and must be an
    # ordinary path: GetFinalPathNameByHandle hands back a \\?\ prefix, which
    # fopen() rejects, so it has to be stripped rather than passed along.
    execute_process(
        COMMAND "${link}" --print-datadir
        RESULT_VARIABLE rc
        OUTPUT_VARIABLE datadir
        ERROR_VARIABLE err
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "--print-datadir through the symlink exited with ${rc}\n${err}")
    endif()

    string(FIND "${datadir}" "?" qmark)
    if(NOT qmark EQUAL -1)
        message(FATAL_ERROR
            "datadir still carries a \\\\?\\ style prefix: ${datadir}")
    endif()

    if(NOT EXISTS "${datadir}/m4sugar/m4sugar.m4")
        message(FATAL_ERROR
            "datadir does not hold m4sugar/m4sugar.m4: ${datadir}")
    endif()
endif()

# And the tool has to actually run: for bison that means m4 expansion really
# found the skeletons, not merely that a plausible path was printed.
if(MODE STREQUAL "bison")
    execute_process(
        COMMAND "${link}" "${FIXTURE}" -o "${out}"
        WORKING_DIRECTORY "${SCRATCH}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE o ERROR_VARIABLE err)
else()
    execute_process(
        COMMAND "${link}" -o "${out}" "${FIXTURE}"
        WORKING_DIRECTORY "${SCRATCH}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE o ERROR_VARIABLE err)
endif()

if(NOT rc EQUAL 0)
    message(FATAL_ERROR
        "${exe_name} through the symlink exited with ${rc}\n--- stdout ---\n${o}\n--- stderr ---\n${err}")
endif()

if(NOT EXISTS "${out}")
    message(FATAL_ERROR "${exe_name} reported success but produced no ${out}")
endif()
