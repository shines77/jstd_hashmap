
#ifndef JSTD_CONFIG_CONFIG_H
#define JSTD_CONFIG_CONFIG_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Minimum requirements: gcc/clang C++ 11 or MSVC 2015 Update 3.
#if (!defined(_MSC_VER) && defined(__cplusplus) && (__cplusplus < 201103L)) \
 || (defined(_MSVC_LANG) && (_MSVC_LANG < 201103L)) \
 || (defined(_MSC_VER) && (_MSC_FULL_VER < 190024210))
#error "jstd requires C++11 support."
#endif

#include "jstd/config/config_pre.h"

#include "jstd/basic/export.h"
#include "jstd/basic/compiler.h"
#include "jstd/basic/platform.h"

#include "jstd/config/version.h"
#include "jstd/config/config_jstd.h"
#include "jstd/config/config_hw.h"
#if JSTD_USE_HW_DETECT
#include "jstd/config/config_hw_detect.h"
#endif
#include "jstd/config/config_cxx.h"
#include "jstd/config/config_post.h"

#if (__has_cpp_attribute(noreturn) >= 200809L) || defined(JSTD_IS_CXX_11)
#ifndef JSTD_NORETURN
#define JSTD_NORETURN   [[noreturn]]
#endif
#else
#define JSTD_NORETURN
#endif

#if (__has_cpp_attribute(deprecated) >= 201309L) || defined(JSTD_IS_CXX_14)
#ifndef JSTD_DEPRECATED
#define JSTD_DEPRECATED [[deprecated]]
#endif
#else
#define JSTD_DEPRECATED
#endif

#if (__has_cpp_attribute(maybe_unused) >= 201603L) || defined(JSTD_IS_CXX_17)
#ifndef JSTD_MAYBE_UNUSED
#define JSTD_MAYBE_UNUSED  [[maybe_unused]]
#endif
#else
#define JSTD_MAYBE_UNUSED
#endif

//
// See: https://blog.csdn.net/qq_38617319/article/details/115099855
//
#if (__has_cpp_attribute(nodiscard) >= 201603L) || defined(JSTD_IS_CXX_17)
#ifndef JSTD_NODISCARD
#define JSTD_NODISCARD  [[nodiscard]]
#endif
#else
#define JSTD_NODISCARD
#endif

#if (__has_cpp_attribute(fallthrough) >= 201603L) || defined(JSTD_IS_CXX_17)
#ifndef JSTD_FALLTHROUGH
#define JSTD_FALLTHROUGH  [[fallthrough]]
#endif
#else
#define JSTD_FALLTHROUGH
#endif

#if (__has_cpp_attribute(likely) >= 201803L) || defined(JSTD_IS_CXX_20)
#ifndef JSTD_LIKELY
#define JSTD_LIKELY     [[likely]]
#endif
#else
#define JSTD_LIKELY
#endif

#if (__has_cpp_attribute(unlikely) >= 201803L) || defined(JSTD_IS_CXX_20)
#ifndef JSTD_UNLIKELY
#define JSTD_UNLIKELY   [[unlikely]]
#endif
#else
#define JSTD_UNLIKELY
#endif

#if (__has_cpp_attribute(no_unique_address) >= 201803L) || defined(JSTD_IS_CXX_20)
#ifndef JSTD_NO_UNIQUE_ADDRESS
#define JSTD_NO_UNIQUE_ADDRESS  [[no_unique_address]]
#endif
#else
#define JSTD_NO_UNIQUE_ADDRESS
#endif

////////////////////////////////////////////////////////////////////////////////

#if __has_builtin(__builtin_addressof) || \
    (defined(__GNUC__) && (__GNUC__ >= 7)) || defined(_MSC_VER)
#define JSTD_BUILTIN_ADDRESSOF
#endif

#if defined(__cpp_constexpr) && (__cpp_constexpr >= 200704L) && \
    !(defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 9))
#define JSTD_CPP11_CONSTEXPR
#endif

#if defined(__cpp_constexpr) && (__cpp_constexpr >= 201304L)
#define JSTD_CPP14_CONSTEXPR
#endif

