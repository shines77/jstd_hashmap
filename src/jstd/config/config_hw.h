#ifndef JSTD_CONFIG_CONFIG_HW_H
#define JSTD_CONFIG_CONFIG_HW_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define JSTD_USE_HW_DETECT      1

#define JSTD_ALWAYS_USE_SIMD    1

#define JSTD_USE_MMX            1
#define JSTD_USE_SSE            1
#define JSTD_USE_SSE2           1
#define JSTD_USE_AES            1
#define JSTD_USE_SSE3           1
#define JSTD_USE_SSSE3          1
#define JSTD_USE_SSE4_1         1
#define JSTD_USE_SSE4_2         1
#define JSTD_USE_AVX            1
#define JSTD_USE_AVX2           1

#define JSTD_USE_F16C           1
#define JSTD_USE_RDRND          1
#define JSTD_USE_SHA            1

#if defined(JSTD_USE_HW_DETECT) && (JSTD_USE_HW_DETECT != 0)
#include "jstd/config/config_hw_detect.h"
#endif

////////////////////////////////////////////////////////////////////////////////

// MMX

#ifdef JSTD_HAVE_MMX
#error "JSTD_HAVE_MMX" cannot be directly set
#elif defined(__MMX__) || defined(HAVE_MMX)
#define JSTD_HAVE_MMX
#elif defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 1))
#define JSTD_HAVE_MMX
#endif

// SSE

//
// JSTD_HAVE_SSE is used for compile-time detection of SSE support.
// See https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html for an overview of
// which architectures support the various x86 instruction sets.
//
#ifdef JSTD_HAVE_SSE
#error "JSTD_HAVE_SSE" cannot be directly set
#elif defined(__SSE__) || defined(HAVE_SSE)
#define JSTD_HAVE_SSE
#elif defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 1))
//
// MSVC only defines _M_IX86_FP for x86 32-bit code, and _M_IX86_FP >= 1
// indicates that at least SSE was targeted with the /arch:SSE option.
// All x86-64 processors support SSE, so support can be assumed.
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
//
#define JSTD_HAVE_SSE
#endif // JSTD_HAVE_SSE

//
// JSTD_HAVE_SSE2 is used for compile-time detection of SSE2 support.
// See https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html for an overview of
// which architectures support the various x86 instruction sets.
//
#ifdef JSTD_HAVE_SSE2
#error "JSTD_HAVE_SSE2" cannot be directly set
#elif defined(__SSE2__) || defined(HAVE_SSE2)
#define JSTD_HAVE_SSE2
#elif defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 2))
//
// MSVC only defines _M_IX86_FP for x86 32-bit code, and _M_IX86_FP >= 2
// indicates that at least SSE2 was targeted with the /arch:SSE2 option.
// All x86-64 processors support SSE2, so support can be assumed.
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
//
#define JSTD_HAVE_SSE2
#endif // JSTD_HAVE_SSE2

#ifdef JSTD_HAVE_SSE3
#error "JSTD_HAVE_SSE3" cannot be directly set
#elif defined(__SSE3__) || defined(HAVE_SSE3)
#define JSTD_HAVE_SSE3
#endif

//
// JSTD_HAVE_SSSE3 is used for compile-time detection of SSSE3 support.
// See https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html for an overview of
// which architectures support the various x86 instruction sets.
//
// MSVC does not have a mode that targets SSSE3 at compile-time. To use SSSE3
// with MSVC requires either assuming that the code will only every run on CPUs
// that support SSSE3, otherwise __cpuid() can be used to detect support at
// runtime and fallback to a non-SSSE3 implementation when SSSE3 is unsupported
// by the CPU.
//
#ifdef JSTD_HAVE_SSSE3
#error "JSTD_HAVE_SSSE3" cannot be directly set
#elif defined(__SSSE3__) || defined(HAVE_SSSE3)
#define JSTD_HAVE_SSSE3
#endif

#ifdef JSTD_HAVE_SSE4_1
#error "JSTD_HAVE_SSE4_1" cannot be directly set
#elif defined(__SSE4_1__) || defined(HAVE_SSE4_1)
#define JSTD_HAVE_SSE4_1
#endif

