cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

set(PROJECT_NAME win_flex)

project(${PROJECT_NAME} C)

# Definition of Macros
add_definitions(-D_CONSOLE)

file(GLOB SOURCE_FILES
    "src/*.c"
    "src/*.h"
)
# exclude some source files
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/libmain.c")
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/libyywrap.c")

# Add executable to build.
add_executable(${PROJECT_NAME} 
   ${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE "src")

target_link_libraries(${PROJECT_NAME} common_lib kernel32.lib user32.lib Ws2_32.lib)
