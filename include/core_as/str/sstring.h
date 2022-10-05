/*
* (c) Проект "Core.As", Александр Орефков orefkov@gmail.com
* Классы для работы со строками
*/
#pragma once
#include "strexpr.h"
#include <core_as/core_as_base.h>
#include <string_view>
#include <vector>
#include <format>
#include <unordered_map>
#include <tuple>
#include <limits>

#ifdef _MSC_VER
// warning C4201 : nonstandard extension used : nameless struct / union
#pragma warning(disable : 4201)
#endif

namespace core_as::str {

template<typename T>
struct unicode_traits {};

template<>
struct unicode_traits<u8symbol> {
    // Эти операции с utf-8 могут изменить длину строки
    // Поэтому их специализации отличаются
    // В функцию помимо текста и адреса буфера для записи передается размер буфера
    // Возвращает длину получающейся строки.
    // Если получающеюся строка не влезает в отведенный буфер, указатели устанавливаются на последние
    // обработанные символы, для повторного возобновления работы,
    // а для оставшихся символов считается нужный размер буфера.
    static COREAS_API size_t upper(const u8symbol*& src, size_t lenStr, u8symbol*& dest, size_t lenBuf);
    static COREAS_API size_t lower(const u8symbol*& src, size_t len, u8symbol*& dest, size_t lenBuf);
    static COREAS_API size_t findFirstLower(const u8symbol* src, size_t lenStr);
    static COREAS_API size_t findFirstUpper(const u8symbol* src, size_t lenStr);

    static COREAS_API int compareiu(const u8symbol* text1, size_t len1, const u8symbol* text2, size_t len2);

    static COREAS_API size_t hashia(const u8symbol* src, size_t l);
    static COREAS_API size_t hashiu(const u8symbol* src, size_t l);
};

template<>
struct unicode_traits<u16symbol> {
    static COREAS_API void upper(const u16symbol* src, size_t len, u16symbol* dest);
    static COREAS_API void lower(const u16symbol* src, size_t len, u16symbol* dest);
    static COREAS_API size_t findFirstLower(const u16symbol* src, size_t lenStr);
    static COREAS_API size_t findFirstUpper(const u16symbol* src, size_t lenStr);

    static COREAS_API int compareiu(const u16symbol* text1, size_t len1, const u16symbol* text2, size_t len2);
    static COREAS_API size_t hashia(const u16symbol* src, size_t l);
    static COREAS_API size_t hashiu(const u16symbol* src, size_t l);
};

template<>
struct unicode_traits<u32symbol> {
    static COREAS_API void upper(const u32symbol* src, size_t len, u32symbol* dest);
    static COREAS_API void lower(const u32symbol* src, size_t len, u32symbol* dest);
    static COREAS_API size_t findFirstLower(const u32symbol* src, size_t lenStr);
    static COREAS_API size_t findFirstUpper(const u32symbol* src, size_t lenStr);

    static COREAS_API int compareiu(const u32symbol* text1, size_t len1, const u32symbol* text2, size_t len2);
    static COREAS_API size_t hashia(const u32symbol* src, size_t s);
    static COREAS_API size_t hashiu(const u32symbol* src, size_t s);
};

template<>
struct unicode_traits<wchar_t> {
    static void upper(const wchar_t* src, size_t len, wchar_t* dest) {
        unicode_traits<wchar_type>::upper(to_w(src), len, to_w(dest));
    }
    static void lower(const wchar_t* src, size_t len, wchar_t* dest) {
        unicode_traits<wchar_type>::lower(to_w(src), len, to_w(dest));
    }
    static size_t findFirstLower(const wchar_t* src, size_t lenStr) {
        return unicode_traits<wchar_type>::findFirstLower(to_w(src), lenStr);
    }
    static size_t findFirstUpper(const wchar_t* src, size_t lenStr) {
        return unicode_traits<wchar_type>::findFirstUpper(to_w(src), lenStr);
    }

    static int compareiu(const wchar_t* text1, size_t len1, const wchar_t* text2, size_t len2) {
        return unicode_traits<wchar_type>::compareiu(to_w(text1), len1, to_w(text2), len2);
    }
    static size_t hashia(const wchar_t* src, size_t s) {
        return unicode_traits<wchar_type>::hashia(to_w(src), s);
    }
    static size_t hashiu(const wchar_t* src, size_t s) {
        return unicode_traits<wchar_type>::hashiu(to_w(src), s);
    }
};


namespace str_pos {
constexpr const size_t badIdx = static_cast<size_t>(-1);
} // namespace str_pos

template<size_t N>
concept is_const_pattern = N > 1 && N <= 17;

template<typename K, size_t I>
struct _ascii_mask {
    constexpr static const size_t value = size_t(K(~0x7F)) << ((I - 1) * sizeof(K) * 8) | _ascii_mask<K, I-1>::value;
};

template<typename K>
struct _ascii_mask<K, 0> {
    constexpr static const size_t value = 0;
};

template<typename K>
struct ascii_mask {
    using uns = typename std::make_unsigned<K>::type;
    constexpr static const size_t width = sizeof(size_t) / sizeof(uns);
    constexpr static const size_t value = _ascii_mask<uns, width>::value;
};

template<typename K>
constexpr static bool isAsciiUpper(K k) {
    return k >= 'A' && k <= 'Z';
}

template<typename K>
constexpr static bool isAsciiLower(K k) {
    return k >= 'a' && k <= 'z';
}

template<typename K>
constexpr static K makeAsciiLower(K k) {
    return isAsciiUpper(k) ? k | 0x20 : k;
}

template<typename K>
constexpr static K makeAsciiUpper(K k) {
    return isAsciiLower(k) ? k & ~0x20 : k;
}

enum TrimSides { trimLeft = 1, trimRight = 2, trimAll = 3 };
template<TrimSides S, typename K, size_t N, bool withSpaces = false>
struct trimOperator;

template<typename K, size_t N, size_t L> struct expr_replaces;

template<typename T>
concept FromIntNumber = is_one_of_type<std::remove_cv_t<T>, int, short, long, long long, unsigned, unsigned short, unsigned long, unsigned long long>::value;

template<typename T>
concept ToIntNumber = FromIntNumber<T> || is_one_of_type<T, int8_t, uint8_t>::value;

#ifdef _MSC_VER
template <typename K, typename... _Args>
using FmtString = std::_Basic_format_string<K, std::type_identity_t<_Args>...>;
#elif __clang_major__ >= 15
template <typename K, typename... _Args>
using FmtString = std::basic_format_string<K, std::type_identity_t<_Args>...>;
#else
template <typename K, typename... _Args>
using FmtString = std::basic_string_view<K>;
#endif

template<typename K, bool I, typename T>
struct need_sign {
    bool sign;
    need_sign(T& t) : sign(t < 0) { if (sign) t = -t; }
    void after(K*& ptr) { if (sign) *--ptr = '-'; }
};

template<typename K, typename T>
struct need_sign<K, false, T> {
    need_sign(T&) {}
    void after(K*&) {}
};

enum class IntConvertResult : char {
    Success,
    BadSymbolAtTail,
    Overflow,
    NotNumber
};

template<bool CanNegate, bool CheckOverflow, typename T>
struct result_type_selector {
    using type = T;
};

template<typename T>
struct result_type_selector<true, false, T> {
    using type = std::make_unsigned_t<T>;
};

template<bool CanNegate>
struct negate_store {
    bool canNegate() const { return false; }
    void setNegate() {}
    bool isNegate() const { return false; }
};

template<>
struct negate_store<true> {
    bool negate {false};
    bool canNegate() const { return true; }
    void setNegate() { negate = true; }
    bool isNegate() const { return negate; }
};

template<bool CanNegate, bool CheckOverflow, typename T>
struct IntProcessor : negate_store<CanNegate> {
protected:
    using type = typename result_type_selector<CanNegate, CheckOverflow, T>::type;
    using negate = negate_store<CanNegate>;
    type result{0};
public:
    void setFirstDigit(type s) {
        if constexpr (CheckOverflow && CanNegate) {
            if (negate::isNegate()) {
                result = -s;
                return;
            }
        }
        result = s;
    }

    bool addWithOverflow(type base, type add) {
        if constexpr (CheckOverflow) {
            if constexpr (CanNegate) {
                if (negate::isNegate()) {
                    if (result < std::numeric_limits<T>::min() / base)
                        return false;
                    result *= base;
                    if (std::numeric_limits<T>::min() - result > -add) {
                        result /= base;
                        return false;
                    }
                    result -= add;
                } else {
                    if (result > std::numeric_limits<T>::max() / base)
                        return false;
                    result *= base;
                    if (std::numeric_limits<T>::max() - result < add) {
                        result /= base;
                        return false;
                    }
                    result += add;
                }
                return true;
            } else {
                if (result > std::numeric_limits<T>::max() / base)
                    return false;
                result *= base;
                if (std::numeric_limits<T>::max() - result < add) {
                    result /= base;
                    return false;
                }
                result += add;
                return true;
            }
        } else {
            result = result * base + add;
            return true;
        }
    }

    T getResult() const {
        if constexpr (!CheckOverflow && CanNegate) {
            if (negate::isNegate()) {
                return -(T)result;
            }
        }
        return result;
    }
};


struct int_convert {
private:
    inline static constexpr uint8_t numbers[] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0,   1,   2,   3,
        4,   5,   6,   7,   8,   9,   255, 255, 255, 255, 255, 255, 255, 10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
        23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  255, 255, 255, 255, 255, 255, 10,  11,  12,  13,  14,  15,  16,
        17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

    template<typename K>
    static uint8_t toDigit(K s) {
        if constexpr (sizeof(K) == 1) {
            return numbers[static_cast<std::make_unsigned_t<K>>(s)];
        } else {
            auto us = static_cast<std::make_unsigned_t<K>>(s);
            return s < 256 ? numbers[us] : 255;
        }
    }
    template<typename K, ToIntNumber T, unsigned Base, bool CheckOverflow>
    requires(Base != 0) static std::tuple<T, IntConvertResult, size_t> parse(const K* start, const K* current, const K* end, bool negate) {
        using u_type = std::make_unsigned_t<T>;
        u_type maxMult, maxAdd;
        if constexpr (CheckOverflow) {
            if constexpr (std::is_signed_v<T>) {
                if (negate) {
                    maxMult = (std::numeric_limits<u_type>::max() / 2 + 1) / Base;
                    maxAdd  = (std::numeric_limits<u_type>::max() / 2 + 1) % Base;
                } else {
                    maxMult = (std::numeric_limits<u_type>::max() / 2) / Base;
                    maxAdd  = (std::numeric_limits<u_type>::max() / 2) % Base;
                }
            } else {
                maxMult = std::numeric_limits<u_type>::max() / Base;
                maxAdd  = std::numeric_limits<u_type>::max() % Base;
            }
        }
        u_type number = 0;
        IntConvertResult error = IntConvertResult::NotNumber;
        for (;;) {
            const unsigned char digit = toDigit(*current);
            if (digit >= Base) {
                break;
            }
            if constexpr (CheckOverflow) {
                if (number < maxMult || (number == maxMult && digit <= maxAdd)) {
                    number = static_cast<u_type>(number * Base + digit);
                } else {
                    error = IntConvertResult::Overflow;
                    break;
                }
            } else {
                number = number * Base + digit;
            }
            error = IntConvertResult::BadSymbolAtTail;
            if (++current == end) {
                error = IntConvertResult::Success;
                break;
            }
        }
        T result;
        if constexpr (std::is_signed_v<T>) {
            result = negate ? 0 - number : number;
        } else {
            result = number;
        }
        return {result, error, current - start};
    }

public:
    template<typename K, ToIntNumber T, unsigned Base = 0, bool CheckOverflow = true, bool SkipWs = true>requires(Base == 0 || (Base > 1 && Base < 37))
    static std::tuple<T, IntConvertResult, size_t> toInteger(const K* start, size_t len) noexcept {
        const K *ptr = start, *end = ptr + len;
        bool negate  = false;
        if constexpr (SkipWs) {
            while (ptr < end && *ptr <= ' ')
                ptr++;
        }
        if (ptr != end) {
            if constexpr (std::is_signed_v<T>) {
                if (*ptr == '-') {
                    negate = true;
                    ptr++;
                } else if (*ptr == '+') {
                    ptr++;
                }
            } else if (*ptr == '+') {
                ptr++;
            }
        }
        if (ptr != end) {
            if constexpr (Base == 0) {
                if (*ptr == '0') {
                    ptr++;
                    if (ptr != end) {
                        if (*ptr == 'x' || *ptr == 'X') {
                            return parse<K, T, 16, CheckOverflow>(start, ++ptr, end, negate);
                        }
                        return parse<K, T, 8, CheckOverflow>(start, --ptr, end, negate);
                    }
                    return {0, IntConvertResult::Success, ptr - start};
                }
                return parse<K, T, 10, CheckOverflow>(start, ptr, end, negate);
            } else
                return parse<K, T, Base, CheckOverflow>(start, ptr, end, negate);
        }
        return {0, IntConvertResult::NotNumber, ptr - start};
    }
};

/*
* Класс с базовыми строковыми алгоритмами
* Является базой для классов, могущих выполнять константные операции со строками.
* Ничего не знает о хранении строк, ни сам, ни у класса наследника, то есть работает
* только с указателем на строку и её длиной.
* Для работы класс-наследник должен реализовать методы:
*   size_t length() const noexcept     - возвращает длину строки
*   const K* symbols() const noexcept  - возвращает указатель на начало строки
*   bool isEmpty() const noexcept    - проверка, не пустая ли строка
* K       - тип символов
* StrRef  - тип хранилища куска строки
* Impl    - конечный класс наследник
*/

template<typename K, typename StrRef, typename Impl>
class str_algs {
    const Impl& d() const noexcept {
        return *static_cast<const Impl*>(this);
    }
    size_t _len() const noexcept {
        return d().length();
    }
    const K* _str() const noexcept {
        return d().symbols();
    }
    bool _isEmpty() const noexcept {
        return d().isEmpty();
    }

public:
    using symb_type = K;
    using StrPiece = StrRef;
    using traits = std::char_traits<K>;
    using uni = unicode_traits<K>;
    using uns_type = std::make_unsigned_t<K>;
    using my_type = Impl;
    using base = str_algs<K, StrRef, Impl>;
    // Пустой конструктор
    str_algs() = default;

    constexpr K* place(K* ptr) const noexcept {
        size_t myLen = _len();
        if (myLen) {
            traits::copy(ptr, _str(), myLen);
            return ptr + myLen;
        }
        return ptr;
    }

    void copyTo(K* buffer, size_t bufSize) {
        size_t tlen = std::min(_len(), bufSize - 1);
        if (tlen)
            traits::copy(buffer, _str(), tlen);
        buffer[tlen] = 0;
    }

