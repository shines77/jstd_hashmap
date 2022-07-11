
#ifndef JSTD_SUPPORT_POWER2_H
#define JSTD_SUPPORT_POWER2_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>          // For std::size_t
#include <cstdbool>
#include <cassert>
#include <limits>           // For std::numeric_limits<T>::max()
#include <type_traits>      // For std::make_unsigned<T>

#include "jstd/support/BitUtils.h"

//////////////////////////////////////////////////////////////////////////
//
// Bit Twiddling Hacks
//
// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
//
//////////////////////////////////////////////////////////////////////////

namespace jstd {

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4293)
#endif

template <typename SizeType>
struct size_type_t {
    typedef typename std::conditional<(sizeof(SizeType) <= 4),
                                      std::uint32_t,
                                      std::uint64_t>::type type;
    typedef typename std::make_unsigned<type>::type unsigned_type;
    typedef typename std::make_signed<type>::type   signed_type;
};

namespace pow2 {

template <typename SizeType>
inline
bool is_pow2(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::is_pow2(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    unsigned_type n = static_cast<unsigned_type>(N);
    return ((n & (n - 1)) == 0);
}

template <typename SizeType>
inline
SizeType clear_low_bit(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::clear_low_bit(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    unsigned_type n = static_cast<unsigned_type>(N);
    return static_cast<SizeType>(n & (n - 1));
}

template <typename SizeType>
inline
std::uint32_t bitScanForward(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::bitScanForward(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    unsigned_type n = static_cast<unsigned_type>(N);
    assert(n != 0);
    if (sizeof(SizeType) <= 4) {
        return BitUtils::bsf32((std::uint32_t)n);
    } if (sizeof(SizeType) == 8) {
        return BitUtils::bsf64(n);
    } else {
        return BitUtils::bsf64((std::uint64_t)n);
    }
}

template <typename SizeType>
inline
std::uint32_t bitScanReverse(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::bitScanReverse(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    unsigned_type n = static_cast<unsigned_type>(N);
    assert(n != 0);
    if (sizeof(SizeType) <= 4) {
        return BitUtils::bsr32((std::uint32_t)n);
    } if (sizeof(SizeType) == 8) {
        return BitUtils::bsr64(n);
    } else {
        return BitUtils::bsr64((std::uint64_t)n);
    }
}

template <typename SizeType>
inline
std::uint32_t countTrailingZeros(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::countTrailingZeros(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    unsigned_type n = static_cast<unsigned_type>(N);
    assert(n != 0);
    if (sizeof(SizeType) <= 4) {
        return BitUtils::countTrailingZeros32((std::uint32_t)n);
    } if (sizeof(SizeType) == 8) {
        return BitUtils::countTrailingZeros64(n);
    } else {
        return BitUtils::countTrailingZeros64((std::uint64_t)n);
    }
}

template <typename SizeType>
inline
std::uint32_t countLeadingZeros(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::countLeadingZeros(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    unsigned_type n = static_cast<unsigned_type>(N);
    assert(n != 0);
    if (sizeof(SizeType) <= 4) {
        return BitUtils::countLeadingZeros32((std::uint32_t)n);
    } if (sizeof(SizeType) == 8) {
        return BitUtils::countLeadingZeros64(n);
    } else {
        return BitUtils::countLeadingZeros64((std::uint64_t)n);
    }
}

//
// PS: | X | = floor(x),  is a rounding function.
//

//
// N = prev_pow2( n ):
//
//   = 2 ^ (|Log2(n)| - 1)
//
// eg:
//   next_pow2(0) = 0, next_pow2(1) = 0
//   next_pow2(4) = 2, next_pow2(5) = 2
//   next_pow2(7) = 2, next_pow2(8) = 4
//

template <typename SizeType, SizeType Min_n = 0>
inline
typename size_type_t<SizeType>::type
prev_pow2(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::prev_pow2(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    typedef typename size_type_t<SizeType>::type        return_type;
    unsigned_type n = static_cast<unsigned_type>(N);
    if ((n > 1) || (Min_n > 1)) {
        assert(n > 1);
        std::uint32_t ms1b = bitScanReverse(n);
        assert(ms1b >= 1);
        return (return_type(1) << (ms1b - 1));
    } else {
        return return_type(0);
    }
}

//
// N = round_down( n ):
//
//   = 2 ^ |Log2(n - 1)|
//
// eg:
//   round_down(0) = 0, round_down(1) = 0, round_down(2) = 1,
//   round_down(4) = 2, round_down(5) = 4,
//   round_down(7) = 4, round_down(8) = 4,
//

template <typename SizeType, SizeType Min_n = 0>
inline
typename size_type_t<SizeType>::type
round_down(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::round_down(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    typedef typename size_type_t<SizeType>::type        return_type;
#ifdef _DEBUG
    typedef typename std::make_signed<SizeType>::type   signed_type;
#endif
    unsigned_type n = static_cast<unsigned_type>(N);
    if ((n > 1) || (Min_n > 1)) {
        assert(signed_type(n - 1) > 0);
        std::uint32_t ms1b = bitScanReverse(n - 1);
        return (return_type(1) << ms1b);
    } else {
        return return_type(0);
    }
}

//
// N = round_to( n ):
//
//   = 2 ^ |Log2(n)|
//
// eg:
//   round_to(0) = 0, round_to(1) = 1, round_to(2) = 2,
//   round_to(4) = 4, round_to(5) = 4,
//   round_to(7) = 4, round_to(8) = 8,
//

template <typename SizeType, SizeType Min_n = 0>
inline
typename size_type_t<SizeType>::type
round_to(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::round_to(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    typedef typename size_type_t<SizeType>::type        return_type;
    unsigned_type n = static_cast<unsigned_type>(N);
    if ((n > 0) || (Min_n > 0)) {
        assert(n > 0);
        std::uint32_t ms1b = bitScanReverse(n);
        return (return_type(1) << ms1b);
    } else {
        return return_type(0);
    }
}

//
// N = round_up( n ):
//
//   = 2 ^ (|Log2(n - 1)| + 1)
//
// eg:
//   round_up(0) = 0, round_up(1) = 1, round_up(2) = 2,
//   round_up(4) = 4, round_up(5) = 8,
//   round_up(7) = 8, round_up(8) = 8,
//

template <typename SizeType, SizeType Min_n = 0>
inline
typename size_type_t<SizeType>::type
round_up(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::round_up(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    typedef typename size_type_t<SizeType>::type        return_type;
#ifdef _DEBUG
    typedef typename std::make_signed<SizeType>::type   signed_type;
#endif
    unsigned_type n = static_cast<unsigned_type>(N);
    if ((n <= ((std::numeric_limits<unsigned_type>::max)() / 2 + 1)) || (sizeof(SizeType) > 4)) {
        if ((n > 1) || (Min_n > 1)) {
            assert(signed_type(n - 1) > 0);
            std::uint32_t ms1b = bitScanReverse(n - 1);
            return (return_type(1) << (ms1b + 1));
        } else {
            return return_type(n);
        }
    }
    else {
        return (return_type)(std::numeric_limits<unsigned_type>::max)();
    }
}

//
// N = next_pow2( n ):
//
//   = 2 ^ (|Log2(n)| + 1)
//
// eg:
//   next_pow2(0) = 1, next_pow2(1) = 2, next_pow2(2) = 4,
//   next_pow2(4) = 8, next_pow2(5) = 8,
//   next_pow2(7) = 8, next_pow2(8) = 16,
//

template <typename SizeType, SizeType Min_n = 0>
inline
typename size_type_t<SizeType>::type
next_pow2(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::next_pow2(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    typedef typename size_type_t<SizeType>::type        return_type;
    unsigned_type n = static_cast<unsigned_type>(N);
    if ((n < ((std::numeric_limits<unsigned_type>::max)() / 2 + 1)) || (sizeof(SizeType) > 4)) {
        if ((n > 0) || (Min_n > 0)) {
            assert(n > 0);
            std::uint32_t ms1b = bitScanReverse(n);
            return (return_type(1) << (ms1b + 1));
        } else {
            return return_type(1);
        }
    }
    else {
        return (return_type)(std::numeric_limits<unsigned_type>::max)();
    }
}

//
// N = log2_int( n ):
//
//   = (|Log2(n - 1)| + 1)
//
// eg:
//   log2_int(0) = 0, log2_int(1) = 0, log2_int(2) = 1,
//   log2_int(4) = 2, log2_int(5) = 3,
//   log2_int(7) = 3, log2_int(8) = 3,
//

template <typename SizeType, SizeType Min_n = 0>
inline
typename size_type_t<SizeType>::type
log2_int(SizeType N)
{
    static_assert(std::is_integral<SizeType>::value,
                  "Error: pow2::log2_int(SizeType n) -- n must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type unsigned_type;
    typedef typename size_type_t<SizeType>::type        return_type;
#ifdef _DEBUG
    typedef typename std::make_signed<SizeType>::type   signed_type;
#endif
    unsigned_type n = static_cast<unsigned_type>(N);
    if ((n <= ((std::numeric_limits<unsigned_type>::max)() / 2 + 1)) || (sizeof(SizeType) > 4)) {
        if ((n > 1) || (Min_n > 1)) {
            assert(signed_type(n - 1) > 0);
            std::uint32_t ms1b = bitScanReverse(n - 1);
            return return_type(ms1b + 1);
        } else {
            return return_type(0);
        }
    }
    else {
        return (return_type)(std::numeric_limits<unsigned_type>::max)();
    }
}

} // namespace pow2

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

} // namespace jstd

//////////////////////////////////////////////////////////////////////////////////
//
// Msvc compiler: Maxinum recursive depth is about 497 layer.
//
//////////////////////////////////////////////////////////////////////////////////

namespace jstd {
namespace compile_time {

//
// is_pow2 = (N && ((N & (N - 1)) == 0);
// Here, N must be a unsigned number.
//
template <std::size_t N>
struct is_pow2 {
    static constexpr bool value = ((N & (N - 1)) == 0);
};

template <std::size_t N>
struct clear_low_bit {
    static constexpr std::size_t value = N & (N - 1);
};

//////////////////////////////////////////////////////////////////////////////////
// struct round_to_pow2<N>
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N, std::size_t Power2>
struct round_to_pow2_impl {
    static constexpr std::size_t max_num = (std::numeric_limits<std::size_t>::max)();
    static constexpr std::size_t max_power2 = (std::numeric_limits<std::size_t>::max)() / 2 + 1;
    static constexpr std::size_t next_power2 = (Power2 < max_power2) ? (Power2 << 1) : 0;

    static constexpr bool too_large = (N > max_power2);
    static constexpr bool reach_limit = (Power2 == max_power2);

    static constexpr std::size_t value = ((N >= max_power2) ? max_power2 :
           (((Power2 == max_power2) || (Power2 >= N)) ? (Power2 / 2) :
            round_to_pow2_impl<N, next_power2>::value));
};

template <std::size_t N>
struct round_to_pow2_impl<N, 0> {
    static constexpr std::size_t value = (std::numeric_limits<std::size_t>::max)() / 2 + 1;
};

template <std::size_t N>
struct round_to_pow2 {
    static constexpr std::size_t value = is_pow2<N>::value ? N : round_to_pow2_impl<N, 1>::value;
};

//////////////////////////////////////////////////////////////////////////////////
// struct round_down_pow2<N>
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N>
struct round_down_pow2 {
    static constexpr std::size_t value = (N != 0) ? round_to_pow2<N - 1>::value : 0;
};

//////////////////////////////////////////////////////////////////////////////////
// struct round_up_pow2<N>
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N, std::size_t Power2>
struct round_up_pow2_impl {
    static constexpr std::size_t max_num = (std::numeric_limits<std::size_t>::max)();
    static constexpr std::size_t max_power2 = (std::numeric_limits<std::size_t>::max)() / 2 + 1;
    static constexpr std::size_t next_power2 = (Power2 < max_power2) ? (Power2 << 1) : 0;

    static constexpr bool too_large = (N >= max_power2);
    static constexpr bool reach_limit = (Power2 == max_power2);

    static constexpr std::size_t value = ((N > max_power2) ? max_num :
           (((Power2 == max_power2) || (Power2 >= N)) ? Power2 :
            round_up_pow2_impl<N, next_power2>::value));
};

template <std::size_t N>
struct round_up_pow2_impl<N, 0> {
    static constexpr std::size_t value = (std::numeric_limits<std::size_t>::max)();
};

template <std::size_t N>
struct round_up_pow2 {
    static constexpr std::size_t value = is_pow2<N>::value ? N : round_up_pow2_impl<N, 1>::value;
};

//////////////////////////////////////////////////////////////////////////////////
// struct next_pow2<N>
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N, std::size_t Power2>
struct next_pow2_impl {
    static constexpr std::size_t max_num = (std::numeric_limits<std::size_t>::max)();
    static constexpr std::size_t max_power2 = (std::numeric_limits<std::size_t>::max)() / 2 + 1;
    static constexpr std::size_t next_power2 = (Power2 < max_power2) ? (Power2 << 1) : 0;

    static constexpr bool too_large = (N >= max_power2);
    static constexpr bool reach_limit = (Power2 == max_power2);

    static constexpr std::size_t value = ((N >= max_power2) ? max_num :
           (((Power2 == max_power2) || (Power2 > N)) ? Power2 :
            next_pow2_impl<N, next_power2>::value));
};

template <std::size_t N>
struct next_pow2_impl<N, 0> {
    static constexpr std::size_t value = (std::numeric_limits<std::size_t>::max)();
};

template <std::size_t N>
struct next_pow2 {
    static constexpr std::size_t value = next_pow2_impl<N, 1>::value;
};

template <>
struct next_pow2<0> {
    static constexpr std::size_t value = 1;
};

//////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4307)
#endif

//////////////////////////////////////////////////////////////////////////////////
// struct round_to_power2_impl<N>
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N>
struct round_to_power2_impl {
    static constexpr std::size_t max_num = (std::numeric_limits<std::size_t>::max)();
    static constexpr std::size_t max_power2 = (std::numeric_limits<std::size_t>::max)() / 2 + 1;
    static constexpr std::size_t N1 = N - 1;
    static constexpr std::size_t N2 = N1 | (N1 >> 1);
    static constexpr std::size_t N3 = N2 | (N2 >> 2);
    static constexpr std::size_t N4 = N3 | (N3 >> 4);
    static constexpr std::size_t N5 = N4 | (N4 >> 8);
    static constexpr std::size_t N6 = N5 | (N5 >> 16);
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) || defined(_M_ARM64)
    static constexpr std::size_t N7 = N6 | (N6 >> 32);
    static constexpr std::size_t value = (N7 != max_num) ? ((N7 + 1) / 2) : max_power2;
#else
    static constexpr std::size_t value = (N6 != max_num) ? ((N6 + 1) / 2) : max_power2;
#endif
};

template <std::size_t N>
struct round_to_power2 {
    static constexpr std::size_t value = is_pow2<N>::value ? N : round_to_power2_impl<N>::value;
};

//////////////////////////////////////////////////////////////////////////////////
// struct round_down_power2<N>
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N>
struct round_down_power2 {
    static constexpr std::size_t value = (N != 0) ? round_to_power2<N - 1>::value : 0;
};

//////////////////////////////////////////////////////////////////////////////////
// struct round_up_power2<N>
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N>
struct round_up_power2_impl {
    static constexpr std::size_t max_num = (std::numeric_limits<std::size_t>::max)();
    static constexpr std::size_t N1 = N - 1;
    static constexpr std::size_t N2 = N1 | (N1 >> 1);
    static constexpr std::size_t N3 = N2 | (N2 >> 2);
    static constexpr std::size_t N4 = N3 | (N3 >> 4);
    static constexpr std::size_t N5 = N4 | (N4 >> 8);
    static constexpr std::size_t N6 = N5 | (N5 >> 16);
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) || defined(_M_ARM64)
    static constexpr std::size_t N7 = N6 | (N6 >> 32);
    static constexpr std::size_t value = (N7 != max_num) ? (N7 + 1) : max_num;
#else
    static constexpr std::size_t value = (N6 != max_num) ? (N6 + 1) : max_num;
#endif
};

template <std::size_t N>
struct round_up_power2 {
    static constexpr std::size_t value = is_pow2<N>::value ? N : round_up_power2_impl<N>::value;
};

//////////////////////////////////////////////////////////////////////////////////
// struct next_power2<N>
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t N>
struct next_power2 {
    static constexpr std::size_t max_num = (std::numeric_limits<std::size_t>::max)();
    static constexpr std::size_t value = (N < max_num) ? round_up_power2<N + 1>::value : max_num;
};

//////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////////////
// struct countLeadingZeros<N>
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t Bits, std::size_t N>
struct countLeadingZeros_impl {
    static constexpr std::size_t value = (N == 0) ? Bits : countLeadingZeros_impl<Bits + 1, N * 2>::value;
};

#if 1
//
// countLeadingZeros_impl<0, 0> = 0
//
template <std::size_t Bits>
struct countLeadingZeros_impl<Bits, std::size_t(0)> {
    static constexpr std::size_t value = (Bits > 0) ? (Bits - 1) : 0;
};

#else
#define COUNT_LEADING_ZEROS_IMPL(bits) \
    template <> \
    struct countLeadingZeros_impl<std::size_t(bits), std::size_t(0)> { \
        static constexpr std::size_t value = (bits > 0) ? (bits - 1) : 0; \
    }
COUNT_LEADING_ZEROS_IMPL(64);
COUNT_LEADING_ZEROS_IMPL(63);
COUNT_LEADING_ZEROS_IMPL(62);
COUNT_LEADING_ZEROS_IMPL(61);
COUNT_LEADING_ZEROS_IMPL(60);
COUNT_LEADING_ZEROS_IMPL(59);
COUNT_LEADING_ZEROS_IMPL(58);
COUNT_LEADING_ZEROS_IMPL(57);
COUNT_LEADING_ZEROS_IMPL(56);
COUNT_LEADING_ZEROS_IMPL(55);
COUNT_LEADING_ZEROS_IMPL(54);
COUNT_LEADING_ZEROS_IMPL(53);
COUNT_LEADING_ZEROS_IMPL(52);
COUNT_LEADING_ZEROS_IMPL(51);
COUNT_LEADING_ZEROS_IMPL(50);
COUNT_LEADING_ZEROS_IMPL(49);
COUNT_LEADING_ZEROS_IMPL(48);
COUNT_LEADING_ZEROS_IMPL(47);
COUNT_LEADING_ZEROS_IMPL(46);
COUNT_LEADING_ZEROS_IMPL(45);
COUNT_LEADING_ZEROS_IMPL(44);
COUNT_LEADING_ZEROS_IMPL(43);
COUNT_LEADING_ZEROS_IMPL(42);
COUNT_LEADING_ZEROS_IMPL(41);
COUNT_LEADING_ZEROS_IMPL(40);
COUNT_LEADING_ZEROS_IMPL(39);
COUNT_LEADING_ZEROS_IMPL(38);
COUNT_LEADING_ZEROS_IMPL(37);
COUNT_LEADING_ZEROS_IMPL(36);
COUNT_LEADING_ZEROS_IMPL(35);
COUNT_LEADING_ZEROS_IMPL(34);
COUNT_LEADING_ZEROS_IMPL(33);
COUNT_LEADING_ZEROS_IMPL(32);
COUNT_LEADING_ZEROS_IMPL(31);
COUNT_LEADING_ZEROS_IMPL(30);
COUNT_LEADING_ZEROS_IMPL(29);
COUNT_LEADING_ZEROS_IMPL(28);
COUNT_LEADING_ZEROS_IMPL(27);
COUNT_LEADING_ZEROS_IMPL(26);
COUNT_LEADING_ZEROS_IMPL(25);
COUNT_LEADING_ZEROS_IMPL(24);
COUNT_LEADING_ZEROS_IMPL(23);
COUNT_LEADING_ZEROS_IMPL(22);
COUNT_LEADING_ZEROS_IMPL(21);
COUNT_LEADING_ZEROS_IMPL(20);
COUNT_LEADING_ZEROS_IMPL(19);
COUNT_LEADING_ZEROS_IMPL(18);
COUNT_LEADING_ZEROS_IMPL(17);
COUNT_LEADING_ZEROS_IMPL(16);
COUNT_LEADING_ZEROS_IMPL(15);
COUNT_LEADING_ZEROS_IMPL(14);
COUNT_LEADING_ZEROS_IMPL(13);
COUNT_LEADING_ZEROS_IMPL(12);
COUNT_LEADING_ZEROS_IMPL(11);
COUNT_LEADING_ZEROS_IMPL(10);
COUNT_LEADING_ZEROS_IMPL(9);
COUNT_LEADING_ZEROS_IMPL(8);
COUNT_LEADING_ZEROS_IMPL(7);
COUNT_LEADING_ZEROS_IMPL(6);
COUNT_LEADING_ZEROS_IMPL(5);
COUNT_LEADING_ZEROS_IMPL(4);
COUNT_LEADING_ZEROS_IMPL(3);
COUNT_LEADING_ZEROS_IMPL(2);
COUNT_LEADING_ZEROS_IMPL(1);
COUNT_LEADING_ZEROS_IMPL(0);
#endif

template <std::size_t N>
struct countLeadingZeros {
    static constexpr std::size_t round_down_2 = is_pow2<N>::value ? N : round_down_pow2<N>::value;
    static constexpr std::size_t value = countLeadingZeros_impl<0, round_down_2>::value;
};

//////////////////////////////////////////////////////////////////////////////////
// struct countTrailingZeros<N>
//////////////////////////////////////////////////////////////////////////////////

template <std::size_t Bits, std::size_t N>
struct countTrailingZeros_impl {
    static constexpr std::size_t value = (N == 0) ? Bits : countTrailingZeros_impl<Bits + 1, N / 2>::value;
};

#if 1
//
// countTrailingZeros<0, 0> = 64
//
template <std::size_t Bits>
struct countTrailingZeros_impl<Bits, std::size_t(0)> {
    static constexpr std::size_t value = (Bits > 0) ? (Bits - 1) : (sizeof(std::size_t) * 8);
};

#else
#define COUNT_TRAILING_ZEROS_IMPL(bits) \
    template <> \
    struct countTrailingZeros_impl<std::size_t(bits), std::size_t(0)> { \
        static constexpr std::size_t value = (bits > 0) ? (bits - 1) : (sizeof(std::size_t) * 8); \
    }
COUNT_TRAILING_ZEROS_IMPL(64);
COUNT_TRAILING_ZEROS_IMPL(63);
COUNT_TRAILING_ZEROS_IMPL(62);
COUNT_TRAILING_ZEROS_IMPL(61);
COUNT_TRAILING_ZEROS_IMPL(60);
COUNT_TRAILING_ZEROS_IMPL(59);
COUNT_TRAILING_ZEROS_IMPL(58);
COUNT_TRAILING_ZEROS_IMPL(57);
COUNT_TRAILING_ZEROS_IMPL(56);
COUNT_TRAILING_ZEROS_IMPL(55);
COUNT_TRAILING_ZEROS_IMPL(54);
COUNT_TRAILING_ZEROS_IMPL(53);
COUNT_TRAILING_ZEROS_IMPL(52);
COUNT_TRAILING_ZEROS_IMPL(51);
COUNT_TRAILING_ZEROS_IMPL(50);
COUNT_TRAILING_ZEROS_IMPL(49);
COUNT_TRAILING_ZEROS_IMPL(48);
COUNT_TRAILING_ZEROS_IMPL(47);
COUNT_TRAILING_ZEROS_IMPL(46);
COUNT_TRAILING_ZEROS_IMPL(45);
COUNT_TRAILING_ZEROS_IMPL(44);
COUNT_TRAILING_ZEROS_IMPL(43);
COUNT_TRAILING_ZEROS_IMPL(42);
COUNT_TRAILING_ZEROS_IMPL(41);
COUNT_TRAILING_ZEROS_IMPL(40);
COUNT_TRAILING_ZEROS_IMPL(39);
COUNT_TRAILING_ZEROS_IMPL(38);
COUNT_TRAILING_ZEROS_IMPL(37);
COUNT_TRAILING_ZEROS_IMPL(36);
COUNT_TRAILING_ZEROS_IMPL(35);
COUNT_TRAILING_ZEROS_IMPL(34);
COUNT_TRAILING_ZEROS_IMPL(33);
COUNT_TRAILING_ZEROS_IMPL(32);
COUNT_TRAILING_ZEROS_IMPL(31);
COUNT_TRAILING_ZEROS_IMPL(30);
COUNT_TRAILING_ZEROS_IMPL(29);
COUNT_TRAILING_ZEROS_IMPL(28);
COUNT_TRAILING_ZEROS_IMPL(27);
COUNT_TRAILING_ZEROS_IMPL(26);
COUNT_TRAILING_ZEROS_IMPL(25);
COUNT_TRAILING_ZEROS_IMPL(24);
COUNT_TRAILING_ZEROS_IMPL(23);
COUNT_TRAILING_ZEROS_IMPL(22);
COUNT_TRAILING_ZEROS_IMPL(21);
COUNT_TRAILING_ZEROS_IMPL(20);
COUNT_TRAILING_ZEROS_IMPL(19);
COUNT_TRAILING_ZEROS_IMPL(18);
COUNT_TRAILING_ZEROS_IMPL(17);
COUNT_TRAILING_ZEROS_IMPL(16);
COUNT_TRAILING_ZEROS_IMPL(15);
COUNT_TRAILING_ZEROS_IMPL(14);
COUNT_TRAILING_ZEROS_IMPL(13);
COUNT_TRAILING_ZEROS_IMPL(12);
COUNT_TRAILING_ZEROS_IMPL(11);
COUNT_TRAILING_ZEROS_IMPL(10);
COUNT_TRAILING_ZEROS_IMPL(9);
COUNT_TRAILING_ZEROS_IMPL(8);
COUNT_TRAILING_ZEROS_IMPL(7);
COUNT_TRAILING_ZEROS_IMPL(6);
COUNT_TRAILING_ZEROS_IMPL(5);
COUNT_TRAILING_ZEROS_IMPL(4);
COUNT_TRAILING_ZEROS_IMPL(3);
COUNT_TRAILING_ZEROS_IMPL(2);
COUNT_TRAILING_ZEROS_IMPL(1);
COUNT_TRAILING_ZEROS_IMPL(0);
#endif

template <std::size_t N>
struct countTrailingZeros {
    static constexpr std::size_t round_up_2 = is_pow2<N>::value ? N : round_up_pow2<N>::value;
    static constexpr std::size_t value = countTrailingZeros_impl<0, round_up_2>::value;
};

//////////////////////////////////////////////////////////////////////////////////

} // namespace compile_time
} // namespace jstd

#endif // JSTD_SUPPORT_POWER2_H
