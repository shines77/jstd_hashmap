
#ifndef JSTD_TYPE_TRAITS_H
#define JSTD_TYPE_TRAITS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stddef.h>     // For offsetof()

#include <cstdint>
#include <cstddef>
#include <cstdbool>
#include <cassert>

#include <string>
#include <utility>      // For std::pair<T1, T2>
#include <type_traits>

#include "jstd/basic/stddef.h"
#include "jstd/traits/has_member.h"

namespace jstd {

struct no_init_t {};

struct false_type {
    typedef false_type type;
    static const bool value = false;
};

struct true_type {
    typedef true_type type;
    static const bool value = true;
};

template <bool condition, class T, class U>
struct condition_if {
    typedef U type;
};

template <class T, class U>
struct condition_if<true, T, U> {
    typedef T type;
};

template <bool condition>
struct boolean_if {
    typedef typename condition_if<condition, true_type, false_type>::type type;
    enum { value = type::value };
};

template <typename T>
struct integral_traits {
    typedef typename std::make_signed<T>::type      signed_type;
    typedef typename std::make_unsigned<T>::type    unsigned_type;

    static_assert(std::is_integral<T>::value,
        "Error: jstd::integral_traits<T> -- T must be a integral type.");

    // Bits
    static constexpr size_t bytes = sizeof(T);
    static constexpr size_t bits = bytes * 8;
    static constexpr size_t max_shift = bits - 1;

    // 0xFFFFFFFFUL;
    static constexpr unsigned_type max_num = static_cast<unsigned_type>(-1);
    // 0x80000000UL;
    static constexpr unsigned_type max_power2 = static_cast<unsigned_type>(1) << max_shift;
};

template <typename T>
constexpr T cmax(const T & a, const T & b) {
  return ((a > b) ? a : b);
}

template <typename T>
constexpr T cmin(const T & a, const T & b) {
  return ((a < b) ? a : b);
}

// Struct void_wrapper
struct void_wrapper {
    void_wrapper() {}

    template <typename ... Args>
    void operator () (Args && ... args) const {
        return void();
    }
};

template <typename... Ts>
struct make_void {
    typedef void type;
};

// Alias template void_t
template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

//
// std::remove_xxxx<T> enhance
//
template <typename T>
struct remove_cvref {
    typedef typename std::remove_cv<
                typename std::remove_reference<T>::type
            >::type type;
};

template <typename T>
struct remove_cvp_ref {
    typedef typename std::remove_cv<
                typename std::remove_reference<
                    typename std::remove_pointer<T>::type
                >::type
            >::type type;
};

template <typename T>
struct remove_cvrp_ext {
    typedef typename std::remove_extent<
                typename std::remove_cv<
                    typename std::remove_reference<
                        typename std::remove_pointer<T>::type
                    >::type
                >::type
            >::type type;
};

template <typename T>
struct remove_all {
    typedef typename std::remove_all_extents<
                typename std::remove_cv<
                    typename std::remove_reference<
                        typename std::remove_pointer<T>::type
                    >::type
                >::type
            >::type type;
};

template <typename T1, typename T2>
struct is_same_ex : public std::is_same<typename remove_cvref<T1>::type,
                                        typename remove_cvref<T2>::type> {
};

template <typename T>
struct is_noexcept_move_constructible {
    static constexpr bool value =
        !(!std::is_nothrow_move_constructible<T>::value &&
		   std::is_copy_constructible<T>::value);
};

template <typename T>
struct is_noexcept_move_assignable {
    static constexpr bool value =
        !(!std::is_nothrow_move_assignable<T>::value &&
		   std::is_copy_assignable<T>::value);
};

//
// is_relocatable<T>
//
// Trait which can be added to user types to enable use of memcpy.
//
// Example:
//   template <>
//   struct is_relocatable<MyType> : std::true_type {};
//

template <typename T>
struct is_relocatable
    : std::integral_constant<bool,
                             (std::is_trivially_copy_constructible<T>::value &&
                              std::is_trivially_destructible<T>::value)> {};

template <typename T, typename U>
struct is_relocatable<std::pair<T, U>>
    : std::integral_constant<bool, (is_relocatable<T>::value &&
                                    is_relocatable<U>::value)> {};

template <typename T>
struct is_relocatable<const T> : is_relocatable<T> {};

template <typename Caller, typename Function, typename = void>
struct is_call_possible : public std::false_type {};

template <typename Caller, typename ReturnType, typename ... Args>
struct is_call_possible<Caller, ReturnType(Args...),
    typename std::enable_if<
        std::is_same<ReturnType, void>::value ||
        std::is_convertible<decltype(
            std::declval<Caller>().operator()(std::declval<Args>()...)
            //          ^^^^^^^^^^ replace this with the member you need.
        ), ReturnType>::value
    >::type
> : public std::true_type {};

//
// See: https://stackoverflow.com/questions/17201329/c11-ways-of-finding-if-a-type-has-member-function-or-supports-operator
//

#define CLASS_IS_SUPPORT(ClassName, Expr)                               \
template <typename T>                                                   \
struct ClassName {                                                      \
    template <typename>                                                 \
    static constexpr std::false_type check(...);                        \
                                                                        \
    template <typename U>                                               \
    static constexpr decltype((Expr), std::true_type{ }) check(void *); \
                                                                        \
    static constexpr bool value = decltype(check<T>(nullptr))::value;   \
};

namespace detail {
    CLASS_IS_SUPPORT(IsSupportBegin, std::begin(std::declval<T>()));
}

//
// See: https://stackoverflow.com/questions/18570285/using-sfinae-to-detect-a-member-function
//

//
// has_size
//
template <typename T, typename SizeType = std::size_t>
struct has_size {
    typedef SizeType size_type;

