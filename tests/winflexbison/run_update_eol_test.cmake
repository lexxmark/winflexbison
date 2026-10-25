# run_update_eol_test.cmake — CTest launcher for "--update keeps line endings".
#
# win_bison forces binary mode for every file it writes, so generated parsers
# are LF (see run_eol_test.cmake). --update/--fixit also writes through that
# path, but the file it writes is the user's own grammar, not generated output.
# Before this was fixed, a CRLF grammar came back entirely LF -- every line
# changed, not just the ones the fixits touched -- while the `<file>~` backup
# kept CRLF.
#
# Rule: the rewritten grammar keeps the line endings it had. A CRLF grammar
# stays CRLF, an LF grammar stays LF.
#
# Invoked as:
#   cmake -DTEST_EXE=<win_bison> -DSCRATCH=<dir> -DFIXTURE_LF=<grammar>
#         -DFIXTURE_CRLF=<grammar> -P run_update_eol_test.cmake
#
# Both fixtures must contain deprecated syntax, so that --update has something
# to rewrite.

foreach(v TEST_EXE SCRATCH FIXTURE_LF FIXTURE_CRLF)
    if(NOT DEFINED ${v})
        message(FATAL_ERROR "run_update_eol_test.cmake: ${v} not set")
    endif()
endforeach()

if(EXISTS "${SCRATCH}")
    file(REMOVE_RECURSE "${SCRATCH}")
endif()
file(MAKE_DIRECTORY "${SCRATCH}")

# Count CR bytes, LF bytes and CRLF pairs in a file.
function(count_eol path out_cr out_lf out_crlf)
    file(READ "${path}" hex HEX)
    string(REGEX MATCHALL "[0-9a-f][0-9a-f]" bytes "${hex}")
    set(cr 0)
    set(lf 0)
    set(crlf 0)
    set(prev "")
    foreach(b IN LISTS bytes)
        if(b STREQUAL "0d")
            math(EXPR cr "${cr}+1")
        elseif(b STREQUAL "0a")
            math(EXPR lf "${lf}+1")
            if(prev STREQUAL "0d")
                math(EXPR crlf "${crlf}+1")
            endif()
        endif()
        set(prev "${b}")
    endforeach()
    set(${out_cr} ${cr} PARENT_SCOPE)
    set(${out_lf} ${lf} PARENT_SCOPE)
    set(${out_crlf} ${crlf} PARENT_SCOPE)
endfunction()

# Run --update on a copy of FIXTURE, check that it was rewritten, and return
# the path of the rewritten copy.
function(run_update fixture label out_path)
    get_filename_component(name "${fixture}" NAME)
    file(MAKE_DIRECTORY "${SCRATCH}/${label}")
    file(COPY "${fixture}" DESTINATION "${SCRATCH}/${label}")
    set(copy "${SCRATCH}/${label}/${name}")
    file(READ "${copy}" before HEX)

    execute_process(
        COMMAND "${TEST_EXE}" --update "${name}"
        WORKING_DIRECTORY "${SCRATCH}/${label}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR
            "${label}: win_bison --update exited with ${rc}\n"
            "--- stdout ---\n${out}\n--- stderr ---\n${err}")
    endif()

    file(READ "${copy}" after HEX)
    if(after STREQUAL before)
        message(FATAL_ERROR
            "${label}: ${name} was not rewritten by --update, so this test "
            "proves nothing. The fixture must use deprecated syntax.")
    endif()

    # The fixits themselves must have landed, not just the line endings.
    file(READ "${copy}" text)
    if(NOT text MATCHES "%no-lines")
        message(FATAL_ERROR "${label}: %no_lines was not rewritten to %no-lines:\n${text}")
    endif()

    set(${out_path} "${copy}" PARENT_SCOPE)
endfunction()

# CRLF grammar: must come back CRLF on every line.
file(READ "${FIXTURE_CRLF}" hex HEX)
if(NOT hex MATCHES "0d0a")
    message(FATAL_ERROR
        "${FIXTURE_CRLF} has no CRLF line endings, so this test would prove "
        "nothing. Check tests/winflexbison/.gitattributes -- the repo root's "
        "\"* text=auto\" must not be allowed to normalize it.")
endif()

run_update("${FIXTURE_CRLF}" "crlf" crlf_copy)
count_eol("${crlf_copy}" cr lf crlf)
if(NOT (cr EQUAL lf AND lf EQUAL crlf))
    message(FATAL_ERROR
        "crlf: --update changed the line endings of a CRLF grammar: it now has "
        "${lf} LF, ${cr} CR and ${crlf} CRLF. Every line must still end in CRLF. "
        "Check fixits_run() in bison/src/fixits.c.")
endif()

# LF grammar: must stay LF.
run_update("${FIXTURE_LF}" "lf" lf_copy)
count_eol("${lf_copy}" cr lf crlf)
if(NOT cr EQUAL 0)
    message(FATAL_ERROR
        "lf: --update added ${cr} CR byte(s) to an LF grammar. It must stay LF. "
        "Check fixits_run() in bison/src/fixits.c.")
endif()