    const K* begin() const noexcept { return _str(); }
    const K* end() const noexcept { return _str() + _len(); }

    // Чтобы быть источником строкового объекта
    constexpr operator StrPiece() const noexcept {
        return StrPiece{ _str(), _len() };
    }
    StrPiece toStr() const noexcept {
        return StrPiece{ _str(), _len() };
    }

    constexpr StrPiece operator () (int from, int len = 0) const noexcept {
        size_t myLen = _len(), idxStart = from >= 0 ? from : myLen + from,
            idxEnd = (len > 0 ? idxStart : myLen) + len;
        if (idxEnd > myLen)
            idxEnd = myLen;
        if (idxStart > idxEnd)
            idxStart = idxEnd;
        return StrPiece { _str() + idxStart, idxEnd - idxStart };
    }
    constexpr StrPiece mid(size_t from, int len = -1) const noexcept {
        size_t myLen = _len(), idxStart = from,
            idxEnd = len >= 0 ? from + len : myLen;
        if (idxEnd > myLen)
            idxEnd = myLen;
        if (idxStart > idxEnd)
            idxStart = idxEnd;
        return StrPiece{ _str() + idxStart, idxEnd - idxStart };
    }
    void store(char*& ptr) const noexcept {
        size_t len = (_len() + 1) * sizeof(K);
        memcpy(ptr, _str(), len);
        ptr += len;
    }

    bool operator!() const noexcept {
        return _isEmpty();
    }
    // Доступ к символу, отрицательные - с конца строки
    K at(int idx) const {
        return _str()[idx >= 0 ? idx : _len() + idx];
    }
    // Сравнение строк
    constexpr int compare(const K* text, size_t len) const { // Сравнение строк
        size_t myLen = _len();
        int cmp      = traits::compare(_str(), text, std::min(myLen, len));
        return cmp == 0 ? (myLen > len ? 1 : myLen == len ? 0 : -1) : cmp;
    }
    constexpr int compare(StrPiece o) const {
        return compare(o.symbols(), o.length());
    }

    constexpr bool isEqual(const K* text, size_t len) const noexcept {
        return len == _len() && traits::compare(_str(), text, len) == 0;
    }

    constexpr bool isEqual(StrPiece other) const noexcept {
        return isEqual(other.symbols(), other.length());
    }

    constexpr bool operator==(const base& other) const noexcept {
        return isEqual(other._str(), other._len());
    }

    int operator<=>(const base& other) const noexcept {
        return compare(other._str(), other._len());
    }

    template<size_t N>
    bool operator == (const K(&other)[N]) const noexcept {
        return N - 1 == _len() && traits::compare(_str(), other, N - 1) == 0;
    }

    template<size_t N>
    int operator <=> (const K(&other)[N]) const noexcept {
        size_t myLen = _len();
        int cmp = traits::compare(_str(), other, std::min(myLen, N - 1));
        return cmp == 0 ? (myLen > N - 1 ? 1 : myLen == N - 1 ? 0 : -1) : cmp;
    }

    // Сравнение ascii строк без учёта регистра
    int compare_ia(const K* text, size_t len) const noexcept {
        if (!len)
            return _isEmpty() ? 0 : 1;
        size_t myLen = _len(), checkLen = std::min(myLen, len);
        const uns_type *ptr1 = reinterpret_cast<const uns_type*>(_str()), *ptr2 = reinterpret_cast<const uns_type*>(text);
        while (checkLen--) {
            uns_type s1 = *ptr1++, s2 = *ptr2++;
            if (s1 == s2)
                continue;
            s1 = makeAsciiLower(s1);
            s2 = makeAsciiLower(s2);
            if (s1 > s2)
                return 1;
            else if (s1 < s2)
                return -1;
        }
        return myLen == len ? 0 : myLen > len ? 1 : -1;
    }
    int compare_ia(StrPiece text) const noexcept {
        return compare_ia(text.symbols(), text.length());
    }

    bool isEqual_ia(StrPiece text) const noexcept {
        return text.length() == _len() && compare_ia(text.symbols(), text.length()) == 0;
    }

    bool isLess_ia(StrPiece text) const noexcept {
        return compare_ia(text.symbols(), text.length()) < 0;
    }

    int compare_iu(const K* text, size_t len) const noexcept {
        if (!len)
            return _isEmpty() ? 0 : 1;
        return uni::compareiu(_str(), _len(), text, len);
    }
    int compare_iu(StrPiece text) const noexcept {
        return compare_iu(text.symbols(), text.length());
    }

    bool isEqual_iu(StrPiece text) const noexcept {
        return text.length() == _len() && compare_iu(text.symbols(), text.length()) == 0;
    }

    bool isLess_iu(StrPiece text) const noexcept {
        return compare_iu(text.symbols(), text.length()) < 0;
    }

    size_t find(const K* pattern, size_t lenPattern, size_t offset) const noexcept {
        size_t lenText = _len();
        // Образец, не вмещающийся в строку и пустой образец не находим
        if (!lenPattern || offset + lenPattern > lenText)
            return str_pos::badIdx;
        lenPattern--;
        const K *text = _str(), *last = text + lenText - lenPattern, first = pattern[0];
        pattern++;
        for (const K *fnd = text + offset; ; ++fnd) {
            fnd = traits::find(fnd, last - fnd, first);
            if (!fnd)
                return str_pos::badIdx;
            if (traits::compare(fnd + 1, pattern, lenPattern) == 0)
                return static_cast<size_t>(fnd - text);
        }
    }
    size_t find(StrPiece pattern, size_t offset = 0) const noexcept {
        return find(pattern.symbols(), pattern.length(), offset);
    }

    size_t find(K s, size_t offset = 0) const noexcept {
        size_t len = _len();
        if (offset < len) {
            const K *str = _str(), *fnd = traits::find(str + offset, len - offset, s);
            if (fnd)
                return static_cast<size_t>(fnd - str);
        }
        return str_pos::badIdx;
    }

    template<typename Op>
    void forAllFind(const Op& op, const K* pattern, size_t patternLen, size_t offset, size_t maxCount) const {
        if (!maxCount)
            maxCount--;
        while (maxCount-- > 0) {
            size_t fnd = find(pattern, patternLen, offset);
            if (fnd == str_pos::badIdx)
                break;
            op(fnd);
            offset = fnd + patternLen;
        }
    }
    template<typename Op>
    void forAllFind(const Op& op, StrPiece pattern, size_t offset = 0, size_t maxCount = 0) const {
        forAllFind(op, pattern.symbols(), pattern.length(), offset, maxCount);
    }

    std::vector<size_t> findAll(const K* pattern, size_t patternLen, size_t offset, size_t maxCount) const {
        std::vector<size_t> result;
        forAllFind([&](auto f) { result.push_back(f); }, pattern, patternLen, offset, maxCount);
        return result;
    }
    std::vector<size_t> findAll(StrPiece pattern, size_t offset = 0, size_t maxCount = 0) const {
        return findAll(pattern.symbols(), pattern.length(), offset, maxCount);
    }

    size_t findReverse(K s) const noexcept {
        size_t len = _len();
        while (len > 0) {
            if (_str()[--len] == s)
                return len;
        }
        return str_pos::badIdx;
    }

    my_type substr(int from, int len = 0) const { // индексация в code units
        return my_type{ d()(from, len) };
    }
    my_type strMid(size_t from, int len = -1) const { // индексация в code units
        return my_type{ d().mid(from, len) };
    }

    template<ToIntNumber T>
    T asInt() const noexcept {
        auto [res, err, _] = int_convert::toInteger<K, T, 0, true>(_str(), _len());
        return err == IntConvertResult::Overflow ? 0 : res;
    }

    template<ToIntNumber T, bool CheckOverflow = true, unsigned Base = 0, bool SkipWs=true>
    std::tuple<T, IntConvertResult, size_t> toInt() const noexcept {
        return int_convert::toInteger<K, T, Base, CheckOverflow, SkipWs>(_str(), _len());
    }

    double toDouble() const noexcept {
        static_assert(sizeof(K) == 1 || sizeof(K) == sizeof(wchar_t), "Only char and wchar available for conversion to double now");
        size_t len = _len();
        if (len) {
            const size_t copyLen = 64;
            K buf[copyLen + 1];
            const K* ptr = _str();
            if (ptr[len] != 0) {
                while (len && *ptr <= ' ') {
                    len--;
                    ptr++;
                }
                if (len) {
                    len = std::min(copyLen, len);
                    traits::copy(buf, ptr, len);
                    buf[len] = 0;
                    ptr = buf;
                }
            }
            if (len) {
            #ifdef _MSC_VER
                static const _locale_t lc = _wcreate_locale(LC_NUMERIC, L"C");
                if constexpr (sizeof(K) == 1) {
                    return _strtod_l(ptr, nullptr, lc);
                } else {
                    return _wcstod_l(ptr, nullptr, lc);
                }
            #else
                if constexpr (sizeof(K) == 1) {
                    return std::strtod(ptr, nullptr);
                } else if constexpr (sizeof(K) == sizeof(wchar_t)) {
                    return std::wcstod((const wchar_t*)ptr, nullptr);
                }
            #endif
            }
        }
        return 0.0;
    }
    
    template<ToIntNumber T>
    void asNumber(T& t) {
        t = asInt<T>();
    }

    void asNumber(double& t) {
        t = toDouble();
    }

    template<typename T, typename Op>
    T splitf(const K* delimeter, size_t lenDelimeter, const Op& beforeFunc, size_t offset) const {
        size_t mylen = _len();
        T results;
        StrPiece me{ _str(), mylen };
        for (;;) {
            size_t beginOfDelim = find(delimeter, lenDelimeter, offset);
            if (beginOfDelim == str_pos::badIdx) {
                StrPiece last{ me.symbols() + offset, me.length() - offset };
                if constexpr (std::is_invocable_v<Op, StrPiece&>) {
                    beforeFunc(last);
                }
                if (last.isSame(me)) {
                    // Пробуем положить весь объект.
                    results.emplace_back(d());
                } else
                    results.emplace_back(last);
                break;
            }
            StrPiece piece{me.symbols() + offset, beginOfDelim - offset};
            if constexpr (std::is_invocable_v<Op, StrPiece&>) {
                beforeFunc(piece);
            }
            results.emplace_back(piece);
            offset = beginOfDelim + lenDelimeter;
        }
        return results;
    }
    template<typename T, typename Op>
    T splitf(StrPiece delimeter, const Op& beforeFunc, size_t offset = 0) const {
        return splitf<T>(delimeter.symbols(), delimeter.length(), beforeFunc, offset);
    }
    // Разбиение строки на части
    template<typename T>
    T split(StrPiece delimeter, size_t offset = 0) const {
        return splitf<T>(delimeter.symbols(), delimeter.length(), 0, offset);
    }

    // Начинается ли эта строка с указанной подстроки
    constexpr bool starts_with(const K* prefix, size_t l) const noexcept {
        return _len() >= l && 0 == traits::compare(_str(), prefix, l);
    }
    constexpr bool starts_with(StrPiece prefix) {
        return starts_with(prefix.symbols(), prefix.length());
    }
    // Начинается ли эта строка с указанной подстроки без учета ascii регистра
    constexpr bool starts_with_ia(const K* prefix, size_t len) const noexcept {
        size_t myLen = _len();
        if (myLen < len) {
            return false;
        }
        const K *ptr1 = _str();
        while (len--) {
            K s1 = *ptr1++, s2 = *prefix++;
            if (s1 == s2)
                continue;
            if (makeAsciiLower(s1) != makeAsciiLower(s2))
                return false;
        }
        return true;
    }
    constexpr bool starts_with_ia(StrPiece prefix) const noexcept {
        return starts_with_ia(prefix.symbols(), prefix.length());
    }
    // Начинается ли эта строка с указанной подстроки без учета unicode регистра
    bool starts_with_iu(const K* prefix, size_t len) const noexcept {
        return _len() >= len && 0 == uni::compareiu(_str(), len, prefix, len);
    }
    bool starts_with_iu(StrPiece prefix) const noexcept {
        return starts_with_iu(prefix.symbols(), prefix.length());
    }

    // Является ли эта строка началом указанной строки
    constexpr bool isPrefixIn(const K* text, size_t len) const noexcept {
        size_t myLen = _len();
        if (myLen > len)
            return false;
        return !myLen || 0 == traits::compare(text, _str(), myLen);
    }
    constexpr bool isPrefixIn(StrPiece text) const noexcept {
        return isPrefixIn(text.symbols(), text.length());
    }
    // Заканчивается ли строка указанной подстрокой
    constexpr bool ends_with(const K* suffix, size_t len) const noexcept {
        size_t myLen = _len();
        return len <= myLen && traits::compare(_str() + myLen - len, suffix, len) == 0;
    }
    constexpr bool ends_with(StrPiece suffix) const noexcept {
        return ends_with(suffix.symbols(), suffix.length());
    }
    // Заканчивается ли строка указанной подстрокой без учета регистра ASCII
    constexpr bool ends_with_ia(const K* suffix, size_t len) const noexcept {
        size_t myLen = _len();
        if (myLen < len) {
            return false;
        }
        const K* ptr1 = _str() + myLen - len;
        while (len--) {
            K s1 = *ptr1++, s2 = *suffix++;
            if (s1 == s2)
                continue;
            if (makeAsciiLower(s1) != makeAsciiLower(s2))
                return false;
        }
        return true;
    }
    constexpr bool ends_with_ia(StrPiece suffix) const noexcept {
        return ends_with_ia(suffix.symbols(), suffix.length());
    }
    // Заканчивается ли строка указанной подстрокой без учета регистра UNICODE
    constexpr bool ends_with_iu(const K* suffix, size_t len) const noexcept {
        size_t myLen = _len();
        return myLen >= len && 0 == uni::compareiu(_str() + myLen - len, len, suffix, len);
    }
    constexpr bool ends_with_iu(StrPiece suffix) const noexcept {
        return ends_with_iu(suffix.symbols(), suffix.length());
    }

    bool isAscii() const noexcept {
        if (_isEmpty())
            return true;
        const int sl = ascii_mask<K>::width;
        const size_t mask = ascii_mask<K>::value;
        size_t len = _len();
        const uns_type* ptr = reinterpret_cast<const uns_type*>(_str());
        if constexpr (sl > 1) {
            while (len >= sl) {
                if (*reinterpret_cast<const size_t*>(ptr) & mask)
                    return false;
                ptr += sl;
                len -= sl;
            }
        }
        while (len--) {
            if (*ptr++ > 127)
                return false;
        }
        return true;
    }
    // ascii версия upper
    template<typename R = my_type>
    R upperedOnlyAscii() const {
        return R::template upperedOnlyAsciiFrom(d());
    }
    // ascii версия lower
    template<typename R = my_type>
    R loweredOnlyAscii() const {
        return R::template loweredOnlyAsciiFrom(d());
    }
    template<typename R = my_type>
    R uppered() const {
        return R::template upperedFrom(d());
    }
    template<typename R = my_type>
    R lowered() const {
        return R::template loweredFrom(d());
    }