    typedef char True;
    struct False {
        char data[2];
    };

    template <typename U, U>
    struct check_has;

    template <typename U>
    static True check(check_has<size_type (U::*)() const, &U::size> *);

    // EDIT: and you can detect one of several overloads... by overloading :)
    template <typename U>
    static True check(check_has<size_type (U::*)(), &U::size> *);

    template <typename>
    static False check(...);

    static const bool value = (sizeof(check<T>(nullptr)) == sizeof(True));
};

template <typename T, typename SizeType = std::size_t>
struct has_size_cxx11 {
    typedef SizeType size_type;

    template <typename U>
    static constexpr auto check(void *)
        -> decltype(std::declval<U>().size(), std::true_type()) {
        return std::true_type();
    }

    template <typename>
    static constexpr std::false_type check(...) {
        return std::false_type();
    }

    static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

//
// has_entry_count
//

template <typename T, typename SizeType = std::size_t>
struct has_entry_count {
    typedef SizeType size_type;

    typedef char True;
    struct False {
        char data[2];
    };

    template <typename U>
    static constexpr auto check(void *)
        -> decltype(std::declval<U>().entry_count(), True{ });

    template <typename>
    static constexpr False check(...);

    static constexpr bool value = (sizeof(check<T>(nullptr)) == sizeof(True));
};

template <typename T, typename SizeType = std::size_t>
struct call_entry_count {
    typedef SizeType size_type;

    template <typename U>
    static auto entry_count_impl(const U * t, size_type * count)
        -> decltype(std::declval<U>().entry_count(), int(0)) {
        *count = t->entry_count();
        return 0;
    }

    template <typename>
    static int entry_count_impl(...) {
        return 0;
    }

    static size_type entry_count(const T & t) {
        size_type count = 0;
        entry_count_impl<T>(&t, &count);
        return count;
    }
};

//
// has_name
//

template <typename T>
struct has_name {
    typedef char True;
    struct False {
        char data[2];
    };

    template <typename U>
    static constexpr auto check(void *)
        -> decltype(std::declval<U>().name(), True{ });

    template <typename>
    static constexpr False check(...);

