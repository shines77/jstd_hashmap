
/************************************************************************************

  CC BY-SA 4.0 License

  Copyright (c) 2018-2022 XiongHui Guo (gz_shines at msn.com)

  https://github.com/shines77/jstd_hashmap
  https://gitee.com/shines77/jstd_hashmap

  -------------------------------------------------------------------

  CC Attribution-ShareAlike 4.0 International

  https://creativecommons.org/licenses/by-sa/4.0/deed.en

************************************************************************************/

#ifndef JSTD_SUPPORT_CPU_PREFETCH_H
#define JSTD_SUPPORT_CPU_PREFETCH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/config/config.h"

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#if defined(_MSC_VER) && defined(JSTD_HAVE_SSE)
#include <intrin.h>
#pragma intrinsic(_mm_prefetch)
#endif

//
// Compatibility wrappers around __builtin_prefetch, to prefetch data
// for read if supported by the toolchain.

// Move data into the cache before it is read, or "prefetch" it.
//
// The value of `addr` is the address of the memory to prefetch. If
// the target and compiler support it, data prefetch instructions are
// generated. If the prefetch is done some time before the memory is
// read, it may be in the cache by the time the read occurs.
//
// The function names specify the temporal locality heuristic applied,
// using the names of Intel prefetch instructions:
//
//   T0  - high degree of temporal locality; data should be left in as
//         many levels of the cache possible
//   T1  - moderate degree of temporal locality
//   T2  - low degree of temporal locality
//   Nta - no temporal locality, data need not be left in the cache
//         after the read
//
// Incorrect or gratuitous use of these functions can degrade
// performance, so use them only when representative benchmarks show
// an improvement.
//
// Example usage:
//
//   jstd::Prefetch_T0(addr);
//
// Currently, the different prefetch calls behave on some Intel
// architectures as follows:
//
//                 SNB..SKL   SKX
//
// PrefetchT0()   L1/L2/L3  L1/L2
// PrefetchT1()      L2/L3     L2
// PrefetchT2()      L2/L3     L2
// PrefetchNta()  L1/--/L3  L1*
//
// * On SKX Prefetch_Nta() will bring the line into L1 but will evict
//   from L3 cache. This might result in surprising behavior.
//
// SNB = Sandy Bridge, SKL = Skylake, SKX = Skylake Xeon.
//

