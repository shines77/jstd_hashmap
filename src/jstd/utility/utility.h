
#ifndef JSTD_UTILITY_H
#define JSTD_UTILITY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <cstdio>
#include <iostream>
#include <cstdint>
#include <cstddef>
#include <cstdbool>
#include <cassert>
#include <memory>
#include <tuple>
#include <utility>
#include <type_traits>

#include "jstd/basic/stddef.h"
#include "jstd/utility/integer_sequence.h"
#include "jstd/traits/type_traits.h"

namespace jstd {
namespace detail {

#if !defined(_MSC_VER) || (_MSC_VER >= 1800)

// function template const_cast

// remove const-ness from a fancy pointer
template <typename Ptr>
inline
auto const_castor(Ptr ptr)
#if !defined(_MSC_VER) || (_MSC_VER < 1900)
                           -> typename std::pointer_traits<
                                typename std::pointer_traits<Ptr>::template rebind<
                                  typename std::remove_const<
                                    typename std::pointer_traits<Ptr>::element_type
                                  >::type
                                >
                              >::pointer
#endif // !_MSC_VER
{
    using Element = typename std::pointer_traits<Ptr>::element_type;
    using Modifiable = typename std::remove_const<Element>::type;
    using Dest = typename std::pointer_traits<Ptr>::template rebind<Modifiable>;

    return (std::pointer_traits<Dest>::pointer_to(const_cast<Modifiable &>(*ptr)));
}

// remove const-ness from a plain pointer
template <typename T>
inline
auto const_castor(T * ptr) -> const typename std::remove_const<T>::type * {
    return (const_cast<typename std::remove_const<T>::type *>(ptr));
}

#endif // (_MSC_VER >= 1800)

} // namespace detail
} // namespace jstd

namespace jstd {

template <typename T = void>
static inline
bool check_alignment(T * address, size_t alignment)
{
    uintptr_t ptr = (uintptr_t)address;
    JSTD_ASSERT(alignment > 0);
    JSTD_ASSERT((alignment & (alignment - 1)) == 0);
    return ((ptr & (alignment - 1)) == 0);
}

template <typename T, size_t alignment = std::alignment_of<T>::value>
static inline
bool check_alignment(T * address)
{
    uintptr_t ptr = (uintptr_t)address;
    JSTD_STATIC_ASSERT((alignment > 0),
                       "check_alignment<T, N>(addr): alignment must bigger than 0.");
    JSTD_STATIC_ASSERT(((alignment & (alignment - 1)) == 0),
                       "check_alignment<T, N>(addr): alignment must be power of 2.");
    return ((ptr & (alignment - 1)) == 0);
}

static inline
std::size_t align_to(std::size_t size, std::size_t alignment)
{
    assert(alignment > 0);
    assert((alignment & (alignment - 1)) == 0);
    size = (size + alignment - 1) & ~(alignment - 1);
    assert((size / alignment * alignment) == size);
    return size;
}

template <typename T>
static inline
T * pointer_align_to(T * address, size_t alignment)
{
    assert(alignment > 0 );
    assert((alignment & (alignment - 1)) == 0);
    uintptr_t ptr = ((uintptr_t)address + alignment - 1) & (~(alignment - 1));
    return reinterpret_cast<T *>(ptr);
}

template <size_t alignment, typename T>
static inline
T * pointer_align_to(T * address)
{
    static_assert((alignment > 0),
                  "pointer_align_to<N>(): alignment must bigger than 0.");
    static_assert(((alignment & (alignment - 1)) == 0),
                  "pointer_align_to<N>(): alignment must be power of 2.");
    uintptr_t ptr = ((uintptr_t)address + alignment - 1) & (~(alignment - 1));
    return reinterpret_cast<T *>(ptr);
}

#ifdef JSTD_EXCHANGE_FUNCTION

/* using override */
using std::exchange;

#else

template <typename T, typename U = T>
inline
#if jstd_cplusplus >= 2020L
constexpr // since C++20
#endif
T exchange(T & target, U && new_value) noexcept(
           std::is_nothrow_move_constructible<T>::value &&
           std::is_nothrow_assignable<T &, U>::value)
{
    T old_value = std::move(target);
    target = std::forward<U>(new_value);
    return old_value;
}
#endif // JSTD_EXCHANGE_FUNCTION

//
// tuple_wrapper<T, DecayT, bool IsIntegral>
//
template <typename T, typename DecayT = typename std::remove_reference<T>::type,
          bool IsIntegralOrPointer = (std::is_integral<DecayT>::value || std::is_pointer<DecayT>::value)>
struct tuple_wrapper : public DecayT {
    using value_type = typename std::remove_reference<T>::type;

