
#ifndef JSTD_BASIC_STDDEF_H
#define JSTD_BASIC_STDDEF_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/config/config.h"

////////////////////////////////////////////////////////////////////////////////

#if __is_identifier(__wchar_t)
  // __wchar_t is not a reserved keyword
  #if !defined(_MSC_VER)
    typedef wchar_t __wchar_t;
  #endif // !_MSC_VER
#endif

////////////////////////////////////////////////////////////////////////////////
//
// C++ compiler macro define
// See: http://www.cnblogs.com/zyl910/archive/2012/08/02/printmacro.html
//

//
// Using unlikely/likely macros from boost?
// See: http://www.boost.org/doc/libs/1_60_0/boost/config/compiler/clang.hpp
//
// See:
//      http://www.boost.org/doc/libs/1_60_0/boost/config/compiler/gcc.hpp10
//      http://www.boost.org/doc/libs/1_60_0/boost/config/compiler/clang.hpp4
//      http://www.boost.org/doc/libs/1_60_0/boost/config/compiler/intel.hpp2
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// LLVM Branch Weight Metadata
// See: http://llvm.org/docs/BranchWeightMetadata.html
//
////////////////////////////////////////////////////////////////////////////////

//
// Since gcc 2.96 or Intel C++ compiler 8.0
//
// I'm not sure Intel C++ compiler 8.0 was the first version to support these builtins,
// update the condition if the version is not accurate. (Andrey Semashev)
//
#if (defined(__GNUC__) && ((__GNUC__ == 2 && __GNUC_MINOR__ >= 96) || (__GNUC__ >= 3))) \
    || (defined(__GNUC__) && (__INTEL_CXX_VERSION >= 800))
  #define SUPPORT_LIKELY        1
#elif defined(__clang__)
  //
  // clang: GCC extensions not implemented yet
  // See: http://clang.llvm.org/docs/UsersManual.html#gcc-extensions-not-implemented-yet
  //
  #if defined(__has_builtin)
    #if __has_builtin(__builtin_expect)
      #define SUPPORT_LIKELY    1
    #endif // __has_builtin(__builtin_expect)
  #endif // defined(__has_builtin)
#endif // SUPPORT_LIKELY

//
// Sample: since clang 3.4
//
#if defined(__clang__) && (__clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 4))
    // This is a sample macro.
#endif

//
// Branch prediction hints
//
#if defined(SUPPORT_LIKELY) && (SUPPORT_LIKELY != 0)
#ifndef likely
#define likely(x)               __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)             __builtin_expect(!!(x), 0)
#endif
#ifndef switch_likely
#define switch_likely(x, v)     __builtin_expect(!!(x), (v))
#endif
#else
#ifndef likely
#define likely(x)               (x)
#endif
#ifndef unlikely
#define unlikely(x)             (x)
#endif
#ifndef switch_likely
#define switch_likely(x, v)     (x)
#endif
#endif // likely() & unlikely()

//
// From: https://hackage.haskell.org/package/LibClang-3.4.0/src/llvm/include/llvm/Support/Compiler.h
//
// __builtin_assume_aligned() is support by GCC >= 4.7 and clang >= 3.6.
//

/// \macro JSTD_ASSUME_ALIGNED
/// \brief Returns a pointer with an assumed alignment.
#if __has_builtin(__builtin_assume_aligned) && __CLANG_PREREQ(3, 6)
# define JSTD_ASSUME_ALIGNED(ptr, alignment)  __builtin_assume_aligned((ptr), (alignment))
#elif __has_builtin(__builtin_assume_aligned) && __GNUC_PREREQ(4, 7)
# define JSTD_ASSUME_ALIGNED(ptr, alignment)  __builtin_assume_aligned((ptr), (alignment))
#elif defined(LLVM_BUILTIN_UNREACHABLE)
// Clang 3.4 does not support __builtin_assume_aligned().
# define JSTD_ASSUME_ALIGNED(ptr, alignment) \
           (((uintptr_t(ptr) % (alignment)) == 0) ? (ptr) : (LLVM_BUILTIN_UNREACHABLE, (ptr)))
#else
# define JSTD_ASSUME_ALIGNED(ptr, alignment)  ((void *)(ptr))
#endif

#if defined(__GNUC__) && !defined(__GNUC_STDC_INLINE__) && !defined(__GNUC_GNU_INLINE__)
  #define __GNUC_GNU_INLINE__   1
#endif

/**
 * For inline, force-inline and no-inline define.
 */
#if defined(_MSC_VER) && !defined(__clang__)

#define JSTD_INLINE             __inline
#define JSTD_FORCED_INLINE      __forceinline
#define JSTD_NO_INLINE          __declspec(noinline)

#define JSTD_RESTRICT           __restrict

#elif (defined(__GNUC__) || defined(__clang__)) && __has_attribute(always_inline)

