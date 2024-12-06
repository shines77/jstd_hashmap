
#ifndef JSTD_CONFIG_CONFIG_POST_H
#define JSTD_CONFIG_CONFIG_POST_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/*
    Sanitize compiler feature availability
*/
#if !defined(JSTD_IS_X86)
  #undef JSTD_HAVE_SSE2
  #undef JSTD_HAVE_SSE3
  #undef JSTD_HAVE_SSSE3
  #undef JSTD_HAVE_SSE4_1
  #undef JSTD_HAVE_SSE4_2
  #undef JSTD_HAVE_AVX
  #undef JSTD_HAVE_AVX2
#endif

#if !defined(JSTD_IS_ARM)
  #undef JSTD_HAVE_NEON
#endif

#if !defined(JSTD_IS_MIPS)
  #undef JSTD_HAVE_MIPS_DSP
  #undef JSTD_HAVE_MIPS_DSPR2
#endif

#endif // JSTD_CONFIG_CONFIG_POST_H
