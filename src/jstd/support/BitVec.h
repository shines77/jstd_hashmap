
#ifndef JSTD_BITVEC_H
#define JSTD_BITVEC_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <bitset>
#include <cstring>          // For std::memset()
#include <type_traits>

#if defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) \
 || defined (_M_IX86) || defined(__i386__)

#if defined(_MSC_VER)

#ifndef __MMX__
#define __MMX__
#endif

#ifndef __SSE__
#define __SSE__
#endif

#ifndef __SSE2__
#define __SSE2__
#endif

#ifndef __SSE3__
#define __SSE3__
#endif

#ifndef __SSSE3__
#define __SSSE3__
#endif

#ifndef __SSE4A__
#define __SSE4A__
#endif

#ifndef __SSE4a__
#define __SSE4a__
#endif

#ifndef __SSE4_1__
#define __SSE4_1__
#endif

#ifndef __SSE4_2__
#define __SSE4_2__
#endif

#ifndef __AVX__
#define __AVX__
#endif

#ifndef __AVX2__
#define __AVX2__
#endif

//#define __AVX512BW__
//#define __AVX512VL__
//#define __AVX512F__

//#undef __SSE4_1__
//#undef __AVX2__

#endif //_MSC_VER

//#undef __AVX512VL__
//#undef __AVX512F__

/*
 * We'll support vectors targeting sse2, sse4_1, avx2, and avx512bitalg instruction sets.
 * While avx2 or avx512 will be ideal, sse4_1 should deliver solid performance. OTOH, sse2
 * performance is seriously handicapped because of our heavy reliance on fast ssse3 shuffles
 * for which there is no great sse2 alternative.
 *
 * sse2 - pentium4 2000
 *   has most of the instructions we'll use, with exceptions noted below
 *
 * ssse3 2006 - core2 2006
 *   _mm_shuffle_epi8      // sse2 alt: kind of a mess. see below.
 *
 * sse4_1 - penryn 2007
 *   _mm_testz_si128       // sse2 alt: movemask(cmpeq(setzero())) in sse2
 *   _mm_blend_epi16       // sse2 alt: &| with masks
 *   _mm_minpos_epu8
 *
 * sse4_2 - nehalem 2007
 *   _mm_cmpgt_epi64
 *
 * avx2 - haswell 2013
 *   _mm256 versions of most everything
 *
 * avx512vl - skylake 2017
 *  _mm(256)_ternarylogic_epi32
 *
 * avx512vpopcntdq, avx512bitalg - ice lake 2019
 *   _mm_popcnt_epi64
 *   _mm256_popcnt_epi16
 *
 * April 2021 Steam monthly hardware survey:
 *   SSE2        100.00%
 *   SSSE3        99.17%
 *   SSE4.1       98.80%
 *   SSE4.2       98.36%
 *   AVX          94.77%
 *   AVX2         82.28%
 */

#include "jstd/basic/stddef.h"

// For SSE2, SSE3, SSSE3, SSE 4.1, AVX, AVX2
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif //_MSC_VER

#include "jstd/support/x86_intrin.h"

/////////////////////////////////////////////

#if defined (_M_IX86) || defined(__i386__)

#ifndef _mm_setr_epi64x
#define _mm_setr_epi64x(high, low)  _mm_setr_epi64(high, low)
#endif

#else

#ifndef _mm_setr_epi64x
#define _mm_setr_epi64x(high, low)  _mm_set_epi64x(low, high)
#endif

#endif // _M_IX86 || __i386__

/////////////////////////////////////////////

#if !defined(_MSC_VER)

#ifndef _mm256_set_m128
#define _mm256_set_m128(hi, lo) \
        _mm256_insertf128_ps(_mm256_castps128_ps256(lo), (hi), 0x1)
#endif

#ifndef _mm256_set_m128d
#define _mm256_set_m128d(hi, lo) \
        _mm256_insertf128_pd(_mm256_castpd128_pd256(lo), (hi), 0x1)
#endif

#ifndef _mm256_set_m128i
#define _mm256_set_m128i(hi, lo) \
        _mm256_insertf128_si256(_mm256_castsi128_si256(lo), (hi), 0x1)
