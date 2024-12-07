
#ifndef JSTD_STRING_TRAITS_H
#define JSTD_STRING_TRAITS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
#include "jstd/basic/stdint.h"
#include "jstd/basic/stdsize.h"

#include <cstdint>
#include <cstddef>

#include "jstd/string/string_utils.h"

namespace jstd {

template <typename CharTy>
struct JSTD_DLL string_traits {
    typedef CharTy          char_type;
    typedef std::int32_t    int_type;
    typedef std::size_t     size_type;
    typedef std::intptr_t   offset_type;
    typedef std::intptr_t   pos_type;
    typedef std::ptrdiff_t  difference_type;
    typedef std::int32_t    state_type;

    static size_type copy(char_type * dest, const char_type * src, size_type count) {
        char_type * result =  str_utils::str_copy(dest, src, count);
        assert(result >= dest);
        return size_type(result - dest);
    }

    static bool is_equal(const char_type * s1, const char_type * s2) {
        return str_utils::is_equal(s1, s2);
    }

    static bool is_equal(const char_type * s1, const char_type * s2, size_type count) {
        return str_utils::is_equal_safe(s1, s2, count);
    }

    static bool is_equal(const char_type * s1, size_type len1, const char_type * s2, size_type len2) {
        return str_utils::is_equal_safe(s1, len1, s2, len2);
    }

    static int compare(const char_type * s1, const char_type * s2) {
        return str_utils::compare(s1, s2);
    }

    static int compare(const char_type * s1, const char_type * s2, size_type count) {
        return str_utils::compare_safe(s1, s2, count);
    }

    static int compare(const char_type * s1, size_type len1, const char_type * s2, size_type len2) {
        return str_utils::compare_safe(s1, len1, s2, len2);
    }
};

} // namespace jstd

#endif // JSTD_STRING_TRAITS_H
