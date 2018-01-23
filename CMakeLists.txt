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