    template<typename R = my_type>
    R replaced(StrPiece pattern, StrPiece repl, size_t offset = 0, size_t maxCount = 0) const {
        return R::template replacedFrom(d(), pattern, repl, offset, maxCount);
    }

    template<size_t N, size_t L>
    expr_replaces<K, N - 1, L - 1> replace_init(const K(&pattern)[N], const K(&repl)[L]) {
        return expr_replaces<K, N - 1, L - 1>{d(), pattern, repl};
    }

    template<StrType<K> From, typename Op>
    static my_type makeTrimOp(const From& from, const Op& opTrim) {
        StrPiece sfrom = from, newPos = opTrim(sfrom);
        return newPos.isSame(sfrom) ? my_type{ from } : my_type{ newPos };
    }
    template<TrimSides S, StrType<K> From>
    static my_type trimStatic(const From& from) {
        return makeTrimOp(from, trimOperator<S, K, static_cast<size_t>(-1), true>{});
    }

    template<TrimSides S, bool withSpaces, size_t N, StrType<K> From> requires is_const_pattern<N>
    static my_type trimStatic(const From& from, const K(&pattern)[N]) {
        return makeTrimOp(from, trimOperator<S, K, N - 1, withSpaces> {pattern});
    }

    template<TrimSides S, bool withSpaces, StrType<K> From>
    static my_type trimStatic(const From& from, StrPiece pattern) {
        return makeTrimOp(from, trimOperator<S, K, 0, withSpaces>{{pattern}});
    }
    // Триминг по пробельным символам - ' ', \t\n\v\f\r
    template<typename R = my_type>
    R trimmed() const {
        return R::template trimStatic<TrimSides::trimAll>(d());
    }
    template<typename R = my_type>
    R trimmedLeft() const {
        return R::template trimStatic<TrimSides::trimLeft>(d());
    }
    template<typename R = my_type>
    R trimmedRight() const {
        return R::template trimStatic<TrimSides::trimRight>(d());
    }
    // Триминг по символам в литерале
    template<typename R = my_type, size_t N> requires is_const_pattern<N>
    R trimmed(const K(&pattern)[N]) const {
        return R::template trimStatic<TrimSides::trimAll, false>(d(), pattern);
    }
    template<typename R = my_type, size_t N> requires is_const_pattern<N>
    R trimmedLeft(const K(&pattern)[N]) const {
        return R::template trimStatic<TrimSides::trimLeft, false>(d(), pattern);
    }
    template<typename R = my_type, size_t N> requires is_const_pattern<N>
    R trimmedRight(const K(&pattern)[N]) const {
        return R::template trimStatic<TrimSides::trimRight, false>(d(), pattern);
    }
    // Триминг по символам в литерале и пробелам
    template<typename R = my_type, size_t N> requires is_const_pattern<N>
    R trimmedWithSpaces(const K(&pattern)[N]) const {
        return R::template trimStatic<TrimSides::trimAll, true>(d(), pattern);
    }
    template<typename R = my_type, size_t N> requires is_const_pattern<N>
    R trimmedLeftWithSpaces(const K(&pattern)[N]) const {
        return R::template trimStatic<TrimSides::trimLeft, true>(d(), pattern);
    }
    template<typename R = my_type, size_t N> requires is_const_pattern<N>
    R trimmedRightWithSpaces(const K(&pattern)[N]) const {
        return R::template trimStatic<TrimSides::trimRight, true>(d(), pattern);
    }
    // Триминг по динамическому источнику
    template<typename R = my_type>
    R trimmed(StrPiece pattern) const {
        return R::template trimStatic<TrimSides::trimAll, false>(d(), pattern);
    }
    template<typename R = my_type>
    R trimmedLeft(StrPiece pattern) const {
        return R::template trimStatic<TrimSides::trimLeft, false>(d(), pattern);
    }
    template<typename R = my_type>
    R trimmedRight(StrPiece pattern) const {
        return R::template trimStatic<TrimSides::trimRight, false>(d(), pattern);
    }
    // Триминг по символам в литерале и пробелам
    template<typename R = my_type>
    R trimmedWithSpaces(StrPiece pattern) const {
        return R::template trimStatic<TrimSides::trimAll, true>(d(), pattern);
    }
    template<typename R = my_type>
    R trimmedLeftWithSpaces(StrPiece pattern) const {
        return R::template trimStatic<TrimSides::trimLeft, true>(d(), pattern);
    }
    template<typename R = my_type>
    R trimmedRightWithSpaces(StrPiece pattern) const {
        return R::template trimStatic<TrimSides::trimRight, true>(d(), pattern);
    }
};

template<typename K>
struct SimpleStrNt;

#ifdef _MSC_VER
#define empty_bases __declspec(empty_bases) 
#else
#define empty_bases
#endif

/*
* Базовая структура с информацией о строке.
* Это структура для невладеющих строк.
* Так как здесь только один базовый класс, MSVC компилятор автоматом применяет empty base optimization,
* в результате размер класса не увеличивается
*/
template<typename K>
struct SimpleStr : str_algs<K, SimpleStr<K>, SimpleStr<K> > {
    using symb_type = K;
    using my_type = SimpleStr<K>;

    const symb_type* str;
    size_t len;
    
    SimpleStr() = default;
    
    template<size_t N>
    constexpr SimpleStr(const K(&v)[N]) noexcept : str(v), len(N - 1) {}
    
    constexpr SimpleStr(const K* p, size_t l) noexcept : str(p), len(l) {}

    constexpr size_t length() const noexcept {
        return len;
    }
    constexpr const symb_type* symbols() const noexcept {
        return str;
    }
    constexpr bool isEmpty() const noexcept {
        return len == 0;
    }

    bool isSame(const SimpleStr<K>& other) const noexcept{
        return str == other.str && len == other.len;
    }
    K operator[] (size_t idx) const {
        return str[idx];
    }
};

/*
* Класс, заявляющий, что ссылается на нуль-терминированную строку.
* Служит для показателя того, что функция параметром хочет получить
* строку с нулем в конце, например, ей надо дальше передавать его в
* стороннее API. Без этого ей надо было бы либо указывать параметром
* конкретный класс строки, что лишает универсальности, либо приводило бы
* к постоянным накладным расходам на излишнее копирование строк во временный
* буфер. Источником нуль-терминированных строк могут быть константные строки
* при компиляции, либо классы, хранящие строки.
*/
template<typename K>
struct SimpleStrNt : SimpleStr<K> {
    using symb_type = K;
    using my_type = SimpleStrNt<K>;
    using base = SimpleStr<K>;
    using base::base;

    constexpr static const K empty_string[1] = { 0 };

    SimpleStrNt() = default;

    explicit SimpleStrNt(const K* p) noexcept {
        base::len = p ? static_cast<size_t>(base::traits::length(p)) : 0;
        base::str = base::len ? p : empty_string;
    }
    static const my_type empty;

    operator const K* () const noexcept {
        return base::str;
    }
};

template<typename K>
inline const SimpleStrNt<K> SimpleStrNt<K>::empty{ SimpleStrNt<K>::empty_string, 0};

using ssa = SimpleStr<u8symbol>;
using ssw = SimpleStr<wchar_t>;
using ssu = SimpleStr<u16symbol>;
using ssuu = SimpleStr<u32symbol>;
using stra = SimpleStrNt<u8symbol>;
using strw = SimpleStrNt<wchar_t>;
using stru = SimpleStrNt<u16symbol>;
using struu = SimpleStrNt<u16symbol>;

template<typename K, bool withSpaces>
struct CheckSpaceTrim {
    bool isTrimSpaces(K s) const {
        return s == ' ' || (s >= 9 && s <= 13);// || isspace(s);
    }
};
template<typename K>
struct CheckSpaceTrim<K, false> {
    bool isTrimSpaces(K) const {
        return false;
    }
};

template<typename K>
struct CheckSymbolsTrim  {
    SimpleStr<K> symbols;
    bool isTrimSybols(K s) const {
        return symbols.len != 0 && SimpleStr<K>::traits::find(symbols.str, symbols.len, s) != nullptr;
    }
};

template<typename K, size_t N>
struct CheckConstSymbolsTrim {
    const K symbols[N];
    
    CheckConstSymbolsTrim() = default;
    
    template <typename... Characters>
    constexpr CheckConstSymbolsTrim(Characters... characters)
        : symbols{ characters... } {
    }
    
    template <size_t... Indexes>
    constexpr CheckConstSymbolsTrim(const K(&value)[N + 1], std::index_sequence<Indexes...>) :
        CheckConstSymbolsTrim(value[Indexes]...) {
    }

    constexpr CheckConstSymbolsTrim(const K(&s)[N + 1]) : CheckConstSymbolsTrim(s, typename std::make_index_sequence<N>{}){
    }

    template<size_t Idx> requires (Idx == N)
    constexpr bool isInSymbols(K) const noexcept {
        return false;
    }

    template<size_t Idx> requires (Idx != N)
    constexpr bool isInSymbols(K s) const noexcept {
        return s == symbols[Idx] || isInSymbols<Idx + 1>(s);
    }

    bool isTrimSybols(K s) const noexcept {
        return isInSymbols<0>(s);
    }
};

template<typename K>
struct CheckConstSymbolsTrim<K, 0> {
    bool isTrimSybols(K) const {
        return false;
    }
};

template<typename K, size_t N>
struct SymbSelector {
    using type = CheckConstSymbolsTrim<K, N>;
};

template<typename K>
struct SymbSelector<K, 0> {
    using type = CheckSymbolsTrim<K>;
};

template<typename K>
struct SymbSelector<K, static_cast<size_t>(-1)> {
    using type = CheckConstSymbolsTrim<K, 0>;
};

template<TrimSides S, typename K, size_t N, bool withSpaces>
struct trimOperator : SymbSelector<K, N>::type, CheckSpaceTrim<K, withSpaces> {
    bool isTrim(K s) const {
        return CheckSpaceTrim<K, withSpaces>::isTrimSpaces(s) || SymbSelector<K, N>::type::isTrimSybols(s);
    }
    SimpleStr<K> operator()(SimpleStr<K> from) const {
        if constexpr ((S & TrimSides::trimLeft) != 0) {
            while (from.len) {
                if (isTrim(*from.str)) {
                    from.str++;
                    from.len--;
                } else
                    break;
            }
        }
        if constexpr ((S & TrimSides::trimRight) != 0) {
            const K* back = from.str + from.len - 1;
            while (from.len) {
                if (isTrim(*back)) {
                    back--;
                    from.len--;
                } else
                    break;
            }
        }
        return from;
    }
};

template<TrimSides S, typename K>
using SimpleTrim = trimOperator<S, K, size_t(-1), true>;
using trim_w = SimpleTrim<TrimSides::trimAll, u16symbol>;
using trim_a = SimpleTrim<TrimSides::trimAll, u8symbol>;
using triml_w = SimpleTrim<TrimSides::trimLeft, u16symbol>;
using triml_a = SimpleTrim<TrimSides::trimLeft, u8symbol>;
using trimr_w = SimpleTrim<TrimSides::trimRight, u16symbol>;
using trimr_a = SimpleTrim<TrimSides::trimRight, u8symbol>;

template<TrimSides S = TrimSides::trimAll, bool withSpaces = false, typename K, size_t N> requires is_const_pattern<N>
static auto trimOp(const K(&pattern)[N]) {
    return trimOperator<S, K, N - 1, withSpaces> {pattern};
}

template<TrimSides S = TrimSides::trimAll, bool withSpaces = false, typename K>
static auto trimOp(SimpleStr<K> pattern) {
    return trimOperator<S, K, 0, withSpaces> {pattern};
}

template<typename Src, typename Dest>
struct utf_convert_selector;

template<>
struct utf_convert_selector<u8symbol, u16symbol> {
    // Максимально один code_point utf8 может преобразоваться в 1 code_unit utf16,
    // а code_point'ов в utf8 максимум может быть столько, сколько code_unit'ов.
    static size_t maxSpace(size_t len) { return len; }
    COREAS_API static size_t convert(const u8symbol* src, size_t srcLen, u16symbol* dest);
};

template<>
struct utf_convert_selector<u8symbol, u32symbol> {
    // Максимально один code_point utf8 может преобразоваться в 1 code_unit utf32,
    // а code_point'ов в utf8 максимум может быть столько, сколько code_unit'ов.
    static size_t maxSpace(size_t len) { return len; }
    COREAS_API static size_t convert(const u8symbol* src, size_t srcLen, u32symbol* dest);
};

template<>
struct utf_convert_selector<u8symbol, wchar_t> {
    static size_t maxSpace(size_t len) {
        return utf_convert_selector<u8symbol, wchar_type>::maxSpace(len);
    }
    static size_t convert(
        const u8symbol* src, size_t srcLen, wchar_t* dest)
    {
        return utf_convert_selector<u8symbol, wchar_type>::convert(
            src, srcLen, to_w(dest));
    }
};

template<>
struct utf_convert_selector<u16symbol, u8symbol> {
    // Максимально один code_unit utf16 может преобразоваться в 3 code_unit utf8,
    // либо 2 code_point utf16 в 4 code_unit utf8
    static size_t maxSpace(size_t len) { return len * 3; }
    COREAS_API static size_t convert(const u16symbol* src, size_t srcLen, u8symbol* dest);
};

template<>
struct utf_convert_selector<u16symbol, u32symbol> {
    // Максимально один code_unit utf16 может преобразоваться в 1 code_unit utf32,
    static size_t maxSpace(size_t len) { return len; }
    COREAS_API static size_t convert(const u16symbol* src, size_t srcLen, u32symbol* dest);
};

template<>
struct utf_convert_selector<u16symbol, u16symbol> {
    // При конвертации char16_t в wchar_t под windows будет вызываться эта реализация
    static size_t maxSpace(size_t len) { return len; }
    static size_t convert(const u16symbol* src, size_t srcLen, u16symbol* dest) {
        std::char_traits<u16symbol>::copy(dest, src, srcLen + 1);
        return srcLen;
    }
};

template<>
struct utf_convert_selector<u32symbol, u32symbol> {
    // При конвертации char32_t в wchar_t под windows будет вызываться эта реализация
    static size_t maxSpace(size_t len) { return len; }
    static size_t convert(const u32symbol* src, size_t srcLen, u32symbol* dest) {
        std::char_traits<u32symbol>::copy(dest, src, srcLen + 1);
        return srcLen;
    }
};

template<>
struct utf_convert_selector<u16symbol, wchar_t> {
    static size_t maxSpace(size_t len)
    {
        return utf_convert_selector<u16symbol, wchar_type>::maxSpace(len);
    }
    static size_t convert(const u16symbol* src, size_t srcLen, wchar_t* dest)
    {
        return utf_convert_selector<u16symbol, wchar_type>::convert(
            src, srcLen, to_w(dest));
    }
};

template<>
struct utf_convert_selector<u32symbol, u8symbol> {
    // Максимально один code_point utf32 может преобразоваться в 4 code_unit utf8,
    static size_t maxSpace(size_t len) { return len * 4; }
    COREAS_API static size_t convert(const u32symbol* src, size_t srcLen, u8symbol* dest);
};

template<>
struct utf_convert_selector<u32symbol, u16symbol> {
    // Максимально один code_point utf32 может преобразоваться в 2 code_unit utf16,
    static size_t maxSpace(size_t len) { return len * 2; }
    COREAS_API static size_t convert(const u32symbol* src, size_t srcLen, u16symbol* dest);
};

template<>
struct utf_convert_selector<u32symbol, wchar_t> {
    static size_t maxSpace(size_t len)
    {
        return utf_convert_selector<u32symbol, wchar_type>::maxSpace(len);
    }
    static size_t convert(const u32symbol* src, size_t srcLen, wchar_t* dest)
    {
        return utf_convert_selector<u32symbol, wchar_type>::convert(
            src, srcLen, to_w(dest));
    }
};

template<>
struct utf_convert_selector<wchar_t, u8symbol> {
    static size_t maxSpace(size_t len)
    {
        return utf_convert_selector<wchar_type, u8symbol>::maxSpace(len);
    }
    static size_t convert(const wchar_t* src, size_t srcLen, u8symbol* dest)
    {
        return utf_convert_selector<wchar_type, u8symbol>::convert(
            to_w(src), srcLen, dest);
    }
};

template<>
struct utf_convert_selector<wchar_t, u16symbol> {
    static size_t maxSpace(size_t len)
    {
        return utf_convert_selector<wchar_type, u16symbol>::maxSpace(len);
    }
    static size_t convert(const wchar_t* src, size_t srcLen, u16symbol* dest)
    {
        return utf_convert_selector<wchar_type, u16symbol>::convert(
            to_w(src), srcLen, dest);
    }
};

template<>
struct utf_convert_selector<wchar_t, u32symbol> {
    static size_t maxSpace(size_t len)
    {
        return utf_convert_selector<wchar_type, u32symbol>::maxSpace(len);
    }
    static size_t convert(const wchar_t* src, size_t srcLen, u32symbol* dest)
    {
        return utf_convert_selector<wchar_type, u32symbol>::convert(
            to_w(src), srcLen, dest);
    }
};


template<typename K, typename Impl>
class from_utf_convertable {
protected:
    from_utf_convertable() = default;
    using my_type = Impl;
    /*
     Эти методы должен реализовать класс-наследник.
     вызывается только при создании объекта
       init(size_t size)
       setSize(size_t size)
    */
public:
    template<typename O> requires (!std::is_same_v<O, K>)
    from_utf_convertable(SimpleStr<O> init) {
        using worker = utf_convert_selector<O, K>;
        Impl* d = static_cast<Impl*>(this);
        size_t len = init.length();
        if (!len)
            d->createEmpty();
        else {
            size_t maxSpace = worker::maxSpace(len);
            if (maxSpace <= 64) {
                K buf[64];
                maxSpace = worker::convert(init.symbols(), len, buf);
                K* ptr = d->init(maxSpace);
                std::char_traits<K>::copy(ptr, buf, maxSpace);
                ptr[maxSpace] = 0;
            } else
                d->setSize(worker::convert(init.symbols(), len, d->init(maxSpace)));
        }
    }
    template<typename O, typename I> requires (!std::is_same_v<O, K>)
    from_utf_convertable(const str_algs<O, SimpleStr<O>, I>& init) : from_utf_convertable(init.toStr()) {}
};

/*
* База для объектов, владеющих строкой
* По прежнему ничего не знает о том, где наследник хранит строку и её размер
* Просто вызывает его методы для получения места, и заполняет его при необходимости.
* Работает только при создании объекта, не работает с модификацией строки после
* ее создания и гарантирует, что если вызываются эти методы, объект еще только
* создается, и какого-либо расшаривания данных еще не было.
* Эти методы должен реализовать класс-наследник, вызываются только при создании объекта
*   K* init(size_t size)     - выделить место для строки указанного размера, вернуть адрес
*   void createEmpty()     - создать пустой объект
*   K* setSize(size_t size)  - перевыделить место для строки, если при создании не угадали
*                            нужный размер и место нужно больше или меньше.
*                            Содержимое строки нужно оставить.
* 
* K     - тип символов
* Impl  - тип наследника
*/
template<typename K, typename Impl>
class str_storeable {
public:
    using my_type = Impl;
    using traits = std::char_traits<K>;
private:
    using uni = unicode_traits<K>;

