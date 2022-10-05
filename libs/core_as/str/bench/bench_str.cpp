#include "bench.h"

COREAS_API void* core_as_malloc(size_t count) {
    return malloc(count);
}

COREAS_API void* core_as_realloc(void* ptr, size_t count) {
    return realloc(ptr, count);
}

COREAS_API void core_as_free(void* ptr) {
    free(ptr);
}

using namespace core_as::str;

#define TEST_TEXT "Test text"
#define LONG_TEXT "123456789012345678901234567890"


void __(benchmark::State& state) {
    for (auto _: state) {
    }
}

#if 0
template<typename T>
BENCH_EXPORT void BM_CreateEmpty(benchmark::State& state) {
    for (auto _: state) {
        T empty_string;
        benchmark::DoNotOptimize(empty_string);
    }
}
BENCHMARK(__)->Name("-----  Create Empty Str ---------");
BENCHMARK(BM_CreateEmpty<std::string>);
BENCHMARK(BM_CreateEmpty<std::string_view>);
BENCHMARK(BM_CreateEmpty<ssa>);
BENCHMARK(BM_CreateEmpty<stringa>);
BENCHMARK(BM_CreateEmpty<lstringa<40>>);
BENCHMARK(BM_CreateEmpty<lstringa<20>>);

template<typename T>
BENCH_EXPORT void BM_CreateShortLiteral(benchmark::State& state) {
    for (auto _: state) {
        T empty_string{TEST_TEXT};
        benchmark::DoNotOptimize(empty_string);
    }
}
BENCHMARK(__)->Name("-----  Create Str from short literal (9 symbols) --------");
BENCHMARK(BM_CreateShortLiteral<std::string>);
BENCHMARK(BM_CreateShortLiteral<std::string_view>);
BENCHMARK(BM_CreateShortLiteral<ssa>);
BENCHMARK(BM_CreateShortLiteral<stringa>);
BENCHMARK(BM_CreateShortLiteral<lstringa<40>>);
BENCHMARK(BM_CreateShortLiteral<lstringa<20>>);

template<typename T>
BENCH_EXPORT void BM_CreateLongLiteral(benchmark::State& state) {
    for (auto _: state) {
        T empty_string{LONG_TEXT};
        benchmark::DoNotOptimize(empty_string);
    }
}
BENCHMARK(__)->Name("-----  Create Str from long literal (30 symbols) ---------");
BENCHMARK(BM_CreateLongLiteral<std::string>);
BENCHMARK(BM_CreateLongLiteral<std::string_view>);
BENCHMARK(BM_CreateLongLiteral<ssa>);
BENCHMARK(BM_CreateLongLiteral<stringa>);
BENCHMARK(BM_CreateLongLiteral<lstringa<40>>);
BENCHMARK(BM_CreateLongLiteral<lstringa<20>>);

