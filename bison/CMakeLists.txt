cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

set(PROJECT_NAME win_bison)

project(${PROJECT_NAME} C)

# Definition of Macros
add_definitions(-D_CONSOLE)

file(GLOB SOURCE_FILES
    "src/*.c"
    "src/*.h"
)

# Add executable to build.
add_executable(${PROJECT_NAME} 
   ${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE "src")

target_link_libraries(${PROJECT_NAME} common_lib kernel32.lib user32.lib)
