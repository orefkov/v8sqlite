/*
* (c) Проект "Core.As", Александр Орефков orefkov@gmail.com
* Описание базового экспортируемого из core_as.dll API
*/
#pragma once
#include <cstdlib>

#ifndef __has_declspec_attribute
#define __has_declspec_attribute(x) 0
#endif

#ifdef CORE_AS_SHARED
#if defined(_MSC_VER) || (defined(__clang__) && __has_declspec_attribute(dllexport))
    #ifdef COREAS_EXPORTS
        #define COREAS_API __declspec(dllexport)
    #else
        #define COREAS_API __declspec(dllimport)
    #endif
#elif (defined(__GNUC__) || defined(__GNUG__)) && defined(COREAS_EXPORTS)
    #define COREAS_API __attribute__((visibility("default")))
#else
    #define COREAS_API
#endif
#else
#define COREAS_API
#endif


enum class LogLevel {
    Critical,
    Warning,
    Normal,
    Info,
    Diagnostic
};

COREAS_API void* core_as_malloc(size_t count);
COREAS_API void* core_as_realloc(void* ptr, size_t count);
COREAS_API void core_as_free(void* ptr);
COREAS_API void core_as_print(const wchar_t* text);