    static constexpr bool value = (sizeof(check<T>(nullptr)) == sizeof(True));
};

template <typename T>
struct call_name {
    template <typename U>
    static auto name_impl(const U * t, std::string * sname)
        -> decltype(std::declval<U>().name(), int(0)) {
        *sname = t->name();
        return 0;
    };

    template <typename>
    static int name_impl(...) {
        return 0;
    }

    static std::string name() {
        T t;
        std::string sname;
        name_impl<T>(&t, &sname);
        return sname;
    }
};

//
// call_static_name
//

template <typename T>
struct has_static_name {
    typedef char True;
    struct False {
        char data[2];
    };

    template <typename U>
    static constexpr auto check(void *)
        -> decltype(U::name(), True{ });

    template <typename>
    static constexpr False check(...);

    static constexpr bool value = (sizeof(check<T>(nullptr)) == sizeof(True));
};

template <typename T>
struct call_static_name {
    template <typename U>
    static auto static_name_impl(const U * t, std::string * sname)
        -> decltype(U::name(), int(0)) {
        *sname = U::name();
        return 0;
    };

    template <typename>
    static int static_name_impl(...) {
        return 0;
    }

    static std::string name() {
        T t;
        std::string sname;
        static_name_impl<T>(&t, &sname);
        return sname;
    }
};

//
// has_c_str
//

template <typename T, typename CharTy>
struct has_c_str {
    typedef char True;
    struct False {
        char data[2];
    };

    template <typename U, U>
    struct check_has;

    template <typename U>
    static True check(check_has<const CharTy * (U::*)() const, &U::c_str> *);

    template <typename>
    static False check(...);

    static const bool value = (sizeof(check<T>(nullptr)) == sizeof(True));
};

template <typename T, typename CharTy>
struct call_c_str {
    template <typename U, U>
    struct check_has;

    template <typename U>
    static const CharTy * c_str_impl(const U * s, check_has<const CharTy * (U::*)() const, &U::c_str> *) {
        return s->c_str();
    }

    template <typename U>
    static CharTy * c_str_impl(U * s, check_has<CharTy * (U::*)(), &U::c_str> *) {
        return s->c_str();
    }

    template <typename>
    static const CharTy * c_str_impl(...) {
        return nullptr;
    }

    static const CharTy * c_str(const T & s) {
        const CharTy * data = c_str_impl<T>(&s, nullptr);
        return data;
    }
};

//
// From: https://github.com/chxuan/easypack/blob/master/easypack/boost_serialization/traits_util.hpp
//

// has_begin_end
template <typename T>
struct has_begin_end {
    template <typename U>
    static constexpr auto check(void *) -> decltype(std::declval<U>().begin(), std::declval<U>().end(), std::true_type());

    template <typename U>
    static constexpr std::false_type check(...);

    static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

template <typename T>
struct has_iterator {
    template <typename U>
    static constexpr std::true_type check(typename U::iterator *);

    template <typename U>
    static constexpr std::false_type check(...);

    static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

template <typename T>
struct has_const_iterator {
    template <typename U>
    static constexpr std::true_type check(typename U::const_iterator *);

    template <typename U>
    static constexpr std::false_type check(...);

    static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

template <typename T>
struct has_mapped_type {
    template <typename U>
    static constexpr std::true_type check(typename U::mapped_type *);

    template <typename U>
    static constexpr std::false_type check(...);

    static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

template <typename T, typename ... Args>
struct has_member_swap {
    template <typename U>
    static constexpr auto check(void *)
        -> decltype(std::declval<U>().swap(std::declval<Args>()...), std::true_type()) {
        return std::true_type();
    }

    template <typename>
    static constexpr std::false_type check(...) {
        return std::false_type();
    }