#endif

/////////////////////////////////////////////

#ifndef _mm256_setr_m128
#define _mm256_setr_m128(lo, hi)    _mm256_set_m128((hi), (lo))
#endif

#ifndef _mm256_setr_m128d
#define _mm256_setr_m128d(lo, hi)   _mm256_set_m128d((hi), (lo))
#endif

#ifndef _mm256_setr_m128i
#define _mm256_setr_m128i(lo, hi)   _mm256_set_m128i((hi), (lo))
#endif

/////////////////////////////////////////////

#ifndef _mm256_test_all_zeros
#define _mm256_test_all_zeros(mask, val) \
        _mm256_testz_si256((mask), (val))
#endif

#ifndef _mm256_test_all_ones
#define _mm256_test_all_ones(val) \
        _mm256_testc_si256((val), _mm256_cmpeq_epi32((val), (val)))
#endif

#ifndef _mm256_test_mix_ones_zeros
#define _mm256_test_mix_ones_zeros(mask, val) \
        _mm256_testnzc_si256((mask), (val))
#endif
#endif // !_MSC_VER

/////////////////////////////////////////////

#ifndef _mm_bslli_si128
#define _mm_bslli_si128 _mm_slli_si128
#endif

#ifndef _mm_bsrli_si128
#define _mm_bsrli_si128 _mm_srli_si128
#endif

/////////////////////////////////////////////

#ifndef _mm_cvtss_i32
#define _mm_cvtss_i32 _mm_cvtss_si32
#endif

#ifndef _mm_cvtsd_i32
#define _mm_cvtsd_i32 _mm_cvtsd_si32
#endif

#ifndef _mm_cvti32_sd
#define _mm_cvti32_sd _mm_cvtsi32_sd
#endif

#ifndef _mm_cvti32_ss
#define _mm_cvti32_ss _mm_cvtsi32_ss
#endif

/////////////////////////////////////////////

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)

#ifndef _mm_cvtss_i64
#define _mm_cvtss_i64 _mm_cvtss_si64
#endif

#ifndef _mm_cvtsd_i64
#define _mm_cvtsd_i64 _mm_cvtsd_si64
#endif

#ifndef _mm_cvti64_sd
#define _mm_cvti64_sd _mm_cvtsi64_sd
#endif

#ifndef _mm_cvti64_ss
#define _mm_cvti64_ss _mm_cvtsi64_ss
#endif

#endif // __amd64__

/////////////////////////////////////////////

// x64 mode have no _mm_setr_epi64()

//
// Missing in MSVC (before 2017) & gcc (before 11.0)
//
#ifndef _mm256_cvtsi256_si32
#define _mm256_cvtsi256_si32(val) \
        _mm_cvtsi128_si32(_mm256_castsi256_si128(val))
#endif // _mm256_cvtsi256_si32

/////////////////////////////////////////////

//
// Intel Intrinsics Guide (SIMD)
//
// https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
//

//
// __m128i _mm_alignr_epi8(__m128i a, __m128i b, int imm8);
//
// Concatenate 16-byte blocks in a and b into a 32-byte temporary result,
// shift the result right by imm8 bytes, and store the low 16 bytes in dst.
//
//   tmp[255:0] := ((a[127:0] << 128)[255:0] OR b[127:0]) >> (imm8*8)
//   dst[127:0] := tmp[127:0]
//

// for functions like extract below where we use switches to determine which immediate to use
// we'll assume only valid values are passed and omit the default, thereby allowing the compiler's
// assumption of defined behavior to optimize away a branch.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif

