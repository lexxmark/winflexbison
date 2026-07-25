# Flex test suite (adapted from upstream flex tests, kept version-matched with
# the vendored flex under flex/src). See docs/specs/03-test-adoption/spec.md.
#
# Phase 1: single-file, C, exit-code "simple" tests. For each case we generate
# the scanner with the just-built win_flex, compile it, and run it under CTest
# feeding the matching .txt on stdin; pass == exit 0 (upstream's own criterion).
# win_flex needs no external m4, so scanners are generated at build time and no
# generated .c is committed.

set(FLEX_CASES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cases")
set(FLEX_RUNNER    "${CMAKE_CURRENT_SOURCE_DIR}/run_scanner_test.cmake")

set(FLEX_SIMPLE_TESTS
    alloc_extra
    array_nr
    array_r
    basic_nr
    basic_r
    ccl
    debug_nr
    debug_r
    extended
    mem_nr
    mem_r
    posix
    prefix_nr
    prefix_r
    quote_in_comment
    quotes
    string_nr
    string_r
    yyextra)

foreach(name ${FLEX_SIMPLE_TESTS})
    set(src "${FLEX_CASES_DIR}/${name}.l")
    set(gen "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")

    # --wincompat swaps <unistd.h> for <io.h> and isatty/fileno for their _
    # prefixed MSVC analogs; it is the winflexbison-recommended flag for making
    # generated scanners compile on Windows, so the suite exercises that path.
    add_custom_command(
        OUTPUT  "${gen}"
        COMMAND $<TARGET_FILE:win_flex> --wincompat -o "${gen}" "${src}"
        DEPENDS win_flex "${src}"
        COMMENT "flex: generating ${name}.c"
        VERBATIM)

    add_executable(flextest_${name} "${gen}")
    # winflexbison_common PUBLICly exposes misc/ which provides config.h.
    target_link_libraries(flextest_${name} PRIVATE winflexbison_common)
    target_include_directories(flextest_${name} PRIVATE "${CMAKE_SOURCE_DIR}/flex/src")

    set(input "${FLEX_CASES_DIR}/${name}.txt")
    if(NOT EXISTS "${input}")
        set(input "")
    endif()

    add_test(NAME flex.${name}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_EXE=$<TARGET_FILE:flextest_${name}>
            "-DINPUT=${input}"
            -P "${FLEX_RUNNER}")
    set_tests_properties(flex.${name} PROPERTIES
        SKIP_RETURN_CODE 77
        LABELS "flex")
endforeach()

# ---------------------------------------------------------------------------
# Phase 2: multi-file C exit-code tests. Each combines one or more generated
# scanners (whose .l carries %option header="<stem>.h", so win_flex emits both
# <stem>.c and <stem>.h) with a hand-written *_main.c. Scanners are generated
# into the build dir (WORKING_DIRECTORY) so the bare header= filename lands
# next to the .c and the main can #include it.
#
#   add_flex_multifile_test(<name> [INPUT <file.txt>]
#                           SCANNERS <a.l> [<b.l> ...]
#                           SRC      <main.c> [...])
function(add_flex_multifile_test name)
    cmake_parse_arguments(T "" "INPUT" "SCANNERS;SRC" ${ARGN})

    set(gen_srcs "")
    foreach(lscan ${T_SCANNERS})
        get_filename_component(stem "${lscan}" NAME_WE)
        set(csrc "${CMAKE_CURRENT_BINARY_DIR}/${stem}.c")
        set(hsrc "${CMAKE_CURRENT_BINARY_DIR}/${stem}.h")
        add_custom_command(
            OUTPUT  "${csrc}" "${hsrc}"
            COMMAND $<TARGET_FILE:win_flex> --wincompat -o "${stem}.c" "${FLEX_CASES_DIR}/${lscan}"
            DEPENDS win_flex "${FLEX_CASES_DIR}/${lscan}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "flex: generating ${stem}.c/.h"
            VERBATIM)
        list(APPEND gen_srcs "${csrc}")
    endforeach()

    set(main_srcs "")
    foreach(m ${T_SRC})
        list(APPEND main_srcs "${FLEX_CASES_DIR}/${m}")
    endforeach()

    add_executable(flextest_${name} ${gen_srcs} ${main_srcs})
    target_link_libraries(flextest_${name} PRIVATE winflexbison_common)
    target_include_directories(flextest_${name} PRIVATE
        "${CMAKE_SOURCE_DIR}/flex/src"
        "${CMAKE_CURRENT_BINARY_DIR}")

    set(input "")
    if(T_INPUT)
        set(input "${FLEX_CASES_DIR}/${T_INPUT}")
    endif()

    add_test(NAME flex.${name}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_EXE=$<TARGET_FILE:flextest_${name}>
            "-DINPUT=${input}"
            -P "${FLEX_RUNNER}")
    set_tests_properties(flex.${name} PROPERTIES
        SKIP_RETURN_CODE 77
        LABELS "flex")
endfunction()

add_flex_multifile_test(header_nr
    INPUT header_nr.txt
    SCANNERS header_nr_scanner.l
    SRC      header_nr_main.c)

add_flex_multifile_test(header_r
    INPUT header_r.txt
    SCANNERS header_r_scanner.l
    SRC      header_r_main.c)

add_flex_multifile_test(top
    INPUT top.txt
    SCANNERS top.l
    SRC      top_main.c)

add_flex_multifile_test(multiple_scanners_nr
    SCANNERS multiple_scanners_nr_1.l multiple_scanners_nr_2.l
    SRC      multiple_scanners_nr_main.c)

add_flex_multifile_test(multiple_scanners_r
    SCANNERS multiple_scanners_r_1.l multiple_scanners_r_2.l
    SRC      multiple_scanners_r_main.c)
