
#ifndef JSTD_CONFIG_CONFIG_PRE_H
#define JSTD_CONFIG_CONFIG_PRE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Clang Language Extensions
//
// See: http://clang.llvm.org/docs/LanguageExtensions.html#checking_language_features
//
////////////////////////////////////////////////////////////////////////////////

//
// Feature testing (C++20)
// See: https://en.cppreference.com/w/cpp/feature_test
//
#ifndef __has_feature                               // Optional of course.
  #define __has_feature(x)              0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_builtin                               // Optional of course.
  #define __has_builtin(x)              0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_extension
  #define __has_extension               __has_feature   // Compatibility with pre-3.0 compilers.
#endif

#ifndef __has_cpp_attribute                         // Optional of course.
  #define __has_cpp_attribute(x)        0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_c_attribute                           // Optional of course.
  #define __has_c_attribute(x)          0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_attribute                             // Optional of course.
  #define __has_attribute(x)            0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_declspec_attribute                    // Optional of course.
  #define __has_declspec_attribute(x)   0           // Compatibility with non-clang compilers.
#endif

#ifndef __is_identifier                             // Optional of course.
  // It evaluates to 1 if the argument x is just a regular identifier and not a reserved keyword.
  #define __is_identifier(x)            1           // Compatibility with non-clang compilers.
#endif

// Since C++ 17
#ifndef __has_include
  #define __has_include(x)              0
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#ifndef __attribute__
  #define __attribute__(x)
#endif
#endif

////////////////////////////////////////////////////////////////////////////////

#if __has_feature(cxx_rvalue_references)
    // This code will only be compiled with the -std=c++11 and -std=gnu++11
    // options, because rvalue references are only standardized in C++11.
#endif

#if __has_extension(cxx_rvalue_references)
    // This code will be compiled with the -std=c++11, -std=gnu++11, -std=c++98
    // and -std=gnu++98 options, because rvalue references are supported as a
    // language extension in C++98.
#endif

////////////////////////////////////////////////////////////////////////////////

/// \macro __GNUC_PREREQ
/// \brief Defines __GNUC_PREREQ if glibc's features.h isn't available.
#ifndef __GNUC_PREREQ
  #if defined(__GNUC__) && defined(__GNUC_MINOR__)
    #define __GNUC_PREREQ(major, minor) \
        (((__GNUC__ << 16) + __GNUC_MINOR__) >= (((major) << 16) + (minor)))
  #else
    #define __GNUC_PREREQ(major, minor)     0
  #endif
#endif

#ifndef __CLANG_PREREQ
  #if defined(__clang__) && defined(__clang_major__) && defined(__clang_minor__)
    #define __CLANG_PREREQ(major, minor) \
        (((__clang_major__ << 16) + __clang_minor__) >= (((major) << 16) + (minor)))
  #else
    #define __CLANG_PREREQ(major, minor)    0
  #endif
#endif

#endif // JSTD_CONFIG_CONFIG_PRE_H
