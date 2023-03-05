
#ifndef JSTD_BITUTILS_H
#define JSTD_BITUTILS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#include "jstd/basic/stddef.h"

#if (defined(_MSC_VER) && (_MSC_VER >= 1500)) && !defined(__clang__)
#include <intrin.h>
#endif

// defined(__GNUC__) && (__GNUC__ * 1000 + __GNUC_MINOR__ >= 4005)
#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))
#include <x86intrin.h>
#endif

//#include <nmmintrin.h>  // For SSE 4.2, _mm_popcnt_u32(), _mm_popcnt_u64()
#include "jstd/support/x86_intrin.h"

#if (defined(_MSC_VER) && (_MSC_VER >= 1500)) && !defined(__clang__) // >= MSVC 2008
    #pragma intrinsic(_BitScanReverse)
    #pragma intrinsic(_BitScanForward)
    #if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64) || defined(_M_ARM64)
        #pragma intrinsic(_BitScanReverse64)
        #pragma intrinsic(_BitScanForward64)
    #endif

    #ifndef __POPCNT__
        #define __POPCNT__
    #endif
#endif // (_MSC_VER && _MSC_VER >= 1500)

namespace jstd {
namespace BitUtils {

    //
    // popcount() algorithm
    //
    // See: http://www.cnblogs.com/Martinium/articles/popcount.html
    // See: https://en.wikipedia.org/wiki/Hamming_weight
    // See: https://stackoverflow.com/questions/757059/position-of-least-significant-bit-that-is-set
    //

    static inline
    unsigned int __internal_popcnt(unsigned int x) {
        x -=  ((x >> 1U) & 0x55555555U);
        x  = (((x >> 2U) & 0x33333333U) + (x & 0x33333333U));
        x  = (((x >> 4U) + x) & 0x0F0F0F0FU);
        x +=   (x >> 8U);
        x +=   (x >> 16U);
        x  = x & 0x0000003FU;
        assert(x >= 0 && x <= 32);
        return x;
    }

    static inline
    unsigned int __internal_popcnt_slow(unsigned int x) {
        x = (x & 0x55555555UL) + ((x >>  1U) & 0x55555555UL);
        x = (x & 0x33333333UL) + ((x >>  2U) & 0x33333333UL);
        x = (x & 0x0F0F0F0FUL) + ((x >>  4U) & 0x0F0F0F0FUL);
        x = (x & 0x00FF00FFUL) + ((x >>  8U) & 0x00FF00FFUL);
        x = (x & 0x0000FFFFUL) + ((x >> 16U) & 0x0000FFFFUL);
        assert(x >= 0 && x <= 32);
        return x;
    }

    static inline
    unsigned int __internal_hakmem_popcnt(unsigned int x) {
        unsigned int tmp;
        tmp = x - ((x >> 1) & 033333333333) - ((x >> 2) & 011111111111);
        return (((tmp + (tmp >> 3)) & 030707070707) % 63U);
    }

