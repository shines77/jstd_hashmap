
#ifndef JSTD_BASIC_COMPILER_H
#define JSTD_BASIC_COMPILER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/macros.h"
#include "jstd/basic/platform.h"

#define JSTD_VISIBILITY_AVAILABLE

/*
   From: /Qt_5.12.1/msvc2015_64/include/QtCore/QCompilerDetection.h

   The compiler, must be one of: (JSTD_CC_xxxx)

     SYM      - Digital Mars C/C++ (used to be Symantec C++)
     MSVC     - Microsoft Visual C/C++, Intel C++ for Windows
     BORLAND  - Borland/Turbo C++
     WATCOM   - Watcom C++
     GNUC     - GNU C++
     COMEAU   - Comeau C++
     EDG      - Edison Design Group C++
     OC       - CenterLine C++
     SUN      - Forte Developer, or Sun Studio C++
     MIPS     - MIPSpro C++
     DEC      - DEC C++
     HPACC    - HP aC++
     USLC     - SCO OUDK and UDK
     CDS      - Reliant C++
     KAI      - KAI C++
     INTEL    - Intel C++ for Linux, Intel C++ for Windows
     HIGHC    - MetaWare High C/C++
     PGI      - Portland Group C++
     GHS      - Green Hills Optimizing C++ Compilers
     RVCT     - ARM Realview Compiler Suite
     CLANG    - C++ front-end for the LLVM compiler

   Should be sorted most to least authoritative.
*/

//
// What compiler is it?
//
#if defined(__DMC__) || defined(__SC__)
  #define JSTD_IS_SYM           1
  #if defined(__SC__)
    #define JSTD_CC_SYM         (__SC__)
  #else
    #define JSTD_CC_SYM         (0x101)
  #endif
  #if defined(__SC__) && (__SC__ < 0x750)
    #error "Compiler not supported"
  #endif
#elif defined(_MSC_VER)
  #ifdef __clang__
    #define JSTD_IS_CLANG       1
    #define JSTD_CC_CLANG       JSTD_MAKE_DEC2(__clang_major__, __clang_minor__)
  #endif
  /* MSVC  */
  #define JSTD_IS_MSVC              1
  #define JSTD_CC_MSVC              (_MSC_VER)
  #define JSTD_CC_MSVC_NET
  #define JSTD_OUTOFLINE_TEMPLATE   inline
  #define JSTD_COMPILER_MANGLES_RETURN_TYPE
  #define JSTD_FUNC_INFO            __FUNCSIG__
  #define JSTD_ALIGNOF(type)        __alignof(type)
  #define JSTD_DECL_ALIGN(n)        __declspec(align(n))
  #define JSTD_ASSUME_IMPL(expr)    __assume(expr)
  #define JSTD_UNREACHABLE_IMPL()   __assume(0)
  #define JSTD_NORETURN             __declspec(noreturn)
  #define JSTD_DECL_DEPRECATED      __declspec(deprecated)
  #ifndef JSTD_CC_CLANG
    #define JSTD_DECL_DEPRECATED_X(text) __declspec(deprecated(text))
  #endif
  #define JSTD_DECL_EXPORT          __declspec(dllexport)
  #define JSTD_DECL_IMPORT          __declspec(dllimport)
  #define JSTD_MAKE_UNCHECKED_ARRAY_ITERATOR(x)   stdext::make_unchecked_array_iterator(x)          // Since _MSC_VER >= 1800
  #define JSTD_MAKE_CHECKED_ARRAY_ITERATOR(x, N)  stdext::make_checked_array_iterator(x, size_t(N)) // Since _MSC_VER >= 1500
  /* Intel C++ disguising as Visual C++: the `using' keyword avoids warnings */
  #if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) \
   || defined(__ECC) || defined(__ICPC) || defined(__ECL)
    #define JSTD_DECL_VARIABLE_DEPRECATED
    #define JSTD_CC_ICC         __INTEL_COMPILER
  #endif

  /* only defined for MSVC since that's the only compiler that actually optimizes for this */
  /* might get overridden further down when JSTD_COMPILER_NOEXCEPT is detected */
  #ifdef __cplusplus
    #define JSTD_DECL_NOEXCEPT  noexcept
    #define JSTD_DECL_NOTHROW   throw()
  #endif

#elif defined(__BORLANDC__) || defined(__TURBOC__)
  #define JSTD_IS_BORLAND       1
  #if defined(__BORLANDC__)
    #define JSTD_CC_BORLAND     (__BORLANDC__)
  #else
    #define JSTD_CC_BORLAND     __TURBOC__
  #endif
  #if (__BORLANDC__ < 0x502)
    #error "Compiler not supported"
  #endif
