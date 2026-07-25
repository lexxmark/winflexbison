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
