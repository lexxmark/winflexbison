# Bison test suite. Two flavors (see docs/specs/03-test-adoption.md, BISON):
#
#   * compile-run (exit-code) — win_bison generates a self-contained parser,
#     MSVC compiles it, and the exe is run; pass == exit 0. No golden needed;
#     this is the truest flex-style analog and proves the Windows/MSVC path.
#
#   * golden-diff — win_bison runs on a grammar and its stdout/stderr/exit are
#     compared against golden captured from the reference bison 3.8.2. The
#     golden files are generated once under MSYS2 by generate.sh and committed.
#     Registered only when the golden dir is populated.

set(BISON_CASES_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/cases")
set(BISON_GOLDEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden")
set(BISON_RUN_PARSER "${CMAKE_CURRENT_SOURCE_DIR}/run_parser_test.cmake")
set(BISON_RUN_GOLDEN "${CMAKE_CURRENT_SOURCE_DIR}/run_bison_test.cmake")

# ---------------------------------------------------------------------------
# compile-run (exit-code) tests.
#   add_bison_run_test(<name> GRAMMAR <file.y> [INPUT <file.txt>])
function(add_bison_run_test name)
    cmake_parse_arguments(T "" "GRAMMAR;INPUT" "" ${ARGN})
    set(gen "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
    add_custom_command(
        OUTPUT  "${gen}"
        COMMAND $<TARGET_FILE:win_bison> -o "${gen}" "${BISON_CASES_DIR}/${T_GRAMMAR}"
        DEPENDS win_bison "${BISON_CASES_DIR}/${T_GRAMMAR}" ${WFB_BISON_SKELETONS}
        COMMENT "bison: generating ${name}.c"
        VERBATIM)
    add_executable(bisontest_${name} "${gen}")
    set(input "")
    if(T_INPUT)
        set(input "${BISON_CASES_DIR}/${T_INPUT}")
    endif()
    add_test(NAME bison.${name}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_EXE=$<TARGET_FILE:bisontest_${name}>
            "-DINPUT=${input}"
            -P "${BISON_RUN_PARSER}")
    set_tests_properties(bison.${name} PROPERTIES SKIP_RETURN_CODE 77 LABELS "bison")
endfunction()

add_bison_run_test(calc GRAMMAR calc.y INPUT calc.txt)

# ---------------------------------------------------------------------------
# Issue #7: a YYSTYPE typedef declared in the grammar's prologue must
# propagate into the generated --defines header too, not just the .c (a
# regression once left the header's YYSTYPE defaulted to int, disagreeing
# with the .c and causing memory errors). Needs its own custom command since
# add_bison_run_test() above doesn't request -d/--defines.
set(_ystype_gen "${CMAKE_CURRENT_BINARY_DIR}/ystype_header.c")
set(_ystype_hdr "${CMAKE_CURRENT_BINARY_DIR}/ystype_header.h")
add_custom_command(
    OUTPUT  "${_ystype_gen}" "${_ystype_hdr}"
    COMMAND $<TARGET_FILE:win_bison> -d -o "${_ystype_gen}" "${BISON_CASES_DIR}/ystype_header.y"
    DEPENDS win_bison "${BISON_CASES_DIR}/ystype_header.y" ${WFB_BISON_SKELETONS}
    COMMENT "bison: generating ystype_header.c/.h"
    VERBATIM)
add_executable(bisontest_ystype_header "${_ystype_gen}" "${BISON_CASES_DIR}/ystype_header_main.c")
target_include_directories(bisontest_ystype_header PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME bison.ystype_header
    COMMAND ${CMAKE_COMMAND}
        -DTEST_EXE=$<TARGET_FILE:bisontest_ystype_header>
        -P "${BISON_RUN_PARSER}")
set_tests_properties(bison.ystype_header PROPERTIES SKIP_RETURN_CODE 77 LABELS "bison")

# ---------------------------------------------------------------------------
# Issue #44: %define lr.type canonical-lr / ielr must actually be honored
# (not silently ignored). Same grammar body as calc.y, reused as two fixtures
# each adding the respective %define; pass = win_bison accepts the option and
# the generated parser still parses calc.txt correctly.
add_bison_run_test(lr_type_canonical GRAMMAR lr_type_canonical.y INPUT calc.txt)
add_bison_run_test(lr_type_ielr      GRAMMAR lr_type_ielr.y      INPUT calc.txt)

# ---------------------------------------------------------------------------
# Issue #95: YYPTRDIFF_T must be pointer-wide. MSVC advertises neither
# __PTRDIFF_TYPE__ nor PTRDIFF_MAX (the latter only via <stdint.h>, which the
# skeleton includes only for C99+), so the width ladder fell through to
# "long" and every x64 build warned C4244. The grammar asserts the widths
# agree at run time; see its header comment for why it must not include
# <stdint.h> itself. Vacuous on Win32, where both types are 32 bits.
add_bison_run_test(ptrdiff_width GRAMMAR ptrdiff_width.y)

# ---------------------------------------------------------------------------
# Issue #48: win_bison must resolve its data/ directory via the executable's
# own location (get_app_path()), not the process's current working directory.
# No compile step needed: just invoke win_bison directly from an unrelated
# WORKING_DIRECTORY with an absolute grammar path; CTest's default
# nonzero-exit-is-fail already catches the historical "cannot open
# data/m4sugar/m4sugar.m4" failure.
add_test(NAME bison.data_dir_from_exe_location
    COMMAND $<TARGET_FILE:win_bison> "${BISON_CASES_DIR}/calc.y"
        -o "${CMAKE_CURRENT_BINARY_DIR}/data_dir_check_out.c"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set_tests_properties(bison.data_dir_from_exe_location PROPERTIES LABELS "bison")

# ---------------------------------------------------------------------------
# Issue #62: `win_bison --update file.y` must not fail with "cannot backup:
# Permission denied" on Windows, and must actually rewrite the file in place
# and leave a `<file>~` backup. update_deprecated.y uses the deprecated
# underscore form `%no_lines` (rewritten to `%no-lines`), which is enabled by
# default (Wdeprecated is part of bison's default warning set).
add_test(NAME bison.update_flag
    COMMAND ${CMAKE_COMMAND}
        -DTEST_EXE=$<TARGET_FILE:win_bison>
        "-DFIXTURE=${BISON_CASES_DIR}/update_deprecated.y"
        "-DSCRATCH=${CMAKE_CURRENT_BINARY_DIR}/update_scratch"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/run_update_test.cmake")
set_tests_properties(bison.update_flag PROPERTIES LABELS "bison")

# ---------------------------------------------------------------------------
# C++ compile-run tests: win_bison --language=c++, compiled as C++. First
# native C++ bison fixtures in the CTest suite (the only prior C++ bison
# grammar lives in the separate, opt-in MSYS2 GNU-Autotest harness).
#   add_bison_cxx_run_test(<name> GRAMMAR <file.y> [SRC <main.cc> ...] [INPUT <file.txt>])
function(add_bison_cxx_run_test name)
    cmake_parse_arguments(T "" "GRAMMAR;INPUT" "SRC" ${ARGN})
    set(gen "${CMAKE_CURRENT_BINARY_DIR}/${name}.cc")
    set(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}.hh")
    add_custom_command(
        OUTPUT  "${gen}" "${hdr}"
        COMMAND $<TARGET_FILE:win_bison> --language=c++ -d -o "${gen}" "${BISON_CASES_DIR}/${T_GRAMMAR}"
        DEPENDS win_bison "${BISON_CASES_DIR}/${T_GRAMMAR}" ${WFB_BISON_SKELETONS}
        COMMENT "bison(c++): generating ${name}.cc/.hh"
        VERBATIM)

    set(main_srcs "")
    foreach(m ${T_SRC})
        list(APPEND main_srcs "${BISON_CASES_DIR}/${m}")
    endforeach()

    add_executable(bisontest_${name} "${gen}" ${main_srcs})
    target_include_directories(bisontest_${name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

    set(input "")
    if(T_INPUT)
        set(input "${BISON_CASES_DIR}/${T_INPUT}")
    endif()
    add_test(NAME bison.${name}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_EXE=$<TARGET_FILE:bisontest_${name}>
            "-DINPUT=${input}"
            -P "${BISON_RUN_PARSER}")
    set_tests_properties(bison.${name} PROPERTIES SKIP_RETURN_CODE 77 LABELS "bison;cxx")
endfunction()

# Issue #10: %language "c++" + %locations MSVC compile fix (location.cc-
# equivalent negate/cast ordering). main exercises @1/@$ to force real use of
# the generated location code, not just its presence.
add_bison_cxx_run_test(cxx_locations
    GRAMMAR cxx_locations.y
    SRC     cxx_locations_main.cc)

# Issue #67: 800+ distinct tokens made the C++ skeleton's YY_ASSERT line in
# symbol_type constructors exceed MSVC's ~21740-char C2026 single-line limit
# (fixed upstream in bison 3.7.4). many_tokens.y is committed directly with
# ~900 tokens; compiling clean is the whole assertion.
add_bison_cxx_run_test(many_tokens
    GRAMMAR many_tokens.y
    SRC     many_tokens_main.cc)
if(MSVC)
    # With ~900 tokens the C++ skeleton writes `switch` blocks that have only
    # a `default:` label (C4065 x13). That comes from the skeleton, not from
    # this test, so turn the warning off.
    # See docs/build-warnings-plan.md, Phase 5.
    target_compile_options(bisontest_many_tokens PRIVATE /wd4065)
endif()

# ---------------------------------------------------------------------------
# golden-diff tests. One per grammar in golden-cases/, comparing win_bison's
# stdout/stderr/exit against golden/ (generated by generate.sh under MSYS2 from
# reference bison 3.8.2). A grammar is only registered when its golden exists,
# so a fresh checkout without golden simply has no bison golden tests rather
# than spurious failures. An optional <name>.args file supplies extra flags.
file(GLOB _bison_golden_grammars
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/golden-cases"
    "${CMAKE_CURRENT_SOURCE_DIR}/golden-cases/*.y")

foreach(gy ${_bison_golden_grammars})
    get_filename_component(gname "${gy}" NAME_WE)
    if(NOT EXISTS "${BISON_GOLDEN_DIR}/${gname}.exit")
        continue()   # golden not generated yet
    endif()

    set(gargs "")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/golden-cases/${gname}.args")
        file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/golden-cases/${gname}.args" _lines)
        foreach(_l ${_lines})
            separate_arguments(_toks NATIVE_COMMAND "${_l}")
            list(APPEND gargs ${_toks})
        endforeach()
    endif()
    # Pass args as a CMake list encoded with escaped ';' so it survives as one -D.
    string(REPLACE ";" "\\;" gargs_enc "${gargs}")

    add_test(NAME bison.golden.${gname}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_EXE=$<TARGET_FILE:win_bison>
            "-DWORKDIR=${CMAKE_CURRENT_SOURCE_DIR}/golden-cases"
            "-DGRAMMAR=${gy}"
            "-DGOLDEN=${BISON_GOLDEN_DIR}"
            "-DNAME=${gname}"
            "-DARGS=${gargs_enc}"
            -P "${BISON_RUN_GOLDEN}")
    set_tests_properties(bison.golden.${gname} PROPERTIES
        SKIP_RETURN_CODE 77 LABELS "bison")
endforeach()
