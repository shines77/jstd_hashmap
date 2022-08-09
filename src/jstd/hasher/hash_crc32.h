
#ifndef JSTD_HASHER_HASH_CRC32C_H
#define JSTD_HASHER_HASH_CRC32C_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include "jstd/hasher/hashes.h"

#include <assert.h>

// Just for coding in msvc or test, please comment it in the release version.
#ifdef _MSC_VER
#ifndef __SSE4_2__
#define __SSE4_2__
#endif
#endif // _MSC_VER

#if (defined(_MSC_VER) && (_MSC_VER >= 1500)) && !defined(__clang__)
#include <intrin.h>
#endif

// defined(__GNUC__) && (__GNUC__ * 1000 + __GNUC_MINOR__ >= 4005)
#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))
#include <x86intrin.h>
#endif

//#include <nmmintrin.h>  // For SSE 4.2, _mm_popcnt_u32(), _mm_popcnt_u64()
#include "jstd/support/x86_intrin.h"

#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32)
#include <arm_acle.h>
#include <arm_neon.h>
#endif

#include "jstd/hasher/hashes.h"

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(__amd64__) || defined(__x86_64__) || defined(__LP64__)
#ifndef JSTD_IS_X86_64
#define JSTD_IS_X86_64      1
#endif
#endif // _WIN64 || __amd64__

//
// Linux shell command:
//      g++ -dM -E -x c /dev/null -march=native | grep -E "(MMX|SSE|AVX|XOP)"
//
// Linux shell command:
//      g++ -dM -E -march=native -</dev/null | grep 'MMX\|SSE\|AVX'
//
// See: http://blog.sina.com.cn/s/blog_89ff8b4b0102xcid.html
//