    Impl& d() noexcept {
        return *static_cast<Impl*>(this);
    }
    const Impl& d() const noexcept {
        return *static_cast<const Impl*>(this);
    }
    explicit constexpr str_storeable(size_t size) {
        if (size)
            d().init(size);
        else
            d().createEmpty();
    }


    template<StrType<K> From, typename Op1>
    static my_type changeCaseAscii(const From& f, const Op1& opMakeNeedCase) {
        my_type result;
        size_t len = f.length();
        if (len) {
            const K* source = f.symbols();
            K* destination = result.init(len);
            for (size_t l = 0; l < len; l++) {
                destination[l] = opMakeNeedCase(source[l]);
            }
        }
        return result;
    }

    template<typename T>
    struct ChangeCase {
        template<typename From, typename Op1>
        static my_type changeCase(const From& f, const Op1& opChangeCase) {
            my_type result;
            size_t len = f.length();
            if (len) {
                opChangeCase(f.symbols(), len, result.init(len));
            }
            return result;
        }
    };
    // Для utf8 сделаем отдельную спецификацию, так как при смене регистра может изменится длина строки
    template<>
    struct ChangeCase<u8symbol> {
        template<typename From, typename Op1>
        static my_type changeCase(const From& f, const Op1& opChangeCase) {
            my_type result;
            size_t len = f.length();
            if (len) {
                const K* ptr = f.symbols();
                K* pWrite =result.init(len);

                const u8symbol* source = ptr;
                u8symbol* dest = pWrite;
                size_t newLen = opChangeCase(source, len, dest, len);
                if (newLen < len) {
                    // Строка просто укоротилась
                    result.setSize(newLen);
                } else if (newLen > len) {
                    // Строка не влезла в буфер.
                    size_t readed = static_cast<size_t>(source - ptr);
                    size_t writed = static_cast<size_t>(dest - pWrite);
                    pWrite = result.setSize(newLen);
                    dest = pWrite + writed;
                    opChangeCase(source, len - readed, dest, newLen - writed);
                }
                pWrite[newLen] = 0;
            }
            return result;
        }
    };

public:

    using SimpleStr = SimpleStr<K>;
    using SimpleStrNt = SimpleStrNt<K>;
    
    constexpr str_storeable() noexcept {
        d().createEmpty();
    }
    
    // Конструктор из другого строкового объекта
    constexpr str_storeable(SimpleStr other) {
        if (other.length()) {
            K* ptr = d().init(other.length());
            traits::copy(ptr, other.symbols(), other.length());
            ptr[other.length()] = 0;
        } else
            d().createEmpty();
    }
    // Конструктор повторения
    constexpr str_storeable(size_t repeat, SimpleStr pattern) {
        size_t l = pattern.length(), allLen = l * repeat;
        if (allLen) {
            K* ptr = d().init(allLen);
            for (size_t i = 0; i < repeat; i++) {
                traits::copy(ptr, pattern.symbols(), l);
                ptr += l;
            }
            *ptr = 0;
        } else
            d().createEmpty();
    }

    str_storeable(size_t count, K pad) {
        if (count) {
            K* str = d().init(count);
            traits::assign(str, count, pad);
            str[count] = 0;
        } else
            d().createEmpty();
    }

    // Конструктор из строкового литерала
    template<size_t N>
    constexpr str_storeable(const K(&value)[N]) {
        if constexpr (N > 1) {
            K* ptr = d().init(N - 1);
            traits::copy(ptr, value, N - 1);
            ptr[N - 1] = 0;
        } else
            d().createEmpty();
    }

    // Конструктор из строкового выражения
    template<StrExprForType<K> A>
    constexpr str_storeable(const A& expr) {
        size_t len = expr.length();
        if (len)
            *expr.place(d().init(len)) = 0;
        else
            d().createEmpty();
    }

    // Конструктор из строкового источника с заменой
    template<StrType<K> From>
    str_storeable(const From& f, SimpleStr pattern, SimpleStr repl, size_t offset = 0, size_t maxCount = 0) {
        auto findes = f.findAll(pattern, offset, maxCount);
        if (!findes.size()) {
            new (this) my_type{ f };
            return;
        }
        size_t srcLen = f.length();
        size_t newSize = srcLen + static_cast<int>((repl.len - pattern.len) * findes.size());
        
        if (!newSize) {
            new (this) my_type{};
            return;
        }
        
        K* ptr = d().init(newSize);
        const K* src = f.symbols();
        size_t from = 0;
        for (const auto& s: findes) {
            size_t copyLen = s - from;
            if (copyLen) {
                traits::copy(ptr, src + from, copyLen);
                ptr += copyLen;
            }
            if (repl.len) {
                traits::copy(ptr, repl.str, repl.len);
                ptr += repl.len;
            }
            from = s + pattern.len;
        }
        srcLen -= from;
        if (srcLen) {
            traits::copy(ptr, src + from, srcLen);
            ptr += srcLen;
        }
        *ptr = 0;
    }

    SimpleStrNt toNts(size_t from = 0) const {
        size_t len = d().length();
        SimpleStrNt result;
        if (from >= len) {
            result = SimpleStrNt::empty;
        } else {
            result.str = d().symbols() + from;
            result.len = len - from;
        }
        return result;
    }

    operator SimpleStrNt() {
        return toNts();
    }

    // Слияние контейнера строк
    template<typename T>
    static my_type join(const T& strings, SimpleStr delimeter, bool tail = false) {
        my_type result;
        if (strings.size()) {
            if (strings.size() == 1 && (!delimeter.length() || !tail)) {
                result = strings.front();
            } else {
                size_t commonLen = 0;
                for (auto it = strings.begin(), e = strings.end(); it != e;) {
                    commonLen += it->length();
                    ++it;
                    if (it != e || tail)
                        commonLen += delimeter.length();
                }
                if (commonLen) {
                    K* ptr = result.init(commonLen);
                    for (auto it = strings.begin(), e = strings.end(); it != e; ) {
                        size_t copyLen = it->length();
                        if (copyLen) {
                            traits::copy(ptr, it->symbols(), copyLen);
                            ptr += copyLen;
                        }
                        ++it;
                        if (delimeter.length() && (it != e || tail)) {
                            traits::copy(ptr, delimeter.symbols(), delimeter.length());
                            ptr += delimeter.length();
                        }
                    }
                    *ptr = 0;
                }
            }
        }
        return result;
    }
    // ascii версия upper
    template<StrType<K> From>
    static my_type upperedOnlyAsciiFrom(const From& f) {
        return changeCaseAscii(f, makeAsciiUpper<K>);
    }

    // ascii версия lower
    template<StrType<K> From>
    static my_type loweredOnlyAsciiFrom(const From& f) {
        return changeCaseAscii(f, makeAsciiLower<K>);
    }

    // Юникодная версия
    template<StrType<K> From>
    static my_type upperedFrom(const From& f) {
        return ChangeCase<K>::changeCase(f, uni::upper);
    }

    // Юникодная версия
    template<StrType<K> From>
    static my_type loweredFrom(const From& f) {
        return ChangeCase<K>::changeCase(f, uni::lower);
    }

    template<StrType<K> From>
    static my_type replacedFrom(const From& f, SimpleStr pattern, SimpleStr repl, size_t offset = 0, size_t maxCount = 0) {
        return my_type{f, pattern, repl, offset, maxCount};
    }
};

template<typename K> class sstring;

template<typename K> struct printf_selector {};
template<>
struct printf_selector<u8symbol> {
    template<typename ...T>
    static int snprintf(u8symbol* buffer, size_t count, const u8symbol* format, T&& ... args) {
        return std::snprintf(buffer, count, format, std::forward<T>(args)...);
    }
    static int vsnprintf(u8symbol* buffer, size_t count, const u8symbol* format, va_list args) {
        return std::vsnprintf(buffer, count, format, args);
    }
};

template<>
struct printf_selector<wchar_t> {
    template<typename ...T>
    static int snprintf(wchar_t* buffer, size_t count, const wchar_t* format, T&& ... args) {
        return std::swprintf(buffer, count, format, args...);
    }
    static int vsnprintf(wchar_t* buffer, size_t count, const wchar_t* format, va_list args) {
        return std::vswprintf(buffer, count, format, args);
    }
};

inline static size_t grow2(size_t ret, size_t current) {
    return ret < current ? ret : ret * 2;
}

/*
* Базовый класс работы с меняющимися inplace строками
* По прежнему ничего не знает о том, где наследник хранит строку и её размер
* Просто вызывает его методы для получения места, и заполняет его при необходимости.
* Для работы класс-наследник должен реализовать методы:
*   size_t length() const noexcept    - возвращает длину строки
*   const K* symbols() const        - возвращает указатель на начало строки
*   bool isEmpty() const noexcept   - проверка, не пустая ли строка
*   K* str() noexcept               - Неконстантный указатель на начало строки
*   K* setSize(size_t size)           - Изменить размер строки, как больше, так и меньше.
*                                     Содержимое строки нужно оставить.
*   K* reserve(size_t size)           - выделить место под строку, старую можно не сохранять
*   size_t capacity() const noexcept  - вернуть текущую ёмкость строки, сколько может поместится
*                                     без алокации
* 
* K      - тип символов
* StrRef - тип хранилища куска строки
* Impl   - тип наследника
*/
template<typename K, typename StrRef, typename Impl>
class str_mutable {
public:
    using my_type = Impl;
private:
    Impl& d() {
        return *static_cast<Impl*>(this);
    }
    const Impl& d() const {
        return *static_cast<const Impl*>(this);
    }
    size_t _len() const noexcept {
        return d().length();
    }
    const K* _str() const noexcept {
        return d().symbols();
    }
    using SimpleStr = StrRef;
    using symb_type = K;
    using traits = std::char_traits<K>;
    using uni = unicode_traits<K>;
    using uns_type = std::make_unsigned_t<K>;

    template<typename Op>
    Impl& makeTrimOp(const Op& op) {
        SimpleStr me = static_cast<SimpleStr>(d()), pos = op(me);
        if (me.length() != pos.length()) {
            if (me.symbols() != pos.symbols())
                traits::move(const_cast<K*>(me.symbols()), pos.symbols(), pos.length());
            d().setSize(pos.length());
        }
        return d();
    }

