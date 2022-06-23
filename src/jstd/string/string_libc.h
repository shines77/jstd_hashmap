
#ifndef JSTD_STRING_LIBC_H
#define JSTD_STRING_LIBC_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <string.h>
#include <wchar.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>      // For std::size_t
#include <cstring>
#include <cwchar>
#include <string>

#include "jstd/string/string_def.h"

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
// TODO:
#endif // _WIN32

namespace jstd {
namespace libc {

static constexpr bool kAssumeAlwaysNotEqual = true;

///////////////////////////////////////////////////////////////////////////////
// libc::StrLen<CharTy>()
///////////////////////////////////////////////////////////////////////////////

template <typename CharTy>
inline std::size_t StrLen(const CharTy * str) {
    assert(str != nullptr);
    const CharTy * start = str;
    while (CharTy(*str) != CharTy(0)) {
        str++;
    }
    assert(str >= start);
    return std::size_t(str - start);
}

template <>
inline std::size_t StrLen(const char * str) {
    assert(str != nullptr);
    return std::strlen(str);
}

template <>
inline std::size_t StrLen(const wchar_t * str) {
    assert(str != nullptr);
    return std::wcslen(str);
}

template <typename CharTy>
inline std::size_t StrLenSafe(const CharTy * str) {
    if (likely(str != nullptr))
        return StrLen(str);
    else
        return 0;
}

///////////////////////////////////////////////////////////////////////////////
// libc::StrEqual<CharTy>()
///////////////////////////////////////////////////////////////////////////////

template <typename CharTy>
inline
bool StrEqual(const CharTy * str1, const CharTy * str2) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    while (*str1 == *str2) {
        if (likely((*str1) != CharTy(0))) {
            str1++;
            str2++;
        } else {
            return true;
        }
    }

    return false;
}

template <>
inline
bool StrEqual(const char * str1, const char * str2) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    return (std::strcmp(str1, str2) == 0);
}

template <>
inline
bool StrEqual(const wchar_t * str1, const wchar_t * str2) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    return (std::wcscmp(str1, str2) == 0);
}

template <typename CharTy>
inline
bool StrEqual(const CharTy * str1, const CharTy * str2, std::size_t count) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    while (count != 0) {
        if (likely(*str1 != *str2)) {
            return false;
        } else {
            str1++;
            str2++;
            count--;
        }
    }

    return true;
}

template <>
inline
bool StrEqual(const char * str1, const char * str2, std::size_t count) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    return (std::strncmp(str1, str2, count) == 0);
}

template <>
inline
bool StrEqual(const wchar_t * str1, const wchar_t * str2, std::size_t count) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    return (std::wcsncmp(str1, str2, count) == 0);
}

template <typename CharTy>
inline
bool StrEqual(const CharTy * str1, std::size_t len1, const CharTy * str2, std::size_t len2) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    if (likely(len1 != len2)) {
        // The length of between str1 and str2 is not equal.
        return false;
    }
    else {
        if (likely(str1 != str2) || kAssumeAlwaysNotEqual) {
            return StrEqual(str1, str2, len1);
        }
        else {
            // The str1 and str2 is a same string.
            return true;
        }
    }
}

template <typename StringTy>
inline
bool StrEqualUnsafe(const StringTy & str1, const StringTy & str2) {
    return StrEqual(str1.data(), str1.size(), str2.data(), str2.size());
}

////////////////////////// libc::StrEqual<CharTy> - Safe //////////////////////////////

