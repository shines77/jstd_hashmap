
#ifndef JSTD_BASIC_COMPILER_H
#define JSTD_BASIC_COMPILER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define JSTD_COMPILER_MSVC      0
#define JSTD_COMPILER_GNU       1
#define JSTD_COMPILER_BORLAND   2
#define JSTD_COMPILER_INTEL     3

#if defined(_MSC_VER)
#  define JSTD_COMPILER     JSTD_COMPILER_MSVC
#elif defined(__BORLANDC__)
#  define JSTD_COMPILER     JSTD_COMPILER_BORLAND
#elif defined(__INTEL_COMPILER)
#  define JSTD_COMPILER     JSTD_COMPILER_INTEL
#elif defined(__GNUC__)
#  define JSTD_COMPILER     JSTD_COMPILER_GNU
#  define GCC_VERSION       (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#  error "FATAL ERROR: Unknown compiler."
#endif

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

//
// What compiler is it?
//
#if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC) \
 || defined(__ICPC) || defined(__ECL)
  #ifndef JSTD_IS_ICC
  #define JSTD_IS_ICC     1
  #endif
#endif

#if defined(_MSC_VER) && !defined(__clang__)
  #ifndef JSTD_IS_MSVC
  #define JSTD_IS_MSVC      1
  #endif
#elif defined(__GNUC__) && !defined(__clang__)
  #ifndef JSTD_IS_GCC
  #define JSTD_IS_GCC       1
  #endif
#elif defined(__clang__)
  #ifndef JSTD_IS_CLANG
  #define JSTD_IS_CLANG     1
  #endif
#elif !defined(JSTD_IS_ICC)
  #ifndef JSTD_IS_UNKNOWN_COMPILER
  #define JSTD_IS_UNKNOWN_COMPILER   1
  #endif
#endif // _MSC_VER

#if defined(__GNUC__) || defined(__clang__) || (defined(JSTD_IS_ICC) && defined(__linux__))
#define JSTD_GCC_STYLE_ASM  1
#endif

#if defined(__GNUC__) && (!defined(__clang__) && !defined(JSTD_IS_ICC))
#define JSTD_IS_PURE_GCC    1
#endif

//
// Intel C++ compiler version
//
#if defined(__INTEL_COMPILER)
  #if (__INTEL_COMPILER == 9999)
    #define JSTD_CC_ICC     1200    // Intel's bug in 12.1.
  #else
    #define JSTD_CC_ICC     __INTEL_COMPILER
  #endif
#elif defined(__ICL)
#  define JSTD_CC_ICC       __ICL
#elif defined(__ICC)
#  define JSTD_CC_ICC       __ICC
#elif defined(__ECC)
#  define JSTD_CC_ICC       __ECC
#endif

#endif // JSTD_BASIC_COMPILER_H
