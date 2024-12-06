
#ifndef JSTD_BASIC_COMPILER_H
#define JSTD_BASIC_COMPILER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/macros.h"
#include "jstd/basic/platform.h"

/*
   The compiler, must be one of: (JSTD_CC_xxxx)

     SYM      - Digital Mars C/C++ (used to be Symantec C++)
     MSVC     - Microsoft Visual C/C++, Intel C++ for Windows
     BORLAND  - Borland/Turbo C++
     WAT      - Watcom C++
     GNU      - GNU C++
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
    #define JSTD_CC_SYM         __SC__
  #else
    #define JSTD_CC_SYM         0x101
  #endif
  #if defined(__SC__) && (__SC__ < 0x750)
    #error "Compiler not supported"
  #endif
#elif defined(_MSC_VER)
  #ifdef __clang__
    #define JSTD_IS_CLANG       1
    #define JSTD_CC_CLANG       1
  #else
    #define JSTD_IS_MSVC        1
    #define JSTD_CC_MSVC        _MSC_VER
  #endif
#elif defined(__BORLANDC__) || defined(__TURBOC__)
  #define JSTD_IS_BORLAND       1
  #if defined(__BORLANDC__)
    #define JSTD_CC_BORLAND     __BORLANDC__
  #else
    #define JSTD_CC_BORLAND     __TURBOC__
  #endif
  #if (__BORLANDC__ < 0x502)
    #error "Compiler not supported"
  #endif
#elif defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) \
   || defined(__ECC) || defined(__ICPC) || defined(__ECL)
  #define JSTD_IS_INTEL         1
  //
  // Intel C++ compiler version
  //
  #if (__INTEL_COMPILER == 9999)
    #define JSTD_CC_INTEL       1200    // Intel's bug in 12.1.
  #elif defined(__INTEL_COMPILER)
    #define JSTD_CC_INTEL       __INTEL_COMPILER
  #elif defined(__ICL)
    #define JSTD_CC_INTEL       __ICL
  #elif defined(__ICC)
    #define JSTD_CC_INTEL       __ICC
  #elif defined(__ECC)
    #define JSTD_CC_INTEL       __ECC
  #elif defined(__ICPC)
    #define JSTD_CC_INTEL       __ICPC
  #elif defined(__ECL)
    #define JSTD_CC_INTEL       __ECL
  #endif
  // Alias
  #define JSTD_IS_ICC           1
  #define JSTD_CC_ICC           JSTD_CC_INTEL
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
  #define JSTD_IS_GNUC          1
  #define JSTD_CC_GNUC          JSTD_MAKE_DEC3(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
  // Alias
  #define JSTD_IS_GCC           JSTD_IS_GNUC
  #define JSTD_CC_GNUC          JSTD_CC_GNUC

  #if defined(__MINGW32__)
    #define JSTD_IS_MINGW       1
    #define JSTD_CC_MINGW       __MINGW32__
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