namespace jstd {

void Prefetch_Read_T0(const void * addr);
void Prefetch_Read_T1(const void * addr);
void Prefetch_Read_T2(const void * addr);
void Prefetch_Read_Nta(const void * addr);

void Prefetch_Write_T0(const void * addr);
void Prefetch_Write_T1(const void * addr);
void Prefetch_Write_T2(const void * addr);
void Prefetch_Write_Nta(const void * addr);

#if __has_builtin(__builtin_prefetch) || defined(__GNUC__)

#define JSTD_HAVE_CPU_PREFETCH  1

//
// See __builtin_prefetch:
// https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html.
//
// These functions speculatively load for read only. This is
// safe for all currently supported platforms. However, prefetch for
// store may have problems depending on the target platform.
//

//
// ARM Prefetching with __builtin_prefetch()
// See: https://developer.arm.com/documentation/101458/2010/Coding-best-practice/Prefetching-with---builtin-prefetch
//
// In Arm Compiler for Linux the target can be instructed to prefetch data using
// the __builtin_prefetch C/C++ function, which takes the syntax:
//
//   __builtin_prefetch (const void * addr[, rw[, locality]]);
//
// where:
//
//   * addr (required)
//
//      Represents the address of the memory.
//
//   * rw (optional)
//
//      A compile-time constant which can take the values:
//
//      0 : (Default), prepare the prefetch for a read.
//      1 : prepare the prefetch for a write to the memory.
//
//   * locality (optional)
//
//      A compile-time constant integer which can take the following temporal locality (L) values:
//
//      0: None, the data can be removed from the cache after the access.
//      1: Low, L3 cache, leave the data in the L3 cache level after the access.
//      2: Moderate, L2 cache, leave the data in L2 and L3 cache levels after the access.
//      3: (Default), High, L1 cache, leave the data in the L1, L2, and L3 cache levels after the access.
//

inline void Prefetch_Read_T0(const void * addr)
{
    // Note: this uses prefetcht0 on Intel.
    __builtin_prefetch(addr, 0, 3);
}

inline void Prefetch_Read_T1(const void * addr)
{
    // Note: this uses prefetcht1 on Intel.
    __builtin_prefetch(addr, 0, 2);
}

inline void Prefetch_Read_T2(const void * addr)
{
    // Note: this uses prefetcht2 on Intel.
    __builtin_prefetch(addr, 0, 1);
}

inline void Prefetch_Read_Nta(const void * addr)
{
    // Note: this uses prefetchtnta on Intel.
    __builtin_prefetch(addr, 0, 0);
}

// -----------------------------------------------------------------

inline void Prefetch_Write_T0(const void * addr)
{
    // Note: this uses prefetcht0 on Intel.
    __builtin_prefetch(addr, 1, 3);
}

inline void Prefetch_Write_T1(const void * addr)
{
    // Note: this uses prefetcht1 on Intel.
    __builtin_prefetch(addr, 1, 2);
}

inline void Prefetch_Write_T2(const void * addr)
{
    // Note: this uses prefetcht2 on Intel.
    __builtin_prefetch(addr, 1, 1);
}

inline void Prefetch_Write_Nta(const void * addr)
{
    // Note: this uses prefetchtnta on Intel.
    __builtin_prefetch(addr, 1, 0);
}

#elif defined(JSTD_HAVE_SSE)

#define JSTD_HAVE_CPU_PREFETCH  1

inline void Prefetch_Read_T0(const void * addr)
{
    _mm_prefetch(reinterpret_cast<const char *>(addr), _MM_HINT_T0);
}

inline void Prefetch_Read_T1(const void * addr)
{
    _mm_prefetch(reinterpret_cast<const char *>(addr), _MM_HINT_T1);
}

inline void Prefetch_Read_T2(const void * addr)
{
    _mm_prefetch(reinterpret_cast<const char *>(addr), _MM_HINT_T2);
}

inline void Prefetch_Read_Nta(const void * addr)
{
    _mm_prefetch(reinterpret_cast<const char *>(addr), _MM_HINT_NTA);
}

// -----------------------------------------------------------------

inline void Prefetch_Write_T0(const void * addr)
{
    _mm_prefetch(reinterpret_cast<const char *>(addr), _MM_HINT_T0);
}

inline void Prefetch_Write_T1(const void * addr)
{
    _mm_prefetch(reinterpret_cast<const char *>(addr), _MM_HINT_T1);
}

inline void Prefetch_Write_T2(const void * addr)
{
    _mm_prefetch(reinterpret_cast<const char *>(addr), _MM_HINT_T2);
}

inline void Prefetch_Write_Nta(const void * addr)
{
    _mm_prefetch(reinterpret_cast<const char *>(addr), _MM_HINT_NTA);
}

#else // !JSTD_HAVE_SSE

inline void Prefetch_Read_T0(const void * addr) {}

inline void Prefetch_Read_T1(const void * addr) {}

inline void Prefetch_Read_T2(const void * addr) {}

inline void Prefetch_Read_Nta(const void * addr) {}

// -------------------------------------------------

inline void Prefetch_Write_T0(const void * addr) {}

inline void Prefetch_Write_T1(const void * addr) {}

inline void Prefetch_Write_T2(const void * addr) {}

inline void Prefetch_Write_Nta(const void * addr) {}

#endif // JSTD_HAVE_SSE

} // namespace jstd

#endif // JSTD_SUPPORT_CPU_PREFETCH_H
