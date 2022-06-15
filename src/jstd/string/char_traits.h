
#ifndef JSTD_CHAR_TRAITS_H
#define JSTD_CHAR_TRAITS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <type_traits>

namespace jstd {

// jstd::is_char8<T>

template <typename CharTy>
struct is_char8 {
    static const bool value = false;
};

template <>
struct is_char8<bool> {
    static const bool value = true;
};

template <>
struct is_char8<char> {
    static const bool value = true;
};

template <>
struct is_char8<unsigned char> {
    static const bool value = true;
};

// jstd::is_char16<T>

template <typename CharTy>
struct is_char16 {
    static const bool value = false;
};

template <>
struct is_char16<signed short> {
    static const bool value = true;
};

template <>
struct is_char16<unsigned short> {
    static const bool value = true;
};

// jstd::is_char32<T>

template <typename CharTy>
struct is_char32 {
    static const bool value = false;
};

template <>
struct is_char32<signed int> {
    static const bool value = true;
};

template <>
struct is_char32<unsigned int> {
    static const bool value = true;
};

// jstd::is_wchar<T>

template <typename CharTy>
struct is_wchar {
    static const bool value = false;
};

template <>
struct is_wchar<short> {
    static const bool value = true;
};

template <>
struct is_wchar<unsigned short> {
    static const bool value = true;
};

template <>
struct is_wchar<wchar_t> {
    static const bool value = true;
};

template <typename CharTy>
struct make_unsigned_char {
    typedef typename std::make_unsigned<CharTy>::type   type;
};

template <typename CharTy>
using make_unsigned_char_t = typename make_unsigned_char<CharTy>::type;

template <typename CharTy>
struct make_signed_char {
    typedef typename std::make_signed<CharTy>::type     type;
};

template <typename CharTy>
using make_signed_char_t = typename make_signed_char<CharTy>::type;

// jstd::char_traits<T>

template <typename CharTy>
struct char_traits {
    typedef CharTy                                  char_type;
    typedef jstd::make_unsigned_char_t<char_type>   uchar_type;
    typedef jstd::make_signed_char_t<char_type>     schar_type;
};

template <typename CharTy>
struct null_terminator {
    static const CharTy value = CharTy(0);
};

template <>
struct null_terminator<char> {
    static const char value = '\0';
};

template <>
struct null_terminator<unsigned char> {
    static const unsigned char value = '\0';
};

template <>
struct null_terminator<wchar_t> {
    static const wchar_t value = L'\0';
};

template <>
struct null_terminator<short> {
    static const short value = L'\0';
};

template <>
struct null_terminator<unsigned short> {
    static const unsigned short value = L'\0';
};

} // namespace jstd

#endif // JSTD_CHAR_TRAITS_H
