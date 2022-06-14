
#ifndef JSTD_BASIC_INTTYPES_H
#define JSTD_BASIC_INTTYPES_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1700)
#include "jstd/basic/msvc/inttypes.h"
#else
#include <inttypes.h>
#endif // _MSC_VER

#if !defined(__cplusplus) && (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)

#if defined(_MSC_VER) && !defined(__clang__)
#  if defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
#    define PRIoSIZE  "I64o"
#    define PRIuSIZE  "I64u"
#    define PRIxSIZE  "I64x"
#    define PRIXSIZE  "I64X"
#  else
#    define PRIoSIZE  "I32o"
#    define PRIuSIZE  "I32u"
#    define PRIxSIZE  "I32x"
#    define PRIXSIZE  "I32X"
#  endif
#else
#  define PRIoSIZE  "zo"
#  define PRIuSIZE  "zu"
#  define PRIxSIZE  "zx"
#  define PRIXSIZE  "zX"
#endif

#ifndef PRIuSIZE
#define PRIuSIZE  "zu"
#endif
#ifndef PRIsSIZE
#define PRIsSIZE  "z"
#endif
#else
#ifndef PRIuSIZE
#define PRIuSIZE  PRIuPTR
#endif
#ifndef PRIsSIZE
#define PRIsSIZE  PRIdPTR
#endif
#endif // __STDC_VERSION__ >= 199901L

#endif // JSTD_BASIC_INTTYPES_H