#if __has_builtin(__builtin_unreachable) || defined(__GNUC__)
#define JSTD_BUILTIN_UNREACHABLE    __builtin_unreachable()
#elif defined(_MSC_VER)
#define JSTD_BUILTIN_UNREACHABLE    __assume(false)
#else
#define JSTD_BUILTIN_UNREACHABLE
#endif

#if __has_feature(cxx_exceptions) || defined(__cpp_exceptions) || \
    (defined(_MSC_VER) && defined(_CPPUNWIND)) || \
    defined(__EXCEPTIONS)
#define JSTD_EXCEPTIONS
#endif

#if defined(__cpp_generic_lambdas) || defined(_MSC_VER)
#define JSTD_GENERIC_LAMBDAS
#endif

#if defined(__cpp_lib_integer_sequence)
#define JSTD_INTEGER_SEQUENCE
#endif

#if (defined(__cpp_decltype_auto) && defined(__cpp_return_type_deduction)) || defined(_MSC_VER)
#define JSTD_RETURN_TYPE_DEDUCTION
#endif

#if defined(__cpp_lib_transparent_operators) || defined(_MSC_VER)
#define JSTD_TRANSPARENT_OPERATORS
#endif

#if defined(__cpp_variable_templates) || defined(_MSC_VER)
#define JSTD_VARIABLE_TEMPLATES
#endif

#if !defined(__GLIBCXX__) || __has_include(<codecvt>)  // >= libstdc++-5
#define JSTD_TRIVIALITY_TYPE_TRAITS
#define JSTD_INCOMPLETE_TYPE_TRAITS
#endif

#if (defined(__cpp_lib_exchange_function)  && (__cpp_lib_exchange_function >= 201304L)) || \
    (jstd_cplusplus >= 2014L)
#ifndef JSTD_EXCHANGE_FUNCTION
#define JSTD_EXCHANGE_FUNCTION
#endif
#endif // __cpp_lib_exchange_function || C++ 14

////////////////////////////////////////////////////////////////////////////////

//
// C++ 17: std::launder()
//
// See: https://stackoverflow.com/questions/39382501/what-is-the-purpose-of-stdlaunder
// See: https://wanghenshui.github.io/2019/04/27/launder.html
// See: https://en.cppreference.com/w/cpp/utility/launder
//

//
// msvc: since VC 2017 in version 15.7.0
// gcc: >= 7.0
// clang:
//
// Note: libc++ 6+ adds std::launder but does not define __cpp_lib_launder
//
#if (defined(__cpp_lib_launder) && (__cpp_lib_launder >= 201606L)) || \
    (defined(_MSC_VER) && (_HAS_LAUNDER != 0 || _MSC_VER >= 1914)) || \
    ((_LIBCPP_VERSION >= (__ANDROID__ ? 7000 : 6000)) && (__cplusplus >= 201703L))
#define JSTD_STD_LAUNDER
#endif

// __builtin_launder
#if __has_builtin(__builtin_launder) || (__GNUC__ >= 7)
#define JSTD_BUILTIN_LAUNDER
#endif

////////////////////////////////////////////////////////////////////////////////

//
// JSTD_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE
//
// Checks whether `std::is_trivially_copy_constructible<T>` and
//                `std::is_trivially_default_constructible<T>`
//  are supported.
//

//
// JSTD_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE
//
// Checks whether `std::is_trivially_copy_assignable<T>` is supported.
//

//
// Notes: Clang with libc++ supports these features, as does gcc >= 7.4 with
// libstdc++, or gcc >= 8.2 with libc++, and Visual Studio (but not NVCC).
//
#if defined(JSTD_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE)
#error "JSTD_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE" cannot be directly set
#elif defined(JSTD_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE)
#error "JSTD_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE" cannot directly set
#elif (defined(__clang__) && defined(_LIBCPP_VERSION)) ||                    \
     (!defined(__clang__) &&                                                 \
     ((__GNUC_PREREQ(7, 4) && defined(__GLIBCXX__)) ||                       \
      (__GNUC_PREREQ(8, 2) &&                                                \
       defined(_LIBCPP_VERSION)))) ||                                        \
      (defined(_MSC_VER) && !defined(__NVCC__))
#define JSTD_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE    1
#define JSTD_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE       1
#endif // JSTD_HAVE_XXXX


#endif // JSTD_CONFIG_CONFIG_H
