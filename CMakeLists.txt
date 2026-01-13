# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.15)
project ("v8sqlite")

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set (MSVC_COMPILER ON)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    set (CLANG_COMPILER ON)
endif ()

option(V8SQLITE_STATIC_RUNTIME "Подключать рантайм-библиотеки статически" ON)
option(V8SQLITE_BUILD_TESTS "Построить тесты" ON)
option(V8SQLITE_BENCHMARKS "Построить замеры производительности" OFF)

# Установим дефолтные значения
if (WIN32)

    # Одинаковые флаги для компиляторов MSVC и clang-cl во всех конфигурациях
    set (V8SQLITE_CXX_FLAGS "/std:c++20 /EHs /utf-8 /Zc:strictStrings /W3" CACHE STRING "")
    # Одинаковые флаги для компиляторов MSVC и clang-cl в релизной конфигурации
    set (V8SQLITE_CXX_FLAGS_RELEASE "/O3 /GS- /DNDEBUG" CACHE STRING "")
    # Одинаковые флаги для компиляторов MSVC и clang-cl в отладочной конфигурации
    set (V8SQLITE_CXX_FLAGS_DEBUG "/Zi /Ob0 /Od /D_DEBUG /RTC1" CACHE STRING "")
    # Флаги только для компилятора MSVC в релизной конфигурации
    set (V8SQLITE_CXX_MSVC_RELEASE "/GL" CACHE STRING "")
    # Флаги только для компилятора MSVC в отладочной конфигурации
    set (V8SQLITE_CXX_MSVC_DEBUG "" CACHE STRING "")
    # Флаги только для компилятора clang-cl в релизной конфигурации
    set (V8SQLITE_CXX_CLANG_RELEASE "-Wno-error=unused-command-line-argument -flto" CACHE STRING "")
    # Флаги только для компилятора clang-cl в отладочной конфигурации
    set (V8SQLITE_CXX_CLANG_DEBUG "-Wno-error=unused-command-line-argument" CACHE STRING "")

    # Флаги для MSVC линкера в релизе
    set (V8SQLITE_LINK_MSVC_RELEASE "/DEBUG:NONE /LTGC" CACHE STRING "")
    # Флаги для MSVC линкера в отладке
    set (V8SQLITE_LINK_MSVC_DEBUG "" CACHE STRING "")
    # Флаги для clang-cl линкера в релизе
    set (V8SQLITE_LINK_CLANG_RELEASE "LINKER:-flto" CACHE STRING "")
    # Флаги для clang-cl линкера в отладке
    set (V8SQLITE_LINK_CLANG_DEBUG "" CACHE STRING "")

elseif (UNIX)
    # Флаги для компилятора clang++ во всех конфигурациях
    set (V8SQLITE_CXX_CLANG_FLAGS "-fPIC -std=c++20 -fexperimental-library -DLIBCXX_ENABLE_INCOMPLETE_FEATURES=ON -stdlib=libstdc++" CACHE STRING "")
    # Флаги для компилятора clang во всех конфигурациях
    set (V8SQLITE_C_CLANG_FLAGS "-fPIC" CACHE STRING "")
    # Флаги для компилятора clang в релизной конфигурации
    set (V8SQLITE_CXX_CLANG_RELEASE "-O2 -DNDEBUG -flto" CACHE STRING "")
    # Флаги для компилятора clang в отладочной конфигурации
    set (V8SQLITE_CXX_CLANG_DEBUG "-O0 -D_DEBUG" CACHE STRING "")

elseif()

    message(FATAL_ERROR "Сборка поддерживается только в Windows и Linux")

endif()