#ifdef JSTD_HAVE_SSE4_2
#error "JSTD_HAVE_SSE4_2" cannot be directly set
#elif defined(__SSE4_2__) || defined(HAVE_SSE4_2)
#define JSTD_HAVE_SSE4_2
#endif

// AES

#ifdef JSTD_HAVE_AES
#error "JSTD_HAVE_AES" cannot be directly set
#elif defined(__AES__) || defined(HAVE_AES)
#define JSTD_HAVE_AES
#endif

// AVX

#ifdef JSTD_HAVE_AVX
#error "JSTD_HAVE_AVX" cannot be directly set
#elif defined(__AVX__) || defined(HAVE_AVX)
#define JSTD_HAVE_AVX
#endif

#ifdef JSTD_HAVE_AVX2
#error "JSTD_HAVE_AVX2" cannot be directly set
#elif defined(__AVX2__) || defined(HAVE_AVX2)
#define JSTD_HAVE_AVX2
#endif

// FMA3

#ifdef JSTD_HAVE_FMA3
#error "JSTD_HAVE_FMA3" cannot be directly set
#elif defined(__FMA3__) || defined(HAVE_FMA3)
#define JSTD_HAVE_FMA3
#endif

// AVX512

#ifdef JSTD_HAVE_AVX512F
#error "JSTD_HAVE_AVX512F" cannot be directly set
#elif defined(__AVX512F__) || defined(HAVE_AVX512F)
#define JSTD_HAVE_AVX512F
#endif

#ifdef JSTD_HAVE_AVX512VL
#error "JSTD_HAVE_AVX512VL" cannot be directly set
#elif defined(__AVX512VL__) || defined(HAVE_AVX512VL)
#define JSTD_HAVE_AVX512VL
#endif

#ifdef JSTD_HAVE_AVX512CD
#error "JSTD_HAVE_AVX512CD" cannot be directly set
#elif defined(__AVX512CD__) || defined(HAVE_AVX512CD)
#define JSTD_HAVE_AVX512CD
#endif

#ifdef JSTD_HAVE_AVX512ER
#error "JSTD_HAVE_AVX512ER" cannot be directly set
#elif defined(__AVX512ER__) || defined(HAVE_AVX512ER)
#define JSTD_HAVE_AVX512ER
#endif

#ifdef JSTD_HAVE_AVX512PF
#error "JSTD_HAVE_AVX512PF" cannot be directly set
#elif defined(__AVX512PF__) || defined(HAVE_AVX512PF)
#define JSTD_HAVE_AVX512PF
#endif

#ifdef JSTD_HAVE_AVX512BW
#error "JSTD_HAVE_AVX512BW" cannot be directly set
#elif defined(__AVX512BW__) || defined(HAVE_AVX512BW)
#define JSTD_HAVE_AVX512BW
#endif

#ifdef JSTD_HAVE_AVX512DQ
#error "JSTD_HAVE_AVX512DQ" cannot be directly set
#elif defined(__AVX512DQ__) || defined(HAVE_AVX512DQ)
#define JSTD_HAVE_AVX512DQ
#endif

#ifdef JSTD_HAVE_AVX512IFMA
#error "JSTD_HAVE_AVX512IFMA" cannot be directly set
#elif defined(__AVX512IFMA__) || defined(HAVE_AVX512IFMA)
#define JSTD_HAVE_AVX512IFMA
#endif

#ifdef JSTD_HAVE_AVX512VBMI
#error "JSTD_HAVE_AVX512VBMI" cannot be directly set
#elif defined(__AVX512VBMI__) || defined(HAVE_AVX512VBMI)
#define JSTD_HAVE_AVX512VBMI
#endif

#ifdef JSTD_HAVE_AVX512BF16
#error "JSTD_HAVE_AVX512BF16" cannot be directly set
#elif defined(__AVX512BF16__) || defined(HAVE_AVX512BF16)
#define JSTD_HAVE_AVX512BF16
#endif

#ifdef JSTD_HAVE_AVX512FP16
#error "JSTD_HAVE_AVX512FP16" cannot be directly set
#elif defined(__AVX512FP16__) || defined(HAVE_AVX512FP16)
#define JSTD_HAVE_AVX512FP16
#endif

