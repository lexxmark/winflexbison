cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

set(PROJECT_NAME common_lib)

project(${PROJECT_NAME} C)

# Definition of Macros
add_definitions(-D_LIB)

file(GLOB SOURCE_FILES
    "misc/*.c"
	"misc/*.h"
    "misc/bitset/*.c"
	"misc/bitset/*.h"
    "misc/glthread/*.c"
	"misc/glthread/*.h"
	"m4/*.c"
	"m4/*.h"
	"m4/lib/*.c"
	"m4/lib/*.h"
)
# exclude some source files
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/m4/lib/regexec.c")
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/m4/lib/regcomp.c")

# Add library to build.
add_library(${PROJECT_NAME} STATIC
   ${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC "misc")
target_include_directories(${PROJECT_NAME} PUBLIC "m4")
target_include_directories(${PROJECT_NAME} PUBLIC "m4/lib")

