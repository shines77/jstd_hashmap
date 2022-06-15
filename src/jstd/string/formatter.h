
#ifndef JSTD_STRING_FORMATTER_H
#define JSTD_STRING_FORMATTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <memory.h>
#include <string.h>
#ifdef _MSC_VER
#include <tchar.h>
#endif
#include <assert.h>

#include <cstdint>
#include <cstddef>

#include <cstring>
#include <string>
#include <list>
#include <vector>
#include <exception>
#include <stdexcept>
#include <type_traits>

#include "jstd/string/char_traits.h"
#include "jstd/string/string_view.h"
#include "jstd/math/log10_int.h"

namespace jstd {

static const uint8_t HexStrs [16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static const uint8_t LowerHexStrs [16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

enum sprintf_error_code {
    Sprintf_Error_First = -16384,
    Sprintf_BadFormat_BrokenEscapeChar,
    Sprintf_BadFormat_UnknownSpecifier_FF,
    Sprintf_BadFormat_UnknownSpecifier,

    Sprintf_InvalidArgmument_MissingParameter,
    Sprintf_InvalidArgmument_ErrorFloatType,

    Sprintf_Success = 0,
    Sprintf_Reached_Endof,
    Sprintf_Error_Last
};

template <typename CharTy>
JSTD_FORCED_INLINE
static CharTy to_hex_char(CharTy hex) {
    return ((hex >= 10) ? (hex - 10 + CharTy('A')) : (hex + CharTy('0')));
}

template <typename CharTy>
JSTD_FORCED_INLINE
static CharTy to_lower_hex_char(CharTy hex) {
    return ((hex >= 10) ? (hex - 10 + CharTy('a')) : (hex + CharTy('0')));
}

template <typename CharTy>
static std::string to_hex(CharTy ch) {
    typedef typename make_unsigned_char<CharTy>::type UCharTy;
    UCharTy high = UCharTy(ch) / 16;
    std::string hex(to_hex_char(high), 1);
    UCharTy low = UCharTy(ch) % 16;
    hex += to_hex_char(low);
    return hex;
}

template <typename CharTy>
static std::string to_lower_hex(CharTy ch) {
    typedef typename make_unsigned_char<CharTy>::type UCharTy;
    UCharTy high = UCharTy(ch) / 16;
    std::string hex(to_lower_hex_char(high), 1);
    UCharTy low = UCharTy(ch) % 16;
    hex += to_lower_hex_char(low);
    return hex;
}

template <std::ssize_t delta = 0>
inline
std::ssize_t count_digits(bool bl1) {
    return 1;
}

template <std::ssize_t delta = 0>
inline
std::ssize_t count_digits(uint8_t u8) {
    if (u8 < 10)
        return (1 - delta);
    else if (u8 < 100)
        return (2 - delta);
    else
        return (3 - delta);
}

template <std::ssize_t delta = 0>
inline
std::ssize_t count_digits(int8_t i8) {
    uint8_t u8;
    if (i8 >= 0)
        u8 = static_cast<uint8_t>(i8);
    else
        u8 = static_cast<uint8_t>(-i8);
    return std::ssize_t(i8 < 0) + count_digits<delta>(u8);
}

template <std::ssize_t delta = 0>
inline
std::ssize_t count_digits(uint16_t u16) {
    if (u16 < 10)
        return (1 - delta);
    else if (u16 < 100)
        return (2 - delta);
    else
        return (3 - delta);
}

template <std::ssize_t delta = 0>
inline
std::ssize_t count_digits(int16_t i16) {
    uint16_t u16;
    if (i16 >= 0)
        u16 = static_cast<uint16_t>(i16);
    else
        u16 = static_cast<uint16_t>(-i16);
    return std::ssize_t(i16 < 0) + count_digits<delta>(u16);
}

template <std::ssize_t delta = 0>
inline
std::ssize_t count_digits(uint32_t u32) {
#if 1
    return (std::ssize_t(jm_log10_u32(u32) + 1) - delta);
#else
    if (u32 < 100000000UL) {
        if (u32 < 10000UL) {
            if (u32 < 100UL) {
                if (u32 < 10UL)
                    return (1 - delta);
                else
                    return (2 - delta);
            }
            else {
                if (u32 < 1000UL)
                    return (3 - delta);
                else
                    return (4 - delta);
            }
        }
        else {
            if (u32 < 1000000UL) {
                if (u32 < 100000UL)
                    return (5 - delta);
                else
                    return (6 - delta);
            }
            else {
                if (u32 < 10000000UL)
                    return (7 - delta);
                else
                    return (8 - delta);
            }
        }
    }
    else {
        if (u32 < 1000000000UL)
            return (9 - delta);
        else
            return (10 - delta);
    }
#endif
}

template <std::ssize_t delta = 0>
inline
std::ssize_t count_digits(int32_t i32) {
    uint32_t u32;
    if (i32 >= 0) {
        u32 = static_cast<uint32_t>(i32);
    }
    else {
        u32 = static_cast<uint32_t>(-i32);
    }
    return std::ssize_t(i32 < 0) + count_digits<delta>(u32);
}

template <std::ssize_t delta = 0>
inline
std::ssize_t count_digits(uint64_t u64) {
#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
    return (std::ssize_t(jm_log10_u64(u64) + 1) - delta);
#else
    if (u64 < 100000000UL) {
        if (u64 < 10000UL) {
            if (u64 < 100UL) {
                if (u64 < 10UL)
                    return (1 - delta);
                else
                    return (2 - delta);
            }
            else {
                if (u64 < 1000UL)
                    return (3 - delta);
                else
                    return (4 - delta);
            }
        }
        else {
            if (u64 < 1000000UL) {
                if (u64 < 100000UL)
                    return (5 - delta);
                else
                    return (6 - delta);
            }
            else {
                if (u64 < 10000000UL)
                    return (7 - delta);
                else
                    return (8 - delta);
            }
        }
    }
    else if (u64 < 10000000000000000ULL) {
        if (u64 < 1000000000000ULL) {
            if (u64 < 10000000000ULL) {
                if (u64 < 1000000000ULL)
                    return (9 - delta);
                else
                    return (10 - delta);
            }
            else {
                if (u64 < 100000000000ULL)
                    return (11 - delta);
                else
                    return (12 - delta);
            }
        }
        else {
            if (u64 < 100000000000000ULL) {
                if (u64 < 10000000000000ULL)
                    return (13 - delta);
                else
                    return (14 - delta);
            }
            else {
                if (u64 < 1000000000000000ULL)
                    return (15 - delta);
                else
                    return (16 - delta);
            }
        }
    }
    else {
        if (u64 < 1000000000000000000ULL) {
            if (u64 < 100000000000000000ULL)
                return (17 - delta);
            else
                return (18 - delta);
        }
        else {
            if (u64 < 10000000000000000000ULL)
                return (19 - delta);
            else
                return (20 - delta);
        }
    }
#endif // __amd64__
}

template <std::ssize_t delta = 0>
inline
std::ssize_t count_digits(int64_t i64) {
    uint64_t u64;
    if (i64 >= 0) {
        u64 = static_cast<uint64_t>(i64);
    }
    else {
        u64 = static_cast<uint64_t>(-i64);
    }
    return std::ssize_t(i64 < 0) + count_digits<delta>(u64);
}

template <std::ssize_t delta = 0>
inline
std::ssize_t count_digits(void * pv) {
    std::size_t pval = reinterpret_cast<std::size_t>(pv);
    return (count_digits<delta>(pval) + std::ssize_t(2));
}

template <typename CharTy, typename Arg>
inline
std::size_t itoa(std::basic_string<CharTy> & str, Arg && arg) {
    std::size_t data_len = 8;
    str.append(data_len, CharTy('?'));
    return data_len;
}

template <typename CharTy>
inline
std::size_t itoa(std::basic_string<CharTy> & str, uint32_t u32) {
    CharTy buf[16]; // Maximum usage is 10
    CharTy * buf_end  = &buf[sizeof(buf) - 5];
    CharTy * buf_last = &buf[sizeof(buf) - 6];  // 16 - 10 = 6

    uint32_t num;
    while (u32 >= 100UL) {
        num = u32 % 10UL;
        u32 /= 10UL;
        *buf_last = static_cast<CharTy>(num) + CharTy('0');
        buf_last--;

        num = u32 % 10UL;
        u32 /= 10UL;
        *buf_last = static_cast<CharTy>(num) + CharTy('0');
        buf_last--;
    }

    if (u32 >= 10UL) {
        num = u32 % 10UL;
        u32 /= 10UL;
        *buf_last = static_cast<CharTy>(num) + CharTy('0');
        buf_last--;
    }

    assert(u32 < 10UL);
    *buf_last = static_cast<CharTy>(u32) + CharTy('0');

    assert(buf_last >= &buf[0]);
    assert(buf_last < buf_end);

    str.append(buf_last, buf_end);

    std::size_t data_len = std::size_t(buf_end - buf_last);
    return data_len;
}

template <typename CharTy>
inline
std::size_t itoa(std::basic_string<CharTy> & str, int32_t i32) {
    uint32_t u32;
    std::size_t sign;
    if (likely(i32 >= 0)) {
        u32 = static_cast<uint32_t>(i32);
        sign = 0;
    }
    else {
        str.push_back(CharTy('-'));
        u32 = static_cast<uint32_t>(-i32);
        sign = 1;
    }
    std::size_t data_len = itoa(str, u32);
    return (data_len + sign);
}

template <typename CharTy>
inline
std::size_t itoa(std::basic_string<CharTy> & str, uint64_t u64) {
    CharTy buf[32]; // Maximum usage is 19
    CharTy * buf_end  = &buf[sizeof(buf) - 14];
    CharTy * buf_last = &buf[sizeof(buf) - 13]; // 32 - 19 = 13

    uint64_t num;
    while (u64 >= 100ULL) {
        num = u64 % 10ULL;
        u64 /= 10ULL;
        *buf_last = static_cast<CharTy>(num) + CharTy('0');
        buf_last--;

        num = u64 % 10ULL;
        u64 /= 10ULL;
        *buf_last = static_cast<CharTy>(num) + CharTy('0');
        buf_last--;
    }

    if (u64 >= 10ULL) {
        num = u64 % 10ULL;
        u64 /= 10ULL;
        *buf_last = static_cast<CharTy>(num) + CharTy('0');
        buf_last--;
    }

    assert(u64 < 10UL);
    *buf_last = static_cast<CharTy>(u64) + CharTy('0');

    assert(buf_last >= &buf[0]);
    assert(buf_last < buf_end);

    str.append(buf_last, buf_end);

    std::size_t data_len = std::size_t(buf_end - buf_last);
    return data_len;
}

template <typename CharTy>
inline
std::size_t itoa(std::basic_string<CharTy> & str, int64_t i64) {
    uint64_t u64;
    std::size_t sign;
    if (likely(i64 >= 0)) {
        u64 = static_cast<uint64_t>(i64);
        sign = 0;
    }
    else {
        str.push_back(CharTy('-'));
        u64 = static_cast<uint64_t>(-i64);
        sign = 1;
    }
    std::size_t data_len = itoa(str, u64);
    return (data_len + sign);
}

template <typename CharTy, typename Arg>
inline
std::size_t itoa(jstd::basic_string_view<CharTy> & str,
                 Arg && arg, std::size_t size) {
    std::size_t data_len = size;
    str.append(size, CharTy('?'));
    return data_len;
}

template <typename CharTy>
inline
std::size_t itoa(jstd::basic_string_view<CharTy> & str,
                 uint32_t u32, std::size_t digits) {
    assert(digits > 0);
    CharTy * buf_last = const_cast<CharTy *>(str.c_str() + digits - 1);
    CharTy * buf_end = buf_last;

    uint32_t num;
    switch (digits) {
        case 10:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 9:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 8:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 7:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 6:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 5:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 4:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 3:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 2:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 1:
            assert(u32 < 10UL);
            *buf_last = static_cast<CharTy>(u32) + CharTy('0');
            buf_last--;
            break;

        case 0:
            throw std::runtime_error("Error: format_and_append_arg(): digits = 0.\n\n");
            break;

        default:
            throw std::runtime_error("Error: format_and_append_arg(): digits = \n\n" +
                                     std::to_string(digits));
            break;
    }

    assert((const CharTy *)buf_last < str.c_str());
    assert(buf_end > buf_last);

    str.commit(digits);

    std::size_t data_len = std::size_t(buf_end - buf_last);
    assert(data_len == digits);
    JSTD_UNUSED_VAR(buf_end);
    JSTD_UNUSED_VAR(data_len);
    return digits;
}

template <typename CharTy>
inline
std::size_t itoa_slow(jstd::basic_string_view<CharTy> & str,
                      uint32_t u32, std::size_t digits) {
    assert(digits > 0);
    CharTy * buf_last = const_cast<CharTy *>(str.c_str() + digits - 1);
    CharTy * buf_end = buf_last;

    uint32_t num;
    while (u32 >= 100UL) {
        num = u32 % 10UL;
        u32 /= 10UL;
        *buf_last = static_cast<CharTy>(num) + CharTy('0');
        buf_last--;

        num = u32 % 10UL;
        u32 /= 10UL;
        *buf_last = static_cast<CharTy>(num) + CharTy('0');
        buf_last--;
    }

    if (u32 >= 10UL) {
        num = u32 % 10UL;
        u32 /= 10UL;
        *buf_last = static_cast<CharTy>(num) + CharTy('0');
        buf_last--;
    }

    assert(u32 < 10UL);
    *buf_last = static_cast<CharTy>(u32) + CharTy('0');
    buf_last--;

    assert((const CharTy *)buf_last < str.c_str());
    assert(buf_last < buf_end);

    str.commit(digits);

    std::size_t data_len = std::size_t(buf_end - buf_last);
    assert(data_len == digits);
    JSTD_UNUSED_VAR(buf_end);
    JSTD_UNUSED_VAR(data_len);
    return digits;
}

template <typename CharTy>
inline
std::size_t itoa(jstd::basic_string_view<CharTy> & str,
                 int32_t i32, std::size_t digits) {
    uint32_t u32;
    std::size_t sign;
    if (likely(i32 >= 0)) {
        u32 = static_cast<uint32_t>(i32);
        sign = 0;
    }
    else {
        str.push_back(CharTy('-'));
        u32 = static_cast<uint32_t>(-i32);
        sign = 1;
    }
    std::size_t data_len = itoa(str, u32, digits - sign) + sign;
    assert(data_len == digits);
    JSTD_UNUSED_VAR(data_len);
    return digits;
}

template <typename CharTy>
inline
std::size_t itoa(jstd::basic_string_view<CharTy> & str,
                 uint64_t u64, std::size_t digits) {
    CharTy * buf_last = const_cast<CharTy *>(str.c_str() + digits - 1);
    CharTy * buf_end = buf_last;

    uint64_t num;
    switch (digits) {
        case 19:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 18:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 17:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 16:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 15:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 14:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 13:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 12:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 11:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 10:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 9:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 8:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 7:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 6:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 5:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 4:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 3:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 2:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 1:
            assert(u64 < 10ULL);
            *buf_last = static_cast<CharTy>(u64) + CharTy('0');
            buf_last--;
            break;

        case 0:
            throw std::runtime_error("Error: format_and_append_arg(): digits = 0.\n\n");
            break;

        default:
            throw std::runtime_error("Error: format_and_append_arg(): digits = \n\n" +
                                     std::to_string(digits));
            break;
    }

    assert((const CharTy *)buf_last < str.c_str());
    assert(buf_last < buf_end);

    str.commit(digits);

    std::size_t data_len = std::size_t(buf_end - buf_last);
    assert(data_len == digits);
    JSTD_UNUSED_VAR(buf_end);
    JSTD_UNUSED_VAR(data_len);
    return digits;
}

template <typename CharTy>
inline
std::size_t itoa(jstd::basic_string_view<CharTy> & str,
                 int64_t i64, std::size_t digits) {
    uint64_t u64;
    std::size_t sign;
    if (likely(i64 >= 0)) {
        u64 = static_cast<uint64_t>(i64);
        sign = 0;
    }
    else {
        str.push_back(CharTy('-'));
        u64 = static_cast<uint64_t>(-i64);
        sign = 1;
    }
    std::size_t data_len = itoa(str, u64, digits - sign) + sign;
    assert(data_len == digits);
    JSTD_UNUSED_VAR(data_len);
    return digits;
}

template <typename CharTy, typename Arg>
inline
std::size_t itoa(std::basic_string<CharTy> & str,
                 Arg && arg, std::size_t size) {
    std::size_t data_len = size;
    str.append(size, CharTy('?'));
    return data_len;
}

template <typename CharTy>
inline
std::size_t itoa(std::basic_string<CharTy> & str,
                 uint32_t u32, std::size_t digits) {
    assert(digits > 0);
    CharTy buf[16]; // Maximum usage is 10
    CharTy * buf_end  = &buf[sizeof(buf) - 5];
    CharTy * buf_last = &buf[sizeof(buf) - 6];  // 16 - 10 = 6

    uint32_t num;
    switch (digits) {
        case 10:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 9:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 8:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 7:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 6:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 5:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 4:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 3:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 2:
            num = u32 % 10UL;
            u32 /= 10UL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 1:
            assert(u32 < 10UL);
            *buf_last = static_cast<CharTy>(u32) + CharTy('0');
            break;

        case 0:
            throw std::runtime_error("Error: format_and_append_arg(): digits = 0.\n\n");
            break;

        default:
            throw std::runtime_error("Error: format_and_append_arg(): digits = \n\n" +
                                     std::to_string(digits));
            break;
    }

    assert(buf_last >= &buf[0]);
    assert(buf_last < buf_end);

    str.append(buf_last, buf_end);

    std::size_t data_len = std::size_t(buf_end - buf_last);
    assert(data_len == digits);
    JSTD_UNUSED_VAR(buf_end);
    JSTD_UNUSED_VAR(data_len);
    return digits;
}

template <typename CharTy>
inline
std::size_t itoa(std::basic_string<CharTy> & str,
                 int32_t i32, std::size_t digits) {
    uint32_t u32;
    std::size_t sign;
    if (likely(i32 >= 0)) {
        u32 = static_cast<uint32_t>(i32);
        sign = 0;
    }
    else {
        str.push_back(CharTy('-'));
        u32 = static_cast<uint32_t>(-i32);
        sign = 1;
    }
    std::size_t data_len = itoa(str, u32, digits - sign) + sign;
    assert(data_len == digits);
    JSTD_UNUSED_VAR(data_len);
    return digits;
}

template <typename CharTy>
inline
std::size_t itoa(std::basic_string<CharTy> & str,
                 uint64_t u64, std::size_t digits) {
    CharTy buf[32]; // Maximum usage is 19
    CharTy * buf_end  = &buf[sizeof(buf) - 14];
    CharTy * buf_last = &buf[sizeof(buf) - 13]; // 32 - 19 = 13

    uint64_t num;
    switch (digits) {
        case 19:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 18:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 17:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 16:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 15:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 14:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 13:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 12:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 11:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 10:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 9:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 8:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 7:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 6:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 5:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 4:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 3:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 2:
            num = u64 % 10ULL;
            u64 /= 10ULL;
            *buf_last = static_cast<CharTy>(num) + CharTy('0');
            buf_last--;

        case 1:
            assert(u64 < 10ULL);
            *buf_last = static_cast<CharTy>(u64) + CharTy('0');
            break;

        case 0:
            throw std::runtime_error("Error: format_and_append_arg(): digits = 0.\n\n");
            break;

        default:
            throw std::runtime_error("Error: format_and_append_arg(): digits = \n\n" +
                                     std::to_string(digits));
            break;
    }

    assert(buf_last >= &buf[0]);
    assert(buf_last < buf_end);

    str.append(buf_last, buf_end);

    std::size_t data_len = std::size_t(buf_end - buf_last);
    assert(data_len == digits);
    JSTD_UNUSED_VAR(buf_end);
    JSTD_UNUSED_VAR(data_len);
    return digits;
}

template <typename CharTy>
inline
std::size_t itoa(std::basic_string<CharTy> & str,
                 int64_t i64, std::size_t digits) {
    uint64_t u64;
    std::size_t sign;
    if (likely(i64 >= 0)) {
        u64 = static_cast<uint64_t>(i64);
        sign = 0;
    }
    else {
        str.push_back(CharTy('-'));
        u64 = static_cast<uint64_t>(-i64);
        sign = 1;
    }
    std::size_t data_len = itoa(str, u64, digits - sign) + sign;
    assert(data_len == digits);
    JSTD_UNUSED_VAR(data_len);
    return digits;
}

template <typename CharTy>
struct sprintf_fmt_node {
    typedef CharTy          char_type;
    typedef std::size_t     size_type;
    typedef std::ssize_t    ssize_type;

    enum Type {
        isNotArg = 0,
        isArg
    };

    const char_type * first;
    const char_type * last;
    size_type         arg_type;
    size_type         arg_size;

    sprintf_fmt_node() noexcept
        : first(nullptr),
          last(nullptr),
          arg_type(isNotArg),
          arg_size(0) {
    }

    sprintf_fmt_node(const char_type * _first, const char_type * _last) noexcept
        : first(_first),
          last(_last),
          arg_type(isNotArg),
          arg_size(0) {
    }

    sprintf_fmt_node(const char_type * _first, const char_type * _last,
                     size_type type, size_type size) noexcept
        : first(_first),
          last(_last),
          arg_type(type),
          arg_size(size) {
    }

    void init(const char_type * first, const char_type * last,
              size_type type, size_type size) {
        this->first = first;
        this->last = last;
        this->arg_type = type;
        this->arg_size = size;
    }

    bool is_arg() const {
        return (this->arg_type != isNotArg);
    }

    bool not_is_arg() const {
        return !(this->is_arg());
    }

    ssize_type fmt_length() const {
        return (this->last - this->first);
    }

    const char_type * get_fmt_first() const {
        return this->first;
    }

    const char_type * get_arg_first() const {
        return this->last;
    }

    size_type get_arg_type() const {
        return this->arg_type;
    }

    size_type get_arg_size() const {
        return this->arg_size;
    }

    void set_fmt_first(const char_type * first) {
        this->first = first;
    }

    void set_fmt_last(const char_type * last) {
        this->last = last;
    }

    void set_fmt(const char_type * first, const char_type * last) {
        this->first = first;
        this->last = last;
    }

    void set_arg(size_type type, size_type size) {
        this->arg_type = type;
        this->arg_size = size;
    }
};

void throw_sprintf_exception(std::ssize_t err_code, std::ssize_t pos, std::size_t arg1)
{
    switch (err_code) {
        case Sprintf_BadFormat_BrokenEscapeChar:
            throw std::runtime_error(
                "Bad format: broken escape char, *fmt pos: "
                + std::to_string(pos)
            );
            break;

        case Sprintf_BadFormat_UnknownSpecifier_FF:
            throw std::runtime_error(
                "Bad format: Unknown specifier: 0xFF, *fmt pos: "
                + std::to_string(pos)
            );
            break;

        case Sprintf_BadFormat_UnknownSpecifier:
            throw std::runtime_error(
                "Bad format: Unknown specifier: "
                + jstd::to_hex(static_cast<uint8_t>(arg1)) +
                ", *fmt pos: " + std::to_string(pos)
            );
            break;

        case Sprintf_InvalidArgmument_MissingParameter:
            throw std::invalid_argument(
                "Invalid argmument: missing parameter, *fmt pos: "
                + std::to_string(pos)
            );
            break;

        case Sprintf_InvalidArgmument_ErrorFloatType:
            throw std::invalid_argument(
                "Invalid argmument: error float type, *fmt pos: "
                + std::to_string(pos)
            );
            break;

        default:
            break;
    }
}

template <typename CharTy>
struct basic_formatter {
    typedef std::size_t     size_type;
    typedef std::ssize_t    ssize_type;

    typedef CharTy                                      char_type;
    typedef typename make_unsigned_char<CharTy>::type   uchar_type;

    typedef sprintf_fmt_node<CharTy>                    fmt_node_t;
    typedef basic_formatter<CharTy>                     this_type;

    template <typename Arg>
    ssize_type sprintf_handle_specifier(const char_type * &fmt,
                                        Arg && arg,
                                        size_type & data_len,
                                        size_type & ex_arg1) {
        typedef typename std::decay<Arg>::type  ArgT;

        ssize_type err_code = Sprintf_Success;

        int8_t  i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t  u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float f;
        double d;
        long double ld;
        void * pvoid;

        uchar_type sp = static_cast<uchar_type>(*fmt);
        switch (sp) {
        case uchar_type('\0'):
            {
                JSTD_UNUSED_VAR(i8);
                JSTD_UNUSED_VAR(u8);
                JSTD_UNUSED_VAR(i16);
                JSTD_UNUSED_VAR(u16);
                JSTD_UNUSED_VAR(i32);
                JSTD_UNUSED_VAR(u32);
                JSTD_UNUSED_VAR(i64);
                JSTD_UNUSED_VAR(u64);
                JSTD_UNUSED_VAR(f);
                JSTD_UNUSED_VAR(d);
                JSTD_UNUSED_VAR(ld);
                JSTD_UNUSED_VAR(pvoid);
                err_code = Sprintf_Reached_Endof;
                break;
            }

        case uchar_type('A'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('E'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('F'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('G'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('X'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('a'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('c'):
            if (std::is_integral<ArgT>::value &&
               (std::is_same<ArgT, char>::value ||
                std::is_same<ArgT, unsigned char>::value ||
                std::is_same<ArgT, short>::value ||
                std::is_same<ArgT, unsigned short>::value ||
                std::is_same<ArgT, wchar_t>::value))
            {
                fmt++;
                u8 = static_cast<unsigned char>(arg);
                data_len = count_digits<0>(u8);
            }
            break;

        case uchar_type('d'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
                i32 = static_cast<int>(arg);
                data_len = count_digits<0>(i32);
            }
            break;

        case uchar_type('e'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('f'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
                if (sizeof(Arg) == sizeof(float)) {
                    f = static_cast<float>(arg);
                }
                else if (sizeof(Arg) == sizeof(double)) {
                    d = static_cast<double>(arg);
                }
                else if (sizeof(Arg) == sizeof(long double)) {
                    ld = static_cast<long double>(arg);
                }
                else {
                    err_code = Sprintf_InvalidArgmument_ErrorFloatType;
                    return err_code;
                }

                data_len = 6;
            }
            break;

        case uchar_type('g'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('i'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('n'):
            if (std::is_pointer<ArgT>::value &&
               (std::is_same<ArgT, int *>::value ||
                std::is_same<ArgT, unsigned int *>::value))
            {
                fmt++;
                break;
            }

        case uchar_type('o'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('p'):
            if (std::is_pointer<ArgT>::value &&
               (std::is_same<ArgT, void *>::value ||
                std::is_convertible<ArgT, void *>::value))
            {
                fmt++;
                pvoid = reinterpret_cast<void *>(static_cast<std::size_t>(arg));
                data_len = count_digits<0>(pvoid);
            }
            break;

        case uchar_type('s'):
            if (std::is_pointer<ArgT>::value &&
               (std::is_same<ArgT, char *>::value ||
                std::is_same<ArgT, unsigned char *>::value ||
                std::is_same<ArgT, short *>::value ||
                std::is_same<ArgT, unsigned short *>::value ||
                std::is_same<ArgT, wchar_t *>::value))
            {
                fmt++;
            }
            break;

        case uchar_type('u'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
                u32 = static_cast<unsigned int>(arg);
                data_len = count_digits<0>(u32);
            }
            break;

        case uchar_type('x'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
            }
            break;
#if 0
        case uchar_type('\xff'):
            {
                err_code = Sprintf_BadFormat_UnknownSpecifier_FF;
                break;
            }
#endif
        default:
            {
                ex_arg1 = sp;
                err_code = Sprintf_BadFormat_UnknownSpecifier;
                break;
            }
        }

        return err_code;
    }

    template <typename Arg>
    ssize_type sprintf_handle_specifier(std::basic_string<char_type> & str,
                                        const char_type * &fmt,
                                        Arg && arg,
                                        size_type & ex_arg1) {
        typedef typename std::decay<Arg>::type  ArgT;

        ssize_type err_code = Sprintf_Success;
        size_type data_len = 0;

        int8_t  i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t  u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float f;
        double d;
        long double ld;
        void * pvoid;

        uchar_type sp = static_cast<uchar_type>(*fmt);
        switch (sp) {
        case uchar_type('\0'):
            {
                JSTD_UNUSED_VAR(i8);
                JSTD_UNUSED_VAR(u8);
                JSTD_UNUSED_VAR(i16);
                JSTD_UNUSED_VAR(u16);
                JSTD_UNUSED_VAR(i32);
                JSTD_UNUSED_VAR(u32);
                JSTD_UNUSED_VAR(i64);
                JSTD_UNUSED_VAR(u64);
                JSTD_UNUSED_VAR(f);
                JSTD_UNUSED_VAR(d);
                JSTD_UNUSED_VAR(ld);
                JSTD_UNUSED_VAR(pvoid);
                JSTD_UNUSED_VAR(data_len);
                err_code = Sprintf_Reached_Endof;
                break;
            }

        case uchar_type('A'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('E'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('F'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('G'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('X'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('a'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('c'):
            if (std::is_integral<ArgT>::value &&
               (std::is_same<ArgT, char>::value ||
                std::is_same<ArgT, unsigned char>::value ||
                std::is_same<ArgT, short>::value ||
                std::is_same<ArgT, unsigned short>::value ||
                std::is_same<ArgT, wchar_t>::value))
            {
                fmt++;
                u8 = static_cast<unsigned char>(arg);
                data_len = itoa(str, static_cast<uint32_t>(u8));
            }
            break;

        case uchar_type('d'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
                i32 = static_cast<int>(arg);
                data_len = itoa(str, i32);
            }
            break;

        case uchar_type('e'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('f'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
                if (sizeof(Arg) == sizeof(float)) {
                    f = static_cast<float>(arg);
                }
                else if (sizeof(Arg) == sizeof(double)) {
                    d = static_cast<double>(arg);
                }
                else if (sizeof(Arg) == sizeof(long double)) {
                    ld = static_cast<long double>(arg);
                }
                else {
                    err_code = Sprintf_InvalidArgmument_ErrorFloatType;
                    return err_code;
                }

                data_len = 6;
            }
            break;

        case uchar_type('g'):
            if (std::is_same<ArgT, float>::value ||
                std::is_same<ArgT, double>::value ||
                std::is_same<ArgT, long double>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('i'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('n'):
            if (std::is_pointer<ArgT>::value &&
               (std::is_same<ArgT, int *>::value ||
                std::is_same<ArgT, unsigned int *>::value))
            {
                fmt++;
                break;
            }

        case uchar_type('o'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
            }
            break;

        case uchar_type('p'):
            if (std::is_pointer<ArgT>::value &&
               (std::is_same<ArgT, void *>::value ||
                std::is_convertible<ArgT, void *>::value))
            {
                fmt++;
                pvoid = reinterpret_cast<void *>(static_cast<std::size_t>(arg));
                data_len = itoa(str, reinterpret_cast<std::size_t>(pvoid));
            }
            break;

        case uchar_type('s'):
            if (std::is_pointer<ArgT>::value &&
               (std::is_same<ArgT, char *>::value ||
                std::is_same<ArgT, unsigned char *>::value ||
                std::is_same<ArgT, short *>::value ||
                std::is_same<ArgT, unsigned short *>::value ||
                std::is_same<ArgT, wchar_t *>::value))
            {
                fmt++;
            }
            break;

        case uchar_type('u'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
                u32 = static_cast<uint32_t>(arg);
                data_len = itoa(str, u32);
            }
            break;

        case uchar_type('x'):
            if (std::is_integral<ArgT>::value)
            {
                fmt++;
            }
            break;

        default:
            {
                ex_arg1 = sp;
                err_code = Sprintf_BadFormat_UnknownSpecifier;
                break;
            }
        }

        return err_code;
    }

    size_type sprintf_calc_space_impl(const char_type * fmt) {
        assert(fmt != nullptr);
        ssize_type rest_size = 0;
        ssize_type err_code = Sprintf_Success;
        size_type ex_arg1 = 0;
        const char_type * fmt_first = fmt;
        while (*fmt != char_type('\0')) {
            if (likely(*fmt != char_type('%'))) {
                // is not "%": direct output
                fmt++;
            }
            else {
                fmt++;
                if (likely(*fmt != char_type('%'))) {
                    // "%?": specifier
                    err_code = Sprintf_InvalidArgmument_MissingParameter;
                    goto Sprintf_Throw_Except;
                }
                else {
                    // "%%" = "%"
                    rest_size--;
                    fmt++;
                }
            }
        }

Sprintf_Throw_Except:
        if (err_code != Sprintf_Success) {
            ssize_type pos = (fmt - fmt_first);
            throw_sprintf_exception(err_code, pos, ex_arg1);
        }

        ssize_type scan_len = (fmt - fmt_first);
        assert(scan_len >= 0);
        assert(scan_len >= rest_size);

        size_type total_size = scan_len + rest_size;
        return total_size;
    }

    template <typename Arg1, typename ...Args>
    size_type sprintf_calc_space_impl(const char_type * fmt,
                                      Arg1 && arg1,
                                      Args && ... args) {
        assert(fmt != nullptr);
        ssize_type rest_size = 0;
        ssize_type err_code = Sprintf_Success;
        size_type ex_arg1 = 0;
        const char_type * fmt_first = fmt;
        while (*fmt != char_type('\0')) {
            if (likely(*fmt != char_type('%'))) {
                // is not "%": direct output
                fmt++;
            }
            else {
                const char_type * arg_first = fmt;
                fmt++;
                if (likely(*fmt != char_type('%'))) {
                    // "%?": specifier
                    size_type data_len = 0;
                    err_code = sprintf_handle_specifier<Arg1>(
                                    fmt, std::forward<Arg1>(arg1),
                                    data_len, ex_arg1);

                    if (err_code < 0) {
                        goto Sprintf_Throw_Except;
                    }
                    else if (err_code == Sprintf_Reached_Endof) {
                        goto Sprintf_Exit;
                    }

                    rest_size += (ssize_type(data_len) - (fmt - arg_first));
                    size_type remain_size = sprintf_calc_space_impl(fmt,
                                                std::forward<Args>(args)...);
                    rest_size += remain_size;
                    goto Sprintf_Exit;
                }
                else {
                    // "%%" = "%"
                    rest_size--;
                    fmt++;
                }
            }
        }

        if (err_code != Sprintf_Success) {
Sprintf_Throw_Except:
            ssize_type pos = (fmt - fmt_first);
            throw_sprintf_exception(err_code, pos, ex_arg1);
        }

Sprintf_Exit:
        assert(rest_size >= 0);
        ssize_type scan_len = (fmt - fmt_first);
        assert(scan_len >= 0);

        size_type total_size = scan_len + rest_size;
        return total_size;
    }

    template <typename ...Args>
    size_type sprintf_calc_space(const char_type * fmt, Args && ... args) {
        return sprintf_calc_space_impl(fmt, std::forward<Args>(args)...);
    }

    size_type sprintf_prepare_space_impl(std::vector<fmt_node_t> & fmt_list,
                                         const char_type * fmt) {
        assert(fmt != nullptr);
        ssize_type rest_size = 0;
        ssize_type err_code = Sprintf_Success;
        size_type ex_arg1 = 0;
        const char_type * fmt_first = fmt;
        fmt_node_t fmt_info;
        while (*fmt != char_type('\0')) {
            if (likely(*fmt != char_type('%'))) {
                // is not "%": direct output
                fmt++;
            }
            else {
                fmt++;
                if (likely(*fmt != char_type('%'))) {
                    // "%?": specifier
                    err_code = Sprintf_InvalidArgmument_MissingParameter;
                    goto Sprintf_Throw_Except;
                }
                else {
                    // "%%" = "%"
                    fmt_info.init(fmt_first, fmt, 0, 0);
                    fmt_list.push_back(fmt_info);

                    rest_size--;
                    fmt++;
                    fmt_first = fmt;
                }
            }
        }

        if (err_code != Sprintf_Success) {
Sprintf_Throw_Except:
            ssize_type pos = (fmt - fmt_first);
            throw_sprintf_exception(err_code, pos, ex_arg1);
        }

        if (fmt > fmt_first) {
            fmt_info.init(fmt_first, fmt, 0, 0);
            fmt_list.push_back(fmt_info);
        }

        ssize_type scan_len = (fmt - fmt_first);
        assert(scan_len >= 0);
        assert(scan_len >= rest_size);

        size_type total_size = scan_len + rest_size;
        return total_size;
    }

    template <typename Arg1, typename ...Args>
    size_type sprintf_prepare_space_impl(std::vector<fmt_node_t> & fmt_list,
                                         const char_type * fmt,
                                         Arg1 && arg1,
                                         Args && ... args) {
        assert(fmt != nullptr);
        ssize_type rest_size = 0;
        ssize_type err_code = Sprintf_Success;
        size_type ex_arg1 = 0;
        const char_type * fmt_first = fmt;
        const char_type * arg_first = nullptr;
        fmt_node_t fmt_info;
        while (*fmt != char_type('\0')) {
            if (likely(*fmt != char_type('%'))) {
                // is not "%": direct output
                fmt++;
            }
            else {
                arg_first = fmt;
                fmt++;
                if (likely(*fmt != char_type('%'))) {
                    // "%?": specifier
                    size_type data_len = 0;
                    err_code = sprintf_handle_specifier<Arg1>(
                                    fmt, std::forward<Arg1>(arg1),
                                    data_len, ex_arg1);

                    if (err_code < 0) {
                        goto Sprintf_Throw_Except;
                    }
                    else if (err_code == Sprintf_Reached_Endof) {
                        goto Sprintf_Endof_Exit;
                    }

                    fmt_node_t arg_info(fmt_first, arg_first, fmt_node_t::isArg, data_len);
                    fmt_list.push_back(arg_info);

                    rest_size += (ssize_type(data_len) - (fmt - arg_first));
                    size_type remain_size = sprintf_prepare_space_impl(
                                                fmt_list, fmt,
                                                std::forward<Args>(args)...);
                    rest_size += remain_size;
                    goto Sprintf_Exit;
                }
                else {
                    // "%%" = "%"
                    fmt_info.init(fmt_first, fmt, 0, 0);
                    fmt_list.push_back(fmt_info);

                    arg_first = nullptr;
                    rest_size--;
                    fmt++;
                    fmt_first = fmt;
                }
            }
        }

        if (err_code != Sprintf_Success) {
Sprintf_Throw_Except:
            ssize_type pos = (fmt - fmt_first);
            throw_sprintf_exception(err_code, pos, ex_arg1);
        }

Sprintf_Endof_Exit:
        arg_first = (arg_first != nullptr) ? arg_first : fmt_first;
        if (fmt > arg_first) {
            fmt_info.init(arg_first, fmt, 0, 0);
            fmt_list.push_back(fmt_info);
        }

Sprintf_Exit:
        assert(rest_size >= 0);
        ssize_type scan_len = (fmt - fmt_first);
        assert(scan_len >= 0);

        size_type total_size = scan_len + rest_size;
        return total_size;
    }

    void sprintf_no_prepare_output_impl(std::basic_string<char_type> & str,
                                        const char_type * fmt) {
        assert(fmt != nullptr);

        ssize_type err_code = Sprintf_Success;
        size_type ex_arg1 = 0;
        const char_type * fmt_first = fmt;
        while (*fmt != char_type('\0')) {
            if (likely(*fmt != char_type('%'))) {
                // is not "%": direct output
                fmt++;
            }
            else {
                fmt++;
                if (likely(*fmt != char_type('%'))) {
                    // "%?": specifier
                    err_code = Sprintf_InvalidArgmument_MissingParameter;
                    goto Sprintf_Throw_Except;
                }
                else {
                    // "%%" = "%"
                    str.append(fmt_first, fmt);
                    fmt++;
                    fmt_first = fmt;
                }
            }
        }

        if (err_code != Sprintf_Success) {
Sprintf_Throw_Except:
            ssize_type pos = (fmt - fmt_first);
            throw_sprintf_exception(err_code, pos, ex_arg1);
        }

        str.append(fmt_first, fmt);

        ssize_type scan_len = (fmt - fmt_first);
        assert(scan_len >= 0);
    }

    template <typename Arg1, typename ...Args>
    void sprintf_no_prepare_output_impl(std::basic_string<char_type> & str,
                                        const char_type * fmt,
                                        Arg1 && arg1,
                                        Args && ... args) {
        assert(fmt != nullptr);
        ssize_type err_code = Sprintf_Success;
        size_type ex_arg1 = 0;
        const char_type * fmt_first = fmt;
        const char_type * arg_first = nullptr;
        while (*fmt != char_type('\0')) {
            if (likely(*fmt != char_type('%'))) {
                // is not "%": direct output
                fmt++;
            }
            else {
                arg_first = fmt;
                fmt++;
                if (likely(*fmt != char_type('%'))) {
                    // "%?": specifier
                    size_type data_len = 0;
                    str.append(fmt_first, arg_first);
                    err_code = sprintf_handle_specifier<Arg1>(
                                    str, fmt, std::forward<Arg1>(arg1),
                                    ex_arg1);

                    if (err_code < 0) {
                        goto Sprintf_Throw_Except;
                    }
                    else if (err_code == Sprintf_Reached_Endof) {
                        goto Sprintf_Endof_Exit;
                    }

                    sprintf_no_prepare_output_impl(str, fmt,
                                                   std::forward<Args>(args)...);
                    goto Sprintf_Exit;
                }
                else {
                    // "%%" = "%"
                    str.append(fmt_first, fmt);
                    arg_first = nullptr;
                    fmt++;
                    fmt_first = fmt;
                }
            }
        }

        if (err_code != Sprintf_Success) {
Sprintf_Throw_Except:
            ssize_type pos = (fmt - fmt_first);
            throw_sprintf_exception(err_code, pos, ex_arg1);
        }

Sprintf_Endof_Exit:
        arg_first = (arg_first != nullptr) ? arg_first : fmt_first;
        str.append(arg_first, fmt);

Sprintf_Exit:
        ssize_type scan_len = (fmt - fmt_first);
        assert(scan_len >= 0);
    }

    size_type sprintf_direct_output_impl(std::basic_string<char_type> & str,
                                         std::vector<fmt_node_t> & fmt_list,
                                         size_type index) {
        size_type total_size = 0;
        if (index < fmt_list.size()) {
            fmt_node_t & fmt_info = fmt_list[index];
            if (fmt_info.fmt_length() > 0) {
                total_size += fmt_info.fmt_length();
                str.append(fmt_info.first, fmt_info.last);
            }

            bool is_arg = fmt_info.is_arg();
            if (!is_arg) {
                //index++;
            }
            else {
                throw std::runtime_error("Error: Wrong fmt_list!");
            }
        }

        return total_size;
    }

    template <typename Arg1, typename ...Args>
    size_type sprintf_direct_output_impl(std::basic_string<char_type> & str,
                                         std::vector<fmt_node_t> & fmt_list,
                                         size_type index,
                                         Arg1 && arg1,
                                         Args && ... args) {
        size_type total_size = 0;
        if (index < fmt_list.size()) {
            fmt_node_t & fmt_info = fmt_list[index];
            if (fmt_info.fmt_length() > 0) {
                total_size += fmt_info.fmt_length();
                str.append(fmt_info.first, fmt_info.last);
            }
            bool is_arg = fmt_info.is_arg();
            if (is_arg) {
                size_type arg_size = fmt_info.arg_size;
                size_type data_len = itoa(str,
                                            std::forward<Arg1>(arg1),
                                            arg_size);

                total_size += data_len;
                size_type remain_size = sprintf_direct_output_impl(
                                            str, fmt_list, index + 1,
                                            std::forward<Args>(args)...);
                total_size += remain_size;
            }
            else {
                size_type rest_args = sizeof...(args);
                if (rest_args > 0) {
                    throw std::runtime_error("Error: Missing argument!");
                }
            }
        }

        return total_size;
    }

    size_type sprintf_prepare_output_impl(jstd::basic_string_view<char_type> & str,
                                          std::vector<fmt_node_t> & fmt_list,
                                          size_type index) {
        size_type total_size = 0;
        if (index < fmt_list.size()) {
            fmt_node_t & fmt_info = fmt_list[index];
            if (fmt_info.fmt_length() > 0) {
                total_size += fmt_info.fmt_length();
                str.append(fmt_info.first, fmt_info.last);
            }

            bool is_arg = fmt_info.is_arg();
            if (!is_arg) {
                //index++;
            }
            else {
                throw std::runtime_error("Error: Wrong fmt_list!");
            }
        }
        else {
            throw std::runtime_error("Error: fmt_list out of bound!");
        }

        return total_size;
    }

    template <typename Arg1, typename ...Args>
    size_type sprintf_prepare_output_impl(jstd::basic_string_view<char_type> & str,
                                          std::vector<fmt_node_t> & fmt_list,
                                          size_type index,
                                          Arg1 && arg1,
                                          Args && ... args) {
        size_type total_size = 0;
        if (index < fmt_list.size()) {
            fmt_node_t & fmt_info = fmt_list[index];
            if (fmt_info.fmt_length() > 0) {
                total_size += fmt_info.fmt_length();
                str.append(fmt_info.first, fmt_info.last);
            }
            bool is_arg = fmt_info.is_arg();
            if (is_arg) {
                size_type arg_size = fmt_info.arg_size;
                size_type data_len = itoa(str,
                                            std::forward<Arg1>(arg1),
                                            arg_size);

                total_size += data_len;
                size_type remain_size = sprintf_prepare_output_impl(
                                            str, fmt_list, index + 1,
                                            std::forward<Args>(args)...);
                total_size += remain_size;
            }
            else {
                size_type rest_args = sizeof...(args);
                if (rest_args > 0) {
                    throw std::runtime_error("Error: Missing argument!");
                }
            }
        }
        else {
            throw std::runtime_error("Error: fmt_list out of bound!");
        }

        return total_size;
    }

    template <typename ...Args>
    size_type sprintf_prepare_space(std::vector<fmt_node_t> & fmt_list,
                                    const char_type * fmt,
                                    Args && ... args) {
        return sprintf_prepare_space_impl(fmt_list, fmt, std::forward<Args>(args)...);
    }

    template <typename ...Args>
    void sprintf_no_prepare_output(std::basic_string<char_type> & str,
                                   const char_type * fmt,
                                   Args && ... args) {
        return sprintf_no_prepare_output_impl(str, fmt, std::forward<Args>(args)...);
    }

    template <typename ...Args>
    size_type sprintf_direct_output(std::basic_string<char_type> & str,
                                    std::vector<fmt_node_t> & fmt_list,
                                    Args && ... args) {
        return sprintf_direct_output_impl(str, fmt_list, 0, std::forward<Args>(args)...);
    }

    template <typename ...Args>
    size_type sprintf_prepare_output(jstd::basic_string_view<char_type> & str,
                                     std::vector<fmt_node_t> & fmt_list,
                                     Args && ... args) {
        return sprintf_prepare_output_impl(str, fmt_list, 0, std::forward<Args>(args)...);
    }

    template <typename ...Args>
    size_type sprintf_no_prepare(std::basic_string<char_type> & str,
                                 const char_type * fmt,
                                 Args && ... args) {
        if (likely(fmt != nullptr)) {
            size_type old_size = str.size();
            sprintf_no_prepare_output(str, fmt, std::forward<Args>(args)...);
            size_type new_size = str.size();
            return (new_size - old_size);
        }

        return 0;
    }

    template <typename ...Args>
    size_type sprintf_direct(std::basic_string<char_type> & str,
                             const char_type * fmt,
                             Args && ... args) {
        if (likely(fmt != nullptr)) {
            std::vector<fmt_node_t> fmt_list;
            size_type sz = sizeof...(args);
            fmt_list.reserve(sz * 2);
            size_type prepare_size = sprintf_prepare_space(fmt_list, fmt, std::forward<Args>(args)...);
            size_type old_size = str.size();
            size_type output_size = sprintf_direct_output(str, fmt_list, std::forward<Args>(args)...);
            assert(output_size == prepare_size);
            size_type new_size = str.size();
            assert(new_size == old_size + output_size);
            return prepare_size;
        }

        return 0;
    }

    template <typename ...Args>
    size_type sprintf(std::basic_string<char_type> & str,
                      const char_type * fmt,
                      Args && ... args) {
        if (likely(fmt != nullptr)) {
            std::vector<fmt_node_t> fmt_list;
            size_type sz = sizeof...(args);
            fmt_list.reserve(sz * 2);
            size_type prepare_size = sprintf_prepare_space(fmt_list, fmt, std::forward<Args>(args)...);
#if 1
            size_type old_size = str.size();
            // Allocate the prepare buffer space.
            str.resize(old_size + prepare_size);
            jstd::basic_string_view<char_type> str_view(str);
            size_type output_size = sprintf_prepare_output(str_view, fmt_list, std::forward<Args>(args)...);
            assert(output_size == prepare_size);
#else
#ifdef NDEBUG
            std::vector<char_type> str_buf;
            // Reserve prepare buffer space.
            str_buf.reserve(prepare_size);
            jstd::basic_string_view<char_type> str_view(str_buf);
            size_type output_size = sprintf_prepare_output(str_view, fmt_list, std::forward<Args>(args)...);
            assert(output_size == prepare_size);
            str.append(str_buf.begin(), str_buf.begin() + output_size);
#else
            std::vector<char_type> str_buf(prepare_size);
            jstd::basic_string_view<char_type> str_view(str_buf);
            size_type output_size = sprintf_prepare_output(str_view, fmt_list, std::forward<Args>(args)...);
            assert(output_size == prepare_size);
            str.append(str_buf.begin(), str_buf.end());
#endif
#endif
            return prepare_size;
        }

        return 0;
    }

    template <typename ...Args>
    size_type output(const char_type * &buf, const char_type * fmt, Args && ... args) {
        int fmt_size = ::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
        if (fmt_size > 0) {
            const char_type * fmt_buf = (const char_type *)::malloc(fmt_size + 1);
            if (fmt_buf != nullptr) {
                int out_size = ::snprintf(const_cast<char_type *>(fmt_buf),
                                          fmt_size + 1, fmt, std::forward<Args>(args)...);
                assert(fmt_size == out_size);
                buf = fmt_buf;
                return out_size;
            }
            else return 0;
        }

        return fmt_size;
    }
};

typedef basic_formatter<char>       formatter;
typedef basic_formatter<wchar_t>    wformatter;

} // namespace jstd

#endif // JSTD_STRING_FORMATTER_H