template <typename CharTy>
inline
bool StrEqualSafe(const CharTy * str1, const CharTy * str2, std::size_t count) {
    if (likely(str1 != nullptr)) {
        if (likely(str2 != nullptr)) {
            return StrEqual(str1, str2, count);
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

template <typename CharTy>
inline
bool StrEqualSafe(const CharTy * str1, std::size_t len1, const CharTy * str2, std::size_t len2) {
    if (likely(len1 != len2)) {
        // The length of str1 and str2 is different, the string must be not equal.
        return false;
    }
    else {
        if (likely(str1 != str2) || kAssumeAlwaysNotEqual) {
            return StrEqualSafe(str1, str2, len1);
        }
        else {
            // The str1 and str2 is a same string.
            return true;
        }
    }
}

template <typename StringTy>
inline
bool StrEqualSafe(const StringTy & str1, const StringTy & str2) {
    return StrEqualSafe(str1.c_str(), str1.size(), str2.c_str(), str2.size());
}

///////////////////////////////////////////////////////////////////////////////
// libc::StrCmp<CharTy>()
///////////////////////////////////////////////////////////////////////////////

template <typename CharTy>
inline
int StrCmp(const CharTy * str1, const CharTy * str2) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    while (*str1 == *str2) {
        if (likely((*str1) != CharTy(0))) {
            str1++;
            str2++;
        } else {
            return CompareResult::IsEqual;
        }
    }

    if (*str1 > *str2)
        return CompareResult::IsBigger;
    else
        return CompareResult::IsSmaller;
}

template <>
inline
int StrCmp(const char * str1, const char * str2) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    return std::strcmp(str1, str2);
}

template <>
inline
int StrCmp(const wchar_t * str1, const wchar_t * str2) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    return std::wcscmp(str1, str2);
}

template <typename CharTy>
inline
int StrCmp(const CharTy * str1, const CharTy * str2, std::size_t count) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);
    while (count != 0) {
        if (likely(*str1 != *str2)) {
            if (*str1 > *str2)
                return CompareResult::IsBigger;
            else
                return CompareResult::IsSmaller;
        } else {
            str1++;
            str2++;
            count--;
        }
    }

    return CompareResult::IsEqual;
}

template <>
inline
int StrCmp(const char * str1, const char * str2, std::size_t count) {
    return std::strncmp(str1, str2, count);
}

template <>
inline
int StrCmp(const wchar_t * str1, const wchar_t * str2, std::size_t count) {
    return std::wcsncmp(str1, str2, count);
}

template <typename CharTy>
inline
int StrCmp(const CharTy * str1, std::size_t len1, const CharTy * str2, std::size_t len2) {
    assert(str1 != nullptr);
    assert(str2 != nullptr);

    std::size_t count = (len1 <= len2) ? len1 : len2;
    int compare = StrCmp(str1, str2, count);
    if (likely(compare != 0)) {
        return compare;
    }
    else {
        if (len1 > len2)
            return CompareResult::IsBigger;
        else if (len1 < len2)
            return CompareResult::IsSmaller;
        else
            return CompareResult::IsEqual;
    }
}

template <typename StringTy>
inline
int StrCmpUnsafe(const StringTy & str1, const StringTy & str2) {
    return StrCmp(str1.data(), str1.size(), str2.data(), str2.size());
}

////////////////////////// libc::StrCmp<CharTy> - Safe /////////////////////////////

template <typename CharTy>
inline
int StrCmpSafe(const CharTy * str1, const CharTy * str2, std::size_t count) {
    if (likely(str1 != nullptr)) {
        if (likely(str2 != nullptr)) {
            return StrCmp(str1, str2, count);
        }
        else {
            assert(str1 != nullptr && str2 == nullptr);
            return CompareResult::IsBigger;
        }
    }
    else {
        assert(str1 == nullptr);
        return ((str2 != nullptr) ? CompareResult::IsSmaller : CompareResult::IsEqual);
    }
}

template <typename CharTy>
inline
int StrCmpSafe(const CharTy * str1, std::size_t len1, const CharTy * str2, std::size_t len2) {
    std::size_t count = (len1 <= len2) ? len1 : len2;
    int compare = StrCmpSafe(str1, str2, count);
    if (likely(compare != CompareResult::IsEqual)) {
        return compare;
    }
    else {
        if (len1 > len2)
            return CompareResult::IsBigger;
        else if (len1 < len2)
            return CompareResult::IsSmaller;
        else
            return CompareResult::IsEqual;
    }
}

template <typename StringTy>
inline
int StrCmpSafe(const StringTy & str1, const StringTy & str2) {
    return StrCmpSafe(str1.c_str(), str1.size(), str2.c_str(), str2.size());
}

} // namespace libc
} // namespace jstd

#endif // JSTD_STRING_LIBC_H
