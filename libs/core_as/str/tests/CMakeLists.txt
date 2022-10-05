# CMakeList.txt : CMake project for core_as, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)

add_executable(testStr test_str.cpp)
add_dependencies(testStr core_as_str)
target_link_libraries(testStr core_as_str gtest_main)
add_test(NAME testStr COMMAND testStr)
