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

# ---------------------------------------------------------------------------
# Phase 3: C++ exit-code tests. Two input flavors, both compiled as C++:
#   .ll  -> a real C++ scanner  (win_flex -+, generated <stem>.cc)
#   .lll -> C output compiled as C++ (win_flex without -+, output named .cc)
# The single-file cases carry their own main() in section 3 (%option nomain);
# cxx_multiple_scanners links a separate *_main.cc against two scanners.
#
#   add_flex_cxx_test(<name> [INPUT <file.txt>]
#                     SCANNERS <a.ll|a.lll> [...]
#                     [SRC <main.cc> ...])
function(add_flex_cxx_test name)
    cmake_parse_arguments(T "" "INPUT" "SCANNERS;SRC" ${ARGN})

    set(gen_srcs "")
    foreach(lscan ${T_SCANNERS})
        get_filename_component(stem "${lscan}" NAME_WE)
        get_filename_component(ext  "${lscan}" EXT)
        set(gen "${CMAKE_CURRENT_BINARY_DIR}/${stem}.cc")
        set(cxxflag "")
        if(ext STREQUAL ".ll")
            set(cxxflag "-+")   # emit a C++ (yyFlexLexer) scanner
        endif()
        add_custom_command(
            OUTPUT  "${gen}"
            COMMAND $<TARGET_FILE:win_flex> ${cxxflag} --wincompat -o "${gen}" "${FLEX_CASES_DIR}/${lscan}"
            DEPENDS win_flex "${FLEX_CASES_DIR}/${lscan}"
            COMMENT "flex(c++): generating ${stem}.cc"
            VERBATIM)
        list(APPEND gen_srcs "${gen}")
    endforeach()

    set(main_srcs "")
    foreach(m ${T_SRC})
        list(APPEND main_srcs "${FLEX_CASES_DIR}/${m}")
    endforeach()

    add_executable(flextest_${name} ${gen_srcs} ${main_srcs})
    # Deliberately do NOT link winflexbison_common here: its PUBLIC include
    # dirs pull in common/m4/lib, gnulib's replacement <stdlib.h>/<stdint.h>
    # which macroize restrict/inline for old C compilers and are illegal in
    # C++. The C++ scanners are self-contained, so we only need config.h (from
    # common/misc, without the gnulib shadow headers) and <FlexLexer.h>.
    target_include_directories(flextest_${name} PRIVATE
        "${CMAKE_SOURCE_DIR}/common/misc"
        "${CMAKE_SOURCE_DIR}/flex/src"
        "${CMAKE_CURRENT_BINARY_DIR}")
    # The root CMakeLists globally does -Dinline=__inline -Drestrict=__restrict
    # for the C product; in C++ these macroize keywords (illegal, e.g. MSVC's
    # <xkeycheck.h> and __declspec(restrict) in the CRT). Undefine them here.
    if(MSVC)
        target_compile_options(flextest_${name} PRIVATE /Uinline /Urestrict)
    endif()

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

add_flex_cxx_test(cxx_basic   INPUT cxx_basic.txt   SCANNERS cxx_basic.ll)
add_flex_cxx_test(cxx_restart INPUT cxx_restart.txt SCANNERS cxx_restart.ll)
add_flex_cxx_test(c_cxx_nr    INPUT c_cxx_nr.txt    SCANNERS c_cxx_nr.lll)
add_flex_cxx_test(c_cxx_r     INPUT c_cxx_r.txt     SCANNERS c_cxx_r.lll)
add_flex_cxx_test(cxx_multiple_scanners
    INPUT    cxx_multiple_scanners.txt
    SCANNERS cxx_multiple_scanners_1.ll cxx_multiple_scanners_2.ll
    SRC      cxx_multiple_scanners_main.cc)

