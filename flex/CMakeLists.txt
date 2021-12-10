cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

set(PROJECT_NAME win_flex)

project(${PROJECT_NAME} C)

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

target_compile_definitions(${PROJECT_NAME}
    PRIVATE "_CONSOLE")

target_include_directories(${PROJECT_NAME} PRIVATE "src")

target_link_libraries(${PROJECT_NAME} winflexbison_common kernel32.lib user32.lib)

# libfl
add_library(fl STATIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/libmain.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/libyywrap.c")

target_include_directories(fl
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")