/* Copyright (C) 2008, 2009 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef JSTD_SUPPORT_X86_INTRIN_H
#define JSTD_SUPPORT_X86_INTRIN_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) \
 || defined (_M_IX86) || defined(__i386__)

//
// See: https://sites.uclouvain.be/SystInfo/usr/include/x86intrin.h.html
//

//
// Intel Intrinsics Guide (SIMD)
//
// https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.htm
//

#if defined(__MMX__)
#include <mmintrin.h>
#endif

#if defined(__SSE__)
#include <xmmintrin.h>
#endif

#if defined(__SSE2__)
#include <emmintrin.h>
#endif

#if defined(__SSE3__)
#include <pmmintrin.h>
#endif

#if defined(__SSSE3__)
#include <tmmintrin.h>
#endif

#if defined(__SSE4A__) || defined(__SSE4a__)
#include <ammintrin.h>
#endif

#if defined(__SSE4_1__)
#include <smmintrin.h>
#endif

#if defined(__SSE4_2__) || defined(__CRC32__)
#include <nmmintrin.h>
#endif

#if defined(__SSE5__)
#if defined(_MSC_VER)
#include <ammintrin.h>
#else
//#include <bmmintrin.h>
#endif
#endif // __SSE5__

#if defined(__AES__) || defined(__VAES__) || defined(__PCLMUL__)  || defined(__PCLMULQDQ__)
/* For AES, VAES && PCLMULQDQ */
#include <wmmintrin.h>
#endif

#if defined(__AVX__) || defined(__AVX2__)
/* For including AVX instructions */
#include <immintrin.h>
#if defined(__GNUC__) || defined(__clang__)
#include <avxintrin.h>
#endif
#endif // __AVX__ || __AVX2__

#if defined(__POPCNT__)
#if defined(_MSC_VER)
#include <nmmintrin.h>
#else
#include <nmmintrin.h>
#include <immintrin.h>
#if defined(__GNUC__) || defined(__clang__)
#include <popcntintrin.h>
#endif
#endif // _MSC_VER
#endif // __POPCNT__

#if defined(__LZCNT__) || defined(__BMI__) || defined(__BMI1__) || defined(__BMI2__) \
 || defined(__SHA__) || defined(__FMA__)
#include <immintrin.h>
#endif

#if defined(__3dNOW__)
#if defined (_M_IX86) || defined(__i386__)
#include <mm3dnow.h>
#endif
#endif // __3dNOW__

#if defined(__FMA4__)
//#include <fma4intrin.h>
#endif

#if (defined(__GNUC__) && (__GNUC__ >= 11)) || (defined(__clang__) && (__clang_major__ >= 15))
// gcc 11.0 higher, or clang 15.0 higher
#include <x86gprintrin.h>
#endif

#endif // _M_IX86 || _M_X64 || __amd64__ || __i386__

#endif // JSTD_SUPPORT_X86_INTRIN_H
