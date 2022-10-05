#pragma once
#include <stdexcept>
/*
* Чтобы тестировать static_assert'ы - в тестовом билде заменяем их на рантайм проверки с выбрасыванием исключения
*/

struct _testable_static_assert {
    _testable_static_assert(bool condition, const char* text) {
        if (!condition) {
            throw std::logic_error{text};
        }
    }
};

#ifdef BUILD_TESTS
#define testable_static_assert(p1, p2)           \
    _testable_static_assert __tsa##__COUNTER__ { \
        p1, p2                                   \
    }
#else
#define testable_static_assert static_assert
#endif