    static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

//////////////////////////////////////////////////////////////////////////////////

template <typename Pair, typename = std::true_type>
struct PairOffsetOf {
    static constexpr std::size_t kFirst  = static_cast<std::size_t>(-1);
    static constexpr std::size_t kSecond = static_cast<std::size_t>(-1);
};

template <typename Pair>
struct PairOffsetOf<Pair, typename std::is_standard_layout<Pair>::type> {
    static constexpr std::size_t kFirst  = offsetof(Pair, first);
    static constexpr std::size_t kSecond = offsetof(Pair, second);
};

template <typename Key, typename Value>
struct is_compatible_kv_layout {
private:
    struct Pair {
        Key     first;
        Value   second;
    };

public:
    // Is class P layout-compatible with class Pair ?
    template <typename P>
    static constexpr bool isCompatiblePairLayout() {
        return (std::is_standard_layout<P>() &&
               (sizeof(P) == sizeof(Pair)) &&
               (alignof(P) == alignof(Pair)) &&
               (PairOffsetOf<P>::kFirst == PairOffsetOf<Pair>::kFirst) &&
               (PairOffsetOf<P>::kSecond == PairOffsetOf<Pair>::kSecond));
    }

    // Whether std::pair<const Key, Value> and std::pair<Key, Value are layout-compatible.
    // If they are, then it is safe to store them in a union and read from either.
    static constexpr bool value = std::is_standard_layout<Key>() &&
                                  std::is_standard_layout<Pair>() &&
                                  (PairOffsetOf<Pair>::kFirst == 0) &&
                                  isCompatiblePairLayout<std::pair<Key, Value>>() &&
                                  isCompatiblePairLayout<std::pair<const Key, Value>>();
};

//
// Pair        = std::pair<const Key, Value>
// MutablePair = std::pair<Key, Value>
//
template <typename ConstPair, typename MutablePair>
struct is_compatible_pair_layout {
public:
    typedef typename ConstPair::first_type      First;
    typedef typename ConstPair::second_type     Second;
    typedef typename MutablePair::first_type    MutableFirst;
    typedef typename MutablePair::second_type   MutableSecond;

private:
    struct Pair {
        MutableFirst    first;
        MutableSecond   second;
    };

    // Is class P layout-compatible with class Pair ?
    template <typename P>
    static constexpr bool isCompatiblePairLayout() {
        return (std::is_standard_layout<P>() &&
               (sizeof(P) == sizeof(Pair)) &&
               (alignof(P) == alignof(Pair)) &&
               (PairOffsetOf<P>::kFirst == PairOffsetOf<Pair>::kFirst) &&
               (PairOffsetOf<P>::kSecond == PairOffsetOf<Pair>::kSecond));
    }

public:
    // Whether std::pair<const Key, Value> and std::pair<Key, Value> are layout-compatible.
    // If they are, then it is safe to store them in a union and read from either.
    static constexpr bool value = std::is_standard_layout<MutableFirst>() &&
                                  std::is_standard_layout<Pair>() &&
                                  (PairOffsetOf<Pair>::kFirst == 0) &&
                                  isCompatiblePairLayout<ConstPair>() &&
                                  isCompatiblePairLayout<MutablePair>();
};

//////////////////////////////////////////////////////////////////////////////////

template <typename T, typename = void>
struct is_transparent : std::false_type {};

template <typename T>
struct is_transparent<T, void_t<typename T::is_transparent>>
    : std::true_type {};

template <typename K, typename key_type, bool is_transparent>
struct key_arg_selector {
    // Transparent. Forward `K`.
    typedef K type;
};

template <typename K, typename key_type>
struct key_arg_selector<K, key_type, false> {
    // Not transparent. Always use `key_type`.
    typedef key_type type;
};

template <bool is_transparent>
struct KeyArgSelector {
    // Transparent. Forward `K`.
    template <typename K, typename key_type>
    using type = K;
};

template <>
struct KeyArgSelector<false> {
    // Not transparent. Always use `key_type`.
    template <typename K, typename key_type>
    using type = key_type;
};

//////////////////////////////////////////////////////////////////////////////////

} // namespace jstd

#endif // JSTD_TYPE_TRAITS_H