#elif defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) \
   || defined(__ECC) || defined(__ICPC) || defined(__ECL)
  #define JSTD_IS_ICC           1
  //
  // Intel C++ compiler version
  //
  #if (__INTEL_COMPILER == 9999)
    #define JSTD_CC_ICC         (1200)  // Intel's bug in 12.1.
  #elif defined(__INTEL_COMPILER)
    #define JSTD_CC_ICC         (__INTEL_COMPILER)
  #elif defined(__ICL)
    #define JSTD_CC_ICC         __ICL
  #elif defined(__ICC)
    #define JSTD_CC_ICC         __ICC
  #elif defined(__ECC)
    #define JSTD_CC_ICC         __ECC
  #elif defined(__ICPC)
    #define JSTD_CC_ICC         __ICPC
  #elif defined(__ECL)
    #define JSTD_CC_ICC         __ECL
  #endif
#elif defined(__ARMCC__) || defined(__CC_ARM)
  /* ARM Realview Compiler Suite
     RVCT compiler also defines __EDG__ and __GNUC__ (if --gnu flag is given),
     so check for it before that */
  #define JSTD_IS_RVCT          1
  #if defined(__ARMCC__)
    #define JSTD_CC_RVCT        __ARMCC__
  #elif defined(__CC_ARM)
    #define JSTD_CC_RVCT        __CC_ARM
  #else
    #error "It's unreachable"
  #endif
  /* work-around for missing compiler intrinsics */
  #define __is_empty(X)         false
  #define __is_pod(X)           false
  #define JSTD_DECL_DEPRECATED  __attribute__((__deprecated__))
  #ifdef JSTD_IS_LINUX
    #define JSTD_DECL_EXPORT    __attribute__((visibility("default")))
    #define JSTD_DECL_IMPORT    __attribute__((visibility("default")))
    #define JSTD_DECL_HIDDEN    __attribute__((visibility("hidden")))
  #else
    #define JSTD_DECL_EXPORT    __declspec(dllexport)
    #define JSTD_DECL_IMPORT    __declspec(dllimport)
  #endif