    template<typename Op>
    Impl& commonChangeCase(const Op& opConvert) {
        size_t len = _len();
        if (len)
            opConvert(_str(), len, str());
        return d();
    }
    template<typename T>
    Impl& upperTraits() {
        return commonChangeCase(unicode_traits<K>::upper);
    }
    template<typename T>
    Impl& lowerTraits() {
        return commonChangeCase(unicode_traits<K>::lower);
    }

    template<typename Op>
    Impl& utf8CaseChange(const Op& op) {
        // Для utf-8 такая операция может изменить длину строки, поэтому для них делаем разные специализации
        size_t len = _len();
        if (len) {
            u8symbol* writePos = str();
            const u8symbol* startData = writePos, * readPos = writePos;
            size_t newLen = op(readPos, len, writePos, len);
            if (newLen < len) {
                // Строка просто укоротилась
                d().setSize(newLen);
            } else if (newLen > len) {
                // Строка не влезла в буфер.
                size_t readed = static_cast<size_t>(readPos - startData);
                size_t writed = static_cast<size_t>(writePos - startData);
                d().setSize(newLen);
                startData = str(); // при изменении размера могло изменится
                readPos = startData + readed;
                writePos = const_cast<u8symbol*>(startData) + writed;
                op(readPos, len - readed, writePos, newLen - writed);
            }
        }
        return d();
    }
    template<>
    Impl& upperTraits<u8symbol>() {
        return utf8CaseChange(&unicode_traits<u8symbol>::upper);
    }
    template<>
    Impl& lowerTraits<u8symbol>() {
        return utf8CaseChange(&unicode_traits<u8symbol>::lower);
    }

    template<TrimSides S, bool withSpaces, size_t N>
    Impl& makeTrim(const K(&pattern)[N]) {
        return makeTrimOp(trimOperator<S, K, N - 1, withSpaces> {pattern});
    }

    template<TrimSides S, bool withSpaces>
    Impl& makeTrim(SimpleStr pattern) {
        return makeTrimOp(trimOperator<S, K, 0, withSpaces> { {pattern} });
    }

public:

    K* str() noexcept {
        return d().str();
    }
    operator K* () noexcept {
        return str();
    }

    Impl& trim() {
        return makeTrimOp(SimpleTrim<TrimSides::trimAll, K>{});
    }
    Impl& trimLeft() {
        return makeTrimOp(SimpleTrim<TrimSides::trimLeft, K>{});
    }
    Impl& trimRight() {
        return makeTrimOp(SimpleTrim<TrimSides::trimRight, K>{});
    }

    template<size_t N> requires is_const_pattern<N>
    Impl& trim(const K(&pattern)[N]) {
        return makeTrim<TrimSides::trimAll, false>(pattern);
    }

    template<size_t N> requires is_const_pattern<N>
    Impl& trimLeft(const K(&pattern)[N]) {
        return makeTrim<TrimSides::trimLeft, false>(pattern);
    }

    template<size_t N> requires is_const_pattern<N>
    Impl& trimRight(const K(&pattern)[N]) {
        return makeTrim<TrimSides::trimRight, false>(pattern);
    }

    template<size_t N> requires is_const_pattern<N>
    Impl& trimWithSpaces(const K(&pattern)[N]) {
        return makeTrim<TrimSides::trimAll, true>(pattern);
    }

    template<size_t N> requires is_const_pattern<N>
    Impl& trimLeftWithSpaces(const K(&pattern)[N]) {
        return makeTrim<TrimSides::trimLeft, true>(pattern);
    }

    template<size_t N> requires is_const_pattern<N>
    Impl& trimRightWithSpaces(const K(&pattern)[N]) {
        return makeTrim<TrimSides::trimRight, true>(pattern);
    }

    Impl& trim(SimpleStr pattern) {
        return pattern.length() ? makeTrim<TrimSides::trimAll, false>(pattern) : d();
    }
    Impl& trimLeft(SimpleStr pattern) {
        return pattern.length() ? makeTrim<TrimSides::trimLeft, false>(pattern) : d();
    }
    Impl& trimRight(SimpleStr pattern) {
        return pattern.length() ? makeTrim<TrimSides::trimRight, false>(pattern) : d();
    }
    Impl& trimWithSpaces(SimpleStr pattern) {
        return makeTrim<TrimSides::trimAll, true>(pattern);
    }
    Impl& trimLeftWithSpaces(SimpleStr pattern) {
        return makeTrim<TrimSides::trimLeft, true>(pattern);
    }
    Impl& trimRightWithSpaces(SimpleStr pattern) {
        return makeTrim<TrimSides::trimRight, true>(pattern);
    }

    Impl& upperOnlyAscii() {
        K *ptr = str();
        for (size_t i = 0, l = _len(); i < l; i++, ptr++) {
            K s = *ptr;
            if (isAsciiLower(s))
                *ptr = s & ~0x20;
        }
        return d();
    }
    Impl& lowerOnlyAscii() {
        K* ptr = str();
        for (size_t i = 0, l = _len(); i < l; i++, ptr++) {
            K s = *ptr;
            if (isAsciiUpper(s))
                *ptr = s | 0x20;
        }
        return d();
    }
    
    Impl& upper() {
        // Для utf-8 такая операция может изменить длину строки, поэтому для них делаем разные специализации
        return upperTraits<K>();
    }
    Impl& lower() {
        // Для utf-8 такая операция может изменить длину строки, поэтому для них делаем разные специализации
        return lowerTraits<K>();
    }

private:

    template<typename T>
    Impl& changeImpl(size_t from, size_t len, T expr) {
        size_t myLen = _len();
        if (from > myLen) {
            from = myLen;
        }
        if (from + len > myLen) {
            len = myLen - from;
        }
        K* buffer = str();
        size_t otherLen = expr.length();
        if (len == otherLen) {
            expr.place(buffer + from);
        } else {
            size_t tailLen = myLen - from - len;
            if (len > otherLen) {
                expr.place(buffer + from);
                traits::move(buffer + from + otherLen, buffer + from + len, tailLen);
                d().setSize(myLen - (len - otherLen));
            } else {
                buffer = d().setSize(myLen + otherLen - len);
                traits::move(buffer + from + otherLen, buffer + from + len, tailLen);
                expr.place(buffer + from);
            }
        }
        return d();
    }

    template<typename T>
    Impl& appendImpl(T expr) {
        if (size_t len = expr.length(); len) {
            size_t size = _len();
            expr.place(d().setSize(size + len) + size);
        }
        return d();
    }

    template<typename T>
    Impl& appendFromImpl(size_t pos, T expr) {
        if (pos > _len())
            pos = _len();
        if (size_t len = expr.length(); len)
            expr.place(d().setSize(pos + len) + pos);
        else
            d().setSize(pos);
        return d();
    }

public:

    Impl& append(SimpleStr other) {
        return appendImpl<SimpleStr>(other);
    }

    template<StrExprForType<K> A>
    Impl& append(const A& expr) {
        return appendImpl<const A&>(expr);
    }

    Impl& operator += (SimpleStr other) {
        return appendImpl<SimpleStr>(other);
    }

    template<StrExprForType<K> A>
    Impl& operator += (const A& expr) {
        return appendImpl<const A&>(expr);
    }

    Impl& appendIn(size_t pos, SimpleStr other) {
        return appendFromImpl<SimpleStr>(pos, other);
    }

    template<StrExprForType<K> A>
    Impl& appendIn(size_t pos, const A& expr) {
        return appendFromImpl<const A&>(pos, expr);
    }

    Impl& change(size_t from, size_t len, SimpleStr other) {
        return changeImpl<SimpleStr>(from, len, other);
    }

    template<StrExprForType<K> A>
    Impl& change(size_t from, size_t len, const A& expr) {
        return changeImpl<const A&>(from, len, expr);
    }

    Impl& insert(size_t to, SimpleStr other) {
        return changeImpl<SimpleStr>(to, 0, other);
    }
    template<StrExprForType<K> A>
    Impl& insert(size_t to, const A& expr) {
        return changeImpl<const A&>(to, 0, expr);
    }

    Impl& remove(size_t from, size_t to) {
        return changeImpl<const empty_expr<K>&>(from, to, {});
    }

    Impl& prepend(SimpleStr other) {
        return changeImpl<SimpleStr>(0, 0, other);
    }
    template<StrExprForType<K> A>
    Impl& prepend(const A& expr) {
        return changeImpl<const A&>(0, 0, expr);
    }

    Impl& replace(SimpleStr pattern, SimpleStr repl, size_t offset = 0, size_t maxCount = 0) {
        if (d().isEmpty() || !pattern || offset + pattern.length() > _len())
            return d();
        if (pattern.length() >= repl.length()) {
            // Заменяем на такой же или более короткий кусок, длина текста уменьшится, идём слева направо
            K* ptr = str();
            size_t posWrite = offset;
            if (!maxCount)
                maxCount--;
            for (size_t i = 0; i < maxCount; i++) {
                size_t idx = d().find(pattern, offset);
                if (idx == str_pos::badIdx)
                    break;
                size_t lenOfPiece = idx - offset;
                if (posWrite < offset && lenOfPiece)
                    traits::move(ptr + posWrite, ptr + offset, lenOfPiece);
                posWrite += lenOfPiece;
                if (repl.length()) {
                    traits::copy(ptr + posWrite, repl.symbols(), repl.length());
                    posWrite += repl.length();
                }
                offset = idx + pattern.length();
            }
            size_t tailLen = _len() - offset;
            if (posWrite < offset && tailLen)
                traits::move(ptr + posWrite, ptr + offset, tailLen);
            d().setSize(posWrite + tailLen);
        } else {
            // Заменяем на более длинный кусок, длина текста увеличится, идём справа налево
            auto finded = d().findAll(pattern, offset, maxCount);
            if (finded.size()) {
                size_t delta = repl.length() - pattern.length();
                size_t allDelta = size_t(delta * finded.size());
                size_t endOfPiece = _len();
                K* ptr = d().setSize(endOfPiece + allDelta);
                for (size_t i = size_t(finded.size()); i--;) {
                    size_t pos = finded[i] + pattern.length();
                    size_t lenOfPiece = endOfPiece - pos;
                    traits::move(ptr + pos + allDelta, ptr + pos, lenOfPiece);
                    traits::copy(ptr + pos + allDelta - repl.length(), repl.symbols(), repl.length());
                    allDelta -= delta;
                    endOfPiece = finded[i];
                }
            }
        }
        return d();
    }

    template<StrType<K> From>
    Impl& replaceFrom(const From& f, SimpleStr pattern, SimpleStr repl, size_t offset = 0, size_t maxCount = 0) {
        d().~my_type();
        new (&d()) my_type{f, pattern, repl, offset, maxCount};
        return d();
    }
    
    // Реализация заполнения данными с проверкой на длину и перевыделением буфера в случае недостаточной длины.
    template<typename Op>
    my_type& funcFill(size_t from, const Op& fillFunction) {
        size_t size = _len();
        if (from > size)
            from = size;
        size_t capacity = d().capacity();
        K* ptr = str();
        capacity -= from;
        for (;;) {
            size_t needSize = (size_t)fillFunction(ptr + from, capacity);
            if (capacity >= needSize) {
                d().setSize(from + needSize);
                break;
            }
            ptr = from == 0 ? d().reserve(needSize) : d().setSize(from + needSize);
            capacity = needSize;
        }
        return d();
    }
    // Реализация заполнения данными с проверкой на длину и перевыделением буфера в случае недостаточной длины.
    template<typename Op> requires std::is_invocable_v<Op, K*, size_t>
    my_type& operator << (const Op& fillFunction) {
        return funcFill(0, fillFunction);
    }
    // Реализация добавления данных с проверкой на длину и перевыделением буфера в случае недостаточной длины.
    template<typename Op> requires std::is_invocable_v<Op, K*, size_t>
    my_type& operator <<= (const Op& fillFunction) {
        return funcFill(_len(), fillFunction);
    }
    template<typename Op> requires std::is_invocable_v<Op, my_type&>
    my_type& operator << (const Op& fillFunction) {
        fillFunction(d());
        return d();
    }
    template<typename... T>
    my_type& printfFrom(size_t from, const K* format, T&& ... args) {
        size_t size = _len();
        if (from > size)
            from = size;
        size_t capacity = d().capacity();
        K* ptr = str();
        capacity -= from;

        int result;
        // Тут грязный хак для u8symbol и wide_char. u8symbol версия snprintf сразу возвращает размер нужного буфера, если он мал
        // а swprintf - возвращает -1. Поэтому для широкой версии надо тупо увеличивать буфер наугад, пока не подойдет
        if constexpr (sizeof(K) == 1) {
            result = printf_selector<K>::snprintf(ptr + from, capacity + 1, format, std::forward<T>(args)...);
            if (result > (int)capacity) {
                ptr = from == 0 ? d().reserve(result) : d().setSize(from + result);
                result = printf_selector<K>::snprintf(ptr + from, result + 1, format, std::forward<T>(args)...);
            }
        } else {
            for (size_t i = 0; ; i++) {
                result = printf_selector<K>::snprintf(ptr + from, capacity + 1, format, std::forward<T>(args)...);
                if (result < 0 && i < 4) {
                    // Не хватило буфера или ошибка конвертации.
                    // Попробуем увеличить буфер в два раза, но не более чем всего в 16 раз
                    capacity *= 2;
                    ptr = from == 0 ? d().reserve(capacity) : d().setSize(from + capacity);
                } else
                    break;
            }
        }
        if (result < 0)
            d().setSize(static_cast<size_t>(traits::length(_str())));
        else
            d().setSize(from + result);
        return d();
    }
    template<typename... T>
    my_type& printf(const K* pattern, T&& ... args) {
        return printfFrom(0, pattern, std::forward<T>(args)...);
    }
    template<typename... T>
    my_type& appendPrintf(const K* pattern, T&& ... args) {
        return printfFrom(_len(), pattern, std::forward<T>(args)...);
    }
    template<typename... T>
    my_type& formatFrom(size_t from, const FmtString<K, T...>& pattern, T&& ... args) {
        size_t size = _len();
        if (from > size)
            from = size;
        size_t capacity = d().capacity();
        K* ptr = str();
        capacity -= from;

        auto result = std::format_to_n(ptr + from, capacity, pattern, std::forward<T>(args)...);
        if (result.size > (int)capacity) {
            ptr = from == 0 ? d().reserve((size_t)result.size) : d().setSize(from + (size_t)result.size);
            result = std::format_to_n(ptr + from, result.size, pattern, std::forward<T>(args)...);
        }
        d().setSize(from + (size_t)result.size);
        return d();
    }
    template<typename... T>
    my_type& format(const FmtString<K, T...>& pattern, T&& ... args) {
        return formatFrom(0, pattern, std::forward<T>(args)...);
    }
    template<typename... T>
    my_type& appendFormated(const FmtString<K, T...>& pattern, T&& ... args) {
        return formatFrom(_len(), pattern, std::forward<T>(args)...);
    }