#define JSTD_INLINE             inline __attribute__((gnu_inline))
#define JSTD_FORCED_INLINE      inline __attribute__((always_inline))
#define JSTD_NO_INLINE          __attribute__((noinline))

#define JSTD_RESTRICT           __restrict__

#else // Unknown compiler

#define JSTD_INLINE             inline
#define JSTD_FORCED_INLINE      inline
#define JSTD_NO_INLINE

#define JSTD_RESTRICT

#endif // _MSC_VER

//
// Aligned prefix and suffix declare
//
#if defined(_MSC_VER)
#ifndef ALIGNED_PREFIX
#define ALIGNED_PREFIX(n)       __declspec(align(n))
#endif
#ifndef ALIGNED_SUFFIX
#define ALIGNED_SUFFIX(n)
#endif
#else
#ifndef ALIGNED_PREFIX
#define ALIGNED_PREFIX(n)
#endif
#ifndef ALIGNED_SUFFIX
#define ALIGNED_SUFFIX(n)       __attribute__((aligned(n)))
#endif
#endif // ALIGNED(n)

/*********************************************************************************
    GCC:

    uint32_t fill_cache(void) __attribute__((naked)); // Declaration
    attribute should be specified in declaration not in implementation

    uint32_t fill_cache(void)
    {
        __asm__ ("addi 3, 0, 0\n");  // R3 = 0
        // More asm here modifying R3 and filling the cache lines.
    }

*********************************************************************************/

#if defined(_MSC_VER) && !defined(__clang__)
#define NAKED_DECL_PREFIX      __declspec(naked)
#define NAKED_DECL_SUFFIX
#elif defined(__attribute__)
#define NAKED_DECL_PREFIX
#define NAKED_DECL_SUFFIX      __attribute__((naked))
#else
#define NAKED_DECL_PREFIX
#define NAKED_DECL_SUFFIX
#endif

#ifndef JSTD_CDECL
#define JSTD_CDECL
#if defined(_MSC_VER) && !defined(__clang__)
#define JSTD_CDECL_PREFIX      __cdecl
#define JSTD_CDECL_SUFFIX
#else
#define JSTD_CDECL_PREFIX
#define JSTD_CDECL_SUFFIX      __attribute__((__cdecl__))
#endif
#endif // JSTD_CDECL

#if !defined(__cplusplus) || (defined(_MSC_VER) && (_MSC_VER < 1400))
  #ifndef nullptr
    #define nullptr     ((void *)NULL)
  #endif
#endif // __cplusplus

#ifndef JSTD_UNUSED
#define JSTD_UNUSED(var) \
    do { \
        (void)var; \
    } while (0)
#endif

#define STD_IOS_RIGHT(width, var) \
    std::right << std::setw(width) << (var)

#define STD_IOS_LEFT(width, var) \
    std::left << std::setw(width) << (var)

#define STD_IOS_DEFAULT() \
    std::left << std::setw(0)

#ifndef JSTD_ASSERT
#ifdef _DEBUG
#define JSTD_ASSERT(express)            assert(!!(express))
#else
#define JSTD_ASSERT(express)            ((void)0)
#endif
#endif // JSTD_ASSERT

#ifndef JSTD_ASSERT_EX
#ifdef _DEBUG
#define JSTD_ASSERT_EX(express, text)   assert(!!(express))
#else
#define JSTD_ASSERT_EX(express, text)   ((void)0)
#endif
#endif // JSTD_ASSERT

#ifndef JSTD_STATIC_ASSERT
#if (__cplusplus < 201103L) || (defined(_MSC_VER) && (_MSC_VER < 1800))
#define JSTD_STATIC_ASSERT(express, text)       assert(!!(express))
#else
#define JSTD_STATIC_ASSERT(express, text)       static_assert(!!(express), text)
#endif
#endif // JSTD_STATIC_ASSERT

//
// Little-Endian or Big-Endian
//
#define JSTD_LITTLE_ENDIAN  1234
#define JSTD_BIG_ENDIAN     4321

/*
* My best guess at if you are big-endian or little-endian.
* This may need adjustment.
*/
#ifndef JSTD_ENDIAN
#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)) || \
    (defined(i386) || defined(__i386__) || defined(__i486__) || \
     defined(__i586__) || defined(__i686__) || defined(vax) || defined(MIPSEL) || \
     defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64) || \
     defined(__amd64__) || defined(__x86_64__) || defined (_M_IX86))
  #define JSTD_ENDIAN  JSTD_LITTLE_ENDIAN
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)) || \
      (defined(sparc) || defined(POWERPC) || defined(mc68000) || defined(sel))
  #define JSTD_ENDIAN  JSTD_BIG_ENDIAN
#else
  #define JSTD_ENDIAN  JSTD_LITTLE_ENDIAN
#endif // __BYTE_ORDER
#endif // !JSTD_ENDIAN

#endif // JSTD_BASIC_STDDEF_H