#elif defined(__GNUC__)
  // JSTD_MAKE_DEC3(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
  #define JSTD_IS_GNUC          1
  #define JSTD_CC_GNUC          JSTD_MAKE_DEC2(__GNUC__, __GNUC_MINOR__)
  // Alias
  #define JSTD_IS_GCC           JSTD_IS_GNUC
  #define JSTD_CC_GCC           JSTD_CC_GNUC

  #if defined(__MINGW32__)
    #define JSTD_IS_MINGW       1
    #define JSTD_CC_MINGW       __MINGW32__
  #endif

  #if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) \
   || defined(__ECC) || defined(__ICPC) || defined(__ECL)
    #define JSTD_IS_ICC         1
    /* Intel C++ also masquerades as GCC */
    #define JSTD_CC_ICC         __INTEL_COMPILER
    #ifdef __clang__
      #define JSTD_IS_CLANG     1
      /* Intel C++ masquerades as Clang masquerading as GCC */
      #define JSTD_CC_CLANG     305
    #endif

    #define JSTD_ASSUME_IMPL(expr)          __assume(expr)
    #define JSTD_UNREACHABLE_IMPL()         __builtin_unreachable()
    #if (__INTEL_COMPILER >= 1300) && !defined(__APPLE__)
      #define JSTD__DECL_DEPRECATED_X(text) __attribute__((__deprecated__(text)))
    #endif
  #elif defined(__clang__)
    #define JSTD_IS_CLANG       1
    /* Clang also masquerades as GCC */
    #if defined(__apple_build_version__)
      /* http://en.wikipedia.org/wiki/Xcode#Toolchain_Versions */
      #if __apple_build_version__ >= 8020041
        #define JSTD_CC_CLANG   309
      #elif __apple_build_version__ >= 8000038
        #define JSTD_CC_CLANG   308
      #elif __apple_build_version__ >= 7000053
        #define JSTD_CC_CLANG   306
      #elif __apple_build_version__ >= 6000051
        #define JSTD_CC_CLANG   305
      #elif __apple_build_version__ >= 5030038
        #define JSTD_CC_CLANG   304
      #elif __apple_build_version__ >= 5000275
        #define JSTD_CC_CLANG   303
      #elif __apple_build_version__ >= 4250024
        #define JSTD_CC_CLANG   302
      #elif __apple_build_version__ >= 3180045
        #define JSTD_CC_CLANG   301
      #elif __apple_build_version__ >= 2111001
        #define JSTD_CC_CLANG   300
      #else
        #error "Unknown Apple Clang version"
      #endif
    #else
      #define JSTD_CC_CLANG             JSTD_MAKE_DEC2(__clang_major__, __clang_minor__)
    #endif
    // clang
    #if __has_builtin(__builtin_assume)
      #define JSTD_ASSUME_IMPL(expr)    __builtin_assume(expr)
    #else
      #define JSTD_ASSUME_IMPL(expr)    if (expr) {} else __builtin_unreachable()
    #endif
    #define JSTD_UNREACHABLE_IMPL()     __builtin_unreachable()
    #if !defined(__has_extension)
      /* Compatibility with older Clang versions */
      #define __has_extension           __has_feature
    #endif
    #if defined(__APPLE__)
      /* Apple/clang specific features */
      #define JSTD_DECL_CF_RETURNS_RETAINED         __attribute__((cf_returns_retained))
      #ifdef __OBJC__
        #define JSTD_DECL_NS_RETURNS_AUTORELEASED   __attribute__((ns_returns_autoreleased))
      #endif
    #endif
    #ifdef __EMSCRIPTEN__
      #define JSTD_CC_EMSCRIPTEN
    #endif
  #else
    /* Plain GCC */
    #if (JSTD_CC_GNUC >= 405)
      #define JSTD_ASSUME_IMPL(expr)        if (expr) {} else __builtin_unreachable()
      #define JSTD_UNREACHABLE_IMPL()       __builtin_unreachable()
      #define JSTD_DECL_DEPRECATED_X(text)  __attribute__((__deprecated__(text)))
    #endif
  #endif

  #ifdef JSTD_IS_WIN
    #define JSTD_DECL_EXPORT        __declspec(dllexport)
    #define JSTD_DECL_IMPORT        __declspec(dllimport)
  #elif defined(JSTD_VISIBILITY_AVAILABLE)
    #define JSTD_DECL_EXPORT        __attribute__((visibility("default")))
    #define JSTD_DECL_IMPORT        __attribute__((visibility("default")))
    #define JSTD_DECL_HIDDEN        __attribute__((visibility("hidden")))
  #endif

  #define JSTD_FUNC_INFO            __PRETTY_FUNCTION__
  #define JSTD_ALIGNOF(type)        __alignof__(type)
  #define JSTD_TYPEOF(expr)         __typeof__(expr)
  #define JSTD_DECL_DEPRECATED      __attribute__((__deprecated__))
  #define JSTD_DECL_ALIGN(n)        __attribute__((__aligned__(n)))
  #define JSTD_DECL_UNUSED          __attribute__((__unused__))
  #define JSTD_LIKELY(expr)         __builtin_expect(!!(expr), true)
  #define JSTD_UNLIKELY(expr)       __builtin_expect(!!(expr), false)
  #define JSTD_NORETURN             __attribute__((__noreturn__))
  #define JSTD_REQUIRED_RESULT      __attribute__((__warn_unused_result__))
  #define JSTD_DECL_PURE_FUNCTION   __attribute__((pure))
  #define JSTD_DECL_CONST_FUNCTION  __attribute__((const))
  #define JSTD_DECL_COLD_FUNCTION   __attribute__((cold))
  #if !defined(QT_MOC_CPP)
    #define JSTD_PACKED             __attribute__((__packed__))
    #ifndef __ARM_EABI__
      #define JSTD_NO_ARM_EABI
    #endif
  #endif
  #if (JSTD_CC_GNUC >= 403) && !defined(JSTD_CC_CLANG)
    #define JSTD_ALLOC_SIZE(x)      __attribute__((alloc_size(x)))
  #endif

#elif defined(__WATCOMC__)
  #define JSTD_IS_WATCOM        1
  #define JSTD_CC_WATCOM        __WATCOMC__
#else
  #error "FATAL ERROR: Unknown compiler."
#endif

#if defined(__GNUC__) || defined(__clang__) || (defined(JSTD_IS_ICC) && defined(__linux__))
#define JSTD_GCC_STYLE_ASM      1
#endif

#if defined(__GNUC__) && (!defined(__clang__) && !defined(JSTD_IS_ICC) && \
    (!defined(__ARMCC__) && !defined(__CC_ARM)))
#define JSTD_IS_PURE_GCC        1
#endif

#endif // JSTD_BASIC_COMPILER_H
