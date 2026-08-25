cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# CXX is enabled here, not in tests/CMakeLists.txt where the only C++ sources
# live, because CMAKE_CXX_FLAGS_* do not exist until the language is enabled.
# The USE_STATIC_RUNTIME block below rewrites /MD -> /MT textually; with CXX
# enabled later, that rewrite silently skipped the C++ flags, so C++ targets got
# MSBuild's default dynamic CRT while -D_DEBUG pulled in the static debug C++
# library -- unresolved _malloc_dbg/_free_dbg/_calloc_dbg at link, Debug only.
# No C++ source enters the product; this only makes the flags exist in time.
project(winflexbison VERSION 2.5.25 LANGUAGES C CXX)

# cmake 3.31+ - enable `install` destination paths normalization.
if(POLICY CMP0177)
  cmake_policy(SET CMP0177 OLD)
endif()

if(NOT MSVC)
   message(WARNING "Only Visual Studio Build is officially supported right now")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-D_DEBUG)
endif()

# next line needed for compile in C (nor CPP) mode (ucrt headers bug)
add_definitions(-Dinline=__inline)
# next line needed for VS2017 only
add_definitions(-Drestrict=__restrict)

# No /MD here: CMAKE_CXX_FLAGS_DEBUG already carries CMake's /MDd, and
# USE_STATIC_RUNTIME rewrites /MD -> /MT textually. Appending /MD produced
# "/MTd ... /MT" in one flag string, so MSVC took the last and compiled C++
# against the RELEASE static CRT while -D_DEBUG still pulled in the debug C++
# library (libcpmtd) -- unresolved _malloc_dbg/_free_dbg/_calloc_dbg at link.
# C targets were unaffected because this line only touches the CXX flags.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /Od /Zi /EHsc")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /GL /Od /Oi /Gy /Zi /EHsc")

# Define Release by default
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")

# Only apply to MSVC frontend (not clang frontends)
if (MSVC AND NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
    # Make __extension__ expand to nothing on MSVC (so GCC/Clang keep the keyword)
    add_compile_definitions("__extension__=")
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Output Variables
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_LIST_DIR}/bin/Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_LIST_DIR}/bin/Release")

    #------------------------------------------------------------------------
    # Static Windows Runtime
    #   Option to statically link to the Windows runtime. Maybe only
    #   applies to WIN32/MSVC.
    #------------------------------------------------------------------------
    if (MSVC)
        # /utf-8 = /source-charset:utf-8 + /execution-charset:utf-8. The
        # execution charset matters: bison embeds UTF-8 glyph literals (the item
        # dot, arrows in graphs/counterexamples); with only /source-charset the
        # execution charset defaults to the system codepage (e.g. CP1252) and
        # MSVC transcodes them (• -> 0x95), diverging from upstream's UTF-8.
        add_compile_options("/utf-8")
        option( USE_STATIC_RUNTIME "Set ON to change /MD(DLL) to /MT(static)" OFF )
        if (USE_STATIC_RUNTIME)
            set(CompilerFlags
                CMAKE_CXX_FLAGS
                CMAKE_CXX_FLAGS_DEBUG
                CMAKE_CXX_FLAGS_RELEASE
                CMAKE_C_FLAGS
                CMAKE_C_FLAGS_DEBUG
                CMAKE_C_FLAGS_RELEASE
                )
            foreach(CompilerFlag ${CompilerFlags})
                string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            endforeach()
            message(STATUS "Using /MT STATIC runtime")
        endif ()
    endif ()
endif ()



add_subdirectory(common)
add_subdirectory(flex)
add_subdirectory(bison)

#------------------------------------------------------------------------
# Warnings in vendored upstream code
#
# common/ (gnulib, GNU m4) and bison/src/ are upstream sources written for
# GCC, where none of the codes below fire at -Wall -- they are MSVC's
# equivalents of -Wconversion and -Wsign-compare. Upstream has never seen
# them. Silencing them at the source would mean editing ~120 lines across 60
# vendored files, and every one of those lines becomes a merge conflict when
# the next flex/bison/m4 release is adopted, so they are turned off per
# target instead:
#
#   C4018  signed/unsigned mismatch
#   C4116  unnamed type definition in parentheses (gnulib obstack idiom)
#   C4146  unary minus applied to unsigned (gnulib overflow idiom)
#   C4244  narrowing conversion
#   C4267  size_t -> smaller type (x64 only)
#   C4308  negative constant converted to unsigned
#
# Deliberately NOT listed, so they keep catching things: C4090 (const
# mismatches -- every one is in port-added code), C4307, C4477.
#
# win_flex, fl and y are deliberately left out: they carry no vendored
# warnings today, so they stay canaries for a future upstream upgrade.
# Neither does tests/winflexbison/ -- that code is ours.
#
# Configure with -DWFB_VENDOR_WARNINGS=ON to get the full log back for a
# periodic audit.
#------------------------------------------------------------------------
option(WFB_VENDOR_WARNINGS "Show MSVC warnings in vendored upstream code" OFF)
if(MSVC AND NOT WFB_VENDOR_WARNINGS)
    set(WFB_VENDORED_WARNINGS /wd4018 /wd4116 /wd4146 /wd4244 /wd4267 /wd4308)
    foreach(_wfb_target winflexbison_common win_bison)
        target_compile_options(${_wfb_target} PRIVATE ${WFB_VENDORED_WARNINGS})
    endforeach()
endif()

# Test suite (opt-in; only for a top-level build). Run with `ctest`.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    enable_testing()
    add_subdirectory(tests)
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # CPACK
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      install(DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/" DESTINATION "./")
    else()
      install(DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/" DESTINATION "./")
    endif()

    install(DIRECTORY "custom_build_rules/" DESTINATION "./custom_build_rules/")
    install(DIRECTORY "bison/data/" DESTINATION "./data/")
    install(FILES "bison/src/COPYING" DESTINATION "./" RENAME "COPYING.bison")
    install(FILES "flex/src/FlexLexer.h" DESTINATION "./")
    install(FILES "flex/src/COPYING" DESTINATION "./" RENAME "COPYING.flex")
    install(FILES "COPYING" DESTINATION "./")
    install(FILES "COPYING.DOC" DESTINATION "./")
    install(FILES "changelog.md" DESTINATION "./")
    install(FILES "README.md" DESTINATION "./")

    set(PACKAGE_GENERATORS_DEFAULT ZIP)

    set(CPACK_GENERATOR ${PACKAGE_GENERATORS_DEFAULT} CACHE STRING "List of CPack Generators which will be created by the 'PACKAGE' target. Default: ZIP")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Winflexbison - Flex and Bison for Windows")

    set(ENV_VERSION "$ENV{WINFLEXBISON_VERSION}")
    if(NOT ENV_VERSION)
      set(CPACK_PACKAGE_VERSION "dev-${PROJECT_VERSION}" CACHE STRING "Complete version string of the created package.")
    else()
      set(CPACK_PACKAGE_VERSION "${ENV_VERSION}" CACHE STRING "Complete version string of the created package.")
    endif()

    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)
    set(CPACK_MONOLITHIC_INSTALL ON)

    set(CPACK_PACKAGE_FILE_NAME "win_flex_bison-${CPACK_PACKAGE_VERSION}")

    include(CPack)
endif()
