
#ifndef JSTD_CONFIG_CONFIG_HW_DETECT_H
#define JSTD_CONFIG_CONFIG_HW_DETECT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef JSTD_HAVE_MMX
#define JSTD_HAVE_MMX           1
#endif

#ifndef JSTD_HAVE_SSE
#define JSTD_HAVE_SSE           1
#endif

#ifndef JSTD_HAVE_SSE2
#define JSTD_HAVE_SSE2          1
#endif

#ifndef JSTD_HAVE_AES
#define JSTD_HAVE_AES           1
#endif

#ifndef JSTD_HAVE_MMX
#define JSTD_HAVE_SSE3          1
#endif

#ifndef JSTD_HAVE_SSSE3
#define JSTD_HAVE_SSSE3         1
#endif

#ifndef JSTD_HAVE_SSE4_1
#define JSTD_HAVE_SSE4_1        1
#endif

#ifndef JSTD_HAVE_SSE4_2
#define JSTD_HAVE_SSE4_2        1
#endif

#ifndef JSTD_HAVE_AVX
#define JSTD_HAVE_AVX           1
#endif

#ifndef JSTD_HAVE_AVX2
#define JSTD_HAVE_AVX2          1
#endif

#ifndef JSTD_HAVE_AVX512
#define JSTD_HAVE_AVX512        0
#endif

#ifndef JSTD_HAVE_F16C
#define JSTD_HAVE_F16C          1
#endif

#ifndef JSTD_HAVE_RDRND
#define JSTD_HAVE_RDRND         1
#endif

#ifndef JSTD_HAVE_SHA
#define JSTD_HAVE_SHA           1
#endif

#endif // JSTD_CONFIG_CONFIG_HW_DETECT_H
