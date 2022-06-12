
#ifndef JSTD_BASIC_STDDEF_H
#define JSTD_BASIC_STDDEF_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Minimum requirements: gcc/clang C++ 11 or MSVC 2015 Update 3.
#if (!defined(_MSC_VER) && defined(__cplusplus) && (__cplusplus < 201103L)) \
 || (defined(_MSC_VER) && (_MSC_FULL_VER < 190024210))
#error "jstd requires C++11 support."
#endif

/// \macro __GNUC_PREREQ
/// \brief Defines __GNUC_PREREQ if glibc's features.h isn't available.
#ifndef __GNUC_PREREQ
# if defined(__GNUC__) && defined(__GNUC_MINOR__)
#  define __GNUC_PREREQ(major, minor) \
    (((__GNUC__ << 16) + __GNUC_MINOR__) >= (((major) << 16) + (minor)))
# else
#  define __GNUC_PREREQ(major, minor) 0
# endif
#endif

#ifndef __CLANG_PREREQ
# if defined(__clang__) && defined(__clang_major__) && defined(__clang_minor__)
#  define __CLANG_PREREQ(major, minor) \
    (((__clang_major__ << 16) + __clang_minor__) >= (((major) << 16) + (minor)))
# else
#  define __CLANG_PREREQ(major, minor) 0
# endif
#endif

#if defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
  #define JSTD_IS_X86           1
  #define JSTD_IS_X86_64        1
  #define JSTD_WORD_SIZE        64
#elif defined (_M_IX86) || defined(__i386__)
  #define JSTD_IS_X86           1
  #define JSTD_IS_X86_I386      1
  #define JSTD_WORD_SIZE        32
#elif defined(__aarch64__) || defined(_M_ARM64) || defined(__ARM64__) || defined(__arm64__)
  #define JSTD_IS_ARM           1
  #define JSTD_IS_ARM_64        1
  #define JSTD_WORD_SIZE        64
#elif defined(__aarch32__) || defined(_M_ARM) || defined(__ARM__) || defined(__arm__)
  #define JSTD_IS_ARM           1
  #define JSTD_IS_ARM_32        1
  #define JSTD_WORD_SIZE        32
#elif defined(_M_MPPC)
  // Power Macintosh PowerPC
  #define JSTD_WORD_SIZE        32
#elif defined(_M_PPC)
  // PowerPC
  #define JSTD_WORD_SIZE        32
#endif

#ifndef JSTD_WORD_SIZE
  #if defined(WIN32) || defined(_WIN32)
    #define JSTD_WORD_SIZE      32
  #elif defined(WIN64) || defined(_WIN64)
    #define JSTD_WORD_SIZE      64
  #endif
#endif // !JSTD_WORD_SIZE

//
// What compiler is it?
//
#if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC) \
 || defined(__ICPC) || defined(__ECL)
  #ifndef JSTD_IS_ICC
  #define JSTD_IS_ICC     1
  #endif
#endif

#if defined(_MSC_VER)
  #ifndef JSTD_IS_MSVC
  #define JSTD_IS_MSVC    1
  #endif
#elif defined(__GNUC__) && !defined(__clang__)
  #ifndef JSTD_IS_GCC
  #define JSTD_IS_GCC     1
  #endif
#elif defined(__clang__)
  #ifndef JSTD_IS_CLANG
  #define JSTD_IS_CLANG   1
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
    #define __INTEL_CXX_VERSION     1200    // Intel's bug in 12.1.
  #else
    #define __INTEL_CXX_VERSION     __INTEL_COMPILER
  #endif
#elif defined(__ICL)
#  define __INTEL_CXX_VERSION       __ICL
#elif defined(__ICC)
#  define __INTEL_CXX_VERSION       __ICC
#elif defined(__ECC)
#  define __INTEL_CXX_VERSION       __ECC
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Clang Language Extensions
//
// See: http://clang.llvm.org/docs/LanguageExtensions.html#checking_language_features
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __has_builtin                               // Optional of course.
  #define __has_builtin(x)              0           // Compatibility with non-clang compilers.
#endif

#ifndef __has_feature                               // Optional of course.
  #define __has_feature(x)              0           // Compatibility with non-clang compilers.
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

#if defined(_MSC_VER)
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

#if defined(_MSC_VER) || __has_declspec_attribute(dllexport)
  #define DLL_EXPORT        __declspec(dllexport)
#else
  #if defined(__GNUC__) || defined(__clang__) || defined(__linux__)
    #define DLL_EXPORT      __attribute__((visibility("default")))
  #else
    #define DLL_EXPORT
  #endif
#endif

#if defined(_MSC_VER) || __has_declspec_attribute(dllimport)
  #define DLL_IMPORT        __declspec(dllimport)
#else
  #define DLL_IMPORT
#endif

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
# define JSTD_ASSUME_ALIGNED(ptr, alignment)  (ptr)
#endif

#if __has_builtin(__builtin_assume_aligned) && __CLANG_PREREQ(3, 6)
#define ASSUME_IS_ALIGNED(ptr, alignment)   __builtin_assume_aligned((ptr), (alignment))
#elif __has_builtin(__builtin_assume_aligned) && __GNUC_PREREQ(4, 7)
#define ASSUME_IS_ALIGNED(ptr, alignment)   __builtin_assume_aligned((ptr), (alignment))
#else
#define ASSUME_IS_ALIGNED(ptr, alignment)   ((void *)(ptr))
#endif

#if defined(__GNUC__) && !defined(__GNUC_STDC_INLINE__) && !defined(__GNUC_GNU_INLINE__)
  #define __GNUC_GNU_INLINE__   1
#endif

/**
 * For inline, force-inline and no-inline define.
 */
#if defined(_MSC_VER)

#define JSTD_INLINE             __inline
#define JSTD_FORCED_INLINE      __forceinline
#define JSTD_NO_INLINE          __declspec(noinline)

#define JSTD_RESTRICT           __restrict

#elif defined(__GNUC__) || defined(__clang__) || defined(__MINGW32__) || defined(__CYGWIN__)

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

#if defined(_MSC_VER)
#define NAKED_DECL      __declspec(naked)
#elif defined(__attribute__)
#define NAKED_DECL      __attribute__((naked))
#else
#define NAKED_DECL
#endif

#ifndef JSTD_CDECL
#if defined(_MSC_VER)
#define JSTD_CDECL      __cdecl
#else
#define JSTD_CDECL      __attribute__((__cdecl__))
#endif
#endif // JSTD_CDECL

/**
 * For exported func
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
  #define JSTD_EXPORTED_FUNC        __cdecl
  #define JSTD_EXPORTED_METHOD      __thiscall
#else
  #define JSTD_EXPORTED_FUNC
  #define JSTD_EXPORTED_METHOD
#endif

#ifndef __cplusplus
  #ifndef nullptr
    #define nullptr     ((void *)NULL)
  #endif
#endif // __cplusplus

#ifndef JSTD_UNUSED_VAR
#define JSTD_UNUSED_VAR(x)      ((void)(x))
#endif

#define UNUSED_VARIABLE(var) \
    do { \
        (void)var; \
    } while (0)

#ifndef JSTD_TO_STRING
#define JSTD_TO_STRING(Text)    #Text
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
#define JSTD_LITTLE_ENDIAN  0
#define JSTD_BIG_ENDIAN     1

#ifndef JSTD_ENDIAN
#define JSTD_ENDIAN  JSTD_LITTLE_ENDIAN
#endif

#endif // JSTD_BASIC_STDDEF_H
