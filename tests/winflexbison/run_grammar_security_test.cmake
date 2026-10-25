# run_grammar_security_test.cmake — a grammar is untrusted input, so it must
# not be able to reach outside the build.
#
# Two bison CVEs, both fixed by backporting upstream's own commits:
#
#   CVE-2026-56390 (8d101c19): %output / %header name an output path, so a
#   grammar could write a file anywhere (e.g. "../x.h"). win_bison now rejects
#   any name with a path separator. Our check adds '\' and ':' to upstream's
#   '/', since on Windows both are separators ("..\x", "C:\x").
#
#   CVE-2026-56389 (3169c1e7): %define tool.xsltproc chose the program --html
#   runs, so a grammar could run anything. win_bison now always runs
#   "xsltproc" from PATH and ignores the define.
#
# Invoked as:
#   cmake -DTEST_EXE=<win_bison> -DSCRATCH=<dir>
#         -DUNSAFE_FIXTURE=<grammar> -DXSLT_FIXTURE=<grammar>
#         -DSAFE_FIXTURE=<grammar> -P run_grammar_security_test.cmake

foreach(v TEST_EXE SCRATCH UNSAFE_FIXTURE XSLT_FIXTURE SAFE_FIXTURE)
    if(NOT DEFINED ${v})
        message(FATAL_ERROR "run_grammar_security_test.cmake: ${v} not set")
    endif()
endforeach()

if(EXISTS "${SCRATCH}")
    file(REMOVE_RECURSE "${SCRATCH}")
endif()
# Run from a subdirectory, so "one level up" is inside SCRATCH where we can see
# it, not somewhere in the real tree.
set(WORK "${SCRATCH}/work")
file(MAKE_DIRECTORY "${WORK}")

# --- CVE-2026-56390: the traversal must be refused and write nothing. --------
file(COPY "${UNSAFE_FIXTURE}" DESTINATION "${WORK}")
get_filename_component(unsafe_name "${UNSAFE_FIXTURE}" NAME)
execute_process(
    COMMAND "${TEST_EXE}" "${unsafe_name}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)

if(rc EQUAL 0)
    message(FATAL_ERROR
        "win_bison accepted a %output/%header path containing '..'; it must "
        "reject it. See valid_output_file_name() in bison/src/parse-gram.y.")
endif()
if(NOT err MATCHES "invalid")
    message(FATAL_ERROR
        "win_bison rejected the unsafe path but not with the expected "
        "'invalid ... file name ignored' message:\n${err}")
endif()
# The whole point: nothing was written one level up.
foreach(leaked pwned_out.c pwned_hdr.h)
    if(EXISTS "${SCRATCH}/${leaked}")
        message(FATAL_ERROR
            "win_bison wrote ${leaked} outside the working directory -- the "
            "path-traversal guard failed (CVE-2026-56390).")
    endif()
endforeach()

# --- CVE-2026-56389: the grammar must not steer the --html tool. -------------
file(COPY "${XSLT_FIXTURE}" DESTINATION "${WORK}")
get_filename_component(xslt_name "${XSLT_FIXTURE}" NAME)
execute_process(
    COMMAND "${TEST_EXE}" --html "${xslt_name}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
# rc is expected non-zero here: xsltproc is usually not installed in CI, so the
# run fails to launch it. That is fine -- what must never happen is the grammar
# choosing the program. If the define still steered the tool, its sentinel name
# would show up in the output.
if(out MATCHES "SENTINEL_TOOL" OR err MATCHES "SENTINEL_TOOL")
    message(FATAL_ERROR
        "the grammar's %define tool.xsltproc reached the --html tool launcher; "
        "it must be ignored (CVE-2026-56389).\nstdout:\n${out}\nstderr:\n${err}")
endif()

# --- A legitimate grammar must still build. ----------------------------------
file(COPY "${SAFE_FIXTURE}" DESTINATION "${WORK}")
get_filename_component(safe_name "${SAFE_FIXTURE}" NAME)
execute_process(
    COMMAND "${TEST_EXE}" -d -o safe_out.c "${safe_name}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR
        "win_bison rejected a legitimate grammar after the security fixes "
        "(rc=${rc}); the guard is too strict.\nstderr:\n${err}")
endif()
if(NOT EXISTS "${WORK}/safe_out.c")
    message(FATAL_ERROR "win_bison produced no output for a legitimate grammar")
endif()