    tuple_wrapper() : value_type() {
    }
    tuple_wrapper(const tuple_wrapper & src)
        noexcept(std::is_nothrow_copy_constructible<value_type>::value)
        : value_type(*static_cast<value_type *>(&src)) {
    }
    tuple_wrapper(tuple_wrapper && src)
        noexcept(std::is_nothrow_move_constructible<value_type>::value)
        : value_type(std::move(*static_cast<value_type *>(&src))) {
    }

    template <typename Tuple, std::size_t... Indexs>
    tuple_wrapper(const Tuple & tuple, std::index_sequence<Indexs...>)
        : value_type(std::get<Indexs>(tuple)...) {
    }

    template <typename Tuple, std::size_t... Indexs>
    tuple_wrapper(Tuple && tuple, std::index_sequence<Indexs...>)
        : value_type(std::get<Indexs>(std::forward<Tuple>(tuple))...) {
    }

    template <typename ... Ts>
    tuple_wrapper(const std::tuple<Ts...> & tuple)
        : tuple_wrapper(tuple, std::make_integer_sequence<std::size_t, sizeof...(Ts)>()) {
    }

    template <typename ... Ts>
    tuple_wrapper(std::tuple<Ts...> && tuple)
        : tuple_wrapper(std::forward<std::tuple<Ts...>>(tuple),
                        std::make_integer_sequence<std::size_t, sizeof...(Ts)>()) {
    }

    value_type & value() {
        return *static_cast<value_type *>(this);
    }

    const value_type & value() const {
        return *static_cast<const value_type *>(const_cast<tuple_wrapper *>(this));
    }
};

//
// tuple_wrapper<T, DecayT, true>
//
template <typename T>
struct tuple_wrapper<T, typename std::remove_reference<T>::type, true> {
    using value_type = typename std::remove_reference<T>::type;

    tuple_wrapper() : value_() {
    }
    tuple_wrapper(const tuple_wrapper & src)
        noexcept(std::is_nothrow_copy_constructible<value_type>::value)
        : value_(src.value_) {
    }
    tuple_wrapper(tuple_wrapper && src)
        noexcept(std::is_nothrow_move_constructible<value_type>::value)
        : value_(std::move(src.value_)) {
    }

    template <typename Tuple, std::size_t... Indexs>
    tuple_wrapper(const Tuple & tuple, std::index_sequence<Indexs...>)
        : value_(std::get<Indexs>(tuple)...) {
    }

    template <typename Tuple, std::size_t... Indexs>
    tuple_wrapper(Tuple && tuple, std::index_sequence<Indexs...>)
        : value_(std::get<Indexs>(std::forward<Tuple>(tuple))...) {
    }

    template <typename ... Ts>
    tuple_wrapper(const std::tuple<Ts...> & tuple)
        : tuple_wrapper(tuple, std::make_integer_sequence<std::size_t, sizeof...(Ts)>()) {
    }

    template <typename ... Ts>
    tuple_wrapper(std::tuple<Ts...> && tuple)
        : tuple_wrapper(std::forward<std::tuple<Ts...>>(tuple),
                        std::make_integer_sequence<std::size_t, sizeof...(Ts)>()) {
    }

    value_type & value() {
        return value_;
    }

    const value_type & value() const {
        return value_;
    }

    value_type value_;
};

//
// From: https://en.cppreference.com/w/cpp/utility/make_from_tuple
//

//
// tuple_wrapper2<T, DecayT, IsIntegral>
//
template <typename T, typename DecayT = typename std::remove_reference<T>::type,
          bool IsIntegralOrPointer = (std::is_integral<DecayT>::value || std::is_pointer<DecayT>::value)>
struct tuple_wrapper2 : DecayT {
    using value_type = typename std::remove_reference<T>::type;

