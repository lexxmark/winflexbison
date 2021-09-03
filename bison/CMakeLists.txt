cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

set(PROJECT_NAME win_bison)

project(${PROJECT_NAME} C)

file(GLOB SOURCE_FILES
    "src/*.c"
    "src/*.h"
)
# exclude some source files
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/scan-code.c")
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/scan-gram.c")
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/scan-skel.c")


# Add executable to build.
add_executable(${PROJECT_NAME} 
   ${SOURCE_FILES}
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE "_CONSOLE")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data"
)

target_include_directories(${PROJECT_NAME} PRIVATE "src")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(${PROJECT_NAME} winflexbison_common kernel32.lib user32.lib)

# liby
file(GLOB liby_src
    "lib/*.c"
    "lib/*.h"
)

add_library(y STATIC ${liby_src})

target_include_directories(y
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")