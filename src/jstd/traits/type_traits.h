
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
#include <tuple>
#include <memory>       // For std::allocator<T>
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
// some type_traits enhance
//

template <bool b, typename T>
using enable_if_t = typename std::enable_if<b, T>::type;

template <bool b, typename T1, typename T2>
using conditional_t = typename std::conditional<b, T1, T2>::type;

//
// std::remove_xxxx<T> enhance
//

template <typename T>
using remove_const_t = typename std::remove_const<T>::type;

template <typename T>
using remove_volatile_t = typename std::remove_volatile<T>::type;

template <typename T>
using remove_ref_t = typename std::remove_reference<T>::type;

template <typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

template <typename T>
struct remove_cvref {
    typedef typename std::remove_cv<
                typename std::remove_reference<T>::type
            >::type type;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template <typename T>
struct remove_cvp_ref {
    typedef typename std::remove_cv<
                typename std::remove_reference<
                    typename std::remove_pointer<T>::type
                >::type
            >::type type;
};

template <typename T>
using remove_cvp_ref_t = typename remove_cvp_ref<T>::type;

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
using remove_cvrp_ext_t = typename remove_cvrp_ext<T>::type;

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

template <typename T>
using remove_all_t = typename remove_all<T>::type;

template <typename T>
struct is_plain_type {
    static constexpr bool value = (std::is_arithmetic<T>::value || std::is_enum<T>::value);
};

template <typename T1, typename T2>
struct is_same_ex : public std::is_same<typename remove_cvref<T1>::type,
                                        typename remove_cvref<T2>::type> {
};

template <typename T, typename = void>
struct is_complete : public std::false_type {
};

template <typename T>
struct is_complete<T, void_t<int[sizeof(T)]> > : public std::true_type {
};

template <typename T>
using is_complete_and_move_constructible = typename std::conditional<is_complete<T>::value,
                                            std::is_move_constructible<T>, std::false_type>::type;

//////////////////////////////////////////////////////////////////////////////////

template <typename T, typename U>
using is_similar = std::is_same<remove_cvref_t<T>, remove_cvref_t<U> >;

template <typename, typename...>
struct is_similar_to_any : std::false_type
{
};

template <typename T, typename U, typename ... Us>
struct is_similar_to_any<T, U, Us...>
    : std::conditional<is_similar<T, U>::value, is_similar<T, U>,
                       is_similar_to_any<T, Us...> >::type
{
};

template <typename T, typename = void>
struct is_transparent : public std::false_type {};

template <typename T>
struct is_transparent<T, void_t<typename T::is_transparent>>
    : public std::true_type {};

template <typename T, typename Hash, typename KeyEqual>
struct are_transparent {
    static const bool value =
        is_transparent<Hash>::value && is_transparent<KeyEqual>::value;
};

template <typename Key, typename UnorderedMap>
struct are_transparent_and_non_iterable {
    typedef typename UnorderedMap::hasher           hash;
    typedef typename UnorderedMap::key_equal        key_equal;
    typedef typename UnorderedMap::iterator         iterator;
    typedef typename UnorderedMap::const_iterator   const_iterator;

    static const bool value =
        are_transparent<Key, hash, key_equal>::value &&
        !std::is_convertible<Key &&, iterator>::value &&
        !std::is_convertible<Key &&, const_iterator>::value;
};

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

template <typename T>
struct is_noexcept_move_constructible {
    static constexpr bool value =
        (std::is_nothrow_move_constructible<T>::value ||
		   !std::is_copy_constructible<T>::value);
};

template <typename T>
struct is_noexcept_move_assignable {
    static constexpr bool value =
        (std::is_nothrow_move_assignable<T>::value ||
		   !std::is_copy_assignable<T>::value);
};

template <typename Hash, typename Key>
struct is_default_std_hash {
    typedef typename std::remove_const<Key>::type key_type;