    tuple_wrapper2() : value_type() {
    }
    tuple_wrapper2(const tuple_wrapper2 & src)
        noexcept(std::is_nothrow_copy_constructible<value_type>::value)
        : value_type(*static_cast<value_type *>(&src)) {
    }
    tuple_wrapper2(tuple_wrapper2 && src)
        noexcept(std::is_nothrow_move_constructible<value_type>::value)
        : value_type(std::move(*static_cast<value_type *>(&src))) {
    }

#if 0
    template <typename... Args>
    tuple_wrapper2(Args && ... args)
        : value_type(std::forward<Args>(args)...) {
    }
#endif

    template <typename Tuple, std::size_t... Indexs>
    tuple_wrapper2(Tuple && tuple, std::index_sequence<Indexs...>)
        : value_type(std::get<Indexs>(std::forward<Tuple>(tuple))...) {
        static_assert(std::is_constructible<value_type,
                      decltype(std::get<Indexs>(std::declval<Tuple>()))...>::value,
                      "std::tuple<...> must be construct to tuple_wrapper2::value_type.");
    }

    template <typename Tuple>
    tuple_wrapper2(Tuple && tuple)
        : tuple_wrapper2(std::forward<Tuple>(tuple), std::make_index_sequence<
                            std::tuple_size<typename std::remove_reference<Tuple>::type>::value
                         >{}) {
    }

    value_type & value() {
        return *static_cast<value_type *>(this);
    }

    const value_type & value() const {
        return *static_cast<const value_type *>(const_cast<tuple_wrapper2 *>(this));
    }
};

//
// tuple_wrapper2<T, DecayT, true>
//
template <typename T>
struct tuple_wrapper2<T, typename std::remove_reference<T>::type, true> {
    using value_type = typename std::remove_reference<T>::type;

    tuple_wrapper2() : value_() {
    }
    tuple_wrapper2(const tuple_wrapper2 & src)
        noexcept(std::is_nothrow_copy_constructible<value_type>::value)
        : value_(src.value_) {
    }
    tuple_wrapper2(tuple_wrapper2 && src)
        noexcept(std::is_nothrow_move_constructible<value_type>::value)
        : value_(std::move(src.value_)) {
    }

#if 0
    template <typename... Args>
    tuple_wrapper2(Args && ... args)
        : value_(std::forward<Args>(args)...) {
    }
#endif

    template <typename Tuple, std::size_t... Indexs>
    tuple_wrapper2(Tuple && tuple, std::index_sequence<Indexs...>)
        : value_(std::get<Indexs>(std::forward<Tuple>(tuple))...) {
        static_assert(std::is_constructible<value_type,
                      decltype(std::get<Indexs>(std::declval<Tuple>()))...>::value,
                      "std::tuple<...> must be construct to tuple_wrapper2::value_type.");
    }

    template <typename Tuple>
    tuple_wrapper2(Tuple && tuple)
        : tuple_wrapper2(std::forward<Tuple>(tuple), std::make_index_sequence<
                            std::tuple_size<typename std::remove_reference<Tuple>::type>::value
                         >{}) {
    }

    value_type & value() {
        return value_;
    }

    const value_type & value() const {
        return value_;
    }

    value_type value_;
};

//
// From: https://blog.csdn.net/netyeaxi/article/details/83539928
//

void print_tuple() {
    std::cout << std::endl;
}

template <typename HeadType, typename... Args>
void print_tuple(HeadType && head, Args && ... args) {
    std::cout << typeid(decltype(head)).name() << ": "
              << std::forward<HeadType>(head) << std::endl;
    print_tuple(std::forward<Args>(args)...);
}

template <typename Tuple, typename... Args>
void tuple_element_type(Tuple && tuple, Args && ... args) {
    print_tuple(std::forward<Args>(args)...);
}

template <typename Tuple, std::size_t... Indexs>
void tuple_element_index(Tuple && tuple, std::index_sequence<Indexs...>) {
    tuple_element_type(std::forward<Tuple>(tuple), std::get<Indexs>(std::forward<Tuple>(tuple))...);
}

template <typename Tuple>
void break_from_tuple(Tuple && tuple) {
    tuple_element_index(std::forward<Tuple>(tuple), std::make_index_sequence<
                            std::tuple_size<typename std::remove_reference<Tuple>::type>::value
                        >{});
}

} // namespace jstd

#endif // JSTD_UTILITY_H
