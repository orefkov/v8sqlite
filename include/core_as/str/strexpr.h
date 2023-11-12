/*
* (c) Проект "Core.As", Александр Орефков orefkov@gmail.com
* База для строковых конкатенаций через выражения времени компиляции
*/
#pragma once
#include <string_view>
#include <type_traits>

namespace core_as::str {

// Выводим типы для 16 и 32 битных символов взависимости от размера wchar_t
using wchar_type = std::conditional<sizeof(wchar_t) == 2, char16_t, char32_t>::type;

inline wchar_type* to_w(wchar_t* p) {
    return (reinterpret_cast<wchar_type*>(p));
}

inline const wchar_type* to_w(const wchar_t* p) {
    return (reinterpret_cast<const wchar_type*>(p));
}

using u8s = char;
using uws = wchar_t;
using u16s = char16_t;
using u32s = char32_t;

using uu8s = std::make_unsigned<u8s>::type;

template<typename A, typename K>
concept StrType = requires(const A& a) {
    { a.isEmpty() } -> std::same_as<bool>;
    { a.length()  } -> std::convertible_to<size_t>;
    { a.symbols() } -> std::same_as<const K*>;
} && std::is_same_v<typename A::symb_type, K>;

/*
* Шаблонные классы для создания строковых выражений из нескольких источников
* Благодаря компиляторно-шаблонной "магии" позволяют максимально эффективно
* получать результирующую строку - сначала вычисляется длина результирующей строки,
* потом один раз выделяется память для результата, после символы помещаются в
* выделенную память.
*/

/* Источником строковых выражений может быть любой объект, поддерживающий эти операции :
* тип symb_type, length(), place()
*/
template<typename A>
concept StrExpr = requires(A&& a) {
    { a.length() } -> std::convertible_to<size_t>;
    { a.place(std::declval<typename A::symb_type*>()) } -> std::same_as<typename A::symb_type*>;
};

template<typename A, typename K>
concept StrExprForType = StrExpr<A> && std::is_same_v<K, typename A::symb_type>;

// Для конкатенация двух объектов строковых выражений в один
template<StrExpr A, StrExprForType<typename A::symb_type> B>
struct strexprjoin {
    using symb_type = typename A::symb_type;
    const A& a;
    const B& b;
    constexpr strexprjoin(const A& a_, const B& b_) : a(a_), b(b_){}
    constexpr size_t length() const noexcept { return a.length() + b.length(); }
    constexpr symb_type* place(symb_type* p) const noexcept { return b.place(a.place(p)); }
    constexpr symb_type* len_and_place(symb_type* p) const noexcept { a.length(); b.length(); return place(p); }
};

template<StrExpr A, StrExprForType<typename A::symb_type> B>
inline auto operator & (const A& a, const B& b) {
    return strexprjoin<A, B>{a, b};
}

// Для возможности конкатенации ссылок на строковое выражение и создаваемого временного объекта, путём его копии
template<StrExpr A, StrExprForType<typename A::symb_type> B>
struct strexprjoin_c {
    using symb_type = typename A::symb_type;
    const A& a;
    B b;
    template<typename...Args>
    constexpr strexprjoin_c(const A& a_, Args&&... args_) : a(a_), b(std::forward<Args>(args_)...) {}
    constexpr size_t length() const noexcept { return a.length() + b.length(); }
    constexpr symb_type* place(symb_type* p) const noexcept { return b.place(a.place(p)); }
    constexpr symb_type* len_and_place(symb_type* p) const noexcept { a.length(); b.length(); return place(p); }
};

template<typename T, typename K = void, typename ...Types>
struct is_one_of_type {
    static constexpr bool value = std::is_same_v<T, K> || is_one_of_type<T, Types...>::value;
};
template<typename T> struct is_one_of_type<T, void> : std::false_type {};

template<typename K>
struct empty_expr {
    using symb_type = K;
    constexpr size_t length() const noexcept { return 0; }
    constexpr symb_type* place(symb_type* p) const noexcept { return p; }
};

inline constexpr empty_expr<u8s>  eea{};
inline constexpr empty_expr<uws>  eew{};
inline constexpr empty_expr<u16s> eeu{};
inline constexpr empty_expr<u32s> eeuu{};

template<typename K>
struct expr_char {
    using symb_type = K;
    K value;
    expr_char(K v) : value(v){}
    constexpr size_t length() const noexcept { return 1; }
    constexpr symb_type* place(symb_type* p) const noexcept {
        *p++ = value;
        return p;
    }
};

template<typename K, StrExprForType<K> A>
constexpr inline auto operator & (const A& a, K s) {
    return strexprjoin_c<A, expr_char<K>>{ a, s };
}

template<typename K, size_t N>
struct expr_literal {
    using symb_type = K;
    const K(&str)[N + 1];
    constexpr size_t length() const noexcept { return N; }
    constexpr symb_type* place(symb_type* p) const noexcept {
        if constexpr (N != 0)
            std::char_traits<K>::copy(p, str, N);
        return p + N;
    }
};

template<typename K, size_t N>
constexpr inline auto e_t(const K(&s)[N]) {
    return expr_literal<K, static_cast<size_t>(N - 1)>{ s };
}

template<bool first, typename K, size_t N, typename A>
struct expr_literal_join {
    using symb_type = K;
    const K(&str)[N + 1];
    const A& a;
    constexpr size_t length() const noexcept { return N + a.length(); }
    constexpr symb_type* place(symb_type* p) const noexcept {
        if constexpr (N != 0) {
            if constexpr (first) {
                std::char_traits<K>::copy(p, str, N);
                return a.place(p + N);
            } else {
                p = a.place(p);
                std::char_traits<K>::copy(p, str, N);
                return p + N;
            }
        } else {
            return a.place(p);
        }
    }
    constexpr symb_type* len_and_place(symb_type* p) const noexcept {
        a.length(); return place(p);
    }
};

template<typename K, StrExprForType<K> A, size_t N>
constexpr inline auto operator & (const A& a, const K(&s)[N]) {
    return expr_literal_join<false, K, (N - 1), A>{ s, a };
}

template<typename K, StrExprForType<K> A, size_t N>
constexpr inline auto operator & (const K(&s)[N], const A& a) {
    return expr_literal_join<true, K, (N - 1), A>{ s, a };
}

template<typename K, size_t N, size_t S = ' '>
struct expr_spaces {
    using symb_type = K;
    constexpr size_t length() const noexcept { return N; }
    constexpr symb_type* place(symb_type* p) const noexcept {
        if constexpr (N != 0)
            std::char_traits<K>::assign(p, N, static_cast<K>(S));
        return p + N;
    }
};

template<size_t N>
constexpr inline auto e_spca() {
    return expr_spaces<u8s, N>();
}

template<size_t N>
constexpr inline auto e_spcw() {
    return expr_spaces<uws, N>();
}

template<typename K>
struct expr_pad {
    using symb_type = K;
    K s;
    size_t len;
    constexpr size_t length() const noexcept { return len; }
    constexpr symb_type* place(symb_type* p) const noexcept {
        if (len)
            std::char_traits<K>::assign(p, len, s);
        return p + len;
    }
};

template<typename K>
constexpr inline auto e_c(K s, size_t l) {
    return expr_pad<K>{ s, l };
}

}// namespace core_as::str
