cmake_minimum_required(VERSION 3.21.0)

# cmake 3.31+ - enable `install` destination paths normalization.
if(POLICY CMP0177)
  cmake_policy(SET CMP0177 OLD)
endif()

project(winflexbison VERSION 2.5.25 LANGUAGES C)

if(NOT MSVC)
   message(WARNING "Only Visual Studio Build is officially supported right now")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
# next line needed for compile in C (nor CPP) mode (ucrt headers bug)
add_definitions(-Dinline=__inline)
# next line needed for VS2017 only
add_definitions(-Drestrict=__restrict)

# Define Release by default
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")

# Only apply to MSVC frontend (not clang frontends)
# Make __extension__ expand to nothing on MSVC (so GCC/Clang keeps the keyword)
if (MSVC AND NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
  add_compile_definitions("__extension__=")
endif()

# Only change those variables for top-level project else use parent values
if(PROJECT_IS_TOP_LEVEL) 
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/bin")
    if(MSVC AND DEFINED USE_STATIC_RUNTIME)
        # Legacy compatibility for top-level builds
        option(WINFLEXBISON_USE_STATIC_RUNTIME "Set ON to change /MD(DLL) to /MT(static)" USE_STATIC_RUNTIME)
    endif()
endif()

if (MSVC)
    add_compile_options("/source-charset:utf-8")
    option(WINFLEXBISON_USE_STATIC_RUNTIME "Set ON to change /MD(DLL) to /MT(static)" Off)
    if (WINFLEXBISON_USE_STATIC_RUNTIME)
        # /MT or /MTd depending on config
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else ()
        # /MD or /MDd depending on config
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif ()
endif ()

add_subdirectory(common)
add_subdirectory(flex)
add_subdirectory(bison)

if(PROJECT_IS_TOP_LEVEL)
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