# Установим основные флаги компилятора
if (WIN32)
    set (CMAKE_CXX_FLAGS                "${V8SQLITE_CXX_FLAGS}")
    set (CMAKE_C_FLAGS                  "${V8SQLITE_CXX_FLAGS}")
    set (CMAKE_CXX_FLAGS_RELEASE        "${V8SQLITE_CXX_FLAGS_RELEASE}")
    set (CMAKE_CXX_FLAGS_DEBUG          "${V8SQLITE_CXX_FLAGS_DEBUG}")
    set (CMAKE_C_FLAGS_RELEASE          "${V8SQLITE_CXX_FLAGS_RELEASE}")
    set (CMAKE_C_FLAGS_DEBUG            "${V8SQLITE_CXX_FLAGS_DEBUG}")

    if (MSVC_COMPILER)
        set (CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS_RELEASE} ${V8SQLITE_CXX_MSVC_RELEASE}")
        set (CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_CXX_FLAGS_DEBUG}   ${V8SQLITE_CXX_MSVC_DEBUG}")
        set (CMAKE_C_FLAGS_RELEASE      "${CMAKE_C_FLAGS_RELEASE}   ${V8SQLITE_CXX_MSVC_RELEASE}")
        set (CMAKE_C_FLAGS_DEBUG        "${CMAKE_C_FLAGS_DEBUG}     ${V8SQLITE_CXX_MSVC_DEBUG}")
    elseif (CLANG_COMPILER)
        set (CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS_RELEASE} ${V8SQLITE_CXX_CLANG_RELEASE}")
        set (CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_CXX_FLAGS_DEBUG}   ${V8SQLITE_CXX_CLANG_DEBUG}")
        set (CMAKE_C_FLAGS_RELEASE      "${CMAKE_C_FLAGS_RELEASE}   ${V8SQLITE_CXX_CLANG_RELEASE}")
        set (CMAKE_C_FLAGS_DEBUG        "${CMAKE_C_FLAGS_DEBUG}     ${V8SQLITE_CXX_CLANG_DEBUG}")
    else ()
        message(FATAL_ERROR "Неизвестный компилятор")
    endif()

elseif(UNIX)

    set (CMAKE_CXX_FLAGS                "${V8SQLITE_CXX_CLANG_FLAGS}")
    set (CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS} ${V8SQLITE_C_CLANG_FLAGS}")
    set (CMAKE_CXX_FLAGS_RELEASE        "${V8SQLITE_CXX_CLANG_RELEASE}")
    set (CMAKE_CXX_FLAGS_DEBUG          "${V8SQLITE_CXX_CLANG_DEBUG} -g")
    set (CMAKE_C_FLAGS_RELEASE          "${V8SQLITE_CXX_CLANG_RELEASE}")
    set (CMAKE_C_FLAGS_DEBUG            "${V8SQLITE_CXX_FLAGS_DEBUG}")

    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        set (CMAKE_C_FLAGS              "${CMAKE_C_FLAGS} ${V8SQLITE_CXX_CLANG_DEBUG}")
    else ()
        set (CMAKE_C_FLAGS              "${CMAKE_C_FLAGS} ${V8SQLITE_CXX_CLANG_RELEASE}")
    endif ()

endif()

# Set static runtime library
if (WIN32)
    # Allow MSVC_RUNTIME_LIBRARY property
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
    if (V8SQLITE_STATIC_RUNTIME)
        set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else ()
        set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDll$<$<CONFIG:Debug>:Debug>")
    endif ()

else()

    if (V8SQLITE_STATIC_RUNTIME)
        set (LINK_OPTIONS "${LINK_OPTIONS} -static-libgcc -static-libstdc++")
    endif ()

endif(WIN32)

if (V8SQLITE_BUILD_TESTS)
    enable_testing()
    set (BUILD_TESTS ON)
    # Load and build GTest
    include(FetchContent)
    FetchContent_Declare(
      googletest
      # Specify the commit you depend on and update it regularly.
      URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt FALSE CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

function (GBencmark)
    # Load and build Google benchmarks
    include(FetchContent)
    FetchContent_Declare(
      googlebench
      # Specify the commit you depend on and update it regularly.
      URL https://github.com/google/benchmark/archive/refs/tags/v1.7.0.zip
    )

    set (CMAKE_CXX_STANDARD 20)
    set (BENCHMARK_ENABLE_TESTING OFF)
    set (BENCHMARK_ENABLE_LTO OFF)
    set (BENCHMARK_ENABLE_INSTALL OFF)
    set (BENCHMARK_INSTALL_DOCS OFF)
    set (BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
    set (BENCHMARK_ENABLE_GTEST_TESTS OFF)
    add_compile_definitions(BENCHMARK_STATIC_DEFINE)
    if (UNIX OR CLANG_COMPILER)
        set (BENCHMARK_USE_LIBCXX ON)
    endif ()

    FetchContent_MakeAvailable(googlebench)
endfunction()

if (V8SQLITE_BENCHMARKS)# AND CMAKE_BUILD_TYPE STREQUAL Release)
    set (BUILD_BENCHMARKS ON)
    GBencmark()
endif ()

include_directories("${CMAKE_SOURCE_DIR}/1C_include")
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/sqlite3")

# Include sub-projects.
add_subdirectory (sqlite3)
add_subdirectory (src)