    static constexpr bool value = std::is_same<Hash, std::hash<key_type>>::value;
};

//////////////////////////////////////////////////////////////////////////////////

struct if_constexpr_void_else {
    void operator()() const {}
};

template <bool Boolean, typename F, typename G = if_constexpr_void_else>
void if_constexpr(F f, G g = { })
{
    std::get<Boolean ? 0 : 1>(std::forward_as_tuple(f, g))();
}

template <bool Boolean, typename T, typename std::enable_if<Boolean>::type * = nullptr>
void copy_assign_if(T & dest, T & src)
{
    dest = src;
}

template <bool Boolean, typename T, typename std::enable_if<!Boolean>::type * = nullptr>
void copy_assign_if(T & dest, T & src)
{
    /* Do nothing ! */
    JSTD_UNUSED(dest);
    JSTD_UNUSED(src);
}

template <bool Boolean, typename T, typename std::enable_if<Boolean>::type * = nullptr>
void move_assign_if(T & dest, T & src)
{
    dest = std::move(src);
}

template <bool Boolean, typename T, typename std::enable_if<!Boolean>::type * = nullptr>
void move_assign_if(T & dest, T & src) {
    /* Do nothing ! */
    JSTD_UNUSED(dest);
    JSTD_UNUSED(src);
}

template <bool Boolean, typename T, typename std::enable_if<Boolean>::type * = nullptr>
void swap_if(T & x, T & y)
{
    using std::swap;
    swap(x, y);
}

template <bool Boolean, typename T, typename std::enable_if<!Boolean>::type * = nullptr>
void swap_if(T & x, T & y)
{
    /* Do nothing ! */
    JSTD_UNUSED(x);
    JSTD_UNUSED(y);
}

template <typename Allocator>
struct is_std_allocator : public std::false_type {};

template <typename T>
struct is_std_allocator<std::allocator<T>> : public std::true_type {};

//////////////////////////////////////////////////////////////////////////////////

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

namespace type_traits_detail {
    using std::swap;

    template <typename T, typename = void>
    struct is_nothrow_swappable_helper {
        static constexpr bool const value = false;
    };

    template <typename T>
    struct is_nothrow_swappable_helper<T,
            void_t<decltype(swap(std::declval<T &>(), std::declval<T &>()))> > {
        static constexpr bool const value = noexcept(swap(std::declval<T &>(), std::declval<T &>()));
    };

    template <typename T, typename = void>
    struct is_swappable_helper {
        static constexpr bool const value = false;
    };