    static inline
    unsigned int __internal_popcnt64(uint64_t x) {
    #if 1
        x -=  ((x >> 1U) & 0x55555555U);
        x  = (((x >> 2U) & 0x33333333U) + (x & 0x33333333U));
        x  = (((x >> 4U) + x) & 0x0F0F0F0FU);
        x +=   (x >> 8U);
        x +=   (x >> 16U);
        x +=   (x >> 32U);
        x  = x & 0x0000007FU;
        assert(x >= 0 && x <= 64);
        return (unsigned int)x;
    #elif 0
        x = (x & 0x5555555555555555ULL) + ((x >>  1U) & 0x5555555555555555ULL);
        x = (x & 0x3333333333333333ULL) + ((x >>  2U) & 0x3333333333333333ULL);
        x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x >>  4U) & 0x0F0F0F0F0F0F0F0FULL);
        x = (x & 0x00FF00FF00FF00FFULL) + ((x >>  8U) & 0x00FF00FF00FF00FFULL);
        x = (x & 0x0000FFFF0000FFFFULL) + ((x >> 16U) & 0x0000FFFF0000FFFFULL);
        x = (x & 0x00000000FFFFFFFFULL) + ((x >> 32U) & 0x00000000FFFFFFFFULL);
        assert(x >= 0 && x <= 64);
        return (unsigned int)x;
    #else
        unsigned int low, high;
        unsigned int n1, n2;
        low  = (unsigned int)(x & 0x00000000FFFFFFFFull);
        high = (unsigned int)(x >> 32u);
        n1 = __internal_popcnt(low);
        n2 = __internal_popcnt(high);
        return (n1 + n2);
    #endif
    }

    static inline
    int __internal_clz(unsigned int x) {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        return (int)(32u - __internal_popcnt(x));
    }

    static inline
    int __internal_clzll(uint64_t x) {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        x |= (x >> 32);
        return (int)(64u - __internal_popcnt64(x));
    }

    static inline
    int __internal_ctz(unsigned int x) {
        return (int)__internal_popcnt((x & -(int)x) - 1);
    }

    static inline
    int __internal_ctzll(uint64_t x) {
        return (int)__internal_popcnt64((x & -(int64_t)x) - 1);
    }

#if (defined(_MSC_VER) && (_MSC_VER >= 1500)) && !defined(__clang__)

    static inline
    unsigned int bsf32(unsigned int x) {
        assert(x != 0);
        unsigned long index;
        ::_BitScanForward(&index, (unsigned long)x);
        return (unsigned int)index;
    }

#if (JSTD_WORD_LEN == 64)
    static inline
    unsigned int bsf64(unsigned __int64 x) {
        assert(x != 0);
        unsigned long index;
        ::_BitScanForward64(&index, x);
        return (unsigned int)index;
    }
#else
    static inline
    unsigned int bsf64(unsigned __int64 x) {
        assert(x != 0);
        unsigned int index;
        unsigned int low = (unsigned int)(x & 0xFFFFFFFFU);
        if (low != 0) {
            index = bsf32(low) + 32;
        }
        else {
            unsigned int high = (unsigned int)(x >> 32U);
            assert(high != 0);
            index = bsf32(high);
        }
        return index;
    }
#endif

    static inline
    unsigned int bsr32(unsigned int x) {
        assert(x != 0);
        unsigned long index;
        ::_BitScanReverse(&index, (unsigned long)x);
        return (unsigned int)index;
    }

#if (JSTD_WORD_LEN == 64)
    static inline
    unsigned int bsr64(unsigned __int64 x) {
        assert(x != 0);
        unsigned long index;
        ::_BitScanReverse64(&index, x);
        return (unsigned int)index;
    }
#else
    static inline
    unsigned int bsr64(unsigned __int64 x) {
        assert(x != 0);
        unsigned int index;
        unsigned int high = (unsigned int)(x >> 32U);
        if (high != 0) {
            index = bsr32(high) + 32;
        }
        else {
            unsigned int low = (unsigned int)(x & 0xFFFFFFFFU);
            assert(low != 0);
            index = bsr32(low);
        }
        return index;
    }
#endif

#elif defined(__GNUC__) || defined(__clang__)
    // gcc support __builtin_bitscan_op_xxxx() since v3.4

    static inline
    unsigned int bsf32(unsigned int x) {
        assert(x != 0);
#if __has_builtin(__builtin_ctz)
        // gcc: __bsfd(x)
        return (unsigned int)__builtin_ctz(x);
#elif defined(__GNUC__) || defined(__clang__)
        return __bsfd(x);
#else
        return (unsigned int)BitUtils::__internal_ctz(x);
#endif
    }