    template<typename Op, typename ...Args>
    my_type& with(const Op& fillFunction, Args&& ... args) {
        fillFunction(d(), std::forward<Args>(args)...);
        return d();
    }
};

template<typename K>
struct SharedStringData {
    std::atomic_size_t ref;	// Счетчик ссылок
    //size_t ref;

    SharedStringData() {
        //ref.store(1, std::memory_order_release);
        ref = 1;
    }
    K* str() const { return (K*)(this + 1); }
    void incr() {
        ref++;
        //ref.fetch_add(1, std::memory_order_acq_rel);
    }
    void decr() {
        /*size_t old = ref.fetch_sub(1, std::memory_order_acq_rel);
        if (old == 1)
            core_as_free(this);
            */
        if (!--ref) {
            core_as_free(this);
        }
    }
    static SharedStringData<K>* create(size_t l) {
        return new (core_as_malloc(sizeof(SharedStringData<K>) + (l + 1) * sizeof(K))) SharedStringData;
    }
    static SharedStringData<K>* from_str(const K* p) {
        return (SharedStringData<K>*)p - 1;
    }
    K* place(K* p, size_t len) {
        std::char_traits<K>::copy(p, str(), len);
        return p + len;
    }
};

/*
* Локальная строка. Хранит в себе длину строки, а за ней либо сами данные до N символов + нуль,
* либо если данные длиннее N, то размер выделенного буфера и указатель на данные.
* При этом, если планируется потом результат переместить в sstring, то для динамического буфера
* выделяется +n байтов, чтобы потом не двигать данные.
* Так как у класса несколько базовых классов, ms компилятор не применяет автоматом empty base optimization,
* и без явного указания - вставит в начало класса пустые байты, сдвинув поле size на 4 байта.
* Укажем ему явно
*/
template<typename K, size_t N, bool forShared = false>
class empty_bases lstring :
    public str_algs<K, SimpleStr<K>, lstring<K, N, forShared>>,
    public str_mutable<K, SimpleStr<K>, lstring<K, N, forShared>>,
    public str_storeable<K, lstring<K, N, forShared>>,
    public from_utf_convertable<K, lstring<K, N, forShared>>
{
public:
    using symb_type = K;
    using my_type = lstring<K, N, forShared>;

protected:

    enum : size_t {
        extra = forShared ? sizeof(SharedStringData<K>) : 0,
        LocalCount = std::max(N, (sizeof(size_t) + sizeof(K*)) / sizeof(K) - 1)
    };

    using base_algs = str_algs<K, SimpleStr<K>, my_type>;
    using base_store = str_storeable<K, my_type>;
    using base_mutable = str_mutable<K, SimpleStr<K>, my_type>;
    using base_utf = from_utf_convertable<K, my_type>;
    using traits = std::char_traits<K>;

    friend base_store;
    friend base_mutable;
    friend base_utf;
    friend class sstring<K>;

    // Данные
    union {
        struct {
            size_t size;
            size_t bufSize;
            K* data;
        };
        struct {
            size_t lsize;
            K local[N + 1];
        };
    };
    
    void createEmpty() {
        size = 0;
        bufSize = 0;
    }
    K* init(size_t s) {
        size = s;
        if (size > N) {
            data = allocPlace(s);
            bufSize = s;
        }
        return str();
    }
    // Методы для себя
    bool isBig() const noexcept {
        return size > N;
    }
    void dealloc() {
        if (isBig())
            core_as_free(toRealAddress(data));
    }

    static K* toRealAddress(void* ptr) {
        return reinterpret_cast<K*>(reinterpret_cast<u8symbol*>(ptr) - extra);
    }
    static K* fromRealAddress(void* ptr) {
        return reinterpret_cast<K*>(reinterpret_cast<u8symbol*>(ptr) + extra);
    }

    static K* allocPlace(size_t newSize) {
        return fromRealAddress(core_as_malloc((newSize + 1) * sizeof(K) + extra));
    }

public:
    using base_store::base_store;
    using base_utf::base_utf;

    //constexpr static size_t forSize(size_t I) { return I - 1 - offsetof(my_type, local); }
    
    lstring() = default;

    ~lstring() {
        dealloc();
    }
    // Копирование из другой строки с таким же размером буфера
    lstring(const my_type& other) {
        if (other.size)
            traits::copy(reserve(other.size), other.symbols(), other.size + 1);
    }

    // Перемещение из другой строки с таким же размером буфера
    lstring(my_type&& other) noexcept {
        if (other.size) {
            size = other.size;
            if (size > N) {
                data = other.data;
                bufSize = other.bufSize;
            } else {
                traits::copy(local, other.symbols(), size);
                local[size] = 0;
            }
            other.size = 0;
            other.local[0] = 0;
        }
    }
    
    lstring(const auto& op) requires requires { op(std::declval<K*>(), 0u); } || requires {op(std::declval<my_type&>());} {
        this->operator<<(op);
    }

    my_type& operator = (const my_type& other) {
        if (&other != this) {
            operator = (other.toStr());
        }
        return *this;
    }

    my_type& operator = (my_type&& other) {
        if (&other != this) {
            this->~lstring(); // Освобождаем хранимое. Боятся потерять не надо, исключений не будет
            new (this) my_type(std::move(other));
        }
        return *this;
    }

    my_type& operator = (SimpleStr<K> other) {
        if (!other.len) {
            setSize(0);
        } else {
            K* myText = str();
            bool isIntersect = !(other.str > myText + size || other.str + other.len < myText);
            if (isIntersect) {
                // Особый случай, нам пытаются присвоить кусок нашей же строки, поэтому освобождать
                // содержимое до копирования нельзя. Просто переместим текст в буфере, и установим
                // новый размер
                if (other.str > myText) {
                    traits::move(myText, other.str, other.len);
                }
                setSize(other.len);
            } else {
                K* place = other.len <= N ? local : allocPlace(other.len);
                dealloc();
                traits::copy(place, other.str, other.len);
                if (other.len > N) {
                    data = place;
                    bufSize = other.len;
                }
                size = other.len;
                str()[size] = 0;
            }
        }
        return *this;
    }

    template<size_t S>
    my_type& operator = (const K(&other)[S]) {
        return operator=(SimpleStr<K>{other});
    }

    // Если в строковом выражении что-либо ссылается на части этой же строки, и результат
    // помещается в локальный буфер строки, это UB.
    template<StrExprForType<K> A>
    my_type& operator = (const A& expr) {
        size_t newLen = expr.length();
        if (!newLen) {
            setSize(0);
        } else {
            K* place = newLen <= N ? local : allocPlace(newLen);
            dealloc();
            *expr.place(place) = 0;
            if (newLen > N) {
                data = place;
                bufSize = newLen;
            }
            size = newLen;
        }
        return *this;
    }

    size_t length() const noexcept {
        return size;
    }
    const K* symbols() const noexcept {
        return isBig() ? data : local;
    }
    K* str() noexcept {
        return isBig() ? data : local;
    }
    bool isEmpty() const noexcept {
        return size == 0;
    }

    size_t capacity() const noexcept {
        return isBig() ? bufSize : N;
    }

    // Выделить буфер, достаточный для размещения newSize символов плюс завершающий ноль
    // Содержимое буфера неопределено, и не гарантируется сохранение старого содержимого
    K* reserve(size_t newSize) {
        if (newSize != size) {
            if (isBig()) {
                // У нас есть динамический буфер
                if (newSize <= N) // и он теперь не нужен
                    dealloc();
                else if (newSize > bufSize) {
                    // он недостаточен
                    data = fromRealAddress(core_as_realloc(toRealAddress(data), (newSize + 1) * sizeof(K) + extra));
                    bufSize = newSize;
                }
            } else {
                // У нас локальный буфер
                if (newSize > N) {
                    // А нужен динамический
                    data = allocPlace(newSize);
                    bufSize = newSize;
                }
            }
            size = newSize;
        }
        return str();
    }

    // Устанавливает размер текущей строки. При необходмости перемещает данные в другой буфер
    // Содержимое сохраняется
    K* setSize(size_t newSize) {
        if (newSize != size) {
            if (isBig()) {
                // У нас есть динамический буфер
                if (newSize <= N) {
                    // А нужен локальный. Скопируем нужное количество данных и освободим память
                    K* dynBuffer = data; // Копирование перезатрет данные
                    if (newSize)
                        traits::copy(local, dynBuffer, newSize);
                    core_as_free(toRealAddress(dynBuffer));
                } else if (newSize > bufSize) {
                    // динамический буфер недостаточный по длине. Расширим его. При этом для строго динамических строк с запасом в полтора раза
                    data = fromRealAddress(core_as_realloc(toRealAddress(data), (newSize + 1 + (N == 0 ? newSize / 2 : 0)) * sizeof(K) + extra));
                    bufSize = newSize + (N == 0 ? newSize / 2 : 0);
                }
            } else {
                // У нас локальный буфер
                if (newSize > N) {
                    // А нужен динамический
                    K* dynBuffer = allocPlace(newSize);
                    if (size)
                        traits::copy(dynBuffer, local, size + 1);
                    data = dynBuffer;
                    bufSize = newSize;
                }
            }
            size = newSize;
        }
        K* res = str();
        res[newSize] = 0;
        return res;
    }
    void defineSize() {
        K* start = str();
        for (size_t i = 0, ie = capacity(); i < ie; i++) {
            if (start[i] == 0) {
                size = i;
                return;
            }
        }
        size = capacity();
        start[size] = 0;
    }
};

template<size_t N> using lstringa = lstring<u8symbol, N>;
template<size_t N> using lstringw = lstring<wchar_t, N>;
template<size_t N> using lstringu = lstring<u16symbol, N>;
template<size_t N> using lstringuu = lstring<u32symbol, N>;
template<size_t N> using lstringsa = lstring<u8symbol, N, true>;
template<size_t N> using lstringsw = lstring<wchar_t, N, true>;
template<size_t N> using lstringsu = lstring<u16symbol, N, true>;
template<size_t N> using lstringsuu = lstring<u32symbol, N, true>;

template<typename K, size_t N> K getLiteralType(const K(&)[N]) { return {}; };

template<size_t Arch, size_t L>
inline constexpr const size_t _local_count = 0;

template<>
inline constexpr const size_t _local_count<8, 1> = 23;
template<>
inline constexpr const size_t _local_count<8, 2> = 15;
template<>
inline constexpr const size_t _local_count<8, 4> = 7;
template<>
inline constexpr const size_t _local_count<4, 1> = 15;
template<>
inline constexpr const size_t _local_count<4, 2> = 11;
template<>
inline constexpr const size_t _local_count<4, 4> = 5;

template<typename T>
constexpr const size_t local_count = _local_count<sizeof(size_t), sizeof(T)>;

/*
* Класс с small string optimization плюс разделяемый иммутабельный буфер строки.
* Так как буфер строки в этом классе иммутабельный, то:
* Во-первых, нет нужды хранить размер выделенного буфера, мы его всё-равно не будем изменять
* Во-вторых, появляется ещё один тип строки - строка, инициализированная константым литералом.
* Для неё просто сохраняем указатель на символы, и не считаем ссылки.
* Таким образом, инициализация строкового объекта в программе литералом - ничего никуда не копирует -
* ни в себя, ни в динамическую память, и не стоит дороже по сравнению с инициализацией
* сырого указателя на строку, и даже ещё оптимальнее, так как ещё и сразу подставляет размер,
* а не вычисляет его в рантайме.
*
*     stringa text = "text or very very very long text"; // ничего не стоит!
*     stringa copy = anotherString; // Стоит только копирование байтов самого объекта плюс возможно один атомарный инкремент
*
* В случае разделяемого буфера размер строки всё-равно храним не в общем буфере, а в каждом объекте
* из-за SSO места всё-равно хватает, а в память лезть за длиной придётся меньше.
* Например, подсчитать сумму длин строк в векторе - пройдётся только по памяти в векторе.
* 
* Размеры:
* для u8symbol  - 24 байта, хранит строки до 23 символов + 0
* для u16symbol - 32 байта, хранит строки до 15 символов + 0
* для u32symbol - 32 байта, хранит строки до 7 символов + 0
*/

template<typename K>
class empty_bases sstring :
    public str_algs<K, SimpleStr<K>, sstring<K>>,
    public str_storeable<K, sstring<K>>,
    public from_utf_convertable<K, sstring<K>> {
public:
    using symb_type = K;
    using uns_type = std::make_unsigned_t<K>;
    using my_type = sstring<K>;
protected:
    using base_algs = str_algs<K, SimpleStr<K>, my_type>;
    using base_store = str_storeable<K, my_type>;
    using base_utf = from_utf_convertable<K, my_type>;

    friend base_store;
    friend base_utf;

    enum { LocalCount = local_count<K> };
    enum Types { Local, Constant, Shared };

    union {
        // Когда у нас короткая строка, она лежит в самом объекте, а в localRemain
        // пишется, сколько символов ещё можно вписать. Когда строка занимает всё
        // возможное место, то localRemain становится 0, type в этом случае тоже 0,
        // и в итоге после символов строки получается 0, как и надо!
        struct {
            K buf[LocalCount];        // Локальный буфер строки
            uns_type localRemain : sizeof(uns_type) * 8 - 2;
            uns_type type : 2;
        };
        struct {
            union {
                const K* cstr; // Указатель на конcтантную строку
                const K* sstr; // Указатель на строку, перед которой лежит SharedStringData
            };
            size_t bigLen;   // Длина не локальной строки.
        };
    };

    void createEmpty() {
        type = Local;
        localRemain = LocalCount;
        buf[0] = 0;
    }
    K* init(size_t s) {
        if (s > LocalCount) {
            type = Shared;
            localRemain = 0;
            bigLen = s;
            sstr = SharedStringData<K>::create(s)->str();
            return (K*)sstr;
        } else {
            type = Local;
            localRemain = LocalCount - s;
            return buf;
        }
    }

    K* setSize(size_t newSize) {
        // вызывается при создании строки при необходимости изменить размер
        // других ссылок на shared bufer нет
        size_t size = length();
        if (newSize != size) {
            if (type == Constant) {
                bigLen = newSize;
            } else {
                if (newSize <= LocalCount) {
                    if (type == Shared) {
                        SharedStringData<K>* str_buf = SharedStringData<K>::from_str(sstr);
                        traits::copy(buf, sstr, newSize);
                        str_buf->decr();
                    }
                    type = Local;
                    localRemain = LocalCount - newSize;
                } else {
                    if (type == Shared) {
                        if (newSize > size || (newSize > 32 && newSize < size * 3 / 4)) // строка сильно изменилась
                            sstr = reinterpret_cast<SharedStringData<K>*>(core_as_realloc(SharedStringData<K>::from_str(sstr), (newSize + 1) * sizeof(K) + sizeof(SharedStringData<K>)))->str();
                    } else if (type == Local) {
                        K* dynBuffer = SharedStringData<K>::create(newSize)->str();
                        if (size)
                            traits::copy(dynBuffer, buf, size);
                        sstr = dynBuffer;
                        type = Shared;
                        localRemain = 0;
                    }
                    bigLen = newSize;
                }
            }
        }
        K* str = type == Local ? buf : (K*)sstr;
        str[newSize] = 0;
        return str;
    }
    using traits = std::char_traits<K>;
    using uni = unicode_traits<K>;

public:
    using base_store::base_store;
    using base_utf::base_utf;

    sstring() = default;

    ~sstring() {
        if (type == Shared)
            SharedStringData<K>::from_str(sstr)->decr();
    }

    sstring(const my_type& other) noexcept {
        memcpy(this, &other, sizeof(other));
        if (type == Shared)
            SharedStringData<K>::from_str(sstr)->incr();
    }

    sstring(my_type&& other) noexcept {
        memcpy(this, &other, sizeof(other));
        other.createEmpty();
    }

    // Конструктор перемещения из локальной строки
    template<size_t N>
    sstring(lstring<K, N, true>&& src) {
        size_t size = src.length();
        if (size) {
            if (size > N) {
                // Там динамический буфер, выделенный с запасом для SharedStringData.
                K* str = src.str();
                if (size > LocalCount) {
                    // Просто присвоим его себе.
                    sstr = str;
                    bigLen = size;
                    type = Shared;
                    localRemain = 0;
                    new (SharedStringData<K>::from_str(str)) SharedStringData<K>;
                } else {
                    // Скопируем локально
                    type = Local;
                    localRemain = LocalCount - size;
                    traits::copy(buf, str, size + 1);
                    // Освободим тот буфер, у локальной строки буфер не делится с другими
                    core_as_free(SharedStringData<K>::from_str(str));
                }
            } else {
                // Копируем из локального буфера
                K* str = init(src.size);
                traits::copy(str, src.symbols(), size);
                str[size] = 0;
            }
            src.size = 0;
            src.local[0] = 0;
        } else
            createEmpty();
    }
    // Инициализация из строкового литерала
    template<size_t N>
    sstring(const K(&s)[N]) {
        if constexpr (N == 1) {
            createEmpty();
        } else {
            type = Constant;
            localRemain = 0;
            cstr = s;
            bigLen = N - 1;
        }
    }

    my_type& operator=(const my_type& other) noexcept {
        if (&other != this) {
            // так как конструктор копирования не выкидывает исключений, можем просто смело просто пересоздать себя
            this->~sstring();
            new (this) my_type(other);
        }
        return *this;
    }

    my_type& operator=(my_type&& other) noexcept {
        if (&other != this) {
            this->~sstring();
            new (this) my_type(std::move(other));
        }
        return *this;
    }
 
    my_type& operator=(SimpleStr<K> other) {
        return operator=(my_type{other});
    }

    template<size_t N>
    my_type& operator=(const K(&other)[N]) {
        this->~sstring();
        new (this) my_type(other);
        return *this;
    }

    template<size_t N, bool S>
    my_type& operator=(const lstring<K, N, S>& other) {
        return operator=(my_type{other.toStr()});
    }

    template<size_t N>
    my_type& operator=(lstring<K, N, true>&& other) {
        return operator=(my_type{std::move(other)});
    }

    my_type& makeEmpty() {
        this->~sstring();
        createEmpty();
        return *this;
    }
    operator const K* () const noexcept {
        return symbols();
    }
    const K* symbols() const noexcept {
        return type == Local ? buf : cstr;
    }

    size_t length() const noexcept {
        return type == Local ? LocalCount - localRemain : bigLen;
    }

    bool isEmpty() const noexcept {
        return length() == 0;
    }
    // Форматирование строки.
    template<typename... T>
    static my_type printf(const K* pattern, T&& ... args) {
        return my_type{ lstring<K, 128, true>{}.printf(pattern, std::forward<T>(args)...) };
    }

    template<typename ...T>
    static my_type format(const FmtString<K, T...>& fmtString, T&& ... args) {
        return my_type{ lstring<K, 128, true>{}.format(fmtString, std::forward<T>(args)...) };
    }
};

template<size_t I>
struct digits_selector {
    using wider_type = uint16_t;
};

template<>
struct digits_selector<2> {
    using wider_type = uint32_t;
};

template<>
struct digits_selector<4> {
    using wider_type = uint64_t;
};

template<typename K, typename T>
constexpr size_t fromInt(K* bufEnd, T val) {
    if (val) {
        need_sign<K, T(-1) < 0, T> sign(val);
        K* itr = bufEnd;
        while (val) {
            *--itr = static_cast<K>('0' + val % 10);
            val /= 10;
        }
        sign.after(itr);
        return size_t(bufEnd - itr);
    }
    bufEnd[-1] = '0';
    return 1;
}

template<typename K, typename T>
struct expr_num {
    using symb_type = K;
    using my_type = expr_num<K, T>;
    
