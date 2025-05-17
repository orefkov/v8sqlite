# CMakeList.txt : CMake project for core_as, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.15)
add_executable(testv8sqlite test_addin_base.cpp ../addin_base.cpp test_sqlite.cpp ../sqlite.cpp ../sqlite3_unicode.c)
target_include_directories(testv8sqlite PRIVATE "..")
add_compile_definitions(BUILD_TESTS SQLITE_CORE SQLITE_ENABLE_UNICODE)
add_dependencies(testv8sqlite simstr)
target_link_libraries(testv8sqlite sqlite3 simstr gtest_main)
add_test(NAME testv8sqlite COMMAND testv8sqlite)
