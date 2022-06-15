
#ifndef JSTD_HASHER_SHA1_H
#define JSTD_HASHER_SHA1_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <assert.h>

#ifdef _MSC_VER
#include <nmmintrin.h>  // For SSE 4.2
#include <immintrin.h>  // For SHA1 & SHA256 instructions.
#include <emmintrin.h>  // For SSE 2
#else
#include <x86intrin.h>
//#include <nmmintrin.h>  // For SSE 4.2
#endif // _MSC_VER

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(__amd64__) || defined(__x86_64__) || defined(__LP64__)
#ifndef CRC32C_IS_X86_64
#define CRC32C_IS_X86_64    1
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
namespace sha1 {

static uint32_t sha1_msg2_x86(const char * data, size_t length)
{
#if JSTD_HAVE_SMID_SHA
    assert(data != nullptr);
    static const ssize_t kMaxSize = 16;
    static const uint64_t kRestMask = (uint64_t)((kMaxSize / 2) - 1);
    static const uint64_t kMaskOne = 0xFFFFFFFFFFFFFFFFULL;

    if (likely(length > 0)) {
        ssize_t remain = (ssize_t)length;

        __m128i __ones = _mm_setzero_si128();
        __m128i __msg1 = _mm_setzero_si128();
        __m128i __msg2 = _mm_setzero_si128();
        __ones = _mm_cmpeq_epi32(__ones, __ones);

        do {
            if (likely(remain <= kMaxSize)) {
                __m128i __data1 = _mm_loadu_si128((const __m128i *)(data + 0));
                __m128i __mask1 = __ones;

                uint64_t rest = (uint64_t)(kMaxSize - remain);
                if (likely(rest <= kRestMask)) {
                    __m128i __rest = _mm_set_epi64x(0, rest * 8);
                    __mask1 = _mm_srl_epi64(__mask1, __rest);
                    __mask1 = _mm_unpackhi_epi64(__ones, __mask1);
                }
                else {
                    __m128i __rest = _mm_set_epi64x(0, (rest * 8U - 64));
                    __mask1 = _mm_srl_epi64(__mask1, __rest);
                    __mask1 = _mm_move_epi64(__mask1);
                }

                __data1 = _mm_and_si128(__data1, __mask1);
                __msg1 = _mm_sha1msg2_epu32(__data1, __msg1);

                remain -= kMaxSize;
                break;
            }
            else if (likely(remain <= kMaxSize * 2)) {
                __m128i __data1 = _mm_loadu_si128((const __m128i *)(data + 0));
                __m128i __data2 = _mm_loadu_si128((const __m128i *)(data + kMaxSize));

                __m128i __mask2 = __ones;

                uint64_t rest = (uint64_t)(kMaxSize * 2 - remain);
                if (likely(rest <= kRestMask)) {
                    __m128i __rest = _mm_set_epi64x(0, rest * 8);
                    __mask2 = _mm_srl_epi64(__mask2, __rest);
                    __mask2 = _mm_unpackhi_epi64(__ones, __mask2);
                }
                else {
                    __m128i __rest = _mm_set_epi64x(0, (rest * 8U - 64));
                    __mask2 = _mm_srl_epi64(__mask2, __rest);
                    __mask2 = _mm_move_epi64(__mask2);
                }

                __data2 = _mm_and_si128(__data2, __mask2);

                __msg1 = _mm_sha1msg2_epu32(__data1, __msg1);
                __msg2 = _mm_sha1msg2_epu32(__data2, __msg2);

                remain -= kMaxSize * 2;
                break;
            }
            else {
                __m128i __data1 = _mm_loadu_si128((const __m128i *)(data + 0));
                __m128i __data2 = _mm_loadu_si128((const __m128i *)(data + kMaxSize));

                __msg1 = _mm_sha1msg2_epu32(__data1, __msg1);
                __msg2 = _mm_sha1msg2_epu32(__data2, __msg2);

                data += kMaxSize * 2;
                remain -= kMaxSize * 2;
            }
        } while (likely(remain > 0));

        __msg1 = _mm_sha1msg1_epu32(__msg1, __msg2);
        __msg1 = _mm_sha1rnds4_epu32(__msg1, __msg2, 1);
        __msg1 = _mm_shuffle_epi32(__msg1, 0x1B);

        uint32_t sha1 = _mm_cvtsi128_si32(__msg1);
        return sha1;
    }
#endif // JSTD_HAVE_SMID_SHA

    return 0;
}

static uint32_t sha1_msg2_x64(const char * data, size_t length)
{
#if JSTD_HAVE_SMID_SHA
#if CRC32C_IS_X86_64
    assert(data != nullptr);
    static const ssize_t kMaxSize = 16;
    static const uint64_t kRestMask = (uint64_t)((kMaxSize / 2) - 1);
    static const uint64_t kMaskOne = 0xFFFFFFFFFFFFFFFFULL;

    if (likely(length > 0)) {
        ssize_t remain = (ssize_t)length;

        __m128i __ones = _mm_setzero_si128();
        __m128i __msg1 = _mm_setzero_si128();
        __m128i __msg2 = _mm_setzero_si128();
        __ones = _mm_cmpeq_epi32(__ones, __ones);

        do {
            if (likely(remain <= kMaxSize)) {
                __m128i __data1 = _mm_loadu_si128((const __m128i *)(data + 0));
#if 1
                __m128i __mask1;

                uint64_t mask1 = kMaskOne;
                uint64_t rest = (uint64_t)(kMaxSize - remain);
                if (likely(rest <= kRestMask)) {
                    mask1 = mask1 >> (rest * 8U);
                    //__mask1 = _mm_cvtsi64_si128(mask1);
                    __mask1 = _mm_set_epi64x(mask1, kMaskOne);
                }
                else if (likely(rest < kMaxSize)) {
                    mask1 = mask1 >> ((rest & kRestMask) * 8U);
                    __mask1 = _mm_set_epi64x(0, mask1);
                }
                else {
                    __mask1 = __ones;
                }
#else
                __m128i __mask1 = __ones;

                uint64_t rest = (uint64_t)(kMaxSize - remain);
                if (likely(rest <= kRestMask)) {
                    __m128i __rest = _mm_set_epi64x(0, rest * 8);
                    //__m128i __rest = _mm_cvtsi32_si128((int)(rest * 8));
                    __mask1 = _mm_srl_epi64(__mask1, __rest);
                    __mask1 = _mm_unpackhi_epi64(__ones, __mask1);

                    //__m128d __mask2d = _mm_shuffle_pd(*(__m128d *)&__ones, *(__m128d *)&__mask1, 0b10);
                    //__data1 = _mm_and_si128(__data1, *(__m128i *)&__mask1d);
                }
                else {
                    __m128i __rest = _mm_set_epi64x(0, (rest * 8U - 64));
                    //__m128i __rest = _mm_cvtsi32_si128((int)(rest * 8U - 64));
                    __mask1 = _mm_srl_epi64(__mask1, __rest);
                    __mask1 = _mm_move_epi64(__mask1);
                }
#endif
                __data1 = _mm_and_si128(__data1, __mask1);
                __msg1 = _mm_sha1msg2_epu32(__data1, __msg1);

                remain -= kMaxSize;
                break;
            }
            else if (likely(remain <= kMaxSize * 2)) {
                __m128i __data1 = _mm_loadu_si128((const __m128i *)(data + 0));
                __m128i __data2 = _mm_loadu_si128((const __m128i *)(data + kMaxSize));
#if 1
                __m128i __mask2;

                uint64_t mask2 = kMaskOne;
                uint64_t rest = (uint64_t)(kMaxSize * 2 - remain);
                if (likely(rest <= kRestMask)) {
                    mask2 = mask2 >> (rest * 8U);
                    //__mask1 = _mm_cvtsi64_si128(mask2);
                    __mask2 = _mm_set_epi64x(mask2, kMaskOne);
                }
                else if (likely(rest < kMaxSize)) {
                    mask2 = mask2 >> ((rest & kRestMask) * 8U);
                    __mask2 = _mm_set_epi64x(0, mask2);
                }
                else {
                    __mask2 = __ones;
                }
#else
                __m128i __mask2 = __ones;

                uint64_t rest = (uint64_t)(kMaxSize * 2 - remain);
                if (likely(rest <= kRestMask)) {
                    __m128i __rest = _mm_set_epi64x(0, rest * 8);
                    //__m128i __rest = _mm_cvtsi32_si128((int)(rest * 8));
                    __mask2 = _mm_srl_epi64(__mask2, __rest);
                    __mask2 = _mm_unpackhi_epi64(__ones, __mask2);

                    //__m128d __mask2d = _mm_shuffle_pd(*(__m128d *)&__ones, *(__m128d *)&__mask2, 0b10);
                    //__data2 = _mm_and_si128(__data2, *(__m128i *)&__mask2d);
                }
                else {
                    __m128i __rest = _mm_set_epi64x(0, (rest * 8U - 64));
                    //__m128i __rest = _mm_cvtsi32_si128((int)(rest * 8U - 64));
                    __mask2 = _mm_srl_epi64(__mask2, __rest);
                    __mask2 = _mm_move_epi64(__mask2);
                }
#endif
                __data2 = _mm_and_si128(__data2, __mask2);

                __msg1 = _mm_sha1msg2_epu32(__data1, __msg1);
                __msg2 = _mm_sha1msg2_epu32(__data2, __msg2);

                remain -= kMaxSize * 2;
                break;
            }
            else {
                __m128i __data1 = _mm_loadu_si128((const __m128i *)(data + 0));
                __m128i __data2 = _mm_loadu_si128((const __m128i *)(data + kMaxSize));

                __msg1 = _mm_sha1msg2_epu32(__data1, __msg1);
                __msg2 = _mm_sha1msg2_epu32(__data2, __msg2);

                data += kMaxSize * 2;
                remain -= kMaxSize * 2;
            }
        } while (likely(remain > 0));

        __msg1 = _mm_sha1msg1_epu32(__msg1, __msg2);
        __msg1 = _mm_sha1rnds4_epu32(__msg1, __msg2, 1);
        __msg1 = _mm_shuffle_epi32(__msg1, 0x1B);

        uint64_t sha1 = _mm_cvtsi128_si64(__msg1);
        sha1 = (sha1 >> 32) ^ (sha1 & 0xFFFFFFFFU);
        return (uint32_t)sha1;
    }
#endif // CRC32C_IS_X86_64
#endif // JSTD_HAVE_SMID_SHA

    return 0;
}

static uint32_t sha1_msg2(const char * data, size_t length)
{
#if CRC32C_IS_X86_64
    return sha1_msg2_x64(data, length);
#else
    return sha1_msg2_x86(data, length);
#endif // CRC32C_IS_X86_64
}

//
// See: https://github.com/noloader/SHA-Intrinsics/blob/master/sha1-x86.c
//

/* initial state */
alignas(16)
static uint32_t s_sha1_state[5] = { 0x67452301U, 0xEFCDAB89U, 0x98BADCFEU, 0x10325476U, 0xC3D2E1F0U };

/* Process multiple blocks. The caller is responsible for setting the initial */
/*  state, and the caller is responsible for padding the final block.        */

static uint32_t sha1_x86(uint32_t state[5], const char * data, size_t length)
{
#if JSTD_HAVE_SMID_SHA
    __m128i ABCD, ABCD_SAVE, E0, E0_SAVE, E1;
    __m128i MSG0, MSG1, MSG2, MSG3;
    static const __m128i MASK = _mm_set_epi64x(0x0001020304050607ULL, 0x08090a0b0c0d0e0fULL);
    alignas(16) char new_data[64];

    /* Load initial values */
    ABCD = _mm_load_si128((const __m128i *)state);
    E0 = _mm_set_epi32(state[4], 0, 0, 0);
    ABCD = _mm_shuffle_epi32(ABCD, 0x1B);

    ssize_t remain = (ssize_t)length;

    while (likely(remain > 0)) {
        /* Save current state  */
        ABCD_SAVE = ABCD;
        E0_SAVE = E0;

        /* Rounds 0-3 */
        if (likely(remain < 16)) {
            memcpy((void *)(new_data + 0), (const void *)(data + 0), remain * sizeof(char));
            memset((void *)(new_data + remain), 0, (16 - remain) * sizeof(char));
            MSG0 = _mm_load_si128((const __m128i *)(new_data + 0));
        }
        else {
            MSG0 = _mm_loadu_si128((const __m128i *)(data + 0));
        }
        MSG0 = _mm_shuffle_epi8(MSG0, MASK);
        E0 = _mm_add_epi32(E0, MSG0);
        E1 = ABCD;
        ABCD = _mm_sha1rnds4_epu32(ABCD, E0, 0);

        /* Rounds 4-7 */
        if (likely(remain <= 16)) {
            MSG1 = _mm_setzero_si128();
        }
        else if (likely(remain < 32)) {
            memcpy((void *)(new_data + 16), (const void *)(data + 16), (remain - 16) * sizeof(char));
            memset((void *)(new_data + remain), 0, (32 - remain) * sizeof(char));
            MSG1 = _mm_load_si128((const __m128i *)(new_data + 16));
        }
        else {
            MSG1 = _mm_loadu_si128((const __m128i *)(data + 16));
        }
        MSG1 = _mm_shuffle_epi8(MSG1, MASK);
        E1 = _mm_sha1nexte_epu32(E1, MSG1);
        E0 = ABCD;
        ABCD = _mm_sha1rnds4_epu32(ABCD, E1, 0);
        MSG0 = _mm_sha1msg1_epu32(MSG0, MSG1);

        /* Rounds 8-11 */
        if (likely(remain <= 32)) {
            MSG2 = _mm_setzero_si128();
        }
        else if (likely(remain < 48)) {
            memcpy((void *)(new_data + 32), (const void *)(data + 32), (remain - 32) * sizeof(char));
            memset((void *)(new_data + remain), 0, (48 - remain) * sizeof(char));
            MSG2 = _mm_load_si128((const __m128i *)(new_data + 32));
        }
        else {
            MSG2 = _mm_loadu_si128((const __m128i *)(data + 32));
        }
        MSG2 = _mm_shuffle_epi8(MSG2, MASK);
        E0 = _mm_sha1nexte_epu32(E0, MSG2);
        E1 = ABCD;
        ABCD = _mm_sha1rnds4_epu32(ABCD, E0, 0);
        MSG1 = _mm_sha1msg1_epu32(MSG1, MSG2);
        MSG0 = _mm_xor_si128(MSG0, MSG2);

        /* Rounds 12-15 */
        if (likely(remain <= 48)) {
            MSG3 = _mm_setzero_si128();
        }
        else if (likely(remain < 64)) {
            memcpy((void *)(new_data + 48), (const void *)(data + 48), (remain - 48) * sizeof(char));
            memset((void *)(new_data + remain), 0, (64 - remain) * sizeof(char));
            MSG3 = _mm_load_si128((const __m128i *)(new_data + 48));
        }
        else {
            MSG3 = _mm_loadu_si128((const __m128i *)(data + 48));
        }
        MSG3 = _mm_shuffle_epi8(MSG3, MASK);
        E1 = _mm_sha1nexte_epu32(E1, MSG3);
        E0 = ABCD;
        MSG0 = _mm_sha1msg2_epu32(MSG0, MSG3);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E1, 0);
        MSG2 = _mm_sha1msg1_epu32(MSG2, MSG3);
        MSG1 = _mm_xor_si128(MSG1, MSG3);

        /* Rounds 16-19 */
        E0 = _mm_sha1nexte_epu32(E0, MSG0);
        E1 = ABCD;
        MSG1 = _mm_sha1msg2_epu32(MSG1, MSG0);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E0, 0);
        MSG3 = _mm_sha1msg1_epu32(MSG3, MSG0);
        MSG2 = _mm_xor_si128(MSG2, MSG0);

