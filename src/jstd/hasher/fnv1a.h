
#ifndef JSTD_HASHER_FNV1A_H
#define JSTD_HASHER_FNV1A_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <assert.h>

#ifdef _MSC_VER
#ifndef __SSE4_1__
// Just for coding in msvc or test, please comment it in the release version.
#define __SSE4_1__      1
#endif
#endif

//
// See: http://isthe.com/chongo/tech/comp/fnv/
// See: http://www.sanmayce.com/Fastest_Hash/index.html
//

#ifdef __SSE2__
#include <emmintrin.h>  // For SSE2
#endif

#ifdef __SSE4_1__
#include <smmintrin.h>  // For SSE 4.1
#endif

#if defined(__AVX__) || defined(__AVX2__)
#include <immintrin.h>  // For AVX
#endif

#define _rotl_twise32(x, n)     (((x) << (n)) | ((x) >> (32 - (n))))
#define _rotl_twise128(x, n)    _mm_or_si128(_mm_slli_si128(x, (n)), _mm_srli_si128(x, (128 - (n))))

#define _xmm_load_128(p)        _mm_load_si128((__m128i const *)(p))
#define _xmm_loadu_128(p)       _mm_loadu_si128((__m128i const *)(p))

#define _xmm_extract_u32(xmm, index) \
                                ((uint32_t)_mm_extract_epi32(xmm, index))

//
// __m128i a = _mm_setr_epi32(0, 1, 2, 3);
//

