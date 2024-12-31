
#ifndef JSTD_STRING_UTILS_H
#define JSTD_STRING_UTILS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <string.h>
#include <wchar.h>
#include <nmmintrin.h>  // For SSE 4.2
#include <assert.h>

#include <cstdint>
#include <cstddef>      // For std::size_t
#include <string>
#include <cwchar>       // For std::wmemcpy()
#include <memory>
#include <utility>      // For std::pointer_traits<T>

#include "jstd/string/string_def.h"
#include "jstd/string/string_libc.h"
#include "jstd/string/string_stl.h"
#include "jstd/string/char_traits.h"
#include "jstd/support/SSEHelper.h"

#include "jstd/traits/type_traits.h"

namespace jstd {
namespace str_utils {

static constexpr bool kAssumeAlwaysNotEqual = true;

template <typename T,
          bool isIntegral = std::is_integral<T>::value,
          bool hasCStr = has_c_str<T, char>::value>
struct JSTD_DLL string_format {
    std::string to_string(const T & val) {
        return std::string("");
    }
};

template <typename T>
struct JSTD_DLL string_format<T, true, false> {
    std::string to_string(const T & val) {
        return std::to_string(val);
    }
};

template <typename T>
struct JSTD_DLL string_format<T, false, true> {
    std::string to_string(const T & val) {
        return std::string(val.c_str(), val.size());
    }
};

template <typename T>
struct JSTD_DLL string_format<T, false, false> {
    std::string to_string(const T & val) {
        return (std::string("0x") + std::string(std::pointer_traits<T>::pointer_to(val)));
    }
};

////////////////////////////////// mem_copy() //////////////////////////////////////

template <typename CharTy>
static inline
CharTy * mem_copy(CharTy * dest, const CharTy * src, std::size_t count)
{
    while (count > 0) {
        *dest++ = *src++;
        count--;
    }
    return dest;
}

static inline
void * mem_copy(void * dest, const void * src, std::size_t count)
{
    return std::memcpy(dest, src, count * sizeof(char));
}

static inline
char * mem_copy(char * dest, const char * src, std::size_t count)
{
    return reinterpret_cast<char *>(std::memcpy((void *)dest, (const void *)src, count * sizeof(char)));
}


static inline
wchar_t * mem_copy(wchar_t * dest, const wchar_t * src, std::size_t count)
{
    return std::wmemcpy(dest, src, count * sizeof(wchar_t));
}

////////////////////////////////// str_copy() //////////////////////////////////////

template <typename CharTy>
static inline
CharTy * str_copy(CharTy * dest, const CharTy * src, std::size_t count)
{
    typedef typename char_traits<CharTy>::uchar_type UCharTy;

    while (count > 0) {
        if (unlikely((UCharTy(*dest) & UCharTy(*src)) == 0))
            break;
        *dest++ = *src++;
        count--;
    }
    return dest;
}

#if (defined(__GNUC__) && (__GNUC__ >= 8)) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

static inline
char * str_copy(char * dest, const char * src, std::size_t count)
{
    return std::strncpy(dest, src, count);
}

static inline
wchar_t * str_copy(wchar_t * dest, const wchar_t * src, std::size_t count)
{
    return std::wcsncpy(dest, src, count);
}

#if (defined(__GNUC__) && (__GNUC__ >= 8)) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

////////////////////////////////// is_equal() //////////////////////////////////////

#if 0

template <typename CharTy>
static inline
bool is_equal(const CharTy * str1, const CharTy * str2, std::size_t length)
{
    assert(str1 != nullptr && str2 != nullptr);

    static constexpr int kMaxSize = jstd::SSEHelper<CharTy>::kMaxSize;
    static constexpr uint8_t _SIDD_CHAR_OPS = jstd::SSEHelper<CharTy>::_SIDD_CHAR_OPS;
    static constexpr uint8_t kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    if (likely(length > 0)) {
        std::ssize_t slength = (std::ssize_t)length;
        do {
            __m128i __str1 = _mm_loadu_si128((const __m128i *)str1);
            __m128i __str2 = _mm_loadu_si128((const __m128i *)str2);
            int len = ((int)slength >= kMaxSize) ? kMaxSize : (int)slength;
            assert(len > 0);

            int full_matched = _mm_cmpestrc(__str1, len, __str2, len, (uint8_t)kEqualEach);
            if (likely(full_matched == 0)) {
                // Full matched, continue match next kMaxSize bytes.
                str1 += kMaxSize;
                str2 += kMaxSize;
                slength -= kMaxSize;
            }
            else {
                // It's dismatched.
                return false;
            }
        } while (slength > 0);
    }

    // It's matched, or the length is equal 0.
    return true;
}

#elif (defined(__SSE4_2__) && (STRING_UTILS_MODE == STRING_UTILS_SSE42))

template <typename CharTy>
static inline
bool is_equal(const CharTy * str1, const CharTy * str2, std::size_t length)
{
    assert(str1 != nullptr && str2 != nullptr);

    static constexpr int kMaxSize = jstd::SSEHelper<CharTy>::kMaxSize;
    static constexpr uint8_t _SIDD_CHAR_OPS = jstd::SSEHelper<CharTy>::_SIDD_CHAR_OPS;
    static constexpr uint8_t kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    int slength = (int)length;
    while (likely(slength > 0)) {
        __m128i __str1 = _mm_loadu_si128((const __m128i *)str1);
        __m128i __str2 = _mm_loadu_si128((const __m128i *)str2);
        int len = (slength >= kMaxSize) ? kMaxSize : slength;
        assert(len > 0);

        int full_matched = _mm_cmpestrc(__str1, len, __str2, len, (uint8_t)kEqualEach);
        str1 += kMaxSize;
        str2 += kMaxSize;
        slength -= kMaxSize;
        if (likely(full_matched == 0)) {
            // Full matched, continue match next kMaxSize bytes.
            continue;
        }
        else {
            // It's dismatched.
            return false;
        }
    }

    // It's matched, or the length is equal 0.
    return true;
}

#elif (STRING_UTILS_MODE == STRING_UTILS_U64)

template <typename CharTy>
static inline
bool is_equal(const CharTy * str1, const CharTy * str2, std::size_t length)
{
    assert(str1 != nullptr && str2 != nullptr);

    static const std::size_t kMaxSize = sizeof(uint64_t);
    static const uint64_t kMaskOne64 = 0xFFFFFFFFFFFFFFFFULL;

    uint64_t * __str1 = (uint64_t * )str1;
    uint64_t * __str2 = (uint64_t * )str2;

    while (likely(length > 0)) {
        assert(__str1 != nullptr && __str2 != nullptr);
        if (likely(length >= kMaxSize)) {
            // Compare 8 bytes each time.
            uint64_t val64_1 = *__str1;
            uint64_t val64_2 = *__str2;
            if (likely(val64_1 == val64_2)) {
                // Full matched, continue match next kMaxSize bytes.
                __str1++;
                __str2++;
                length -= kMaxSize;
                continue;
            }
            else {
                // It's dismatched.
                return false;
            }
        }
        else {
            // Get the mask64
            assert(length > 0 && length < kMaxSize);
            uint32_t rest = (uint32_t)(kMaxSize - length);
            assert(rest > 0 && rest < (uint32_t)kMaxSize);
            uint64_t mask64 = kMaskOne64 >> (rest * 8U);

            // Compare the remain bytes.
            uint64_t val64_1 = *__str1;
            uint64_t val64_2 = *__str2;
            val64_1 &= mask64;
            val64_2 &= mask64;
            if (likely(val64_1 == val64_2)) {
                // Full matched, return true.
                return true;
            }
            else {
                // It's dismatched.
                return false;
            }
        }
    }

    // It's matched, or the length is equal 0.
    return true;
}

#elif (STRING_UTILS_MODE == STRING_UTILS_STL)

template <typename CharTy>
static inline
bool is_equal(const CharTy * str1, const CharTy * str2, std::size_t count)
{
    return stl::StrEqual(str1, str2, count);
}

#else

template <typename CharTy>
static inline
bool is_equal(const CharTy * str1, const CharTy * str2, std::size_t count)
{
    return libc::StrEqual(str1, str2, count);
}

#endif // STRING_UTILS_MODE

template <typename CharTy>
static inline
bool is_equal(const CharTy * str1, std::size_t len1, const CharTy * str2, std::size_t len2)
{
    if (likely(len1 != len2)) {
        // The length of str1 and str2 is different, the string must be not equal.
        return false;
    }
    else {
        if (likely(str1 != str2) || kAssumeAlwaysNotEqual) {
            return str_utils::is_equal(str1, str2, len1);
        }
        else {
            // The str1 and str2 is a same string.
            return true;
        }
    }
}

template <typename StringType>
static inline
bool is_equal_flat(const StringType & str1, const StringType & str2)
{
    assert(str1.size() == str2.size());
    return str_utils::is_equal(str1.c_str(), str2.c_str(), str1.size());
}

template <typename CharTy>
static inline
bool is_equal_unsafe(const CharTy * str1, std::size_t len1, const CharTy * str2, std::size_t len2)
{
    if (likely(len1 != len2)) {
        // The length of str1 and str2 is different, the string must be not equal.
        return false;
    }
    else {
        if (likely(str1 != str2) || kAssumeAlwaysNotEqual) {
            return str_utils::is_equal(str1, str2, len1);
        }
        else {
            // The str1 and str2 is a same string.
            return true;
        }
    }
}

template <typename StringType>
static inline
bool is_equal_unsafe(const StringType & str1, const StringType & str2)
{
    return str_utils::is_equal_unsafe(str1.c_str(), str1.size(), str2.c_str(), str2.size());
}

////////////////////// str_utils::is_equal<CharTy> - Safe //////////////////////////

template <typename CharTy>
static inline
bool is_equal_safe(const CharTy * str1, const CharTy * str2, std::size_t count)
{
    if (likely(str1 != nullptr)) {
        if (likely(str2 != nullptr)) {
            return str_utils::is_equal(str1, str2, count);
        }
        else {
            assert(str1 != nullptr && str2 == nullptr);
            return false;
        }
    }
    else {
        assert(str1 == nullptr);
        return (str2 == nullptr);
    }
}

template <typename StringType>
static inline
bool is_equal_flat_safe(const StringType & str1, const StringType & str2)
{
    assert(str1.size() == str2.size());
    return str_utils::is_equal_safe(str1.c_str(), str2.c_str(), str1.size());
}

template <typename CharTy>
static inline
bool is_equal_safe(const CharTy * str1, std::size_t len1, const CharTy * str2, std::size_t len2)
{
    if (likely(len1 != len2)) {
        // The length of str1 and str2 is different, the string must be not equal.
        return false;
    }
    else {
        if (likely(str1 != str2) || kAssumeAlwaysNotEqual) {
            return str_utils::is_equal_safe(str1, str2, len1);
        }
        else {
            // The str1 and str2 is a same string.
            return true;
        }
    }
}

template <typename StringType>
static inline
bool is_equal_safe(const StringType & str1, const StringType & str2)
{
    return str_utils::is_equal_safe(str1.c_str(), str1.size(), str2.c_str(), str2.size());
}

/////////////////////////////////  compare () //////////////////////////////////////

template <typename CharTy>
static inline
int compare(const CharTy * str1, const CharTy * str2)
{
#if (STRING_UTILS_MODE == STRING_UTILS_STL)
    return stl::StrCmp(str1, str2);
#else
    return libc::StrCmp(str1, str2);
#endif
}

template <typename CharTy>
static inline
int compare(const CharTy * str1, const CharTy * str2, std::size_t count)
{
#if (defined(__SSE4_2__) && (STRING_UTILS_MODE == STRING_UTILS_SSE42))
    assert(str1 != nullptr);
    assert(str2 != nullptr);

    static constexpr int kMaxSize = jstd::SSEHelper<CharTy>::kMaxSize;
    static constexpr uint8_t _SIDD_CHAR_OPS = jstd::SSEHelper<CharTy>::_SIDD_CHAR_OPS;
    static constexpr uint8_t kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    typedef typename jstd::char_traits<CharTy>::uchar_type UCharTy;

    int slen = (int)count;

    while (likely(slen > 0)) {
        __m128i __str1 = _mm_loadu_si128((const __m128i *)str1);
        __m128i __str2 = _mm_loadu_si128((const __m128i *)str2);
        int len = (slen >= kMaxSize) ? kMaxSize : slen;
        assert(len > 0);

        int full_matched = _mm_cmpestrc(__str1, len, __str2, len, (uint8_t)kEqualEach);
        int matched_index = _mm_cmpestri(__str1, len, __str2, len, (uint8_t)kEqualEach);
        str1 += kMaxSize;
        str2 += kMaxSize;
        slen -= kMaxSize;
        if (likely(full_matched == 0)) {
            // Full matched, continue match next kMaxSize bytes.
            assert(matched_index >= kMaxSize);
            if (slen <= 0) {
                assert((matched_index == kMaxSize) ||
                       (len == (matched_index - 1)));
            }
            continue;
        }
        else {
            // It's dismatched.
            assert(matched_index >= 0 && matched_index < kMaxSize);
            assert(matched_index <= len);
            int offset = (kMaxSize - matched_index);
            UCharTy ch1 = *((const UCharTy *)str1 - offset);
            UCharTy ch2 = *((const UCharTy *)str2 - offset);
            // The value of ch1 and ch2 must be not equal!
            assert(ch1 != ch2);
            if (ch1 > ch2)
                return CompareResult::IsBigger;
            else
                return CompareResult::IsSmaller;
        }
    }

    // It's matched, or the length is equal 0.
    return CompareResult::IsEqual;

#elif (STRING_UTILS_MODE == STRING_UTILS_STL)
    return stl::StrCmp(str1, str2, count);
#else
    return libc::StrCmp(str1, str2, count);
#endif // STRING_UTILS_MODE
}

template <typename CharTy>
static inline
int compare(const CharTy * str1, std::size_t len1, const CharTy * str2, std::size_t len2)
{
#if (defined(__SSE4_2__) && (STRING_UTILS_MODE == STRING_UTILS_SSE42))
    assert(str1 != nullptr);
    assert(str2 != nullptr);

    static constexpr int kMaxSize = jstd::SSEHelper<CharTy>::kMaxSize;
    static constexpr uint8_t _SIDD_CHAR_OPS = jstd::SSEHelper<CharTy>::_SIDD_CHAR_OPS;
    static constexpr uint8_t kEqualEach = _SIDD_CHAR_OPS | _SIDD_CMP_EQUAL_EACH
                                | _SIDD_NEGATIVE_POLARITY | _SIDD_LEAST_SIGNIFICANT;

    typedef typename jstd::char_traits<CharTy>::uchar_type UCharTy;

    std::size_t count = (len1 <= len2) ? len1 : len2;
    int slen = (int)count;

    while (likely(slen > 0)) {
        __m128i __str1 = _mm_loadu_si128((const __m128i *)str1);
        __m128i __str2 = _mm_loadu_si128((const __m128i *)str2);
        int len = (slen >= kMaxSize) ? kMaxSize : slen;
        assert(len > 0);

        int full_matched = _mm_cmpestrc(__str1, len, __str2, len, (uint8_t)kEqualEach);
        int matched_index = _mm_cmpestri(__str1, len, __str2, len, (uint8_t)kEqualEach);
        str1 += kMaxSize;
        str2 += kMaxSize;
        slen -= kMaxSize;
        if (likely(full_matched == 0)) {
            // Full matched, continue match next kMaxSize bytes.
            assert(matched_index >= kMaxSize);
            if (slen <= 0) {
                assert((matched_index == kMaxSize) ||
                       (len == (matched_index - 1)));
            }
            continue;
        }
        else {
            // It's dismatched.
            assert(matched_index >= 0 && matched_index < kMaxSize);
            assert(matched_index <= len);
            int offset = (kMaxSize - matched_index);
            UCharTy ch1 = *((const UCharTy *)str1 - offset);
            UCharTy ch2 = *((const UCharTy *)str2 - offset);
            // The value of ch1 and ch2 must be not equal!
            assert(ch1 != ch2);
            if (ch1 > ch2)
                return CompareResult::IsBigger;
            else
                return CompareResult::IsSmaller;
        }
    }

    // It's matched, or the length is equal 0.
    if (len1 > len2)
        return CompareResult::IsBigger;
    else if (len1 < len2)
        return CompareResult::IsSmaller;
    else
        return CompareResult::IsEqual;

#elif (STRING_UTILS_MODE == STRING_UTILS_STL)
    return stl::StrCmp(str1, len1, str2, len2);
#else
    return libc::StrCmp(str1, len1, str2, len2);
#endif // STRING_UTILS_MODE
}

template <typename CharTy>
static inline
int compare_unsafe(const CharTy * str1, const CharTy * str2, std::size_t count)
{
    if (likely(str1 != str2) || kAssumeAlwaysNotEqual) {
        int result = str_utils::compare(str1, str2, count);
        return result;
    } else {
        return CompareResult::IsEqual;
    }
}

template <typename CharTy>
static inline
int compare_unsafe(const CharTy * str1, std::size_t len1, const CharTy * str2, std::size_t len2)
{
    if (likely(str1 != str2) || kAssumeAlwaysNotEqual) {
        std::size_t count = (len1 <= len2) ? len1 : len2;
        int result = str_utils::compare(str1, str2, count);
        if (likely(result != CompareResult::IsEqual)) {
            return result;
        } else {
            if (likely(len1 > len2))
                return CompareResult::IsBigger;
            else if (likely(len1 < len2))
                return CompareResult::IsSmaller;
            else
                return CompareResult::IsEqual;
        }
    } else {
        assert(str1 == nullptr && str2 == nullptr && len1 != len2);
        if (likely(len1 == len2))
            return CompareResult::IsEqual;
        else if (likely(len1 > len2))
            return CompareResult::IsBigger;
        else
            return CompareResult::IsSmaller;
    }
}

template <typename StringType>
static inline
int compare_unsafe(const StringType & str1, const StringType & str2)
{
    return str_utils::compare_unsafe(str1.c_str(), str1.size(), str2.c_str(), str2.size());
}

template <typename CharTy>
static inline
int compare_safe_impl(const CharTy * str1, const CharTy * str2, std::size_t count)
{
    assert(str1 != str2);
    if (likely(str1 != nullptr)) {
        if (likely(str2 != nullptr)) {
            return str_utils::compare(str1, str2, count);
        } else {
            assert(str1 != nullptr && str2 == nullptr);
            return CompareResult::IsBigger;
        }
    } else {
        assert(str1 == nullptr && str2 != nullptr);
        return CompareResult::IsSmaller;
    }
}

template <typename CharTy>
static inline
int compare_safe(const CharTy * str1, const CharTy * str2, std::size_t count)
{
    if (likely(str1 != str2) || kAssumeAlwaysNotEqual) {
        int result = str_utils::compare_safe_impl(str1, str2, count);
        return result;
    } else {
        return CompareResult::IsEqual;
    }
}

template <typename CharTy>
static inline
int compare_safe(const CharTy * str1, std::size_t len1, const CharTy * str2, std::size_t len2)
{
    if (likely(str1 != str2) || kAssumeAlwaysNotEqual) {
        std::size_t count = (len1 <= len2) ? len1 : len2;
        int result = str_utils::compare_safe_impl(str1, str2, count);
        if (likely(result != CompareResult::IsEqual)) {
            return result;
        } else {
            if (likely(len1 > len2))
                return CompareResult::IsBigger;
            else if (likely(len1 < len2))
                return CompareResult::IsSmaller;
            else
                return CompareResult::IsEqual;
        }
    } else {
        assert(((str1 == nullptr) && (str2 == nullptr) && (len1 == len2)) ||
               ((str1 != nullptr) && (str2 != nullptr) && (str1 == str2)));
        if (likely(len1 == len2))
            return CompareResult::IsEqual;
        else if (likely(len1 > len2))
            return CompareResult::IsBigger;
        else
            return CompareResult::IsSmaller;
    }
}

template <typename StringType>
static inline
int compare_safe(const StringType & str1, const StringType & str2)
{
    return str_utils::compare_safe(str1.c_str(), str1.size(), str2.c_str(), str2.size());
}

//////////////////////////////////////////////////////////////////////////////////////

} // namespace str_utils
} // namespace jstd

#endif // JSTD_STRING_UTILS_H