    template <typename T>
    struct is_swappable_helper<T,
            void_t<decltype(swap(std::declval<T &>(), std::declval<T &>()))> > {
        static constexpr bool const value = true;
    };

} // namespace type_traits_detail

template <typename T>
struct is_nothrow_swappable : public std::integral_constant<bool,
                                        (is_plain_type<T>::value || type_traits_detail::is_nothrow_swappable_helper<T>::value)>
{
};

template <typename T>
struct is_swappable : public std::integral_constant<bool,
                                (is_plain_type<T>::value || type_traits_detail::is_swappable_helper<T>::value)>
{
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
struct is_layout_compatible_kv {
private:
    struct Pair {
        Key   first;
        Value second;
    };

public:
    // Is class P layout-compatible with class Pair ?
    template <typename P>
    static constexpr bool isLayoutCompatible() {
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
                                  isLayoutCompatible<std::pair<Key, Value>>() &&
                                  isLayoutCompatible<std::pair<const Key, Value>>();
};

//
// Pair        = std::pair<const Key, Value>
// MutablePair = std::pair<Key, Value>
//
template <typename ConstPair, typename MutablePair>
struct is_layout_compatible_pair {
public:
    typedef typename ConstPair::first_type      First;
    typedef typename ConstPair::second_type     Second;
    typedef typename MutablePair::first_type    MutableFirst;
    typedef typename MutablePair::second_type   MutableSecond;

private:
    struct Pair {
        MutableFirst  first;
        MutableSecond second;
    };

    // Is class P layout-compatible with class Pair ?
    template <typename P>
    static constexpr bool isLayoutCompatible() {
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
                                  isLayoutCompatible<ConstPair>() &&
                                  isLayoutCompatible<MutablePair>();
};

//////////////////////////////////////////////////////////////////////////////////

// Define integer sequence template
template <int...>
struct index_sequence {};

// Recursive generation of integer sequences
template <int N, int... Is>
struct make_index_sequence : make_index_sequence<N - 1, N - 1, Is...> {};

// Recursive termination condition
template <int... Is>
struct make_index_sequence<0, Is...> {
    using type = index_sequence<Is...>;
};

// Simplified use of alias templates
template <int N>
using make_index_sequence_t = typename make_index_sequence<N>::type;

//////////////////////////////////////////////////////////////////////////////////

//
// Constructs T into uninitialized storage pointed by `ptr` using the args
// specified in the tuple.
//
template <class Alloc, class T, class Tuple, size_t ... I>
void ConstructFromTupleImpl(Alloc * alloc, T * ptr, Tuple && t,
                            jstd::index_sequence<I...>) {
    std::allocator_traits<Alloc>::construct(*alloc, ptr, std::get<I>(std::forward<Tuple>(t))...);
}

template <class T, class First>
struct WithConstructedImplF {
    template <class ... Args>
    decltype(std::declval<First>()(std::declval<T>())) operator()(
        Args && ... args) const {
        return std::forward<First>(f)(T(std::forward<Args>(args)...));
    }
    First && f;
};

template <class T, class Tuple, size_t... Is, class First>
decltype(std::declval<First>()(std::declval<T>())) WithConstructedImpl(
    Tuple && t, jstd::index_sequence<Is...>, First&& f) {
    return WithConstructedImplF<T, First>{std::forward<First>(f)}(
        std::get<Is>(std::forward<Tuple>(t))...);
}

template <class T, size_t ... Is>
auto TupleRefImpl(T && t, jstd::index_sequence<Is...>)
-> decltype(std::forward_as_tuple(std::get<Is>(std::forward<T>(t))...)) {
    return std::forward_as_tuple(std::get<Is>(std::forward<T>(t))...);
}

//
// Returns a tuple of references to the elements of the input tuple. T must be a
// tuple.
//
template <class T>
auto TupleRef(T && t) -> decltype(
    TupleRefImpl(std::forward<T>(t),
                 jstd::make_index_sequence<std::tuple_size<typename std::decay<T>::type>::value>())) {
    return TupleRefImpl(
        std::forward<T>(t),
        jstd::make_index_sequence<std::tuple_size<typename std::decay<T>::type>::value>());
}

template <class First, class K, class V>
decltype(std::declval<First>()(std::declval<const K &>(), std::piecewise_construct,
                               std::declval<std::tuple<K>>(), std::declval<V>()))
    DecomposePairImpl(First && first, std::pair<std::tuple<K>, V> pair) {
    const auto & key = std::get<0>(pair.first);
    return std::forward<First>(first)(key, std::piecewise_construct,
                                      std::move(pair.first),
                                      std::move(pair.second));
}

//
// Constructs T into uninitialized storage pointed by `ptr` using the args
// specified in the tuple.
//
template <class Alloc, class T, class Tuple>
void ConstructFromTuple(Alloc * alloc, T * ptr, Tuple && t) {
    ConstructFromTupleImpl(
        alloc, ptr, std::forward<Tuple>(t),
        jstd::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>());
}

//
// Constructs T using the args specified in the tuple and calls F with the
// constructed value.
//
template <class T, class Tuple, class First>
decltype(std::declval<First>()(std::declval<T>()))
WithConstructed(Tuple && t, First && first) {
    return WithConstructedImpl<T>(
        std::forward<Tuple>(t),
        jstd::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>(),
        std::forward<First>(first));
}

//
// Given arguments of an std::pair's consructor, PairArgs() returns a pair of
// tuples with references to the passed arguments. The tuples contain
// constructor arguments for the first and the second elements of the pair.
//
// The following two snippets are equivalent.
//
// 1. std::pair<First, Second> p(args...);
//
// 2. auto a = PairArgs(args...);
//    std::pair<First, Second> p(std::piecewise_construct,
//                               std::move(a.first), std::move(a.second));
//
inline
std::pair<std::tuple<>, std::tuple<>>
PairArgs() {
    return { };
}

template <class First, class Second>
std::pair<std::tuple<First &&>, std::tuple<Second &&>>
PairArgs(First && first, Second && second) {
    return { std::piecewise_construct,
             std::forward_as_tuple(std::forward<First>(first)),
             std::forward_as_tuple(std::forward<Second>(second)) };
}

template <class First, class Second>
std::pair<std::tuple<const First &>, std::tuple<const Second &>>
PairArgs(const std::pair<First, Second> & pair) {
    return PairArgs(pair.first, pair.second);
}

template <class First, class Second>
std::pair<std::tuple<First &&>, std::tuple<Second &&>>
PairArgs(std::pair<First, Second> && pair) {
    return PairArgs(std::forward<First>(pair.first), std::forward<Second>(pair.second));
}

template <class First, class Second>
auto PairArgs(std::piecewise_construct_t, First && first, Second && second)
-> decltype(std::make_pair(TupleRef(std::forward<First>(first)),
                           TupleRef(std::forward<Second>(second)))) {
    return std::make_pair(TupleRef(std::forward<First>(first)),
                          TupleRef(std::forward<Second>(second)));
}

// A helper function for implementing apply() in map policies.
template <class First, class ... Args>
auto DecomposePair(First && f, Args && ... args)
-> decltype(DecomposePairImpl(std::forward<First>(f), PairArgs(std::forward<Args>(args)...))) {
    return DecomposePairImpl(std::forward<First>(f), PairArgs(std::forward<Args>(args)...));
}

// A helper function for implementing apply() in set policies.
template <class First, class Arg>
decltype(std::declval<First>()(std::declval<const Arg &>(), std::declval<Arg>()))
DecomposeValue(First && f, Arg && arg) {
    const auto & key = arg;
    return std::forward<First>(f)(key, std::forward<Arg>(arg));
}

//////////////////////////////////////////////////////////////////////////////////

} // namespace jstd

#endif // JSTD_TYPE_TRAITS_H