#if (JSTD_WORD_LEN == 64)
    static inline
    unsigned int bsf64(uint64_t x) {
        assert(x != 0);
#if __has_builtin(__builtin_ctzll)
        // gcc: __bsfq(x)
        return (unsigned int)__builtin_ctzll((unsigned long long)x);
#elif defined(__GNUC__) || defined(__clang__)
        return __bsfq(x);
#else
        return (unsigned int)BitUtils::__internal_ctzll(x);
#endif
    }
#else
    static inline
    unsigned int bsf64(uint64_t x) {
        assert(x != 0);
        unsigned int index;
        unsigned int low = (unsigned int)(x & 0xFFFFFFFFU);
        if (low != 0) {
            index = bsf32(low) + 32;
        }
        else {
            unsigned int high = (unsigned int)(x >> 32U);
            assert(high != 0);
            index = bsf32(high);
        }
        return index;
    }
#endif

    static inline
    unsigned int bsr32(unsigned int x) {
        assert(x != 0);
#if __has_builtin(__builtin_clz)
        // gcc: __bsrd(x)
        return (unsigned int)(31 - __builtin_clz(x));
#elif defined(__GNUC__) || defined(__clang__)
        return __bsrd(x);
#else
        return (unsigned int)(31 - BitUtils::__internal_clz(x));
#endif
    }

#if (JSTD_WORD_LEN == 64)
    static inline
    unsigned int bsr64(uint64_t x) {
        assert(x != 0);
#if __has_builtin(__builtin_clzll)
        // gcc: __bsrq(x)
        return (unsigned int)(63 - __builtin_clzll((unsigned long long)x));
#elif defined(__GNUC__) || defined(__clang__)
        return __bsrq(x);
#else
        return (unsigned int)(63 - BitUtils::__internal_clzll(x));
#endif
    }
#else
    static inline
    unsigned int bsr64(uint64_t x) {
        assert(x != 0);
        unsigned int index;
        unsigned int high = (unsigned int)(x >> 32U);
        if (high != 0) {
            index = bsr32(high) + 32;
        }
        else {
            unsigned int low = (unsigned int)(x & 0xFFFFFFFFU);
            assert(low != 0);
            index = bsr32(low);
        }
        return index;
    }
#endif // (JSTD_WORD_LEN == 64)

#else // !(_MSC_VER && _MSC_VER >= 1500)

    static inline
    unsigned int bsf32(unsigned int x) {
        assert(x != 0);
        return (unsigned int)BitUtils::__internal_ctz(x);
    }

    static inline
    unsigned int bsf64(uint64_t x) {
        assert(x != 0);
        return (unsigned int)BitUtils::__internal_ctzll(x);
    }

    static inline
    unsigned int bsr32(unsigned int x) {
        assert(x != 0);
        return (unsigned int)BitUtils::__internal_clz(x);
    }

    static inline
    unsigned int bsr64(uint64_t x) {
        assert(x != 0);
        return (unsigned int)BitUtils::__internal_clzll(x);
    }

#endif // (_MSC_VER && _MSC_VER >= 1500)

    static inline
    unsigned int countTrailingZeros32(unsigned int x) {
        return bsf32(x);
    }

    static inline
    unsigned int countTrailingZeros64(uint64_t x) {
        return bsf64(x);
    }

    static inline
    unsigned int countLeadingZeros32(unsigned int x) {
        return (unsigned int)(31u - bsr32(x));
    }

    static inline
    unsigned int countLeadingZeros64(uint64_t x) {
        return (unsigned int)(63u - bsr64(x));
    }

    static inline
    unsigned int popcnt32(unsigned int x) {
#if __has_builtin(__builtin_popcount)
        return (unsigned int)__builtin_popcount(x);
#elif defined(__POPCNT__)
        int popcount = _mm_popcnt_u32(x);
        return (unsigned int)popcount;
#else
        int popcount = __internal_popcnt(x);
        return (unsigned int)popcount;
#endif
    }

