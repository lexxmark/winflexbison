# run_bison_test.cmake — CTest golden-diff launcher for bison diagnostics.
#
# Runs win_bison on a grammar (from WORKDIR, by bare filename so diagnostics
# print a stable "<name>.y:line:" path) and compares its stdout / stderr / exit
# code against golden captured from the reference bison 3.8.2 (generate.sh).
# The actual output is normalized for Windows-isms before comparison:
#   * CRLF -> LF
#   * program-name prefix "win_bison[.exe]:" -> "bison:"
#   * backslash path separators -> "/"
#
# Invoked as:
#   cmake -DTEST_EXE=<win_bison> -DWORKDIR=<cases> -DGRAMMAR=<name.y>
#         -DGOLDEN=<golden-dir> -DNAME=<name> [-DARGS=<a\;b\;c>]
#         -P run_bison_test.cmake

foreach(v TEST_EXE WORKDIR GRAMMAR GOLDEN NAME)
    if(NOT DEFINED ${v})
        message(FATAL_ERROR "run_bison_test.cmake: ${v} not set")
    endif()
endforeach()

# -fno-caret: match generate.sh — compare the semantic diagnostic, not the
# fragile caret/source-line rendering.
set(cmd "${TEST_EXE}" "-fno-caret")
if(DEFINED ARGS AND NOT "${ARGS}" STREQUAL "")
    foreach(a ${ARGS})
        list(APPEND cmd "${a}")
    endforeach()
endif()
list(APPEND cmd "${GRAMMAR}")

execute_process(
    COMMAND ${cmd}
    WORKING_DIRECTORY "${WORKDIR}"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE  err)

# Clean up any parser/report files bison may have written into the cases dir.
file(REMOVE
    "${WORKDIR}/${NAME}.tab.c" "${WORKDIR}/${NAME}.tab.h"
    "${WORKDIR}/${NAME}.output")

function(normalize var)
    set(s "${${var}}")
    string(REPLACE "\r\n" "\n" s "${s}")
    string(REPLACE "\r" "\n" s "${s}")
    string(REGEX REPLACE "win_bison(\\.exe)?:" "bison:" s "${s}")
    string(REPLACE "\\" "/" s "${s}")
    # Bison's gnulib 'quote' module uses locale-dependent quotes: reference
    # bison on a UTF-8 Linux emits U+2018/U+2019 (and U+201C/D); win_bison emits
    # ASCII '. Fold the fancy quotes to ASCII so both compare equal.
    string(REPLACE "‘" "'" s "${s}")
    string(REPLACE "’" "'" s "${s}")
    string(REPLACE "“" "\"" s "${s}")
    string(REPLACE "”" "\"" s "${s}")
    set(${var} "${s}" PARENT_SCOPE)
endfunction()

normalize(out)
normalize(err)

# Read golden (missing golden file == expected empty).
set(gold_out "")
set(gold_err "")
if(EXISTS "${GOLDEN}/${NAME}.out")
    file(READ "${GOLDEN}/${NAME}.out" gold_out)
endif()
if(EXISTS "${GOLDEN}/${NAME}.err")
    file(READ "${GOLDEN}/${NAME}.err" gold_err)
endif()
normalize(gold_out)
normalize(gold_err)

set(gold_exit 0)
if(EXISTS "${GOLDEN}/${NAME}.exit")
    file(READ "${GOLDEN}/${NAME}.exit" gold_exit)
    string(STRIP "${gold_exit}" gold_exit)
endif()

set(fail "")
if(NOT "${rc}" STREQUAL "${gold_exit}")
    string(APPEND fail "  exit: got ${rc}, want ${gold_exit}\n")
endif()
if(NOT "${err}" STREQUAL "${gold_err}")
    string(APPEND fail "--- stderr GOT ---\n${err}\n--- stderr WANT ---\n${gold_err}\n")
endif()
if(NOT "${out}" STREQUAL "${gold_out}")
    string(APPEND fail "--- stdout GOT ---\n${out}\n--- stdout WANT ---\n${gold_out}\n")
endif()

if(NOT "${fail}" STREQUAL "")
    message(FATAL_ERROR "bison golden mismatch for ${NAME}:\n${fail}")
endif()
