# winflexbison test suites. Wired in from the root CMakeLists via
# enable_testing() + add_subdirectory(tests). Run with `ctest` from the build
# tree. See docs/specs/03-test-adoption/spec.md.

# C++ is needed for the flex C++ scanner tests (.ll / .lll). The main product
# builds as C only; enable CXX here so it is scoped to the test tree.
enable_language(CXX)

add_subdirectory(flex)
