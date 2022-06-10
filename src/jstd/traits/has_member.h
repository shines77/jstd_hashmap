
#ifndef JSTD_HAS_MEMBER_H
#define JSTD_HAS_MEMBER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <cstdint>
#include <cstddef>
#include <cstdbool>

#include <type_traits>

//
// Reference: https://github.com/jackygx/EasyCPP/blob/master/Inc/Meta/HasMember.hpp
//

#define JSTD_TEST_MEMBER(func)                                                      \
    template <typename T, typename ... Args>                                        \
    struct test_member_##func {                                                     \
        typedef decltype(std::declval<T>().func(std::declval<Args>()...)) type;     \
    }

#define JSTD_HAS_MEMBER(func)                                                       \
    template <typename T, typename ... Args>                                        \
    struct has_member_##func {                                                      \
        template <typename U>                                                       \
        static constexpr auto check(void *)                                         \
        ->decltype(std::declval<U>().func(std::declval<Args>()...),                 \
                std::true_type()) {                                                 \
            return std::true_type();                                                \
        }                                                                           \
                                                                                    \
        template <typename U>                                                       \
        static constexpr std::false_type check(...) {                               \
            return std::false_type();                                               \
        }                                                                           \
                                                                                    \
        static constexpr bool value = std::is_same<decltype(check<T>(nullptr)),     \
                                                   std::true_type>::value;          \
    };                                                                              \
                                                                                    \
    template <typename T, typename ... Args>                                        \
    struct has_pointer_member_##func {                                              \
        template <typename U>                                                       \
        static constexpr auto check(void *)                                         \
        ->decltype(std::declval<U>()->func(std::declval<Args>()...),                \
                std::true_type()) {                                                 \
            return std::true_type();                                                \
        }                                                                           \
                                                                                    \
        template <typename U>                                                       \
        static constexpr std::false_type check(...) {                               \
            return std::false_type();                                               \
        }                                                                           \
                                                                                    \
        static constexpr bool value = std::is_same<decltype(check<T>(nullptr)),     \
                                                   std::true_type>::value;          \
    };                                                                              \
                                                                                    \
    template <typename ... Args>                                                    \
    struct has_template_member_##func {};                                           \
                                                                                    \
    template <typename T, typename ... Types, typename ... Args>                    \
    struct has_template_member_##func<T, types_pack<Types...>, Args...> {           \
        template <typename U>                                                       \
        static constexpr auto check(void *)                                         \
        ->decltype(std::declval<U>().template func<Types...>(std::declval<Args>()...), \
                std::true_type()) {                                                 \
            return std::true_type();                                                \
        }                                                                           \
                                                                                    \
        template <typename U>                                                       \
        static constexpr std::false_type check(...) {                               \
            return std::false_type();                                               \
        }                                                                           \
                                                                                    \
        static constexpr bool value = std::is_same<decltype(check<T>(nullptr)),     \
                                                   std::true_type>::value;          \
    };                                                                              \
                                                                                    \
    template <typename ... Args>                                                    \
    struct has_template_pointer_member_##func {};                                   \
                                                                                    \
    template <typename T, typename ... Types, typename ... Args>                    \
    struct has_template_pointer_member_##func<T, types_pack<Types...>, Args...> {   \
        template <typename U>                                                       \
        static constexpr auto check(void *)                                         \
        ->decltype(std::declval<U>()->template func<Types...>(std::declval<Args>()...), \
                std::true_type()) {                                                 \
            return std::true_type();                                                \
        }                                                                           \
                                                                                    \
        template <typename U>                                                       \
        static constexpr std::false_type check(...) {                               \
            return std::false_type();                                               \
        }                                                                           \
                                                                                    \
        static constexpr bool value = std::is_same<decltype(check<T>(nullptr)),     \
                                                   std::true_type>::value;          \
    };                                                                              \
                                                                                    \
    template <typename ... Args>                                                    \
    struct has_derived_member_##func {};                                            \
                                                                                    \
    template <typename T, typename Types, typename ... Args>                        \
    struct has_derived_member_##func<T, Types, Args...> {                           \
        template <typename U>                                                       \
        static constexpr auto check(void *)                                         \
        ->decltype(std::declval<U>().Types::func(std::declval<Args>()...),          \
                std::true_type()) {                                                 \
            return std::true_type();                                                \
        }                                                                           \
                                                                                    \
        template <typename U>                                                       \
        static constexpr std::false_type check(...) {                               \
            return std::false_type();                                               \
        }                                                                           \
                                                                                    \
        static constexpr bool value = std::is_same<decltype(check<T>(nullptr)),     \
                                                   std::true_type>::value;          \
    }

