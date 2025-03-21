
#ifndef JSTD_BASIC_STDSIZE_H
#define JSTD_BASIC_STDSIZE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Define type size_t
#ifndef _SIZE_T_DEFINED
//#include <crtdefs.h>
#include <stddef.h>
#endif  /* _SIZE_T_DEFINED */

// Linux maybe need #include <sys/types.h>

/////////////////////////////////////////////////////////////////////////
// For std::ssize_t
/////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && (_MSC_VER < 1700)

    #include "jstd/basic/msvc/stdint.h"
    #include <stddef.h>

    #ifdef __cplusplus
    namespace std {
        typedef uintptr_t   ssize_t;
    }
    #endif // __cplusplus

#else

    #include <stdint.h>
    #include <stddef.h>

    #include <cstdint>
    #include <cstddef>  // For std::ptrdiff_t

    #ifdef __cplusplus
    namespace std {
        typedef std::ptrdiff_t   ssize_t;
    }
    #endif // __cplusplus

#endif // _MSC_VER

// __ssize_t_defined for GNUC
#if !(defined(_SSIZE_T_DEFINED) || defined(__ssize_t_defined) || defined(_SSIZE_T_) || defined(_SIZE_T_) || defined(_BSD_SIZE_T_) || defined(_SIZE_T))
#if (defined(_WIN32) || defined(_WIN64)) \
    && (!(defined(__MINGW__) || defined(__MINGW32__) || defined(__MINGW64__))) && (!defined(__CYGWIN__))

typedef uintptr_t   ssize_t;

#if 0
#ifdef _WIN64
typedef signed __int64      ssize_t;
#else // !_WIN64
typedef _W64 signed int     ssize_t;
#endif // _WIN64
#endif

#else

#ifndef __size_t__	/* BeOS */
#ifndef __SIZE_T__	/* Cray Unicos/Mk */
#ifndef _SIZE_T	/* in case <sys/types.h> has defined it. */
#ifndef _SYS_SIZE_T_H
#ifndef _T_SIZE_
#ifndef _T_SIZE
#ifndef __SIZE_T
#ifndef _SIZE_T_
#ifndef _BSD_SIZE_T_
#ifndef _SIZE_T_DEFINED_
#ifndef __ssize_t_defined
#ifndef _SIZE_T_DEFINED
#ifndef _BSD_SIZE_T_DEFINED_	/* Darwin */
#ifndef _SIZE_T_DECLARED	    /* FreeBSD 5 */
#ifndef ___int_size_t_h
#ifndef _GCC_SIZE_T
#ifndef _SIZET_
#ifndef __size_t

typedef uintptr_t   ssize_t;

#if 0
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) \
    || defined(_M_X64) || defined(_M_AMD64) || defined(_WIN64)
typedef signed long long    ssize_t;
#else  /* !_M_X64 */
typedef signed int          ssize_t;
#endif  /* _M_X64 */
#endif

#endif /* __size_t */
#endif /* _SIZET_ */
#endif /* _GCC_SIZE_T */
#endif /* ___int_size_t_h */
#endif /* _SIZE_T_DECLARED */
#endif /* _BSD_SIZE_T_DEFINED_ */
#endif /* _SIZE_T_DEFINED */
#endif /* _SIZE_T_DEFINED_ */
#endif /* __ssize_t_defined */
#endif /* _BSD_SIZE_T_ */
#endif /* _SIZE_T_ */
#endif /* __SIZE_T */
#endif /* _T_SIZE */
#endif /* _T_SIZE_ */
#endif /* _SYS_SIZE_T_H */
#endif /* _SIZE_T */
#endif /* __SIZE_T__ */
#endif /* __size_t__ */

#endif // defined(_WIN32) || defined(_WIN64)
#define _SSIZE_T_DEFINED
#define __ssize_t_defined
#endif // _SSIZE_T_DEFINED

#endif // JSTD_BASIC_STDSIZE_H