namespace jstd {

#pragma pack(push, 1)

union alignas(16) IntVec128 {
    int8_t   i8[16];
    uint8_t  u8[16];
    int16_t  i16[8];
    uint16_t u16[8];
    int32_t  i32[4];
    uint32_t u32[4];
    int64_t  i64[2];
    uint64_t u64[2];
    __m128i  m128;
};

union alignas(32) IntVec256 {
    int8_t   i8[32];
    uint8_t  u8[32];
    int16_t  i16[16];
    uint16_t u16[16];
    int32_t  i32[8];
    uint32_t u32[8];
    int64_t  i64[4];
    uint64_t u64[4];
    __m256i  m256;
};

struct IntVec2x64 {
    uint64_t u64_0;
    uint64_t u64_1;
};

struct IntVec4x64 {
    uint64_t u64_0;
    uint64_t u64_1;
    uint64_t u64_2;
    uint64_t u64_3;
};

#pragma pack(pop)

#if defined(JSTD_IS_X86_I386)

#ifndef _mm_extract_epi64
#define _mm_extract_epi64(src, index)           SSE::mm_extract_epi64<(index)>(src);
#endif

#ifndef _mm_insert_epi64
#define _mm_insert_epi64(target, index, value)  SSE::mm_insert_epi64<(index)>((src), (value));
#endif

#endif // JSTD_IS_X86_I386

//
// https://github.com/abseil/abseil-cpp/issues/209
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=87853
// _mm_cmpgt_epi8 is broken under GCC with -funsigned-char
// Work around this by using the portable implementation of Group
// when using -funsigned-char under GCC.
//
inline __m128i _mm_cmpgt_epi8_fixed(__m128i A, __m128i B) {
#if defined(__GNUC__) && !defined(__clang__)
    if (std::is_unsigned<char>::value) {
        const __m128i mask = _mm_set1_epi8(0x80);
        const __m128i diff = _mm_subs_epi8(B, A);
        return _mm_cmpeq_epi8(_mm_and_si128(diff, mask), mask);
    }
#endif
    return _mm_cmpgt_epi8(A, B);
}

inline __m128i _mm_cmplt_epi8_fixed(__m128i A, __m128i B) {
#if defined(__GNUC__) && !defined(__clang__)
    if (std::is_unsigned<char>::value) {
        const __m128i mask = _mm_set1_epi8(0x80);
        const __m128i diff = _mm_subs_epi8(A, B);
        return _mm_cmpeq_epi8(_mm_and_si128(diff, mask), mask);
    }
#endif
    return _mm_cmplt_epi8(A, B);
}

struct SSE {

static inline
uint32_t mm_cvtsi128_si32_low(__m128i m128)
{
    uint32_t low32 = _mm_cvtsi128_si32(m128);
    return (low32 & 0xFFFFUL);
}

static inline
uint32_t mm_cvtsi128_si32_high(__m128i m128)
{
    uint32_t low32 = _mm_cvtsi128_si32(m128);
    return (low32 >> 16U);
}

#if defined(JSTD_IS_X86_I386)

#if 0

#define _mm_extract_epi8(x, imm) \
    ((((imm) & 0x1) == 0) ? \
    _mm_extract_epi16((x), (imm) >> 1) & 0xff : \
    _mm_extract_epi16(_mm_srli_epi16((x), 8), (imm) >> 1))

#define _mm_extract_epi32(x, imm) \
    _mm_cvtsi128_si32(_mm_srli_si128((x), 4 * (imm)))

#endif

template <int index>
static inline
int64_t mm_extract_epi64(__m128i src)
{
    uint32_t low  = _mm_extract_epi32(src, index * 2);
    uint32_t high = _mm_extract_epi32(src, index * 2 + 1);
    uint64_t result = ((uint64_t)high << 32) | low;
    return (int64_t)result;
}

template <int index>
static inline
__m128i mm_insert_epi64(__m128i target, int64_t value)
{
    uint32_t low  = (uint32_t)((uint64_t)value & 0xFFFFFFFFul)
    uint32_t high = (uint32_t)((uint64_t)value >> 32);
    __m128i result;
    result = _mm_extract_epi32(target, low, index * 2);
    result = _mm_extract_epi32(result, high, index * 2 + 1);
    return result;
}

#endif // JSTD_IS_X86_I386

}; // SSE Wrapper

#if defined(_MSC_VER)

//
// Missing in MSVC (before 2017)
//

#ifndef _mm256_extract_epi16
#define _mm256_extract_epi16(src, index)            AVX::template mm256_extract_epi16<(index)>(src)
#endif

#ifndef _mm256_insert_epi16
#define _mm256_insert_epi16(target, value, index)   AVX::template mm256_insert_epi16<(index)>((target), (value))
#endif

#ifndef _mm256_extract_epi32
#define _mm256_extract_epi32(src, index)            AVX::template mm256_extract_epi32<(index)>(src)
#endif

#ifndef _mm256_insert_epi32
#define _mm256_insert_epi32(target, value, index)   AVX::template mm256_insert_epi32<(index)>((target), (value))
#endif

#ifndef _mm256_extract_epi64
#define _mm256_extract_epi64(src, index)            AVX::template mm256_extract_epi64<(index)>(src)
#endif

#ifndef _mm256_insert_epi64
#define _mm256_insert_epi64(target, value, index)   AVX::template mm256_insert_epi64<(index)>((target), (value))
#endif

#endif // _MSC_VER

struct AVX {

#if defined(__AVX__)

static inline
uint32_t mm256_cvtsi256_si32_low(__m256i src)
{
    return (uint32_t)(_mm256_cvtsi256_si32(src) & 0xFFFFUL);
}

static inline
uint32_t mm256_cvtsi256_si32_high(__m256i src)
{
    return (uint32_t)(_mm256_cvtsi256_si32(src) >> 16U);
}

template <int index>
static inline
int mm256_extract_epi16(__m256i src)
{
    static_assert((index >= 0 && index < 16), "AVX::mm256_extract_epi16(): index must be [0-15]");
#if defined(__AVX2__)
    if (index >= 0 && index < 8) {
        __m128i low128 = _mm256_castsi256_si128(src);
        int result = _mm_extract_epi16(low128, (index % 8));
        return result;
    }
    else if (index >= 8 && index < 16) {
        __m128i high128 = _mm256_extracti128_si256(src, (index >> 3));
        int result = _mm_extract_epi16(high128, (index % 8));
        return result;
    }
    else {
        assert(false);
    }
#elif defined(__AVX__)
    if (index >= 0 && index < 8) {
        __m128i low128 = _mm256_castsi256_si128(src);
        int result = _mm_extract_epi16(low128, (index % 8));
        return result;
    }
    else if (index >= 8 && index < 16) {
        __m128i high128 = _mm256_extractf128_si256(src, (index >> 3));
        int result = _mm_extract_epi16(high128, (index % 8));
        return result;
    }
    else {
        assert(false);
    }
#else
    // This is gcc original version
    __m128i partOfExtract = _mm256_extractf128_si256(src, (index >> 3));
    int result = _mm_extract_epi16(partOfExtract, (index % 8));
    return result;
#endif // __AVX2__
    return 0;
}

//
// See: https://agner.org/optimize/
//
// See: https://stackoverflow.com/questions/54048226/move-an-int64-t-to-the-high-quadwords-of-an-avx2-m256i-vector
// See: https://stackoverflow.com/questions/58303958/how-to-implement-16-and-32-bit-integer-insert-and-extract-operations-with-avx-51
//
template <int index>
static inline
__m256i mm256_insert_epi16(__m256i target, int value)
{
    static_assert((index >= 0 && index < 16), "AVX::mm256_insert_epi16(): index must be [0-15]");
    __m256i result;
#if defined(__AVX2__)
    if (index >= 0 && index < 8) {
#if 1
        __m128i low128 = _mm256_castsi256_si128(target);
        __m128i low_mixed128 = _mm_insert_epi16(low128, value, (index % 8));
        __m256i low_mixed256 = _mm256_castsi128_si256(low_mixed128);
        result = _mm256_blend_epi32(low_mixed256, target, 0b11110000);
#else
        //
        // There maybe is a bug because the value of the high 128 bits maybe lost.
        // But it's faster than the above version.
        //
        __m128i low128 = _mm256_castsi256_si128(target);
        __m128i result128 = _mm_insert_epi16(low128, value, (index < 8) ? index : 0);
        __m256i result256 = _mm256_castsi128_si256(result128);
        result = result256;
#endif
    }
    else if (index >= 8 && index < 16) {
        __m128i high128 = _mm256_extracti128_si256(target, (index >> 3));
        __m128i high_mixed128 = _mm_insert_epi16(high128, value, (index % 8));
        result = _mm256_inserti128_si256(target, high_mixed128, (index >> 3));
    }
    else {
        assert(false);
    }
#elif defined(__AVX__)
    if (index >= 0 && index < 8) {
        __m128i low128 = _mm256_castsi256_si128(target);
        __m128i low_mixed128 = _mm_insert_epi16(low128, value, (index % 8));
        result = _mm256_insertf128_si256(target, low_mixed128, (index >> 3));
    }
    else if (index >= 8 && index < 16) {
        __m128i high128 = _mm256_extractf128_si256(target, (index >> 3));
        __m128i high_mixed128 = _mm_insert_epi16(high128, value, (index % 8));
        result = _mm256_insertf128_si256(target, high_mixed128, (index >> 3));
    }
    else {
        assert(false);
    }
#else
    // This is gcc original version
    __m128i partOf128 = _mm256_extractf128_si256(target, (index >> 3));
    __m128i mixed128 = _mm_insert_epi16(partOf128, value, (index % 8));
    result = _mm256_insertf128_si256(target, mixed128, (index >> 3));
#endif
    return result;
}

template <int index>
static inline
__m256i mm256_insert_epi16_gcc(__m256i target, int value)
{
    __m128i partOf128 = _mm256_extractf128_si256(target, (index >> 3));
    __m128i mixed128 = _mm_insert_epi16(partOf128, value, (index % 8));
    __m256i result = _mm256_insertf128_si256(target, mixed128, (index >> 3));
    return result;
}

template <int index>
static inline
int mm256_extract_epi32(__m256i src)
{
    static_assert((index >= 0 && index < 8), "AVX::mm256_extract_epi32(): index must be [0-7]");
#if defined(__AVX__) && defined(__SSE4_1__)
    // Maybe faster than the below version
    if (index == 0) {
        __m128i m128 = _mm256_castsi256_si128(src);
        return _mm_cvtsi128_si32(m128);     // SSE2
    }
    else if (index >= 1 && index < 4) {
        __m128i m128 = _mm256_castsi256_si128(src);
        return _mm_extract_epi32(m128, index % 4);
    }
    else if (index == 4) {
        __m128i m128 = _mm256_extractf128_si256(src, index >> 2);
        return _mm_cvtsi128_si32(m128);     // SSE2
    }
    else if (index >= 5 && index < 8) {
        __m128i m128 = _mm256_extractf128_si256(src, index >> 2);
        return _mm_extract_epi32(m128, index % 4);
    }
    else {
        assert(false);
    }
#elif defined(__AVX2__)
    if (index >= 0 && index < 4) {
        __m128i low128 = _mm256_castsi256_si128(src);
        int result = _mm_extract_epi32(low128, (index % 4));
        return result;
    }
    else if (index >= 4 && index < 8) {
        __m128i high128 = _mm256_extracti128_si256(src, (index >> 2));
        int result = _mm_extract_epi32(high128, (index % 4));
        return result;
    }
    else {
        assert(false);
    }
#elif defined(__AVX__)
    if (index >= 0 && index < 4) {
        __m128i low128 = _mm256_castsi256_si128(src);
        int result = _mm_extract_epi32(low128, (index % 8));
        return result;
    }
    else if (index >= 4 && index < 8) {
        __m128i high128 = _mm256_extractf128_si256(src, (index >> 2));
        int result = _mm_extract_epi32(high128, (index % 4));
        return result;
    }
    else {
        assert(false);
    }
#else
    // This is gcc original version
    __m128i partOfExtract = _mm256_extractf128_si256(src, (index >> 2));
    int result = _mm_extract_epi32(partOfExtract, (index % 4));
    return result;
#endif // __AVX2__
    return 0;
}

template <int index>
static inline
__m256i mm256_insert_epi32(__m256i target, int64_t value)
{
    static_assert((index >= 0 && index < 8), "AVX::mm256_insert_epi32(): index must be [0-7]");
    __m256i result;
#if defined(__AVX2__)
    if (index == 0) {
        __m128i low32 = _mm_cvtsi32_si128(value);
        __m256i low256 = _mm256_castsi128_si256(low32);
        result = _mm256_blend_epi32(target, low256, 0b00000001);
    }
    else if (index >= 1 && index < 8) {
        static const int blend_mask = 0b00000001 << index;
        __m128i low32 = _mm_cvtsi32_si128(value);
        __m256i repeat256 = _mm256_broadcastq_epi64(low32);
        result = _mm256_blend_epi32(target, repeat256, blend_mask);
    }
    else {
        assert(false);
    }
#elif defined(__AVX__)
    if (index >= 0 && index < 4) {
        __m128i low128 = _mm256_castsi256_si128(target);
        __m128i low128_insert = _mm_insert_epi32(low128, value, index % 4);
        result = _mm256_insertf128_si256 (target, low128_insert, index >> 2);
    }
    else if (index >= 4 && index < 8) {
        __m128i high128 = _mm256_extractf128_si256(target, index >> 2);
        __m128i high128_insert = _mm_insert_epi32(high128, value, index % 4);
        result = _mm256_insertf128_si256 (target, high128_insert, index >> 2);
    }
    else {
        assert(false);
    }
#else
    // This is original version
    __m128i partOfInsert = _mm256_extractf128_si256(target, index >> 2);
    partOfInsert = _mm_insert_epi32(partOfInsert, value, index % 4);
    result = _mm256_insertf128_si256 (target, partOfInsert, index >> 2);
#endif
    return result;
}

#if defined(JSTD_IS_X86_64)

//
// See: /gcc/config/i386/avxintrin.h   (gcc 9.x)
//
template <int index>
static inline
int64_t mm256_extract_epi64(__m256i src)
{
    static_assert((index >= 0 && index < 4), "AVX::mm256_extract_epi64(): index must be [0-3]");
#if defined(__AVX__) && defined(__SSE4_1__)
    // Maybe faster than the below version
    if (index == 0) {
        __m128i m128 = _mm256_castsi256_si128(src);
        return _mm_cvtsi128_si64(m128);
    }
    else if (index == 1) {
        __m128i m128 = _mm256_castsi256_si128(src);
        return _mm_extract_epi64(m128, index % 2);
    }
    else if (index == 2) {
        __m128i m128 = _mm256_extractf128_si256(src, index >> 1);
        return _mm_cvtsi128_si64(m128);
    }
    else if (index == 3) {
        __m128i m128 = _mm256_extractf128_si256(src, index >> 1);
        return _mm_extract_epi64(m128, index % 2);
    }
    else {
        assert(false);
    }
#elif defined(__AVX2__) && defined(__SSE4_1__)
    if (index >= 0 && index < 2) {
        __m128i m128 = _mm256_castsi256_si128(src);
        return _mm_extract_epi64(m128, index % 2);
    }
    else (index >= 2 && index < 4) {
        __m128i m128 = _mm256_extracti128_si256(src, index >> 1);
        return _mm_extract_epi64(m128, index % 2);
    }
    else {
        assert(false);
    }
#elif defined(__AVX__) && defined(__SSE4_1__)
    if (index >= 0 && index < 2) {
        __m128i m128 = _mm256_extractf128_si256(src);
        return _mm_extract_epi64(m128, index % 2);
    }
    else (index >= 2 && index < 4) {
        __m128i m128 = _mm256_extractf128_si256(src, index >> 1);
        return _mm_extract_epi64(m128, index % 2);
    }
    else {
        assert(false);
    }
#else
    // __AVX__ && __SSE2__
    if (index >= 0 && index < 2) {
        __m128i m128 = _mm256_castsi256_si128(src);
        if (index == 1)
            m128 = _mm_srli_si128(m128, 8);
        // SSE2
        return _mm_cvtsi128_si64(m128);
    }
    else (index >= 2 && index < 4) {
        __m128i m128 = _mm256_extractf128_si256(src, index >> 1);
        if (index == 3)
            m128 = _mm_srli_si128(m128, 8);
        // SSE2
        return _mm_cvtsi128_si64(m128);
    }
    else {
        assert(false);
    }
#endif
    return 0;
}

template <int index>
static inline
__m256i mm256_insert_epi64(__m256i target, int64_t value)
{
    static_assert((index >= 0 && index < 4), "AVX::mm256_insert_epi64(): index must be [0-3]");
    __m256i result;
#if defined(__AVX2__)
    if (index == 0) {
        __m128i low64 = _mm_cvtsi64_si128(value);
        __m256i low256 = _mm256_castsi128_si256(low64);
        result = _mm256_blend_epi32(target, low256, 0b00000011);
    }
    else if (index >= 1 && index < 4) {
        static const int blend_mask = 0b00000011 << (index * 2);
        __m128i low64 = _mm_cvtsi64_si128(value);
        __m256i repeat256 = _mm256_broadcastq_epi64(low64);
        result = _mm256_blend_epi32(target, repeat256, blend_mask);
    }
    else {
        assert(false);
    }
#elif defined(__AVX__)
    if (index >= 0 && index < 2) {
        __m128i low128 = _mm256_castsi256_si128(target);
        __m128i low128_insert = _mm_insert_epi64(low128, value, index % 2);
        result = _mm256_insertf128_si256 (target, low128_insert, index >> 1);
    }
    else if (index >= 2 && index < 4) {
        __m128i high128 = _mm256_extractf128_si256(target, index >> 1);
        __m128i high128_insert = _mm_insert_epi64(high128, value, index % 2);
        result = _mm256_insertf128_si256 (target, high128_insert, index >> 1);
    }
    else {
        assert(false);
    }
#else
    // This is original version
    __m128i partOfInsert = _mm256_extractf128_si256(target, index >> 1);
    partOfInsert = _mm_insert_epi64(partOfInsert, value, index % 2);
    result = _mm256_insertf128_si256 (target, partOfInsert, index >> 1);
#endif
    return result;
}

#endif // JSTD_IS_X86_64

#if defined(JSTD_IS_X86_I386)

template <int index>
static inline
int64_t mm256_extract_epi64(__m256i src)
{
    uint32_t low  = _mm256_extract_epi32(src, index * 2);
    uint32_t high = _mm256_extract_epi32(src, index * 2 + 1);
    uint64_t result = ((uint64_t)high << 32) | low;
    return (int64_t)result;
}

template <int index>
static inline
__m256i mm256_insert_epi64(__m256i target, int64_t value)
{
    uint32_t low  = (uint32_t)((uint64_t)value & 0xFFFFFFFFul)
    uint32_t high = (uint32_t)((uint64_t)value >> 32);
    __m256i result;
    result = _mm256_insert_epi32(target, low, index * 2);
    result = _mm256_insert_epi32(result, high, index * 2 + 1);
    return result;
}

#endif // JSTD_IS_X86_I386

#endif // __AVX__

}; // AVX Wrapper

struct AVX512 {

#if !(defined(__AVX512__) && defined(__AVX512_FP16__))

static inline uint32_t mm_cvtsi128_si16(__m128i m128)
{
    uint32_t low32 = _mm_cvtsi128_si32(m128);   // SSE2
    return (low32 & 0xFFFFUL);
}

#endif

}; // AVX512 Wrapper

#if defined(__SSE2__) || defined(__SSE3__) || defined(__SSSE3__) || defined(__SSE4A__)

static inline
__m128i _mm_setones_si128()
{
    __m128i ones{};
    ones = _mm_cmpeq_epi16(ones, ones);
    return ones;
}

#endif // __SSE2__

#if defined(__AVX__) || defined(__AVX2__)

static inline
__m256i _mm256_setones_si256()
{
    __m256i ones{};
    ones = _mm256_cmpeq_epi16(ones, ones);
    return ones;
}

#endif // __AVX__

} // namespace jstd

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif // _M_X64 || __amd64__ || _M_IX86 || __i386__

#endif // JSTD_BITVEC_H
