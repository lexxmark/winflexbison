# run_content_check.cmake — generic content assertion over a generated file.
#
# Invoked as:
#   cmake -DFILE=<path> -DPATTERN=<regex> [-DEXPECT_MATCH=FALSE] -P run_content_check.cmake
#
# FILE         : the file to inspect (e.g. a flex/bison-generated source).
# PATTERN      : a CMake-dialect regex (see `string(REGEX MATCH ...)`).
# EXPECT_MATCH : TRUE (default) to require the pattern be found; FALSE to
#                require it be absent (used for "must not regress" guards).

foreach(v FILE PATTERN)
    if(NOT DEFINED ${v})
        message(FATAL_ERROR "run_content_check.cmake: ${v} not set")
    endif()
endforeach()

if(NOT DEFINED EXPECT_MATCH)
    set(EXPECT_MATCH TRUE)
endif()

if(NOT EXISTS "${FILE}")
    message(FATAL_ERROR "run_content_check.cmake: FILE does not exist: ${FILE}")
endif()

file(READ "${FILE}" contents)
# Pad with a non-identifier boundary character on each side so patterns
# relying on a leading/trailing non-word-char boundary still match a hit at
# the very start/end of the file.
set(contents " ${contents} ")

string(REGEX MATCH "${PATTERN}" m "${contents}")

if(EXPECT_MATCH AND "${m}" STREQUAL "")
    message(FATAL_ERROR "expected pattern not found in ${FILE}: '${PATTERN}'")
elseif(NOT EXPECT_MATCH AND NOT "${m}" STREQUAL "")
    message(FATAL_ERROR "forbidden pattern found in ${FILE}: '${PATTERN}' matched '${m}'")
endif()
