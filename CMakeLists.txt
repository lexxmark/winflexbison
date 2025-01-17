cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(winflexbison VERSION 2.5.25 LANGUAGES C)

# cmake 3.31+ - enable `install` destination paths normalization.
if(POLICY CMP0177)
  cmake_policy(SET CMP0177 OLD)
endif()

if(NOT MSVC)
   message( WARNING "Visual Studio Build supported only" )
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-D_DEBUG)
endif()

# next line needed for compile in C (nor CPP) mode (ucrt headers bug)
add_definitions(-Dinline=__inline)
# next line needed for VS2017 only
add_definitions(-Drestrict=__restrict)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /MD /Od /Zi /EHsc")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /GL /Od /Oi /Gy /Zi /EHsc")

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Output Variables
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_LIST_DIR}/bin/Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_LIST_DIR}/bin/Release")

    #------------------------------------------------------------------------
    # Static Windows Runtime
    #   Option to statically link to the Windows runtime. Maybe only
    #   applies to WIN32/MSVC.
    #------------------------------------------------------------------------
    if (MSVC)
        add_compile_definitions("__extension__")
        add_compile_options("/source-charset:utf-8")
        option( USE_STATIC_RUNTIME "Set ON to change /MD(DLL) to /MT(static)" OFF )
        if (USE_STATIC_RUNTIME)
            set(CompilerFlags
                CMAKE_CXX_FLAGS
                CMAKE_CXX_FLAGS_DEBUG
                CMAKE_CXX_FLAGS_RELEASE
                CMAKE_C_FLAGS
                CMAKE_C_FLAGS_DEBUG
                CMAKE_C_FLAGS_RELEASE
                )
            foreach(CompilerFlag ${CompilerFlags})
                string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            endforeach()
            message(STATUS "Using /MT STATIC runtime")
        endif ()
    endif ()
endif ()



add_subdirectory(common)
add_subdirectory(flex)
add_subdirectory(bison)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
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
