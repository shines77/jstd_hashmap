
#ifndef JSTD_UTILITY_H
#define JSTD_UTILITY_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <cstdint>
#include <cstddef>
#include <cstdbool>
#include <cassert>
#include <memory>
#include <type_traits>

#include "jstd/utility/integer_sequence.h"
#include "jstd/type_traits.h"

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

template <typename T, typename DecayT = typename std::remove_reference<T>::type,
                      bool IsIntegral = std::is_integral<DecayT>::value>
struct tuple_wrapper : public DecayT {
    using value_type = typename std::remove_reference<T>::type;

    tuple_wrapper() : value_type() {
    }
    tuple_wrapper(const tuple_wrapper & src)
        noexcept(std::is_nothrow_copy_constructible<value_type>::value)
        : value_type(src) {
    }
    tuple_wrapper(tuple_wrapper && src)
        noexcept(std::is_nothrow_move_constructible<value_type>::value)
        : value_type(std::move(src)) {
    }

    template <typename Tuple, std::size_t... Indexs>
    tuple_wrapper(const Tuple & tuple, std::integer_sequence<std::size_t, Indexs...>)
        : value_type(std::get<Indexs>(tuple)...) {
    }

    template <typename Tuple, std::size_t... Indexs>
    tuple_wrapper(Tuple && tuple, std::integer_sequence<std::size_t, Indexs...>)
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
    tuple_wrapper(const Tuple & tuple, std::integer_sequence<std::size_t, Indexs...>)
        : value_(std::get<Indexs>(tuple)...) {
    }

    template <typename Tuple, std::size_t... Indexs>
    tuple_wrapper(Tuple && tuple, std::integer_sequence<std::size_t, Indexs...>)
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
// From: https://blog.csdn.net/netyeaxi/article/details/83539928
//
void print() {
    std::cout << std::endl;
}

template <typename HeadType, typename... Types>
void print(HeadType arg, Types... Args) {
    std::cout << typeid(HeadType).name() << ": " << arg << std::endl;
    print(Args...);
}

template <typename Tuple, typename... Types>
void tuple_element_type(Tuple && tuple, Types... Args) {
    print(Args...);
}

template <typename Tuple, std::size_t... Indexs>
void tuple_element_index(Tuple && tuple, std::index_sequence<Indexs...>) {
    tuple_element_type(tuple, std::get<Indexs>(std::forward<Tuple>(tuple))...);
}

template <typename Tuple>
void break_tuple(Tuple && tuple) {
    tuple_element_index(tuple, std::make_index_sequence<
                        std::tuple_size<typename std::remove_reference<Tuple>::type>::value
                        >{});
}

} // namespace jstd

#endif // JSTD_UTILITY_H
