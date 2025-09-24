cmake_minimum_required(VERSION 3.21.0)
if(POLICY CMP0177)
  cmake_policy(SET CMP0177 NEW)
endif()

project (winflexbison C)

if(NOT MSVC)
   message(FATAL_ERROR "Visual Studio Build supported only" )
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
# next line needed for compile in C (nor CPP) mode (ucrt headers bug)
add_definitions(-Dinline=__inline)
# next line needed for VS2017 only
add_definitions(-Drestrict=__restrict)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /EHsc")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /GL /Oi /Gy")

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif()

if(PROJECT_IS_TOP_LEVEL)
    # Output Variables
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/bin/Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/bin/Release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}/bin/RelWithDebInfo")
endif()
    # Only apply to MSVC frontend (not clang frontends)
if (MSVC AND NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
  # Make __extension__ expand to nothing on MSVC (so GCC/Clang keep the keyword)
  add_compile_definitions("__extension__=")
endif()

#if (MSVC)
add_compile_options("/source-charset:utf-8")
option(USE_STATIC_RUNTIME "Set ON to change /MD(DLL) to /MT(static)" OFF )
if (USE_STATIC_RUNTIME)
  # /MT or /MTd depending on config
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
  # /MD or /MDd depending on config
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
#endif ()

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
    install(FILES "flex/src/FlexLexer.h" DESTINATION "./")
    install(FILES "changelog.md" DESTINATION "./")
    install(FILES "COPYING" DESTINATION "./")
    install(FILES "COPYING.DOC" DESTINATION "./")
    install(FILES "README.md" DESTINATION "./")

    set(PACKAGE_GENERATORS_DEFAULT ZIP)

    set(CPACK_GENERATOR ${PACKAGE_GENERATORS_DEFAULT} CACHE STRING "List of CPack Generators which will be created by the 'PACKAGE' target. Default: ZIP")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Winflexbison - Flex and Bison for Windows")

    set(ENV_VERSION "$ENV{WINFLEXBISON_VERSION}")
    if(NOT ENV_VERSION)
      set(CPACK_PACKAGE_VERSION "master" CACHE STRING "Complete version of the created package.")
    else()
      set(CPACK_PACKAGE_VERSION "${ENV_VERSION}" CACHE STRING "Complete version of the created package.")
    endif()

    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)
    set(CPACK_MONOLITHIC_INSTALL ON)

    set(CPACK_PACKAGE_FILE_NAME "win_flex_bison-${CPACK_PACKAGE_VERSION}")

    include(CPack)
endif()