    enum {bufSize = 24 };
    mutable T value;
    mutable K buf[bufSize];

    expr_num(T t) : value(t) {}
    expr_num(expr_num<K, T>&& t) : value(t.value) {}
    
    size_t length() const noexcept {
        value = (T)fromInt(buf + bufSize, value);
        return (size_t)value;
    }
    K* place(K* ptr) const noexcept {
        std::char_traits<K>::copy(ptr, buf + bufSize - (size_t)value, (size_t)value);
        return ptr + (size_t)value;
    }
};

template<StrExpr A, FromIntNumber T>
constexpr static auto operator & (const A& a, T s) {
    return strexprjoin_c<A, expr_num<typename A::symb_type, T>>{ a, s };
}

template<typename K, typename T>
constexpr static auto e_num(T t) {
    return expr_num<K, T>{t};
}

template<typename K>
SimpleStrNt<K> select_str(SimpleStrNt<u8symbol> s8, SimpleStrNt<uwsymbol> sw) {
    if constexpr (sizeof(K) == 1)
        return s8;
    else
        return sw;
}

#define uni_string(K, p) select_str<K>(p, L##p)

template<typename K>
struct expr_real {
    using symb_type = K;
    mutable K buf[40];
    mutable size_t l;
    double v;
    expr_real(double d) : v(d) {}
    expr_real(float d) : v(d) {}

    size_t length() const noexcept {
        printf_selector<K>::snprintf(buf, 40, uni_string(K, "%.16g"), v);
        l = (size_t)std::char_traits<K>::length(buf);
        return l;
    }
    K* place(K* ptr) const noexcept {
        std::char_traits<K>::copy(ptr, buf, l);
        return ptr + l;
    }
};

template<StrExpr A>
constexpr static auto operator & (const A& a, double s) {
    return strexprjoin_c<A, expr_real<typename A::symb_type>>{ a, s };
}

template<StrExpr A>
constexpr static auto operator & (const A& a, float s) {
    return strexprjoin_c<A, expr_real<typename A::symb_type>>{ a, s };
}

template<typename K>
constexpr static auto e_real(double t) {
    return expr_real<K>{t};
}

/*
* Для создания строковых конкатенаций с векторами и списками, сджойненными константным разделителем
* K - тип символов строки
* T - тип контейнера строк (vector, list)
* I - длина разделителя в символах
* tail - добавлять разделитель после последнего элемента контейнера.
*        Если контейнер пустой, разделитель в любом случае не добавляется
*/

template<typename K, typename T, size_t I, bool tail>
struct expr_lst {
    using symb_type = K;
    using my_type = expr_lst<K, T, I, tail>;
    
    const T& s;
    const K* delim;
    
    constexpr size_t length() const noexcept {
        size_t l = 0;
        for (const auto& t : s)
            l += t.length() + I;
        return l - (l != 0 && !tail ? I : 0);
    }
    constexpr K* place(K* ptr) const noexcept {
        if (!s.empty()) {
            for (auto t = s.begin(), e = s.end(); ;) {
                size_t copyLen = t->length();
                if (copyLen) {
                    std::char_traits<K>::copy(ptr, t->symbols(), copyLen);
                    ptr += copyLen;
                }
                ++t;
                if (t == e) {
                    // Последний элемент контейнера
                    if constexpr (I > 0 && tail) {
                        std::char_traits<K>::copy(ptr, delim, I);
                        ptr += I;
                    }
                    break;
                }
                if constexpr (I > 0) {
                    std::char_traits<K>::copy(ptr, delim, I);
                    ptr += I;
                }
            }
        }
        return ptr;
    }
};

template<bool t = false, size_t I, typename K, typename T>
constexpr static auto e_ls(const T& s, const K(&d)[I]) {
    return expr_lst<K, T, I - 1, t>{ s, d };
}

template<typename K, size_t N, size_t L>
struct expr_replaces {
    using symb_type = K;
    using my_type = expr_replaces<K, N, L>;
    SimpleStr<K> what;
    const K* pattern;
    const K* repl;
    mutable std::vector<size_t> positions;

    constexpr expr_replaces(SimpleStr<K> w, const K* p, const K* r) : what(w), pattern(p), repl(r) {}

    constexpr size_t length() const {
        positions = what.findAll(SimpleStr<K>{pattern, N});
        return what.length() + static_cast<int>(L - N) * static_cast<size_t>(positions.size());
    }
    constexpr K* place(K* ptr) const noexcept {
        size_t from = 0;
        for (size_t k : positions) {
            size_t copyLen = k - from;
            if (copyLen) {
                std::char_traits<K>::copy(ptr, what.symbols() + from, copyLen);
                ptr += copyLen;
            }
            if constexpr (L > 0) {
                std::char_traits<K>::copy(ptr, repl, L);
                ptr += L;
            }
            if constexpr (N > 0)
                from = k + N;
        }
        size_t tailLen = what.length() - from;
        if (tailLen) {
            std::char_traits<K>::copy(ptr, what.symbols() + from, tailLen);
            ptr += tailLen;
        }
        return ptr;
    }
};

template<typename K, size_t N, size_t L> requires (N > 1)
constexpr static auto e_repl(SimpleStr<K> w, const K(&p)[N], const K(&r)[L]) {
    return expr_replaces<K, N - 1, L - 1>{w, p, r};
}

template<StrExpr A, StrExprForType<typename A::symb_type> B>
struct expr_choice {
    using symb_type = typename A::symb_type;
    using my_type = expr_choice<A, B>;
    const A& a;
    const B& b;
    bool choice;

    constexpr size_t length() const noexcept {
        return choice ? a.length() : b.length();
    }
    constexpr symb_type* place(symb_type* ptr) const noexcept {
        return choice ? a.place(ptr) : b.place(ptr);
    }
};

template<StrExpr A, StrExprForType<typename A::symb_type> B>
constexpr static auto e_choice(bool c, const A& a, const B& b) {
    return expr_choice<A, B> {a, b, c};
}

template<typename K>
struct StoreType {
    SimpleStr<K> str;
    size_t hash;
    char node[sizeof(sstring<K>)];

    const SimpleStrNt<K>& to_nt() const noexcept {
        return static_cast<const SimpleStrNt<K>&>(str);
    }
    const sstring<K>& to_str() const noexcept {
        return *reinterpret_cast<const sstring<K>*>(node);
    }
};

using HashKeyA = StoreType<u8symbol>;
using HashKeyW = StoreType<u16symbol>;
using HashKeyU = StoreType<u32symbol>;

template<bool Wide>
struct fnv_const {
    static inline constexpr size_t basis = static_cast<size_t>(14695981039346656037ULL);
    static inline constexpr size_t prime = static_cast<size_t>(1099511628211ULL);
};

template<>
struct fnv_const<false> {
    static inline constexpr size_t basis = static_cast<size_t>(2166136261U);
    static inline constexpr size_t prime = static_cast<size_t>(16777619U);
};

using fnv = fnv_const<sizeof(size_t) == 8>;

inline constexpr size_t maxLenForHash = 16;

template<typename K>
constexpr static size_t fnv_hash(const K* ptr, size_t l) {
    size_t h = fnv::basis;
    for (size_t i = 0; i < std::min(l, maxLenForHash); i++)
        h = (h ^ ptr[i]) * fnv::prime;
    return h;
};

template<typename K>
constexpr static size_t fnv_hash_ia(const K* ptr, size_t l) {
    size_t h = fnv::basis;
    for (size_t i = 0; i < std::min(l, maxLenForHash); i++) {
        K s = ptr[i];
        h = (h ^ (s >= 'A' && s <= 'Z' ? s | 0x20 : s)) * fnv::prime;
    }
    return h;
};

template<typename K, size_t N>
constexpr static size_t fnv_hash(const K(&value)[N]) {
    size_t h = fnv::basis;
    for (size_t i = 0; i < std::min(N - 1, maxLenForHash); i++)
        h = (h ^ value[i]) * fnv::prime;
    return h;
};

template<typename K, size_t N>
constexpr static size_t fnv_hash_ia(const K(&value)[N]) {
    size_t h = fnv::basis;
    for (size_t i = 0; i < std::min(N - 1, maxLenForHash); i++) {
        K s = value[i];
        h = (h ^ (s >= 'A' && s <= 'Z' ? s | 0x20 : s)) * fnv::prime;
    }
    return h;
};


template<typename K>
consteval static size_t fnv_hash_compile(const K* ptr, size_t l) {
    return fnv_hash(ptr, l);
};

template<typename K>
consteval static size_t fnv_hash_ia_compile(const K* ptr, size_t l) {
    return fnv_hash_ia(ptr, l);
};

/*
* Контейнер для более эффективного поиска по строковым ключам.
* Как unordered_map, но чуть лучше. В качестве ключей хранит SimpleStr вместе с посчитанным хешем.
* Чтобы SimpleStr было на что ссылатся, строковые значения ключей кладёт в список,
* с ключом запоминает позицию в списке. При удалении ключа удаляет и из списка.
* Позволяет использовать для поиска строковые литералы, не создавая для них объекта sstring.
* Начиная с С++20 в unordered_map появилась возможность для гетерогенного поиска по ключу с типом,
* отличным от типа хранящегося ключа. Однако удаление по прежнему только по типу ключа,
* что сводит на нет улучшения.
* Да и хэш тоже не хранит, каждый раз вычисляя заново.
*/
template<typename K, typename T, typename H, typename E>
class hashStrMap : public std::unordered_map<StoreType<K>, T, H, E> {
protected:
    using InStore = StoreType<K>;
public:
    using my_type = hashStrMap<K, T, H, E>;
    using hash_t = std::unordered_map<InStore, T, H, E>;
    using hasher = H;

    ~hashStrMap() {
        for (auto& k : *this)
            ((sstring<K>*)k.first.node)->~sstring();
    }