# ---------------------------------------------------------------------------
# Phase 4: reject + table tests. All four are generated from the single
# reject.l4 template with different win_flex flags and compile defines:
#   reject_nr  : plain                          -> reads stdin
#   reject_r   : --reentrant, -DTEST_IS_REENTRANT -> reads stdin
#   reject_ver : --tables-verify --tables-file=  -> external tables, verifies
#                -DTEST_HAS_TABLES_EXTERNAL         and exits 0
#   reject_ser : --tables-file=                  -> external tables, then scans
#                -DTEST_HAS_TABLES_EXTERNAL
# reject.l4 uses m4 macros in section 3, so --unsafe-no-m4-sect3-escape is
# required (win_flex expands them). Table tests take the .tables path and the
# input path as argv (not stdin).
#
#   add_flex_generated_test(<name> SRC <file.l4>
#       [FLEXFLAGS <...>] [DEFINES <...>] [TABLES]
#       [INPUT <txt>] [INPUT_AS_ARG])
function(add_flex_generated_test name)
    cmake_parse_arguments(T "TABLES;STDIN_INPUT" "SRC;INPUT" "FLEXFLAGS;DEFINES" ${ARGN})

    set(gen "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
    set(outputs "${gen}")
    set(tables_flag "")
    if(T_TABLES)
        set(tables "${CMAKE_CURRENT_BINARY_DIR}/${name}.tables")
        list(APPEND outputs "${tables}")
        set(tables_flag "--tables-file=${name}.tables")
    endif()

    add_custom_command(
        OUTPUT  ${outputs}
        COMMAND $<TARGET_FILE:win_flex> --wincompat ${T_FLEXFLAGS} ${tables_flag}
                -o "${name}.c" "${FLEX_CASES_DIR}/${T_SRC}"
        DEPENDS win_flex "${FLEX_CASES_DIR}/${T_SRC}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "flex: generating ${name}.c"
        VERBATIM)

    add_executable(flextest_${name} "${gen}")
    target_link_libraries(flextest_${name} PRIVATE winflexbison_common)
    target_include_directories(flextest_${name} PRIVATE "${CMAKE_SOURCE_DIR}/flex/src")
    if(T_TABLES)
        # Serialized-tables output #includes <netinet/in.h> (ntohs/ntohl); MSVC
        # lacks it, so provide the test shim (see tests/flex/shim/netinet/in.h).
        target_include_directories(flextest_${name} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/shim")
    endif()
    if(T_DEFINES)
        target_compile_definitions(flextest_${name} PRIVATE ${T_DEFINES})
    endif()

    set(test_args
        -DTEST_EXE=$<TARGET_FILE:flextest_${name}>)
    if(T_TABLES)
        # argv[1] = tables file. Input goes either on stdin (STDIN_INPUT, e.g.
        # tableopts, whose reentrant main relies on the default stdin yyin) or
        # as argv[2] (reject_ver/ser, whose main opens argv[2]).
        list(APPEND test_args -DARG1=${tables})
        if(T_INPUT)
            if(T_STDIN_INPUT)
                list(APPEND test_args "-DINPUT=${FLEX_CASES_DIR}/${T_INPUT}")
            else()
                list(APPEND test_args "-DARG2=${FLEX_CASES_DIR}/${T_INPUT}")
            endif()
        endif()
    elseif(T_INPUT)
        list(APPEND test_args "-DINPUT=${FLEX_CASES_DIR}/${T_INPUT}")
    endif()

    add_test(NAME flex.${name}
        COMMAND ${CMAKE_COMMAND} ${test_args} -P "${FLEX_RUNNER}")
    set_tests_properties(flex.${name} PROPERTIES
        SKIP_RETURN_CODE 77
        LABELS "flex")
endfunction()

add_flex_generated_test(reject_nr
    SRC reject.l4  INPUT reject.txt
    FLEXFLAGS --unsafe-no-m4-sect3-escape)

add_flex_generated_test(reject_r
    SRC reject.l4  INPUT reject.txt
    FLEXFLAGS --unsafe-no-m4-sect3-escape --reentrant
    DEFINES   TEST_IS_REENTRANT)

add_flex_generated_test(reject_ver
    SRC reject.l4  INPUT reject.txt  TABLES
    FLEXFLAGS --unsafe-no-m4-sect3-escape --tables-verify
    DEFINES   TEST_HAS_TABLES_EXTERNAL)

add_flex_generated_test(reject_ser
    SRC reject.l4  INPUT reject.txt  TABLES
    FLEXFLAGS --unsafe-no-m4-sect3-escape
    DEFINES   TEST_HAS_TABLES_EXTERNAL)

# ---------------------------------------------------------------------------
# Phase 5a: "direct" tests. These run from the cases dir with the input file as
# argv[1]; the scanner opens secondary include files (_2/_3) by relative name,
# so the working directory must be the cases dir (mirrors testwrapper-direct.sh).
function(add_flex_direct_test name)
    cmake_parse_arguments(T "" "SCANNER;INPUT" "" ${ARGN})
    set(gen "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
    add_custom_command(
        OUTPUT  "${gen}"
        COMMAND $<TARGET_FILE:win_flex> --wincompat -o "${gen}" "${FLEX_CASES_DIR}/${T_SCANNER}"
        DEPENDS win_flex "${FLEX_CASES_DIR}/${T_SCANNER}"
        COMMENT "flex: generating ${name}.c"
        VERBATIM)
    add_executable(flextest_${name} "${gen}")
    target_link_libraries(flextest_${name} PRIVATE winflexbison_common)
    target_include_directories(flextest_${name} PRIVATE "${CMAKE_SOURCE_DIR}/flex/src")
    add_test(NAME flex.${name}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_EXE=$<TARGET_FILE:flextest_${name}>
            "-DARG1=${FLEX_CASES_DIR}/${T_INPUT}"
            "-DWORKDIR=${FLEX_CASES_DIR}"
            -P "${FLEX_RUNNER}")
    set_tests_properties(flex.${name} PROPERTIES SKIP_RETURN_CODE 77 LABELS "flex")
endfunction()

add_flex_direct_test(include_by_buffer    SCANNER include_by_buffer.direct.l    INPUT include_by_buffer.direct.txt)
add_flex_direct_test(include_by_push      SCANNER include_by_push.direct.l      INPUT include_by_push.direct.txt)
add_flex_direct_test(include_by_reentrant SCANNER include_by_reentrant.direct.l INPUT include_by_reentrant.direct.txt)
add_flex_direct_test(rescan_nr            SCANNER rescan_nr.direct.l            INPUT rescan_nr.direct.txt)
add_flex_direct_test(rescan_r            SCANNER rescan_r.direct.l            INPUT rescan_r.direct.txt)

# ---------------------------------------------------------------------------
# Phase 5b: lineno comparison-mode tests. Pass == the scanner's computed
# yylineno equals the reference newline count (run twice, outputs compared).
set(FLEX_COMPARE_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/run_compare_test.cmake")
function(add_flex_compare_test name scanner input)
    set(gen "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
    add_custom_command(
        OUTPUT  "${gen}"
        COMMAND $<TARGET_FILE:win_flex> --wincompat -o "${gen}" "${FLEX_CASES_DIR}/${scanner}"
        DEPENDS win_flex "${FLEX_CASES_DIR}/${scanner}"
        COMMENT "flex: generating ${name}.c"
        VERBATIM)
    add_executable(flextest_${name} "${gen}")
    target_link_libraries(flextest_${name} PRIVATE winflexbison_common)
    target_include_directories(flextest_${name} PRIVATE "${CMAKE_SOURCE_DIR}/flex/src")
    add_test(NAME flex.${name}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_EXE=$<TARGET_FILE:flextest_${name}>
            "-DINPUT=${FLEX_CASES_DIR}/${input}"
            -P "${FLEX_COMPARE_RUNNER}")
    set_tests_properties(flex.${name} PROPERTIES SKIP_RETURN_CODE 77 LABELS "flex")
endfunction()

add_flex_compare_test(lineno_nr       lineno_nr.l       lineno_nr.one.txt)
add_flex_compare_test(lineno_r        lineno_r.l        lineno_r.one.txt)
add_flex_compare_test(lineno_trailing lineno_trailing.l lineno_trailing.one.txt)

# ---------------------------------------------------------------------------
# Phase 5c: posixly_correct — generated with POSIXLY_CORRECT=1 in the flex env.
add_custom_command(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/posixly_correct.c"
    COMMAND ${CMAKE_COMMAND} -E env POSIXLY_CORRECT=1
            $<TARGET_FILE:win_flex> --wincompat
            -o "${CMAKE_CURRENT_BINARY_DIR}/posixly_correct.c"
            "${FLEX_CASES_DIR}/posixly_correct.l"
    DEPENDS win_flex "${FLEX_CASES_DIR}/posixly_correct.l"
    COMMENT "flex: generating posixly_correct.c (POSIXLY_CORRECT=1)"
    VERBATIM)
add_executable(flextest_posixly_correct "${CMAKE_CURRENT_BINARY_DIR}/posixly_correct.c")
target_link_libraries(flextest_posixly_correct PRIVATE winflexbison_common)
target_include_directories(flextest_posixly_correct PRIVATE "${CMAKE_SOURCE_DIR}/flex/src")
add_test(NAME flex.posixly_correct
    COMMAND ${CMAKE_COMMAND}
        -DTEST_EXE=$<TARGET_FILE:flextest_posixly_correct>
        -P "${FLEX_RUNNER}")
set_tests_properties(flex.posixly_correct PROPERTIES SKIP_RETURN_CODE 77 LABELS "flex")

# ---------------------------------------------------------------------------
# Phase 5d: cxx_yywrap — C++ scanner whose main() takes input file(s) as argv
# (the .i3 case feeds the same input three times). Uses the C++ include/flag
# workarounds from add_flex_cxx_test().
add_custom_command(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/cxx_yywrap.cc"
    COMMAND $<TARGET_FILE:win_flex> -+ --wincompat -o "cxx_yywrap.cc" "${FLEX_CASES_DIR}/cxx_yywrap.ll"
    DEPENDS win_flex "${FLEX_CASES_DIR}/cxx_yywrap.ll"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "flex(c++): generating cxx_yywrap.cc"
    VERBATIM)
add_executable(flextest_cxx_yywrap "${CMAKE_CURRENT_BINARY_DIR}/cxx_yywrap.cc")
target_include_directories(flextest_cxx_yywrap PRIVATE
    "${CMAKE_SOURCE_DIR}/common/misc"
    "${CMAKE_SOURCE_DIR}/flex/src")
if(MSVC)
    target_compile_options(flextest_cxx_yywrap PRIVATE /Uinline /Urestrict)
endif()
add_test(NAME flex.cxx_yywrap
    COMMAND ${CMAKE_COMMAND}
        -DTEST_EXE=$<TARGET_FILE:flextest_cxx_yywrap>
        "-DARG1=${FLEX_CASES_DIR}/cxx_yywrap.txt"
        "-DARG2=${FLEX_CASES_DIR}/cxx_yywrap.txt"
        "-DARG3=${FLEX_CASES_DIR}/cxx_yywrap.txt"
        -P "${FLEX_RUNNER}")
set_tests_properties(flex.cxx_yywrap PROPERTIES SKIP_RETURN_CODE 77 LABELS "flex")

# ---------------------------------------------------------------------------
# Phase 5e: bison tests. Each links a bison-generated parser, a flex-generated
# scanner, and a hand-written *_main.c. These exercise win_bison + win_flex
# together, so they only register when the win_bison target is present.
function(add_flex_bison_test name)
    set(pgen "${CMAKE_CURRENT_BINARY_DIR}/${name}_parser.c")
    set(phdr "${CMAKE_CURRENT_BINARY_DIR}/${name}_parser.h")
    add_custom_command(
        OUTPUT  "${pgen}" "${phdr}"
        COMMAND $<TARGET_FILE:win_bison> -d -p test -o "${name}_parser.c" "${FLEX_CASES_DIR}/${name}_parser.y"
        DEPENDS win_bison "${FLEX_CASES_DIR}/${name}_parser.y"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "bison: generating ${name}_parser.c/.h"
        VERBATIM)

    set(sgen "${CMAKE_CURRENT_BINARY_DIR}/${name}_scanner.c")
    set(shdr "${CMAKE_CURRENT_BINARY_DIR}/${name}_scanner.h")
    add_custom_command(
        OUTPUT  "${sgen}" "${shdr}"
        COMMAND $<TARGET_FILE:win_flex> --wincompat -o "${name}_scanner.c" "${FLEX_CASES_DIR}/${name}_scanner.l"
        DEPENDS win_flex "${FLEX_CASES_DIR}/${name}_scanner.l" "${phdr}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "flex: generating ${name}_scanner.c/.h"
        VERBATIM)

    add_executable(flextest_${name} "${pgen}" "${sgen}" "${FLEX_CASES_DIR}/${name}_main.c")
    target_link_libraries(flextest_${name} PRIVATE winflexbison_common)
    target_include_directories(flextest_${name} PRIVATE
        "${CMAKE_SOURCE_DIR}/flex/src"
        "${CMAKE_CURRENT_BINARY_DIR}")
    add_test(NAME flex.${name}
        COMMAND ${CMAKE_COMMAND}
            -DTEST_EXE=$<TARGET_FILE:flextest_${name}>
            "-DINPUT=${FLEX_CASES_DIR}/${name}.txt"
            -P "${FLEX_RUNNER}")
    set_tests_properties(flex.${name} PROPERTIES SKIP_RETURN_CODE 77 LABELS "flex;bison")
endfunction()

if(TARGET win_bison)
    add_flex_bison_test(bison_nr)
    add_flex_bison_test(bison_yylloc)
    add_flex_bison_test(bison_yylval)
endif()

# ---------------------------------------------------------------------------
# Phase 5f: tableopts — the table-compression matrix. One scanner per
# table-option (-Ca -Ce ...) x threading (nr/r) x mode (opt/ser/ver), all from
# tableopts.l4 (same main() contract as reject.l4). Upstream generates this
# list with a shell script; we enumerate it directly in CMake (no sh needed)
# and reuse add_flex_generated_test().
set(FLEX_TABLEOPTS -Ca -Ce -Cf -CF -Cm -Cem -Cae -Caef -CaeF -Cam -Caem)

foreach(opt ${FLEX_TABLEOPTS})
    # Build a case-insensitively-unique label: -Cf and -CF (also -Caef/-CaeF)
    # differ only in case and would collide as filenames on Windows. Insert '_'
    # before each capital, then lowercase: Cf -> c_f... wait -> "_cf", CF -> "_c_f".
    string(REGEX REPLACE "^-" "" optname "${opt}")
    string(REGEX REPLACE "([A-Z])" "_\\1" optname "${optname}")
    string(TOLOWER "${optname}" optname)
    string(REGEX REPLACE "^_" "" optname "${optname}")
    foreach(thread nr r)
        set(rflag "")
        set(rdef  "")
        if(thread STREQUAL "r")
            set(rflag --reentrant)
            set(rdef  TEST_IS_REENTRANT)
        endif()

        # opt: no external tables.
        add_flex_generated_test(tableopts_opt_${thread}_${optname}
            SRC tableopts.l4  INPUT tableopts.txt
            FLEXFLAGS --unsafe-no-m4-sect3-escape ${opt} ${rflag}
            DEFINES   ${rdef})

        # ser: serialized external tables (input on stdin, like upstream -r -t).
        add_flex_generated_test(tableopts_ser_${thread}_${optname}
            SRC tableopts.l4  INPUT tableopts.txt  TABLES STDIN_INPUT
            FLEXFLAGS --unsafe-no-m4-sect3-escape ${opt} ${rflag}
            DEFINES   TEST_HAS_TABLES_EXTERNAL ${rdef})

        # ver: serialized + verified external tables.
        add_flex_generated_test(tableopts_ver_${thread}_${optname}
            SRC tableopts.l4  INPUT tableopts.txt  TABLES STDIN_INPUT
            FLEXFLAGS --unsafe-no-m4-sect3-escape ${opt} ${rflag} --tables-verify
            DEFINES   TEST_HAS_TABLES_EXTERNAL ${rdef})
    endforeach()
endforeach()

# ---------------------------------------------------------------------------
# Intentionally NOT adopted on MSVC (documented in spec 03):
#   pthread.pthread — needs a POSIX pthreads library.
#   options.cn      — a shell (sh) option-conformance script, not a scanner.
