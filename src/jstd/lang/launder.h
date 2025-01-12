
#ifndef JSTD_LANG_LAUNDER_H
#define JSTD_LANG_LAUNDER_H

#pragma once

#include <new>

#include "jstd/basic/stddef.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif // _WIN32

namespace jstd {

//
// Reference from: https://github.com/facebook/folly/blob/main/folly/lang/Launder.h
//

//
// Note: libc++ 6+ adds std::launder but does not define __cpp_lib_launder
// msvc: since VC 2017 in version 15.7.0
//
#if (defined(__cpp_lib_launder) && (__cpp_lib_launder >= 201606L)) || \
    (defined(_MSC_VER) && (_HAS_LAUNDER != 0 || _MSC_VER >= 1914)) || \
    ((_LIBCPP_VERSION >= (__ANDROID__ ? 7000 : 6000)) && (__cplusplus >= 201703L))

/* using override */
using std::launder;

#else // !JSTD_STD_LAUNDER

/**
 * Approximate backport from C++17 of std::launder. It should be `constexpr`
 * but that can't be done without specific support from the compiler.
 */
template <typename T>
JSTD_NODISCARD
inline
T * launder(T * p) noexcept
{
#if __has_builtin(__builtin_launder) || (__GNUC__ >= 7)
    // The builtin has no unwanted side-effects.
    return __builtin_launder(p);
#elif defined(_MSC_VER)
    // MSVC does not currently have optimizations around const members of structs.
    // _ReadWriteBarrier() will prevent compiler reordering memory accesses.
    _ReadWriteBarrier();
    return p;
#elif defined(__GNUC__) || defined(__clang__)
    // This inline assembler block declares that `p` is an input and an output,
    // so the compiler has to assume that it has been changed inside the block.
    __asm__("" : "+r"(p));
    return p;
#else
    static_assert(false, "jstd::launder() is not implemented for this environment.");
#endif
}

/* The standard explicitly forbids laundering these */
inline
void launder(void *) = delete;

inline
void launder(void const *) = delete;

inline
void launder(void volatile *) = delete;

inline
void launder(void const volatile *) = delete;

template <typename T, typename... Args>
inline
void launder(T (*)(Args...)) = delete;

#endif // JSTD_STD_LAUNDER

} // namespace jstd

#endif // JSTD_LANG_LAUNDER_H