    hashStrMap() = default;
    hashStrMap(const my_type&) = default;
    hashStrMap(my_type&& o) = default;
    my_type& operator = (const my_type&) = default;
    my_type& operator = (my_type&&) = default;

    hashStrMap(std::initializer_list<std::pair<const StoreType<K>, T>> init) {
        for (const auto& e : init)
            emplace(e.first, e.second);
    }

    hashStrMap(std::initializer_list<std::pair<const sstring<K>, T>> init) {
        for (const auto& e : init)
            emplace(e.first, e.second);
    }

    // При входе хэш должен быть уже посчитан
    template<typename...ValArgs>
    auto emplace(const StoreType<K>& key, ValArgs&&... args) {
        auto it = hash_t::try_emplace(key, std::forward<ValArgs>(args)...);
        if (it.second) {
            InStore& stored = const_cast<InStore&>(it.first->first);
            new (stored.node) sstring<K>(key.str);
            stored.str.str = stored.to_str().symbols();
        }
        return it;
    }

    template<typename...ValArgs>
    auto emplace(SimpleStr<K> key, ValArgs&&... args) {
        return emplace(StoreType<K>{ key, H{}(key), {} }, std::forward<ValArgs>(args)...);
    }

    template<typename...ValArgs>
    auto emplace(const sstring<K>& key, ValArgs&&... args) {
        auto it = hash_t::try_emplace(StoreType<K>{ key.toStr(), H{}(key), {}}, std::forward<ValArgs>(args)...);
        if (it.second) {
            InStore& stored = const_cast<InStore&>(it.first->first);
            new (stored.node) sstring<K>(key);
            stored.str.str = stored.to_str().symbols();
        }
        return it;
    }

    template<typename...ValArgs>
    auto emplace_or_assign(const StoreType<K>& key, ValArgs&&... args) {
        auto it = emplace(key, std::forward<ValArgs>(args)...);
        if (!it.second) {
            it.first->second = T{std::forward<ValArgs>(args)...};
        }
        return it;
    }

    template<typename...ValArgs>
    auto emplace_or_assign(SimpleStr<K> key, ValArgs&&... args) {
        return emplace_or_assign(StoreType<K>{ key, H{}(key), {} }, std::forward<ValArgs>(args)...);
    }
    
    template<typename...ValArgs>
    auto emplace_or_assign(const sstring<K>& key, ValArgs&&... args) {
        auto it = emplace(key, std::forward<ValArgs>(args)...);
        if (!it.second) {
            it.first->second = T{std::forward<ValArgs>(args)...};
        }
        return it;
    }
    
    auto find(const StoreType<K>& key) const {
        return hash_t::find(key);
    }
    
    auto find(SimpleStr<K> key) const {
        return find(StoreType<K>{ key, H{}(key), {} });
    }
    
    auto find(const StoreType<K>& key) {
        return hash_t::find(key);
    }
    
    auto find(SimpleStr<K> key) {
        return find(StoreType<K>{ key, H{}(key), {} });
    }
    
    auto erase(typename hash_t::const_iterator it) {
        if (it != hash_t::end()) {
            ((sstring<K>*)it->first.node)->~sstring();
        }
        return hash_t::erase(it);
    }

    auto erase(const StoreType<K>& key) {
        auto it = hash_t::find(key);
        if (it != hash_t::end()) {
            ((sstring<K>*)it->first.node)->~sstring();
            hash_t::erase(it);
            return 1;
        }
        return 0;
    }
    
    auto erase(SimpleStr<K> key) {
        return erase(StoreType<K> { key, H{}(key) });
    }

    bool lookup(const K* txt, T& val) const {
        auto it = find(e_s(txt));
        if (it != hash_t::end()) {
            val = it->second;
            return true;
        }
        return false;
    }

    bool lookup(SimpleStr<K> txt, T& val) const {
        auto it = find(txt);
        if (it != hash_t::end()) {
            val = it->second;
            return true;
        }
        return false;
    }

    void clear() {
        for (auto& k : *this)
            ((sstring<K>*)k.first.node)->~sstring();
        hash_t::clear();
    }
};

template<typename K>
struct streql {
    bool operator()(const StoreType<K>& _Left, const StoreType<K>& _Right) const {
        return _Left.hash == _Right.hash && _Left.str == _Right.str;
    }
};

template<typename K>
struct strhash {	// hash functor for basic_string
    size_t operator()(SimpleStr<K> _Keyval) const {
        return fnv_hash(_Keyval.symbols(), _Keyval.length());
    }
    size_t operator()(const StoreType<K>& _Keyval) const {
        return _Keyval.hash;
    }
};

template<typename K>
struct streqlia {
    bool operator()(const StoreType<K>& _Left, const StoreType<K>& _Right) const {
        return _Left.hash == _Right.hash && _Left.str.isEqual_ia(_Right.str);
    }
};

template<typename K>
struct strhashia {
    size_t operator()(SimpleStr<K> _Keyval) const {
        return fnv_hash_ia(_Keyval.symbols(), _Keyval.length());
    }
    size_t operator()(const StoreType<K>& _Keyval) const {
        return _Keyval.hash;
    }
};

template<typename K>
struct streqliu {
    bool operator()(const StoreType<K>& _Left, const StoreType<K>& _Right) const {
        return _Left.hash == _Right.hash && _Left.str.isEqual_iu(_Right.str);
    }
};

template<typename K>
struct strhashiu {
    size_t operator()(SimpleStr<K> _Keyval) const {
        return unicode_traits<K>::hashiu(_Keyval.symbols(), _Keyval.length());
    }
    size_t operator()(const StoreType<K>& _Keyval) const {
        return _Keyval.hash;
    }
};

/*
* Для построения длинных динамических строк конкатенацией мелких кусочков.
* Выделяет по мере надобности отдельные блоки заданного размера (или кратного ему для больших вставок),
* чтобы избежать релокации длинных строк.
* После построения можно слить в одну строку
*/
template<typename K>
class chunked_string_concatenator {
    std::vector<std::pair<std::unique_ptr<K>, size_t>> chunks;// блоки и длина данных в них
    K* write{ nullptr }; // Текущая позиция записи
    size_t len{ 0 };// Общая длина
    size_t remain{ 0 }; // Сколько осталось места в текущем блоке
    size_t align{ 1024 };
public:
    using my_type = chunked_string_concatenator<K>;
    using symb_type = K;
    chunked_string_concatenator() = default;
    chunked_string_concatenator(size_t a) : align(a) {};
    chunked_string_concatenator(const my_type&) = delete;
    chunked_string_concatenator(my_type&& other) :
        chunks(std::move(other.chunks)), write(other.write), len(other.len), remain(other.remain), align(other.align)  {
        other.len = other.remain = 0;
        other.write = nullptr;
    }
    my_type& operator = (my_type other) noexcept {
        this->~my_type();
        new (this) my_type(std::move(other));
        return *this;
    }
    // Добавление порции данных
    my_type& operator << (SimpleStr<K> data) {
        if (data.len) {
            len += data.len;
            if (data.len <= remain) {
                // Добавляемые данные влезают в выделенный блок, просто скопируем их
                std::char_traits<K>::copy(write, data.str, data.len);
                write += data.len;// Сдвинем позицию  записи
                chunks.back().second += data.len;// Увеличим длину хранимых в блоке данных
                remain -= data.len;// Уменьшим остаток места в блоке
            } else {
                // Не влезают
                if (remain) {
                    // Сначала запишем сколько влезет
                    std::char_traits<K>::copy(write, data.str, remain);
                    data.len -= remain;
                    data.str += remain;
                    chunks.back().second += remain; // Увеличим длину хранимых в блоке данных
                }
                // Выделим новый блок и впишем в него данные
                size_t blockSize = (data.len + align - 1) / align * align;  // Рассчитаем размер блока, кратного заданному выравниванию
                chunks.emplace_back(new K[blockSize], data.len);
                write = chunks.back().first.get();
                std::char_traits<K>::copy(write, data.str, data.len);
                write += data.len;
                remain = blockSize - data.len;
            }
        }
        return *this;
    }
    template<StrExprForType<K> A>
    my_type& operator << (const A& expr) {
        size_t l = expr.length();
        if (l) {
            if (l < remain) {
                write = expr.place(write);
                chunks.back().second += l;
                len += l;
                remain -= l;
            } else {
                std::unique_ptr<K> store(new K[l]);
                expr.place(store.get());
                return operator<<({ store.get(), l });
            }
        }
        return *this;
    }
    constexpr size_t length() const noexcept { return len; }
    constexpr K* place(K* p) const noexcept {
        for (const auto& block: chunks) {
            std::char_traits<K>::copy(p, block.first.get(), block.second);
            p += block.second;
        }
        return p;
    }

    template<typename Op>
    void out(const Op& o) const {
        for (const auto& block: chunks)
            o(block.first.get(), block.second);
    }

    bool isContinuous() const {
        if (chunks.size()) {
            const char* ptr = chunks.front().first.get();
            for (const auto& chunk : chunks) {
                if (chunk.first.get() != ptr)
                    return false;
                ptr += chunk.second;
            }
        }
        return true;
    }
    const K* begin() const {
        return chunks.size() ? chunks.front().first.get() : nullptr;
    }

    void clear() {
        size_t a = align;
        this->~chunked_string_concatenator<K>();
        new(this)chunked_string_concatenator<K>(a);
    }
    struct portionStore {
        typename decltype(chunks)::const_iterator it, end;
        size_t writedFromCurrentChunk;

        bool isEnd() {
            return it == end;
        }
        size_t store(K* buffer, size_t size) {
            size_t writed = 0;
            while (size && !isEnd()) {
                size_t remain = it->second - writedFromCurrentChunk;
                size_t write = std::min(size, remain);
                std::char_traits<K>::copy(buffer, it->first.get() + writedFromCurrentChunk, write);
                writed += write;
                remain -= write;
                size -= write;
                if (!remain) {
                    ++it;
                    writedFromCurrentChunk = 0;
                } else
                    writedFromCurrentChunk += write;
            }
            return writed;
        }
    };
    portionStore getPortion() const { return { chunks.begin(), chunks.end(), 0 }; }
};

inline static char hexDigit(int t) { return static_cast<char>(t < 10 ? '0' + t : 'a' + t - 10); }

using stringa = sstring<u8symbol>;
using stringw = sstring<wchar_t>;
using stringu = sstring<u16symbol>;
using stringuu = sstring<u32symbol>;

template<typename T>
using hashStrMapA = hashStrMap<u8symbol, T, strhash<u8symbol>, streql<u8symbol>>;
template<typename T>
using hashStrMapAIA = hashStrMap<u8symbol, T, strhashia<u8symbol>, streqlia<u8symbol>>;
template<typename T>
using hashStrMapAIU = hashStrMap<u8symbol, T, strhashiu<u8symbol>, streqliu<u8symbol>>;

template<typename T>
using hashStrMapW = hashStrMap<wchar_t, T, strhash<wchar_t>, streql<wchar_t>>;
template<typename T>
using hashStrMapWIA = hashStrMap<wchar_t, T, strhashia<wchar_t>, streqlia<wchar_t>>;
template<typename T>
using hashStrMapWIU = hashStrMap<wchar_t, T, strhashiu<wchar_t>, streqliu<wchar_t>>;

template<typename T>
using hashStrMapU = hashStrMap<u16symbol, T, strhash<u16symbol>, streql<u16symbol>>;
template<typename T>
using hashStrMapUIA = hashStrMap<u16symbol, T, strhashia<u16symbol>, streqlia<u16symbol>>;
template<typename T>
using hashStrMapUIU = hashStrMap<u16symbol, T, strhashiu<u16symbol>, streqliu<u16symbol>>;

template<typename T>
using hashStrMapUU = hashStrMap<u32symbol, T, strhash<u32symbol>, streql<u32symbol>>;
template<typename T>
using hashStrMapUUIA = hashStrMap<u32symbol, T, strhashia<u32symbol>, streqlia<u32symbol>>;
template<typename T>
using hashStrMapUUIU = hashStrMap<u32symbol, T, strhashiu<u32symbol>, streqliu<u32symbol>>;

inline constexpr SimpleStrNt<u8symbol> utf8_bom{ "\xEF\xBB\xBF", 3};

consteval StoreType<u8symbol> operator""_h(const u8symbol* ptr, size_t l) {
    return StoreType<u8symbol>{{ptr, (size_t)l}, fnv_hash_compile(ptr, (size_t)l), {}};
}

consteval StoreType<u8symbol> operator""_ia(const u8symbol* ptr, size_t l) {
    return StoreType<u8symbol>{{ptr, (size_t)l}, fnv_hash_ia_compile(ptr, (size_t)l), {}};
}

inline static StoreType<u8symbol> operator""_iu(const u8symbol* ptr, size_t l) {
    return StoreType<u8symbol>{{ptr, (size_t)l}, strhashiu<u8symbol>{}(SimpleStr<u8symbol>{ptr, (size_t)l}), {}};
}

consteval StoreType<u16symbol> operator""_h(const u16symbol* ptr, size_t l) {
    return StoreType<u16symbol>{{ptr, (size_t)l}, fnv_hash_compile(ptr, (size_t)l), {}};
}

consteval StoreType<u16symbol> operator""_ia(const u16symbol* ptr, size_t l) {
    return StoreType<u16symbol>{{ptr, (size_t)l}, fnv_hash_ia_compile(ptr, (size_t)l), {}};
}

inline static StoreType<u16symbol> operator""_iu(const u16symbol* ptr, size_t l) {
    return StoreType<u16symbol>{{ptr, (size_t)l}, strhashiu<u16symbol>{}(SimpleStr<u16symbol>{ptr, (size_t)l}), {}};
}

consteval StoreType<u32symbol> operator""_h(const u32symbol* ptr, size_t l) {
    return StoreType<u32symbol>{{ptr, (size_t)l}, fnv_hash_compile(ptr, (size_t)l), {}};
}

consteval StoreType<u32symbol> operator""_ia(const u32symbol* ptr, size_t l) {
    return StoreType<u32symbol>{{ptr, (size_t)l}, fnv_hash_ia_compile(ptr, (size_t)l), {}};
}

inline static StoreType<u32symbol> operator""_iu(const u32symbol* ptr, size_t l) {
    return StoreType<u32symbol>{{ptr, (size_t)l}, strhashiu<u32symbol>{}(SimpleStr<u32symbol>{ptr, (size_t)l}), {}};
}

} // namespace core_as::str

template<typename K>
struct std::formatter<core_as::str::SimpleStr<K>, K> : std::formatter<std::basic_string_view<K>, K> {
    // Define format() by calling the base class implementation with the wrapped value
    template<typename FormatContext>
    auto format(core_as::str::SimpleStr<K> t, FormatContext& fc) const {
        return std::formatter<std::basic_string_view<K>, K>::format({t.str, t.len}, fc);
    }
};
