# CMakeList.txt : CMake project for core_as, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

# Add source to this project's executable.
add_library (core_as_str
    sstring.cpp
    simple_unicode.cpp
)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()

if (BUILD_BENCHMARKS)
    add_subdirectory(bench)
endif ()
