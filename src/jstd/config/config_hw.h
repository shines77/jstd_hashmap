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

////////////////////////////////////////////////////////////////////////////////

//
// JSTD_HAVE_SSE is used for compile-time detection of SSE support.
// See https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html for an overview of
// which architectures support the various x86 instruction sets.
//
#ifdef JSTD_HAVE_SSE
#error "JSTD_HAVE_SSE" cannot be directly set
#elif defined(__SSE__)
#define JSTD_HAVE_SSE       1
#elif defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 1))
//
// MSVC only defines _M_IX86_FP for x86 32-bit code, and _M_IX86_FP >= 1
// indicates that at least SSE was targeted with the /arch:SSE option.
// All x86-64 processors support SSE, so support can be assumed.
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
//
#define JSTD_HAVE_SSE       1
#endif // JSTD_HAVE_SSE

//
// JSTD_HAVE_SSE2 is used for compile-time detection of SSE2 support.
// See https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html for an overview of
// which architectures support the various x86 instruction sets.
//
#ifdef JSTD_HAVE_SSE2
#error "JSTD_HAVE_SSE2" cannot be directly set
#elif defined(__SSE2__)
#define JSTD_HAVE_SSE2      1
#elif defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 2))
//
// MSVC only defines _M_IX86_FP for x86 32-bit code, and _M_IX86_FP >= 2
// indicates that at least SSE2 was targeted with the /arch:SSE2 option.
// All x86-64 processors support SSE2, so support can be assumed.
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
//
#define JSTD_HAVE_SSE2      1
#endif // JSTD_HAVE_SSE2

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
#elif defined(__SSSE3__)
#define JSTD_HAVE_SSSE3     1
#endif // JSTD_HAVE_SSSE3

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
#elif defined(__ARM_ACLE) && defined(__clang__) && \
    __has_builtin(__builtin_arm_cls64) &&          \
    __has_builtin(__builtin_arm_rbit64)
#define JSTD_HAVE_ARM_ACLE      1
#endif // JSTD_HAVE_ARM_ACLE

//
// JSTD_HAVE_ARM_NEON is used for compile-time detection of NEON (ARM SIMD).
//
#ifdef JSTD_HAVE_ARM_NEON
#error "JSTD_HAVE_ARM_NEON" cannot be directly set
#elif defined(__ARM_NEON)
#define JSTD_HAVE_ARM_NEON      1
#endif // JSTD_HAVE_ARM_NEON

////////////////////////////////////////////////////////////////////////////////

#endif // JSTD_CONFIG_CONFIG_HW_H