template<typename T>
BENCH_EXPORT void BM_CopyShortString(benchmark::State& state) {
    T x{TEST_TEXT};
    for (auto _: state) {
        T copy{x};
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(__)->Name("-----  Create copy of Str with 9 symbols ---------");
BENCHMARK(BM_CopyShortString<std::string>);
BENCHMARK(BM_CopyShortString<std::string_view>);
BENCHMARK(BM_CopyShortString<ssa>);
BENCHMARK(BM_CopyShortString<stringa>);
BENCHMARK(BM_CopyShortString<lstringa<40>>);
BENCHMARK(BM_CopyShortString<lstringa<20>>);

template<typename T>
BENCH_EXPORT void BM_CopyLongString(benchmark::State& state) {
    T x = LONG_TEXT;
    for (auto _: state) {
        T copy{x};
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(__)->Name("-----  Create copy of Str with 30 symbols ---------");
BENCHMARK(BM_CopyLongString<std::string>);
BENCHMARK(BM_CopyLongString<std::string_view>);
BENCHMARK(BM_CopyLongString<ssa>);
BENCHMARK(BM_CopyLongString<stringa>);
BENCHMARK(BM_CopyLongString<lstringa<40>>);
BENCHMARK(BM_CopyLongString<lstringa<20>>);

template<typename T>
BENCH_EXPORT void BM_Find(benchmark::State& state) {
    T x{LONG_TEXT LONG_TEXT LONG_TEXT TEST_TEXT};
    for (auto _: state) {
        int i = (int)x.find(TEST_TEXT);
        benchmark::DoNotOptimize(i);
        benchmark::DoNotOptimize(x);
    }
}
BENCHMARK(__)->Name("-----  Find 9 symbols text in end of 99 symbols text ---------");
BENCHMARK(BM_Find<std::string>);
BENCHMARK(BM_Find<std::string_view>);
BENCHMARK(BM_Find<ssa>);
BENCHMARK(BM_Find<stringa>);
BENCHMARK(BM_Find<lstringa<40>>);
BENCHMARK(BM_Find<lstringa<20>>);

template<typename T>
BENCH_EXPORT void BM_CopyLongDyn(benchmark::State& state) {
    T x(state.range(0), 'a');
    for (auto _: state) {
        T copy{x};
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(__)->Name("-----  Copy long not literal Str (100 symbol)---------");
BENCHMARK(BM_CopyLongDyn<std::string>)
    ->Arg(15)
    ->Arg(16)
    ->Arg(17)
    ->Arg(18)
    ->Arg(19)
    ->Arg(20)
    ->Arg(21)
    ->Arg(22)
    ->Arg(23)
    ->Arg(24)
    ->RangeMultiplier(2)
    ->Range(32, 4096);
BENCHMARK(BM_CopyLongDyn<stringa>)->Arg(15)->Arg(16)->Arg(23)->Arg(24)->RangeMultiplier(2)->Range(32, 4096);
BENCHMARK(BM_CopyLongDyn<lstringa<16>>)->Arg(15)->Arg(16)->Arg(23)->Arg(24)->RangeMultiplier(2)->Range(32, 4096);
BENCHMARK(BM_CopyLongDyn<lstringa<512>>)->RangeMultiplier(2)->Range(8, 4096);
#endif


int to_int(const std::string& s) {
    //return std::stoi(s);
    return strtol(s.c_str(), 0, 0);
}

int to_int(const std::string_view& s) {
    int i3;
    if (s[0] >'9') {
        std::from_chars(s.data(), s.data() + s.size(), i3, 16);
    } else {
        std::from_chars(s.data(), s.data() + s.size(), i3, 10);
    }
    return i3;
}

template<StrType<u8symbol> T>
int to_int(const T& t) {
    if (t.at(0) > '9') {
        auto r = t. template toInt<int, true, 16, false>();
        return std::get<0>(r);
    }
    auto r = t.template toInt<int, true, 10, false>();
    return std::get<0>(r);
}

template<typename T>
BENCH_EXPORT void BM_ToInt(benchmark::State& state, T t, int c) {
    for (auto _: state) {
        int res = to_int(t);
        if (res != c) {
            state.SkipWithError("not equal");
            break;
        }
        benchmark::DoNotOptimize(t);
        benchmark::DoNotOptimize(res);
    }
}
BENCHMARK(__)->Name("-----  Convert to int '  1234567'  ---------");
BENCHMARK_CAPTURE(BM_ToInt, std::string, std::string{"123456789"}, 123456789);
BENCHMARK_CAPTURE(BM_ToInt, std::from_chars, std::string_view{"123456789"}, 123456789);
BENCHMARK_CAPTURE(BM_ToInt, stringa, stringa{"123456789"}, 123456789);
BENCHMARK_CAPTURE(BM_ToInt, ssa, ssa{"123456789"}, 123456789);
BENCHMARK_CAPTURE(BM_ToInt, lstringa<20>, lstringa<20>{"123456789"}, 123456789);
BENCHMARK(__)->Name("-----  Convert to int '  0xabcDef'  ---------");
BENCHMARK_CAPTURE(BM_ToInt, std::string, std::string{"0xabcDef"}, 0xabcDef);
BENCHMARK_CAPTURE(BM_ToInt, std::from_chars, std::string_view{"abcDef"}, 0xabcDef);
BENCHMARK_CAPTURE(BM_ToInt, stringa, stringa{"abcDef"}, 0xabcDef);
BENCHMARK_CAPTURE(BM_ToInt, ssa, ssa{"abcDef"}, 0xabcDef);
BENCHMARK_CAPTURE(BM_ToInt, lstringa<20>, lstringa<20>{"abcDef"}, 0xabcDef);
