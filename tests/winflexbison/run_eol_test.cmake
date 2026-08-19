# run_eol_test.cmake — CTest launcher for "generated output must not contain CR".
#
# win_flex/win_bison write LF, like upstream, not the CRLF that MSVC's text mode
# would produce. That guarantee has been lost before, silently: commits a32e862 /
# 31807e4 (2018) opened every output in binary mode and shipped it in 2.5.16, and
# the bison 3.3.1 re-vendor for 2.5.17 dropped the bison half of it -- scan-skel.c
# (the generated parser and header), print.c (.output) and print-graph.c (.dot)
# went back to text mode and stayed that way through 2.5.25. print-xml.c happened
# to survive, so 2.5.25 emits CRLF parsers next to an LF .xml.
#
# Nothing caught that for nine releases. The golden-diff harness normalizes CRLF
# before comparing (tests/bison/run_bison_test.cmake), the compile-run tests pass
# under either convention, and the MSYS2 autotest that would notice is marked
# allow_failures in .appveyor.yml, i.e. deliberately non-gating. This test is the
# gate: it is cheap, native, and fails the build.
#
# Invoked as:
#   cmake -DTEST_EXE=<tool> -DSCRATCH=<dir> -DFIXTURE=<grammar> -DMODE=bison|flex
#         [-DFIXTURE_CRLF=<grammar>] -P run_eol_test.cmake
#
# For bison, FIXTURE_CRLF adds a second pass over a grammar stored with CRLF line
# endings. That guards the other half of the fix -- reads stay in text mode, so a
# Windows-authored grammar still works and its CRs do not reach the output.
#
# The CRLF fixture is committed rather than generated here on purpose: CMake's
# file(WRITE) translates LF to CRLF on Windows, so writing "\r\n" from a script
# produces "\r\r\n", and the stray lone CRs are then legitimately copied into the
# parser as ordinary text -- a fixture bug that reads exactly like a tool bug.

foreach(v TEST_EXE SCRATCH FIXTURE MODE)
    if(NOT DEFINED ${v})
        message(FATAL_ERROR "run_eol_test.cmake: ${v} not set")
    endif()
endforeach()

if(EXISTS "${SCRATCH}")
    file(REMOVE_RECURSE "${SCRATCH}")
endif()
file(MAKE_DIRECTORY "${SCRATCH}")

# Fail on any CR byte. These are ASCII text outputs: a lone CR is no more
# expected than a CRLF, and treating both as failure keeps the check honest.
function(assert_no_cr path label)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR
            "${label}: expected output ${path} was not produced -- the test cannot "
            "pass by generating nothing")
    endif()

    file(READ "${path}" hex HEX)

    # Fast path: no "0d" anywhere in the hex means no CR byte, aligned or not.
    string(FIND "${hex}" "0d" maybe)
    if(maybe EQUAL -1)
        return()
    endif()

    # "0d" can also straddle two bytes (e.g. 0x?0 0xD?), so only now, on the rare
    # path, split into byte pairs and count the real ones.
    string(REGEX MATCHALL "[0-9a-f][0-9a-f]" bytes "${hex}")
    set(cr 0)
    foreach(b IN LISTS bytes)
        if(b STREQUAL "0d")
            math(EXPR cr "${cr}+1")
        endif()
    endforeach()

    if(cr GREATER 0)
        get_filename_component(name "${path}" NAME)
        message(FATAL_ERROR
            "${label}: ${name} contains ${cr} CR byte(s); generated output must be "
            "LF only. This is the 2.5.17 regression: check that bison's xfopen() "
            "still forces binary mode for writes (bison/src/files.c) and that "
            "flex's outputs are still opened \"wb\".")
    endif()
endfunction()

if(MODE STREQUAL "bison")
    # Every output kind bison can write, since they are opened independently and
    # 2.5.25 got four of the five wrong.
    set(args -d -v --graph=g.dot --xml=g.xml -o g.tab.c)
    set(outputs g.tab.c g.tab.h g.output g.dot g.xml)

    # Pass 1: the fixture as committed.
    file(COPY "${FIXTURE}" DESTINATION "${SCRATCH}")
    get_filename_component(fixture_name "${FIXTURE}" NAME)

    execute_process(
        COMMAND "${TEST_EXE}" ${args} "${fixture_name}"
        WORKING_DIRECTORY "${SCRATCH}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR
            "win_bison exited with ${rc}\n--- stdout ---\n${out}\n--- stderr ---\n${err}")
    endif()
    foreach(f IN LISTS outputs)
        assert_no_cr("${SCRATCH}/${f}" "LF input")
    endforeach()

    # Pass 2: a grammar stored with CRLF line endings. Reads stay in text mode so
    # the CRs are stripped on the way in; none may reach the output.
    if(DEFINED FIXTURE_CRLF)
        # The fixture only tests anything if it really is CRLF. A checkout that
        # normalized it (see tests/winflexbison/.gitattributes) must fail here
        # rather than silently repeat pass 1.
        file(READ "${FIXTURE_CRLF}" crlf_hex HEX)
        string(REGEX MATCHALL "0d0a" crlf_pairs "${crlf_hex}")
        list(LENGTH crlf_pairs crlf_count)
        if(crlf_count EQUAL 0)
            message(FATAL_ERROR
                "${FIXTURE_CRLF} has no CRLF line endings, so the CRLF pass would "
                "prove nothing. Check tests/winflexbison/.gitattributes -- the "
                "repo root's \"* text=auto\" must not be allowed to normalize it.")
        endif()

        file(COPY "${FIXTURE_CRLF}" DESTINATION "${SCRATCH}")
        get_filename_component(crlf_name "${FIXTURE_CRLF}" NAME)

        execute_process(
            COMMAND "${TEST_EXE}" ${args} "${crlf_name}"
            WORKING_DIRECTORY "${SCRATCH}"
            RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
        if(NOT rc EQUAL 0)
            message(FATAL_ERROR
                "win_bison failed on a CRLF-line-ending grammar with ${rc}\n"
                "--- stdout ---\n${out}\n--- stderr ---\n${err}")
        endif()
        foreach(f IN LISTS outputs)
            assert_no_cr("${SCRATCH}/${f}" "CRLF input")
        endforeach()
    endif()
else()
    file(COPY "${FIXTURE}" DESTINATION "${SCRATCH}")
    get_filename_component(fixture_name "${FIXTURE}" NAME)

    execute_process(
        COMMAND "${TEST_EXE}" --header-file=s.h -o s.c "${fixture_name}"
        WORKING_DIRECTORY "${SCRATCH}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR
            "win_flex exited with ${rc}\n--- stdout ---\n${out}\n--- stderr ---\n${err}")
    endif()
    foreach(f IN ITEMS s.c s.h)
        assert_no_cr("${SCRATCH}/${f}" "LF input")
    endforeach()
endif()