#if (JSTD_WORD_LEN == 64)
    static inline
    unsigned int popcnt64(uint64_t x) {
#if __has_builtin(__builtin_popcountll)
        return (unsigned int)__builtin_popcountll((unsigned long long)x);
#elif defined(__POPCNT__)
        int64_t popcount = _mm_popcnt_u64(x);
        return (unsigned int)popcount;
#else
        unsigned int popcount = __internal_popcnt64(x);
        return popcount;
#endif
    }
#else
    static inline
    unsigned int popcnt64(uint64_t x) {
        unsigned int popcount = popcnt32((unsigned int)(x >> 32)) +
                                popcnt32((unsigned int)(x & 0xFFFFFFFFUL));
        return popcount;
    }
#endif // (JSTD_WORD_LEN == 64)

    static inline
    unsigned int bsf(size_t x) {
#if (JSTD_WORD_LEN == 64)
        return BitUtils::bsf64(x);
#else
        return BitUtils::bsf32(x);
#endif
    }

    static inline
    unsigned int bsr(size_t x) {
#if (JSTD_WORD_LEN == 64)
        return BitUtils::bsr64(x);
#else
        return BitUtils::bsr32(x);
#endif
    }

    static inline
    unsigned int countTrailingZeros(size_t x) {
#if (JSTD_WORD_LEN == 64)
        return BitUtils::countTrailingZeros64(x);
#else
        return BitUtils::countTrailingZeros32(x);
#endif
    }

    static inline
    unsigned int countLeadingZeros(size_t x) {
#if (JSTD_WORD_LEN == 64)
        return BitUtils::countLeadingZeros64(x);
#else
        return BitUtils::countLeadingZeros32(x);
#endif
    }

    static inline
    unsigned int popcnt(size_t x) {
#if (JSTD_WORD_LEN == 64)
        return BitUtils::popcnt64(x);
#else
        return BitUtils::popcnt32(x);
#endif
    }

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4146)
#endif

    //
    // The least significant 1 bit (LSB)
    //
    static inline uint32_t ls1b32(uint32_t x) {
        return (x & (uint32_t)-x);
    }

    static inline uint64_t ls1b64(uint64_t x) {
        return (x & (uint64_t)-x);
    }

    static inline size_t ls1b(size_t x) {
        return (x & (size_t)-x);
    }

    //
    // The most significant 1 bit (MSB)
    //
    static inline uint32_t ms1b32(uint32_t x) {
        return (x != 0) ? (uint32_t)(1ul << bsr32(x)) : 0;
    }

    static inline uint64_t ms1b64(uint64_t x) {
        return (x != 0) ? (uint64_t)(1ull << bsr64(x)) : 0;
    }

    static inline size_t ms1b(size_t x) {
        return (x != 0) ? ((size_t)(1u) << bsr(x)) : 0;
    }

    //
    // Clear the least significant 1 bit (LSB)
    //
    static inline uint32_t clearLowBit32(uint32_t x) {
        return (x & (uint32_t)(x - 1));
    }

    static inline uint64_t clearLowBit64(uint64_t x) {
        return (x & (uint64_t)(x - 1));
    }

    static inline size_t clearLowBit(size_t x) {
        return (x & (size_t)(x - 1));
    }

    //
    // log2_int()
    //
    static inline uint32_t log2_32(uint32_t n) {
        return (n > 1) ? (uint32_t)(bsr32(n - 1) + 1) : n;
    }

    static inline uint64_t log2_64(uint64_t n) {
        return (n > 1) ? (uint64_t)(bsr64(n - 1) + 1) : n;
    }

    static inline size_t log2_int(size_t n) {
#if (JSTD_WORD_LEN == 64)
        return log2_64(n);
#else
        return log2_32(n);
#endif
    }

#ifdef _MSC_VER
#pragma warning (pop)
#endif

} // namespace BitUtils
} // namespace jstd

#endif // JSTD_BITUTILS_H
