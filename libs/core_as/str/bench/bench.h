#pragma once

#include <core_as/str/sstring.h>
#include <benchmark/benchmark.h>

#if defined(_MSC_VER) || (defined(__clang__) && __has_declspec_attribute(dllexport))
#define BENCH_EXPORT __declspec(dllexport)
#elif (defined(__GNUC__) || defined(__GNUG__)) && defined(COREAS_EXPORTS)
#define BENCH_EXPORT __attribute__((visibility("default")))
#else
#define BENCH_EXPORT
#endif
