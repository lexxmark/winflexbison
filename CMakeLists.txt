cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project (winflexbison)

if(NOT MSVC)
   message( FATAL_ERROR "Visual Studio Build supported only" )
endif()

# Output Variables
set(OUTPUT_DEBUG "${CMAKE_CURRENT_LIST_DIR}/bin/Debug")
set(OUTPUT_RELEASE "${CMAKE_CURRENT_LIST_DIR}/bin/Release")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DEBUG}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_RELEASE}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-D_DEBUG)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /MD /Od /Zi /EHsc")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /GL /Od /Oi /Gy /Zi /EHsc")

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif()

add_subdirectory(common)
add_subdirectory(flex)
add_subdirectory(bison)


# CPACK
install(DIRECTORY "bin/Release/" DESTINATION "./")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  install(DIRECTORY "bin/Debug/" DESTINATION "./")
endif()

install(DIRECTORY "custom_build_rules/" DESTINATION "./custom_build_rules/")
install(DIRECTORY "bison/data/" DESTINATION "./data/")
install(FILES "flex/src/FlexLexer.h" DESTINATION "./")

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