#ifdef JSTD_HAVE_AVX512VNNI
#error "JSTD_HAVE_AVX512VNNI" cannot be directly set
#elif defined(__AVX512VNNI__) || defined(HAVE_AVX512VNNI)
#define JSTD_HAVE_AVX512VNNI
#endif

#ifdef JSTD_HAVE_AVX512VPOPCNTDQ
#error "JSTD_HAVE_AVX512VPOPCNTDQ" cannot be directly set
#elif defined(__AVX512VPOPCNTDQ__) || defined(HAVE_AVX512VPOPCNTDQ)
#define JSTD_HAVE_AVX512VPOPCNTDQ
#endif

#ifdef JSTD_HAVE_AVX512BITALG
#error "JSTD_HAVE_AVX512BITALG" cannot be directly set
#elif defined(__AVX512BITALG__) || defined(HAVE_AVX512BITALG)
#define JSTD_HAVE_AVX512BITALG
#endif

#ifdef JSTD_HAVE_AVX512VP2INTERSECT
#error "JSTD_HAVE_AVX512VP2INTERSECT" cannot be directly set
#elif defined(__AVX512VP2INTERSECT__) || defined(HAVE_AVX512VP2INTERSECT)
#define JSTD_HAVE_AVX512VP2INTERSECT
#endif

#ifdef JSTD_HAVE_AVX5124FMAPS
#error "JSTD_HAVE_AVX5124FMAPS" cannot be directly set
#elif defined(__AVX5124FMAPS__) || defined(HAVE_AVX5124FMAPS)
#define JSTD_HAVE_AVX5124FMAPS
#endif

#ifdef JSTD_HAVE_AVX5124VNNIW
#error "JSTD_HAVE_AVX5124VNNIW" cannot be directly set
#elif defined(__AVX5124VNNIW__) || defined(HAVE_AVX5124VNNIW)
#define JSTD_HAVE_AVX5124VNNIW
#endif

#ifdef JSTD_HAVE_AVX5124VBMI2
#error "JSTD_HAVE_AVX5124VBMI2" cannot be directly set
#elif defined(__AVX5124VBMI2__) || defined(HAVE_AVX5124VBMI2)
#define JSTD_HAVE_AVX5124VBMI2
#endif

#ifdef JSTD_HAVE_AVX5124GFNI
#error "JSTD_HAVE_AVX5124GFNI" cannot be directly set
#elif defined(__AVX5124GFNI__) || defined(HAVE_AVX5124GFNI)
#define JSTD_HAVE_AVX5124GFNI
#endif

#ifdef JSTD_HAVE_AVX5124VPCLMULQDQ
#error "JSTD_HAVE_AVX5124VPCLMULQDQ" cannot be directly set
#elif defined(__AVX5124VPCLMULQDQ__) || defined(HAVE_AVX5124VPCLMULQDQ)
#define JSTD_HAVE_AVX5124VPCLMULQDQ
#endif

//
// JSTD_HAVE_ARM_ACLE is used for compile-time detection of ACLE (ARM
// C language extensions).
//
#ifdef JSTD_HAVE_ARM_ACLE
#error "JSTD_HAVE_ARM_ACLE" cannot be directly set
//
// __cls, __rbit were added quite late in clang. They are not supported
// by GCC as well. __cls can be replaced with __builtin_clrsb but clang does
// not recognize cls instruction in latest versions.
// TODO(b/233604649): Relax to __builtin_clrsb and __builtin_bitreverse64 (note
// that the latter is not supported by GCC).
//
#elif (defined(__ARM_ACLE) || defined(HAVE_ARM_ACLE)) && defined(__clang__) &&  \
    __has_builtin(__builtin_arm_cls64) &&                                       \
    __has_builtin(__builtin_arm_rbit64)
#define JSTD_HAVE_ARM_ACLE
#endif

//
// JSTD_HAVE_ARM_NEON is used for compile-time detection of NEON (ARM SIMD).
//
#ifdef JSTD_HAVE_ARM_NEON
#error "JSTD_HAVE_ARM_NEON" cannot be directly set
#elif defined(__ARM_NEON) || defined(HAVE_ARM_NEON)
#define JSTD_HAVE_ARM_NEON
#endif

////////////////////////////////////////////////////////////////////////////////

#endif // JSTD_CONFIG_CONFIG_HW_H
