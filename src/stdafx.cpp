#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

COREAS_API void* core_as_malloc(size_t count) {
    void* res = malloc(count);
    if (!res)
        throw std::bad_alloc{};
    return res;
}

COREAS_API void* core_as_realloc(void* ptr, size_t count) {
    ptr = realloc(ptr, count);
    if (!ptr)
        throw std::bad_alloc{};
    return ptr;
}
COREAS_API void core_as_free(void* ptr) {
    free(ptr);
}

COREAS_API void core_as_print(const wchar_t* /*text*/) {}
