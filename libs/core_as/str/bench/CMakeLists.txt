# CMakeList.txt : CMake project for core_as, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

add_executable(benchStr bench_str.cpp "bench.h")
add_dependencies(benchStr core_as_str)
target_link_libraries(benchStr core_as_str benchmark::benchmark benchmark::benchmark_main)

if (UNIX)
    target_link_options(benchStr PRIVATE -Xlinker --version-script -Xlinker ${CMAKE_CURRENT_SOURCE_DIR}/version.script)
else ()
    target_link_options(benchStr PRIVATE /DEBUG:FULL)
    target_compile_options(benchStr PRIVATE /Z7)
endif ()
