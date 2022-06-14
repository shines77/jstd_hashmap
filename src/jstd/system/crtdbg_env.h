
#ifndef JSTD_SYSTEM_CRTDEG_ENV_H
#define JSTD_SYSTEM_CRTDEG_ENV_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

//#include "jstd/config/config.h"

//
// See: http://msdn.microsoft.com/zh-cn/library/e5ewb1h3%28v=vs.90%29.aspx
// See: http://msdn.microsoft.com/en-us/library/x98tx3cf.aspx
//
#if defined(JSTD_USE_CRTDBG_CHECK) && (JSTD_USE_CRTDBG_CHECK != 0)
#if defined(_DEBUG) || !defined(NDEBUG)
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif
#endif // _DEBUG
#endif // JSTD_USE_CRTDBG_CHECK

/* Memory leak detection based on CRT */
#if defined(JSTD_USE_CRTDBG_CHECK) && (JSTD_USE_CRTDBG_CHECK != 0)

//If this macro is not defined, malloc leaks in C mode will not be recorded.
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif

// _CRTDBG_MAP_ALLOC macro must be include before <stdlib.h> file
#include <stdlib.h>

// crtdbg.h must be behind the stdlib.h
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

//
// Memory leak detection based on CRT in C++
//
// See: http://www.cnblogs.com/weiym/archive/2013/02/25/2932810.html
//

#if (defined(_DEBUG) || !defined(NDEBUG))
  #ifndef DBG_NEW
    #if defined(_MSC_VER) && (_MSC_VER >= 1600)
      #define DBG_NEW  new(_CLIENT_BLOCK, __FILE__, __LINE__)
    #else
      #define DBG_NEW  new(_NORMAL_BLOCK, __FILE__, __LINE__)
    #endif
  #endif  /* DBG_NEW */
#else
  #ifndef DBG_NEW
    #define DBG_NEW   new
  #endif  /* DBG_NEW */
#endif  /* _DEBUG */

#if defined(_DEBUG) || !defined(NDEBUG)
  #if defined(_MSC_VER) && (_MSC_VER >= 1600)
    #ifndef new
      #define new DBG_NEW
    #endif
  #endif  /* defined(_MSC_VER) && (_MSC_VER >= 1600) */
#endif  /* defined(_DEBUG) || !defined(NDEBUG) */

#else

// crtdbg.h must be behind the stdlib.h
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#endif // JSTD_USE_CRTDBG_CHECK

#define DONT_DISPLAY_MEMORY_LEAK    0
#define ALLOW_DISPLAY_MEMORY_LEAK   1

#define DONT_CHECK_BOUNDS           0
#define ALWAYS_CHECK_BOUNDS         1

#ifdef __cplusplus
extern "C" {
#endif

/* Set the environment of CRTDBG (check memory overrun and memory leak in debug mode) */
void jstd_set_crtdbg_env(int display_memory_leak, int always_check_bounds);

#ifdef __cplusplus
}
#endif

#endif // JSTD_SYSTEM_CRTDEG_ENV_H