namespace jstd {
namespace hashes {

static const uint32_t kInitPrime32 = 0x165667C5UL;

#ifdef __SSE4_2__

static size_t intel_simple_int_crc32(size_t value)
{
#if JSTD_IS_X86_64
    uint64_t crc32 = ~uint64_t(0);
    crc32 = _mm_crc32_u64(crc32, static_cast<uint64_t>(value));
    return static_cast<size_t>(crc32);
#else
    uint32_t crc32 = ~uint32_t(0);
    crc32 = _mm_crc32_u32(crc32, static_cast<uint32_t>(value));
    return static_cast<size_t>(crc32);
#endif
}

static uint32_t intel_int_crc32_x86(uint32_t value)
{
    uint32_t crc32 = ~uint32_t(0);
    crc32 = _mm_crc32_u32(crc32, value);
    return crc32;
}

#if JSTD_IS_X86_64

static uint64_t intel_int_crc32_x64(uint64_t value)
{
    uint64_t crc32  = ~uint64_t(0);
    uint64_t crc32r =  uint64_t(0);
    crc32  = _mm_crc32_u64(crc32,  value);
    crc32r = _mm_crc32_u64(crc32r, value);
    return ((crc32 & 0x00000000FFFFFFFFull) | (crc32r << 32));
}

#endif // JSTD_IS_X86_64

static uint32_t intel_crc32_x86(const char * data, size_t length)
{
    assert(data != nullptr);

    static const ssize_t kStepSize = sizeof(uint32_t);
    static const uint32_t kMaskOne = 0xFFFFFFFFUL;
    const char * data_end = data + length;

    uint32_t crc32 = ~uint32_t(0);
    ssize_t remain = static_cast<ssize_t>(length);

    do {
        if (likely(remain >= kStepSize)) {
            assert(data < data_end);
            uint32_t data32 = *(uint32_t *)(data);
            crc32 = _mm_crc32_u32(crc32, data32);
            data += kStepSize;
            remain -= kStepSize;
        }
        else {
            assert((data_end - data) >= 0 && (data_end - data) < kStepSize);
            assert((data_end - data) == remain);
            assert(remain >= 0);
            if (likely(remain > 0)) {
                uint32_t data32 = *(uint32_t *)(data);
                uint32_t rest = (uint32_t)(kStepSize - remain);
                assert(rest > 0 && rest < (uint32_t)kStepSize);
                uint32_t mask = kMaskOne >> (rest * 8U);
                data32 &= mask;
                crc32 = _mm_crc32_u32(crc32, data32);
            }
            break;
        }
    } while (1);

    return crc32;
}

#if JSTD_IS_X86_64

static uint32_t intel_crc32_x64(const char * data, size_t length)
{
    assert(data != nullptr);

    static const ssize_t kStepSize = sizeof(uint64_t);
    static const uint64_t kMaskOne = 0xFFFFFFFFFFFFFFFFULL;
    const char * data_end = data + length;

    uint64_t crc64 = ~uint64_t(0);
    ssize_t remain = static_cast<ssize_t>(length);

    do {
        if (likely(remain >= kStepSize)) {
            assert(data < data_end);
            uint64_t data64 = *(uint64_t *)(data);
            crc64 = _mm_crc32_u64(crc64, data64);
            data += kStepSize;
            remain -= kStepSize;
        }
        else {
            assert((data_end - data) >= 0 && (data_end - data) < kStepSize);
            assert((data_end - data) == remain);
            assert(remain >= 0);
            if (likely(remain > 0)) {
                uint64_t data64 = *(uint64_t *)(data);
                size_t rest = (size_t)(kStepSize - remain);
                assert(rest > 0 && rest < (size_t)kStepSize);
                uint64_t mask = kMaskOne >> (rest * 8U);
                data64 &= mask;
                crc64 = _mm_crc32_u64(crc64, data64);
            }
            break;
        }
    } while (1);

    return static_cast<uint32_t>(crc64);
}

#endif //JSTD_IS_X86_64

static uint32_t intel_crc32_simple_x86(const char * data, size_t length)
{
    assert(data != nullptr);
    uint32_t crc32 = ~uint32_t(0);

    static const size_t kStepSize = sizeof(uint32_t);
    uint32_t * src = (uint32_t *)data;
    uint32_t * src_end = src + (length / kStepSize);

    while (likely(src < src_end)) {
        crc32 = _mm_crc32_u32(crc32, *src);
        ++src;
    }

    unsigned char * src8 = (unsigned char *)src;
    unsigned char * src8_end = (unsigned char *)(data + length);

    while (likely(src8 < src8_end)) {
        crc32 = _mm_crc32_u8(crc32, *src8);
        ++src8;
    }

    return ~crc32;
}

#if JSTD_IS_X86_64

static uint32_t intel_crc32_simple_x64(const char * data, size_t length)
{
    assert(data != nullptr);
    uint64_t crc64 = ~uint64_t(0);

    static const size_t kStepSize = sizeof(uint64_t);
    uint64_t * src = (uint64_t *)data;
    uint64_t * src_end = src + (length / kStepSize);

    while (likely(src < src_end)) {
        crc64 = _mm_crc32_u64(crc64, *src);
        ++src;
    }

    uint32_t crc32 = static_cast<uint32_t>(crc64);
    unsigned char * src8 = (unsigned char *)src;
    unsigned char * src8_end = (unsigned char *)(data + length);

    while (likely(src8 < src8_end)) {
        crc32 = _mm_crc32_u8(crc32, *src8);
        ++src8;
    }

    return ~crc32;
}

#endif // JSTD_IS_X86_64

#endif // __SSE4_2__

static uint32_t hash_crc32(const char * data, size_t length)
{
#ifdef __SSE4_2__
  #if JSTD_IS_X86_64
    return intel_crc32_x64(data, length);
  #else
    return intel_crc32_x86(data, length);
  #endif
#else
    return hashes::Times31(data, length);
#endif
}

static size_t int_hash_crc32(size_t value)
{
#ifdef __SSE4_2__
  #if JSTD_IS_X86_64
    return intel_int_crc32_x64(value);
  #else
    return intel_int_crc32_x86(value);
  #endif
#else
    return hashes::Times31(value, sizeof(value));
#endif
}

static size_t simple_int_hash_crc32(size_t value)
{
#ifdef __SSE4_2__
    return intel_simple_int_crc32(value);
#else
    return hashes::Times31(value, sizeof(value));
#endif
}

} // namespace hashes
} // namespace jstd

#endif // JSTD_HASHER_HASH_CRC32C_H