#define JSTD_HAS_STATIC_MEMBER(func)                                                \
    template <typename T, typename ... Args>                                        \
    struct has_static_member_##func {                                               \
        template <typename U>                                                       \
        static constexpr auto check(void *)                                         \
        ->decltype(U::func(std::declval<Args>()...), std::true_type()) {            \
            return std::true_type();                                                \
        }                                                                           \
                                                                                    \
        template <typename U>                                                       \
        static constexpr std::false_type check(...) {                               \
            return std::false_type();                                               \
        }                                                                           \
                                                                                    \
        static constexpr bool value = std::is_same<decltype(check<T>(nullptr)),     \
                                                   std::true_type>::value;          \
    }

#define JSTD_HAS_MEMBER_OPS(name, ops)                                              \
    template <typename T, typename ... Args>                                        \
    struct has_member_operator_##name {                                             \
        template <typename U>                                                       \
        static constexpr auto check(void *)                                         \
        ->decltype(std::declval<U>().operator ops (std::declval<Args>()...),        \
                std::true_type()) {                                                 \
            return std::true_type();                                                \
        }                                                                           \
                                                                                    \
        template <typename U>                                                       \
        static constexpr std::false_type check(...) {                               \
            return std::false_type();                                               \
        }                                                                           \
                                                                                    \
        static constexpr bool value = std::is_same<decltype(check<T>(nullptr)),     \
                                                   std::true_type>::value;          \
    }

#define JSTD_HAS_OPS(name, ops)                                                     \
    template <typename T1, typename T2 = T1>                                        \
    struct has_operator_##name {                                                    \
        template <typename U1, typename U2>                                         \
        static constexpr auto check(void *)                                         \
        ->decltype(std::declval<U1>() ops std::declval<U2>(), std::true_type()) {   \
            return std::true_type();                                                \
        }                                                                           \
                                                                                    \
        template <typename U1, typename U2>                                         \
        static constexpr std::false_type check(...) {                               \
            return std::false_type();                                               \
        }                                                                           \
                                                                                    \
        static constexpr bool value = std::is_same<decltype(check<T1, T2>(nullptr)), \
                                      std::true_type>::value;                       \
    }

namespace jstd {

template <typename ... Args>
struct types_pack {};

JSTD_HAS_MEMBER_OPS(assign, =);
JSTD_HAS_MEMBER_OPS(add, +);
JSTD_HAS_MEMBER_OPS(sub, -);
JSTD_HAS_MEMBER_OPS(mul, *);
JSTD_HAS_MEMBER_OPS(div, /);
JSTD_HAS_MEMBER_OPS(add_assign, +=);
JSTD_HAS_MEMBER_OPS(sub_assign, -=);
JSTD_HAS_MEMBER_OPS(mul_assign, *=);
JSTD_HAS_MEMBER_OPS(div_assign, /=);
JSTD_HAS_MEMBER_OPS(inc, ++);
JSTD_HAS_MEMBER_OPS(dec, --);

JSTD_HAS_MEMBER_OPS(is_equal, ==);
JSTD_HAS_MEMBER_OPS(is_not_equal, !=);
JSTD_HAS_MEMBER_OPS(bigger_than, >);
JSTD_HAS_MEMBER_OPS(smaller_than, <);
JSTD_HAS_MEMBER_OPS(bigger_or_eauql, >=);
JSTD_HAS_MEMBER_OPS(smaller_or_eauql, <=);

JSTD_HAS_MEMBER_OPS(pointer_cast, ->);
JSTD_HAS_MEMBER_OPS(square_brackets, []);

JSTD_HAS_OPS(assign, =);
JSTD_HAS_OPS(add, +);
JSTD_HAS_OPS(sub, -);
JSTD_HAS_OPS(mul, *);
JSTD_HAS_OPS(div, /);
JSTD_HAS_OPS(add_assign, +=);
JSTD_HAS_OPS(sub_assign, -=);
JSTD_HAS_OPS(mul_assign, *=);
JSTD_HAS_OPS(div_assign, /=);

JSTD_HAS_OPS(is_equal, ==);
JSTD_HAS_OPS(is_not_equal, !=);
JSTD_HAS_OPS(bigger_than, >);
JSTD_HAS_OPS(smaller_than, <);
JSTD_HAS_OPS(bigger_or_eauql, >=);
JSTD_HAS_OPS(smaller_or_eauql, <=);

template <typename T, typename ... Args>
struct has_constructor {
    template <typename U>
    static constexpr auto check(void *)
    ->decltype(U(std::declval<Args>()...), std::true_type()) {
        return std::true_type();
    }

    template <typename U>
    static constexpr std::false_type check(...) {
        return std::false_type();
    }

    static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

template <typename T, typename ... Args>
struct has_call_operator {
    template <typename U>
    static constexpr auto check(void *)
    ->decltype(std::declval<U>().operator()(std::declval<Args>()...), std::true_type()) {
        return std::true_type();
    }

    template <typename U>
    static constexpr std::false_type check(...) {
        return std::false_type();
    }

    static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

} // namespace jstd

#endif // JSTD_HAS_MEMBER_H