        /* Rounds 20-23 */
        E1 = _mm_sha1nexte_epu32(E1, MSG1);
        E0 = ABCD;
        MSG2 = _mm_sha1msg2_epu32(MSG2, MSG1);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E1, 1);
        MSG0 = _mm_sha1msg1_epu32(MSG0, MSG1);
        MSG3 = _mm_xor_si128(MSG3, MSG1);

        /* Rounds 24-27 */
        E0 = _mm_sha1nexte_epu32(E0, MSG2);
        E1 = ABCD;
        MSG3 = _mm_sha1msg2_epu32(MSG3, MSG2);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E0, 1);
        MSG1 = _mm_sha1msg1_epu32(MSG1, MSG2);
        MSG0 = _mm_xor_si128(MSG0, MSG2);

        /* Rounds 28-31 */
        E1 = _mm_sha1nexte_epu32(E1, MSG3);
        E0 = ABCD;
        MSG0 = _mm_sha1msg2_epu32(MSG0, MSG3);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E1, 1);
        MSG2 = _mm_sha1msg1_epu32(MSG2, MSG3);
        MSG1 = _mm_xor_si128(MSG1, MSG3);

        /* Rounds 32-35 */
        E0 = _mm_sha1nexte_epu32(E0, MSG0);
        E1 = ABCD;
        MSG1 = _mm_sha1msg2_epu32(MSG1, MSG0);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E0, 1);
        MSG3 = _mm_sha1msg1_epu32(MSG3, MSG0);
        MSG2 = _mm_xor_si128(MSG2, MSG0);

        /* Rounds 36-39 */
        E1 = _mm_sha1nexte_epu32(E1, MSG1);
        E0 = ABCD;
        MSG2 = _mm_sha1msg2_epu32(MSG2, MSG1);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E1, 1);
        MSG0 = _mm_sha1msg1_epu32(MSG0, MSG1);
        MSG3 = _mm_xor_si128(MSG3, MSG1);

        /* Rounds 40-43 */
        E0 = _mm_sha1nexte_epu32(E0, MSG2);
        E1 = ABCD;
        MSG3 = _mm_sha1msg2_epu32(MSG3, MSG2);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E0, 2);
        MSG1 = _mm_sha1msg1_epu32(MSG1, MSG2);
        MSG0 = _mm_xor_si128(MSG0, MSG2);

        /* Rounds 44-47 */
        E1 = _mm_sha1nexte_epu32(E1, MSG3);
        E0 = ABCD;
        MSG0 = _mm_sha1msg2_epu32(MSG0, MSG3);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E1, 2);
        MSG2 = _mm_sha1msg1_epu32(MSG2, MSG3);
        MSG1 = _mm_xor_si128(MSG1, MSG3);

        /* Rounds 48-51 */
        E0 = _mm_sha1nexte_epu32(E0, MSG0);
        E1 = ABCD;
        MSG1 = _mm_sha1msg2_epu32(MSG1, MSG0);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E0, 2);
        MSG3 = _mm_sha1msg1_epu32(MSG3, MSG0);
        MSG2 = _mm_xor_si128(MSG2, MSG0);

        /* Rounds 52-55 */
        E1 = _mm_sha1nexte_epu32(E1, MSG1);
        E0 = ABCD;
        MSG2 = _mm_sha1msg2_epu32(MSG2, MSG1);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E1, 2);
        MSG0 = _mm_sha1msg1_epu32(MSG0, MSG1);
        MSG3 = _mm_xor_si128(MSG3, MSG1);

        /* Rounds 56-59 */
        E0 = _mm_sha1nexte_epu32(E0, MSG2);
        E1 = ABCD;
        MSG3 = _mm_sha1msg2_epu32(MSG3, MSG2);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E0, 2);
        MSG1 = _mm_sha1msg1_epu32(MSG1, MSG2);
        MSG0 = _mm_xor_si128(MSG0, MSG2);

        /* Rounds 60-63 */
        E1 = _mm_sha1nexte_epu32(E1, MSG3);
        E0 = ABCD;
        MSG0 = _mm_sha1msg2_epu32(MSG0, MSG3);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E1, 3);
        MSG2 = _mm_sha1msg1_epu32(MSG2, MSG3);
        MSG1 = _mm_xor_si128(MSG1, MSG3);

        /* Rounds 64-67 */
        E0 = _mm_sha1nexte_epu32(E0, MSG0);
        E1 = ABCD;
        MSG1 = _mm_sha1msg2_epu32(MSG1, MSG0);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E0, 3);
        MSG3 = _mm_sha1msg1_epu32(MSG3, MSG0);
        MSG2 = _mm_xor_si128(MSG2, MSG0);

        /* Rounds 68-71 */
        E1 = _mm_sha1nexte_epu32(E1, MSG1);
        E0 = ABCD;
        MSG2 = _mm_sha1msg2_epu32(MSG2, MSG1);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E1, 3);
        MSG3 = _mm_xor_si128(MSG3, MSG1);

        /* Rounds 72-75 */
        E0 = _mm_sha1nexte_epu32(E0, MSG2);
        E1 = ABCD;
        MSG3 = _mm_sha1msg2_epu32(MSG3, MSG2);
        ABCD = _mm_sha1rnds4_epu32(ABCD, E0, 3);

        /* Rounds 76-79 */
        E1 = _mm_sha1nexte_epu32(E1, MSG3);
        E0 = ABCD;
        ABCD = _mm_sha1rnds4_epu32(ABCD, E1, 3);

        /* Combine state */
        E0 = _mm_sha1nexte_epu32(E0, E0_SAVE);
        ABCD = _mm_add_epi32(ABCD, ABCD_SAVE);

        data += 64;
        remain -= 64;
    }

    /* Save state */
    ABCD = _mm_shuffle_epi32(ABCD, 0x1B);
    //_mm_storeu_si128((__m128i *)state, ABCD);
    //state[4] = _mm_extract_epi32(E0, 3);

#if CRC32C_IS_X86_64 & 1
    uint64_t sha1 = _mm_cvtsi128_si64(ABCD);
    sha1 = (sha1 >> 32) ^ (sha1 & 0xFFFFFFFFU);
    return (uint32_t)sha1;

    //uint32_t sha1 = state[0] ^ state[1];
    //uint32_t sha1 = state[0];
#else
    uint32_t sha1 = _mm_cvtsi128_si32(ABCD);
    return sha1;
#endif // CRC32C_IS_X86_64

#else
    return 0;
#endif // JSTD_HAVE_SMID_SHA
}

static uint32_t sha1_x86(const char * data, size_t length)
{
    return sha1_x86(s_sha1_state, data, length);
}

} // namespace sha1
} // namespace jstd

#endif // JSTD_HASHER_SHA1_H
