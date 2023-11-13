CMAKE_MINIMUM_REQUIRED(VERSION 3.6 FATAL_ERROR)

if (WIN32)
    ENABLE_LANGUAGE(RC)
endif ()

set (V8SQLITE_SRC
    v8sqlite_addin.cpp
    v8sqlite_addin.h
    stdafx.cpp
    stdafx.h
    addin_base.cpp
    sqlite.cpp
    sqlite3_unicode.c
)

if (WIN32)
    LIST(APPEND V8SQLITE_SRC dllmain.cpp v8sqlite.rc v8sqlite.def)
endif()

add_library(${PROJECT_NAME} SHARED ${V8SQLITE_SRC})
target_link_libraries(${PROJECT_NAME} sqlite3 core_as_str)

add_compile_definitions(
    SQLITE_CORE
    SQLITE_ENABLE_UNICODE
)

if (WIN32)
    add_compile_definitions(
        UNICODE
        _WINDOWS
        _USRDLL
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        ADDINCPP_EXPORTS
    )
else ()
    if (V8SQLITE_STATIC_RUNTIME)
        target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
    endif ()
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--no-undefined -Xlinker --version-script -Xlinker ${CMAKE_CURRENT_SOURCE_DIR}/version.script)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

set_target_properties( ${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON 
)

if (NOT WIN32)
    add_custom_command(
      TARGET ${PROJECT_NAME} POST_BUILD
      DEPENDS ${PROJECT_NAME}
      COMMAND $<$<CONFIG:release>:strip>
      ARGS --strip-all $<TARGET_FILE:${PROJECT_NAME}>
    )
endif()

if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()
