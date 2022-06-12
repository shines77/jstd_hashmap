
#ifndef JSTD_INTEGER_SEQUENCE_H
#define JSTD_INTEGER_SEQUENCE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if (!defined(_MSC_VER) && defined(__cplusplus) && (__cplusplus < 201402L)) \
 || (defined(_MSC_VER) && (_MSC_FULL_VER < 190024210))

#include <cstdint>
#include <cstddef>
#include <cstdbool>
#include <cassert>

#include <type_traits>

namespace std {

template <class T, T... Indexs>
struct integer_sequence {
    // sequence of integer parameters
    static_assert(std::is_integral<T>::value,
                  "std::integer_sequence<T, I...> requires T to be an integral type.");

    typedef integer_sequence<T, Indexs...> type;
    typedef T value_type;

    // get length of parameter list
    static constexpr std::size_t size() noexcept {
        return (sizeof...(Indexs));
    }
};

// Alias template std::make_integer_sequence<T, Size>

// explodes gracefully below 0
template <bool IsNegative, bool IsZero, class IntegralConstant, class IntegralSequence>
struct make_integer_sequence_impl {
    static_assert(!IsNegative, "std::make_integer_sequence<T, Size> requires Size to be non-negative.");
};

// ends recursion at 0
template <class T, T... Indexs>
struct make_integer_sequence_impl<false, true,
                                  std::integral_constant<T, 0>,
                                  integer_sequence<T, Indexs...>>
    : integer_sequence<T, Indexs...> {
};

// counts down to 0
template<class T, T N, T... Indexs>
struct make_integer_sequence_impl<false, false,
                                  std::integral_constant<T, N>,
                                  integer_sequence<T, Indexs...>>
    : make_integer_sequence_impl<false, (N == 1),
                                 std::integral_constant<T, N - 1>,
                                 integer_sequence<T, N - 1, Indexs...>> {
};

template <class T, T Size>
using make_integer_sequence = typename make_integer_sequence_impl<
                              (Size < 0), (Size == 0),
                              std::integral_constant<T, Size>, integer_sequence<T>>::type;

template <std::size_t... Indexs>
using index_sequence = std::integer_sequence<std::size_t, Indexs...>;

template <std::size_t Size>
using make_index_sequence = std::make_integer_sequence<std::size_t, Size>;

template <class... Ts>
using index_sequence_for = std::make_index_sequence<sizeof...(Ts)>;

} // namespace std

#endif // (__cplusplus < 201402L)

#endif // JSTD_INTEGER_SEQUENCE_H