namespace jstd {
namespace hashes {

//
// [North Star One-Sword School]
// - My name is Kanichiro Yoshimura.
//   I'm a new man. Just so you'll know who I am...
//   Saito-sensei.
//   What land are you from?
// - 'Land'?
//   Yes.
//   I was born in Morioka, in Nanbu, Oshu.
//   It's a beautiful place.
//   Please...
//   Away to the south is Mt Hayachine...
//   with Mt Nansho and Mt Azumane to the west.
//   In the north are Mt Iwate and Mt Himekami.
//   Out of the high mountains flows the Nakatsu River...
//   through the castle town into the Kitakami below Sakuranobaba.
//   Ah, it's pretty as a picture!
//   There's nowhere like it in all Japan!
// ...
// - Hijikata-sensei... as you're aware, the circumstances... made the task quite difficult.
//   It caused a chip in the blade of my sword.
//   Could I perhaps ask for... the cost of a new sword?
// - That should do, Your blade doesn't bear its maker's name.
// - You're too kind. My humble thanks!
// - What kind of a samurai is that?
// - He's really something!
// ...
// - Where's it chipped?
// - My sword's as worn down as I am.
// ...
// The Shinsengumi was... the sterile flower of the Shoguns' end.
// /Paragon Kiichi Nakai in the paragon piece-of-art 'The Wolves of Mibu' aka 'WHEN THE LAST SWORD IS DRAWN'/
// As I said on one Japanese forum, Kiichi Nakai deserves an award worth his weight in gold, nah-nah, in DIAMONDS!
//
static
uint32_t FNV1A_Yoshimura(const char * data, size_t data_len)
{
    static const size_t kStepSize = (2 * sizeof(uint32_t)) * 2;     // 16 bytes one time
    static const size_t kHalfStepSize = kStepSize / 2;              // 8  bytes
    static const uint32_t kPRIME = 709607;

    uint32_t hash32A = 2166136261;
    uint32_t hash32B = 2166136261;

    assert(data != nullptr);
    const char * p = data;

    // kStepSize = (2 * sizeof(uint32_t)) * 2 = 16
    if (data_len >= kStepSize) {
        size_t loop_cnt = data_len / kStepSize;
        if ((data_len % kStepSize) != 0) {
            loop_cnt++;
        }
        assert(data_len >= loop_cnt * kHalfStepSize);
        size_t line_offset = data_len - loop_cnt * kHalfStepSize;
        for (; loop_cnt > 0; loop_cnt--) {
            // revision 1:
            // hash32A = (hash32A ^ (_rotl_KAZE32(*(uint32_t *)(p + 0), 5) ^ *(uint32_t *)(p + 4))) * kPRIME;
            // hash32B = (hash32B ^ (_rotl_KAZE32(*(uint32_t *)(p + 0 + line_offset), 5) ^ *(uint32_t *)(p + 4 + line_offset))) * kPRIME;
            // revision 2:
            hash32A = (hash32A ^ (_rotl_twise32(*(uint32_t *)(p + 0), 5) ^ *(uint32_t *)(p + 0 + line_offset))) * kPRIME;
            hash32B = (hash32B ^ (_rotl_twise32(*(uint32_t *)(p + 4 + line_offset), 5) ^ *(uint32_t *)(p + 4))) * kPRIME;
            // kHalfStepSize = 2 * sizeof(uint32_t) = 8
            p += kHalfStepSize;
        }
    }
    else {
        // Cases: 0, 1, 2, 3, 4, 5, 6, 7, ..., 15
        if (data_len & (2 * sizeof(uint32_t))) {
            hash32A = (hash32A ^ *(uint32_t *)(p + 0)) * kPRIME;
            hash32B = (hash32B ^ *(uint32_t *)(p + 4)) * kPRIME;
            p += 4 * sizeof(uint16_t);
        }
        // Cases: 0, 1, 2, 3, 4, 5, 6, 7
        if (data_len & sizeof(uint32_t)) {
            hash32A = (hash32A ^ *(uint16_t *)(p + 0)) * kPRIME;
            hash32B = (hash32B ^ *(uint16_t *)(p + 2)) * kPRIME;
            p += 2 * sizeof(uint16_t);
        }
        if (data_len & sizeof(uint16_t)) {
            hash32A = (hash32A ^ *(uint16_t *)p) * kPRIME;
            p += sizeof(uint16_t);
        }
        if (data_len & sizeof(uint8_t)) {
            hash32A = (hash32A ^ *p) * kPRIME;
        }
    }

    uint32_t hash32 = (hash32A ^ _rotl_twise32(hash32B, 5)) * kPRIME;
    return (hash32 ^ (hash32 >> 16));
}

//
// FNV1A_Yoshimitsu_TRIADii_xmm revision 1+ aka FNV1A_SaberFatigue, copyleft 2013-Apr-26 Kaze.
// Targeted purpose: x-gram table lookups for Leprechaun r17.
// Targeted machine: assuming SSE2 is present always - no non-SSE2 counterpart.
//
static
uint32_t FNV1A_Yoshimitsu_TRIADii_xmm(const char * data, size_t data_len)
{
    static const size_t kStepSize = 3 * 4 * (2 * sizeof(uint32_t)); // 96 bytes one time
    static const size_t kHalfStepSize = kStepSize / 2;              // 48 bytes
    static const size_t kMinStepSize = 3 * (2 * sizeof(uint32_t));  // 24 bytes one time
    static const size_t kHalfMinStepSize = kMinStepSize / 2;        // 12 bytes
    static const uint32_t kPRIME = 709607;

    uint32_t hash32A = 2166136261UL;
    uint32_t hash32B = 2166136261UL;
    uint32_t hash32C = 2166136261UL;

    assert(data != nullptr);
    const char * p = data;

    size_t loop_cnt;
    size_t line_offset;

#if defined(__SSE2__) || defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX2__)
    __m128i xmm0;
    __m128i xmm1;
    __m128i xmm2;
    __m128i xmm3;
    __m128i xmm4;
    __m128i xmm5;
    __m128i __hash32A = _mm_set1_epi32(2166136261UL);
    __m128i __hash32B = _mm_set1_epi32(2166136261UL);
    __m128i __hash32C = _mm_set1_epi32(2166136261UL);
    __m128i __kPRIME  = _mm_set1_epi32(709607UL);
#endif

#if defined(__SSE2__) || defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX2__)
    // kStepSize = 3 * 4 * (2 * sizeof(uint32_t)) = 3 * 32 = 96
    if (data_len >= kStepSize) {
        // Actually 3 * 32 is the minimum and not useful, 200++ makes more sense.
        loop_cnt = data_len / kStepSize;
        if ((data_len % kStepSize) != 0) {
            loop_cnt++;
        }
        assert(data_len >= loop_cnt * kHalfStepSize);
        line_offset = data_len - loop_cnt * kHalfStepSize;
        for (; loop_cnt > 0; loop_cnt--) {
            xmm0 = _xmm_loadu_128(p + 0 * 16);
            xmm1 = _xmm_loadu_128(p + 0 * 16 + line_offset);
            xmm2 = _xmm_loadu_128(p + 1 * 16);
            xmm3 = _xmm_loadu_128(p + 1 * 16 + line_offset);
            xmm4 = _xmm_loadu_128(p + 2 * 16);
            xmm5 = _xmm_loadu_128(p + 2 * 16 + line_offset);
#if defined(__SSE2__)
            __hash32A = _mm_mullo_epi16(_mm_xor_si128(__hash32A, _mm_xor_si128(_rotl_twise128(xmm0, 5), xmm1)), __kPRIME);
            __hash32B = _mm_mullo_epi16(_mm_xor_si128(__hash32B, _mm_xor_si128(_rotl_twise128(xmm3, 5), xmm2)), __kPRIME);
            __hash32C = _mm_mullo_epi16(_mm_xor_si128(__hash32C, _mm_xor_si128(_rotl_twise128(xmm4, 5), xmm5)), __kPRIME);
#else
            __hash32A = _mm_mullo_epi32(_mm_xor_si128(__hash32A, _mm_xor_si128(_rotl_twise128(xmm0, 5), xmm1)), __kPRIME);
            __hash32B = _mm_mullo_epi32(_mm_xor_si128(__hash32B, _mm_xor_si128(_rotl_twise128(xmm3, 5), xmm2)), __kPRIME);
            __hash32C = _mm_mullo_epi32(_mm_xor_si128(__hash32C, _mm_xor_si128(_rotl_twise128(xmm4, 5), xmm5)), __kPRIME);
#endif // __SSE2__
            // kHalfStepSize = 3 * 2 * (2 * sizeof(uint32_t)) = 48
            p += kHalfStepSize;
        }

#if defined(__SSE2__)
        __hash32A = _mm_mullo_epi16(_mm_xor_si128(__hash32A, __hash32B), __kPRIME);
        __hash32A = _mm_mullo_epi16(_mm_xor_si128(__hash32A, __hash32C), __kPRIME);
#else
        __hash32A = _mm_mullo_epi32(_mm_xor_si128(__hash32A, __hash32B), __kPRIME);
        __hash32A = _mm_mullo_epi32(_mm_xor_si128(__hash32A, __hash32C), __kPRIME);
#endif // __SSE2__

#ifdef _MSC_VER
        hash32A = (hash32A ^ __hash32A.m128i_u32[0]) * kPRIME;
        hash32B = (hash32B ^ __hash32A.m128i_u32[3]) * kPRIME;
        hash32A = (hash32A ^ __hash32A.m128i_u32[1]) * kPRIME;
        hash32B = (hash32B ^ __hash32A.m128i_u32[2]) * kPRIME;
// #elif defined(__SSE4_1__)
        // uint32_t hash32_0 = _xmm_extract_u32(__hash32A, 0x00);
        // uint32_t hash32_1 = _xmm_extract_u32(__hash32A, 0x01);
        // uint32_t hash32_2 = _xmm_extract_u32(__hash32A, 0x02);
        // uint32_t hash32_3 = _xmm_extract_u32(__hash32A, 0x03);

        // hash32A = (hash32A ^ hash32_0) * kPRIME;
        // hash32B = (hash32B ^ hash32_3) * kPRIME;
        // hash32A = (hash32A ^ hash32_2) * kPRIME;
        // hash32B = (hash32B ^ hash32_1) * kPRIME;
#else
        alignas(16) uint32_t m128i_u32[4];
        _mm_store_si128((__m128i *)&m128i_u32[0], __hash32A);
        hash32A = (hash32A ^ m128i_u32[0]) * kPRIME;
        hash32B = (hash32B ^ m128i_u32[3]) * kPRIME;
        hash32A = (hash32A ^ m128i_u32[1]) * kPRIME;
        hash32B = (hash32B ^ m128i_u32[2]) * kPRIME;
#endif
    }
    else if (data_len >= kMinStepSize) {    // kMinStepSize = 3 * (2 * sizeof(uint32_t)) = 24
#else
    if (data_len >= kMinStepSize) {         // kMinStepSize = 3 * (2 * sizeof(uint32_t)) = 24
#endif // __SSE2__ || __SSE4_1__ || __AVX__
        loop_cnt = data_len / kMinStepSize;
        if ((data_len % kMinStepSize) != 0) {
            loop_cnt++;
        }
        // kHalfMinStepSize = 3 * sizeof(uint32_t) = 12
        assert(data_len >= loop_cnt * kHalfMinStepSize);
        line_offset = data_len - loop_cnt * kHalfMinStepSize;
        for (; loop_cnt > 0; loop_cnt--) {
            hash32A = (hash32A ^ (_rotl_twise32(*(uint32_t *)(p + 0), 5) ^ *(uint32_t *)(p + 0 + line_offset))) * kPRIME;
            hash32B = (hash32B ^ (_rotl_twise32(*(uint32_t *)(p + 4 + line_offset), 5) ^ *(uint32_t *)(p + 4))) * kPRIME;
            hash32C = (hash32C ^ (_rotl_twise32(*(uint32_t *)(p + 8), 5) ^ *(uint32_t *)(p + 8 + line_offset))) * kPRIME;
            // kHalfMinStepSize = 3 * sizeof(uint32_t) = 12
            p += kHalfMinStepSize;
        }

        // The value of hash32B is merged at the end, see below.
        hash32A = (hash32A ^ _rotl_twise32(hash32C, 5)) * kPRIME;
    }
    else {
        // Cases: 0, 1, 2, 3, 4, 5, 6, 7, ..., 31
        if (data_len & (4 * sizeof(uint32_t))) {
            hash32A = (hash32A ^ (_rotl_twise32(*(uint32_t *)(p + 0), 5) ^ *(uint32_t *)(p + 4))) * kPRIME;
            hash32B = (hash32B ^ (_rotl_twise32(*(uint32_t *)(p + 8), 5) ^ *(uint32_t *)(p + 12))) * kPRIME;
            p += 8 * sizeof(uint16_t);
        }
        // Cases: 0, 1, 2, 3, 4, 5, 6, 7, ..., 15
        if (data_len & (2 * sizeof(uint32_t))) {
            hash32A = (hash32A ^ *(uint32_t *)(p + 0)) * kPRIME;
            hash32B = (hash32B ^ *(uint32_t *)(p + 4)) * kPRIME;
            p += 4 * sizeof(uint16_t);
        }
        // Cases: 0, 1, 2, 3, 4, 5, 6, 7
        if (data_len & sizeof(uint32_t)) {
            hash32A = (hash32A ^ *(uint16_t *)(p + 0)) * kPRIME;
            hash32B = (hash32B ^ *(uint16_t *)(p + 2)) * kPRIME;
            p += 2 * sizeof(uint16_t);
        }
        if (data_len & sizeof(uint16_t)) {
            hash32A = (hash32A ^ *(uint16_t *)p) * kPRIME;
            p += sizeof(uint16_t);
        }
        if (data_len & sizeof(uint8_t)) {
            hash32A = (hash32A ^ *p) * kPRIME;
        }
    }

    uint32_t hash32 = (hash32A ^ _rotl_twise32(hash32B, 5)) * kPRIME;
    return (hash32 ^ (hash32 >> 16));
}

//
// PENUMBRA: Any partial shade or shadow round a thing; a surrounding area of uncertain extent (lit. & fig.).
//           [mod. Latin, from Latin paene almost + umbra shadow.]
//
// Many dependencies, many mini-goals, many restrictions... Blah-blah-blah...
// Yet in my amateurish view the NIFTIEST HT lookups function emerged, it is FNV1A_YoshimitsuTRIADii.
// Main feature: general purpose HT lookups function targeted as 32bit code and 32bit stamp,
// superfast for 'any length' keys, escpecially useful for text messages.
//
static
uint32_t FNV1A_penumbra(const char * data, size_t data_len)
{
    static const size_t kStepSize = 3 * 2 * 4 * (2 * sizeof(uint32_t)); // 192 bytes one time
    static const size_t kHalfStepSize = kStepSize / 2;                  // 96 bytes
    static const size_t kMinStepSize = 3 * (2 * sizeof(uint32_t));      // 24 bytes one time
    static const size_t kHalfMinStepSize = kMinStepSize / 2;            // 12 bytes
    static const uint32_t kPRIME = 709607;

    uint32_t hash32A = 2166136261UL;
    uint32_t hash32B = 2166136261UL;
    uint32_t hash32C = 2166136261UL;
    const char * p = data;
    size_t loop_cnt;
    size_t line_offset;

#if defined(__SSE2__) || defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX2__)
    __m128i xmm0;
    __m128i xmm1;
    __m128i xmm2;
    __m128i xmm3;
    __m128i xmm4;
    __m128i xmm5;
    __m128i xmm0nd;
    __m128i xmm1nd;
    __m128i xmm2nd;
    __m128i xmm3nd;
    __m128i xmm4nd;
    __m128i xmm5nd;
    __m128i __hash32A = _mm_set1_epi32(2166136261UL);
    __m128i __hash32B = _mm_set1_epi32(2166136261UL);
    __m128i __hash32C = _mm_set1_epi32(2166136261UL);
    __m128i __kPRIME  = _mm_set1_epi32(709607UL);
#endif

#if defined(__SSE2__) || defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX2__)
    // kStepSize = 3 * 2 * 4 * (2 * sizeof(uint32_t)) = 3 * 2 * 32 = 192
    if (data_len >= kStepSize) {
        // Actually 3 * 2 * 32 is the minimum and not useful, 200++ makes more sense.
        loop_cnt = data_len / kStepSize;
        if ((data_len % kStepSize) != 0) {
            loop_cnt++;
        }
        assert(data_len >= loop_cnt * kHalfStepSize);
        line_offset = data_len - loop_cnt * kHalfStepSize;
        for (; loop_cnt > 0; loop_cnt--) {
            xmm0 = _xmm_loadu_128(p + 0 * 16);
            xmm1 = _xmm_loadu_128(p + 0 * 16 + line_offset);
            xmm2 = _xmm_loadu_128(p + 1 * 16);
            xmm3 = _xmm_loadu_128(p + 1 * 16 + line_offset);
            xmm4 = _xmm_loadu_128(p + 2 * 16);
            xmm5 = _xmm_loadu_128(p + 2 * 16 + line_offset);
            xmm0nd = _xmm_loadu_128(p + 3 * 16);
            xmm1nd = _xmm_loadu_128(p + 3 * 16 + line_offset);
            xmm2nd = _xmm_loadu_128(p + 4 * 16);
            xmm3nd = _xmm_loadu_128(p + 4 * 16 + line_offset);
            xmm4nd = _xmm_loadu_128(p + 5 * 16);
            xmm5nd = _xmm_loadu_128(p + 5 * 16 + line_offset);
#if defined(__SSE2__)
            __hash32A = _mm_mullo_epi16(_mm_xor_si128(__hash32A, _mm_xor_si128(_rotl_twise128(xmm0, 5), xmm1)), __kPRIME);
            __hash32B = _mm_mullo_epi16(_mm_xor_si128(__hash32B, _mm_xor_si128(_rotl_twise128(xmm3, 5), xmm2)), __kPRIME);
            __hash32C = _mm_mullo_epi16(_mm_xor_si128(__hash32C, _mm_xor_si128(_rotl_twise128(xmm4, 5), xmm5)), __kPRIME);
            __hash32A = _mm_mullo_epi16(_mm_xor_si128(__hash32A, _mm_xor_si128(_rotl_twise128(xmm0nd, 5), xmm1nd)), __kPRIME);
            __hash32B = _mm_mullo_epi16(_mm_xor_si128(__hash32B, _mm_xor_si128(_rotl_twise128(xmm3nd, 5), xmm2nd)), __kPRIME);
            __hash32C = _mm_mullo_epi16(_mm_xor_si128(__hash32C, _mm_xor_si128(_rotl_twise128(xmm4nd, 5), xmm5nd)), __kPRIME);
#else
            __hash32A = _mm_mullo_epi32(_mm_xor_si128(__hash32A, _mm_xor_si128(_rotl_twise128(xmm0, 5), xmm1)), __kPRIME);
            __hash32B = _mm_mullo_epi32(_mm_xor_si128(__hash32B, _mm_xor_si128(_rotl_twise128(xmm3, 5), xmm2)), __kPRIME);
            __hash32C = _mm_mullo_epi32(_mm_xor_si128(__hash32C, _mm_xor_si128(_rotl_twise128(xmm4, 5), xmm5)), __kPRIME);
            __hash32A = _mm_mullo_epi32(_mm_xor_si128(__hash32A, _mm_xor_si128(_rotl_twise128(xmm0nd, 5), xmm1nd)), __kPRIME);
            __hash32B = _mm_mullo_epi32(_mm_xor_si128(__hash32B, _mm_xor_si128(_rotl_twise128(xmm3nd, 5), xmm2nd)), __kPRIME);
            __hash32C = _mm_mullo_epi32(_mm_xor_si128(__hash32C, _mm_xor_si128(_rotl_twise128(xmm4nd, 5), xmm5nd)), __kPRIME);
#endif // __SSE2__
            // kHalfStepSize = 3 * 4 * (2 * sizeof(uint32_t)) = 3 * 32 = 96
            p += kHalfStepSize;
        }

#if defined(__SSE2__)
        __hash32A = _mm_mullo_epi16(_mm_xor_si128(__hash32A, __hash32B), __kPRIME);
        __hash32A = _mm_mullo_epi16(_mm_xor_si128(__hash32A, __hash32C), __kPRIME);
#else
        __hash32A = _mm_mullo_epi32(_mm_xor_si128(__hash32A, __hash32B), __kPRIME);
        __hash32A = _mm_mullo_epi32(_mm_xor_si128(__hash32A, __hash32C), __kPRIME);
#endif // __SSE2__

#ifdef _MSC_VER
        hash32A = (hash32A ^ __hash32A.m128i_u32[0]) * kPRIME;
        hash32B = (hash32B ^ __hash32A.m128i_u32[3]) * kPRIME;
        hash32A = (hash32A ^ __hash32A.m128i_u32[1]) * kPRIME;
        hash32B = (hash32B ^ __hash32A.m128i_u32[2]) * kPRIME;
// #elif defined(__SSE4_1__)
        // uint32_t hash32_0 = _xmm_extract_u32(__hash32A, 0x00);
        // uint32_t hash32_1 = _xmm_extract_u32(__hash32A, 0x01);
        // uint32_t hash32_2 = _xmm_extract_u32(__hash32A, 0x02);
        // uint32_t hash32_3 = _xmm_extract_u32(__hash32A, 0x03);

        // hash32A = (hash32A ^ hash32_0) * kPRIME;
        // hash32B = (hash32B ^ hash32_3) * kPRIME;
        // hash32A = (hash32A ^ hash32_2) * kPRIME;
        // hash32B = (hash32B ^ hash32_1) * kPRIME;
#else
        alignas(16) uint32_t m128i_u32[4];
        _mm_store_si128((__m128i *)&m128i_u32[0], __hash32A);
        hash32A = (hash32A ^ m128i_u32[0]) * kPRIME;
        hash32B = (hash32B ^ m128i_u32[3]) * kPRIME;
        hash32A = (hash32A ^ m128i_u32[1]) * kPRIME;
        hash32B = (hash32B ^ m128i_u32[2]) * kPRIME;
#endif
    }
    else if (data_len >= kMinStepSize) {    // kMinStepSize = 3 * (2 * sizeof(uint32_t)) = 24
#else
    if (data_len >= kMinStepSize) {         // kMinStepSize = 3 * (2 * sizeof(uint32_t)) = 24
#endif // __SSE2__ || __SSE4_1__ || __AVX__
        loop_cnt = data_len / kMinStepSize;
        if ((data_len % kMinStepSize) != 0) {
            loop_cnt++;
        }
        // kHalfMinStepSize = 3 * sizeof(uint32_t) = 12
        assert(data_len >= loop_cnt * kHalfMinStepSize);
        line_offset = data_len - loop_cnt * kHalfMinStepSize;
        for (; loop_cnt > 0; loop_cnt--) {
            hash32A = (hash32A ^ (_rotl_twise32(*(uint32_t *)(p + 0), 5) ^ *(uint32_t *)(p + 0 + line_offset))) * kPRIME;
            hash32B = (hash32B ^ (_rotl_twise32(*(uint32_t *)(p + 4 + line_offset), 5) ^ *(uint32_t *)(p + 4))) * kPRIME;
            hash32C = (hash32C ^ (_rotl_twise32(*(uint32_t *)(p + 8), 5) ^ *(uint32_t *)(p + 8 + line_offset))) * kPRIME;
            // kHalfMinStepSize = 3 * sizeof(uint32_t) = 12
            p += kHalfMinStepSize;
        }

        // The value of hash32B is merged at the end, see below.
        hash32A = (hash32A ^ _rotl_twise32(hash32C, 5)) * kPRIME;
    }
    else {
        // Cases: 0, 1, 2, 3, 4, 5, 6, 7, ..., 31
        if (data_len & (4 * sizeof(uint32_t))) {
            hash32A = (hash32A ^ (_rotl_twise32(*(uint32_t *)(p + 0), 5) ^ *(uint32_t *)(p + 4))) * kPRIME;
            hash32B = (hash32B ^ (_rotl_twise32(*(uint32_t *)(p + 8), 5) ^ *(uint32_t *)(p + 12))) * kPRIME;
            p += 8 * sizeof(uint16_t);
        }
        // Cases: 0, 1, 2, 3, 4, 5, 6, 7, ..., 15
        if (data_len & (2 * sizeof(uint32_t))) {
            hash32A = (hash32A ^ *(uint32_t *)(p + 0)) * kPRIME;
            hash32B = (hash32B ^ *(uint32_t *)(p + 4)) * kPRIME;
            p += 4 * sizeof(uint16_t);
        }
        // Cases: 0, 1, 2, 3, 4, 5, 6, 7
        if (data_len & sizeof(uint32_t)) {
            hash32A = (hash32A ^ *(uint16_t *)(p + 0)) * kPRIME;
            hash32B = (hash32B ^ *(uint16_t *)(p + 2)) * kPRIME;
            p += 2 * sizeof(uint16_t);
        }
        if (data_len & sizeof(uint16_t)) {
            hash32A = (hash32A ^ *(uint16_t *)p) * kPRIME;
            p += sizeof(uint16_t);
        }
        if (data_len & sizeof(uint8_t)) {
            hash32A = (hash32A ^ *p) * kPRIME;
        }
    }

    uint32_t hash32 = (hash32A ^ _rotl_twise32(hash32B, 5)) * kPRIME;
    return (hash32 ^ (hash32 >> 16));
}

} // namespace hasher
} // namespace jstd

//#undef JSTD_IS_X86_64

#endif // JSTD_HASHER_FNV1A_H
