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

#ifndef JSTD_MSVC_X86_INTRIN_H
#define JSTD_MSVC_X86_INTRIN_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_M_IX86) || defined(_M_X64)

//
// See: https://sites.uclouvain.be/SystInfo/usr/include/x86intrin.h.html
//

//
// Intel Intrinsics Guide (SIMD)
//
// https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.htm
//

#ifdef __MMX__
#include <mmintrin.h>
#endif

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef __SSE3__
#include <pmmintrin.h>
#endif

#ifdef __SSSE3__
#include <tmmintrin.h>
#endif

#if defined(__SSE4A__) || defined(__SSE4a__)
#include <ammintrin.h>
#endif

#ifdef __SSE4_1__
#include <smmintrin.h>
#endif

#ifdef __SSE4_2__
#include <nmmintrin.h>
#endif

#ifdef __SSE5__
//#include <bmmintrin.h>
#endif

#if defined(__AES__) || defined(__PCLMUL__)
/* For AES && PCLMULQDQ */
#include <wmmintrin.h>
#endif

#if defined(__AVX__) || defined(__AVX2__)
/* For including AVX instructions */
#include <immintrin.h>
#if defined(__GNUC__)
#include <avxintrin.h>
#endif
#endif // __AVX__ || __AVX2__

#ifdef __3dNOW__
#if defined(_M_IX86)
#include <mm3dnow.h>
#endif
#endif

#ifdef __FMA4__
//#include <fma4intrin.h>
#endif

#endif // _M_IX86 || _M_X64

#endif // JSTD_MSVC_X86_INTRIN_H
