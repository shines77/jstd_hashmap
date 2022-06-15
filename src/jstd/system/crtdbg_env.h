
#ifndef JSTD_SYSTEM_CRTDEG_ENV_H
#define JSTD_SYSTEM_CRTDEG_ENV_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/config/config.h"

//
// See: http://msdn.microsoft.com/zh-cn/library/e5ewb1h3%28v=vs.90%29.aspx
// See: http://msdn.microsoft.com/en-us/library/x98tx3cf.aspx
//
#if defined(JSTD_USE_CRTDBG_CHECK) && (JSTD_USE_CRTDBG_CHECK != 0)
#ifdef _DEBUG
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

#ifdef _DEBUG
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

#ifdef _DEBUG
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

/***********************************************************************************

    Set the environment of CRTDBG (check for memory out-of-bounds and
    memory leak problems in debug mode):

    Always_check_bounds non-zero means that every time memory is allocated/released,
    the system will automatically call _CrtCheckMemory() to check the memory
    out-of-bounds situation Use_vld non-zero means, use vld to check for memory
    overlap problems, instead of using CrtDbg's memory allocation detection.

 ************************************************************************************/
static inline
void jstd_set_crtdbg_env(int display_memory_leak, int always_check_bounds)
{
#if defined(JSTD_USE_CRTDBG_CHECK) && (JSTD_USE_CRTDBG_CHECK != 0)

#if defined(_MSC_VER)
#ifdef _DEBUG || !defined(NDEBUG)
    // Using CRTDBG will check the memory out of bounds problem,
    // if you use vld, the memory leak information can be closed.
    int dbg_flag = 0;

    // Setting CRT report mode
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);

    dbg_flag |= _CRTDBG_ALLOC_MEM_DF;

    if (display_memory_leak != 0) {
        // If vld.h has been referenced, the memory leak information of crtdbg is not displayed
#ifndef VLD_RPTHOOK_INSTALL
        // When the process exits, display memory leak information.
        dbg_flag |= _CRTDBG_LEAK_CHECK_DF;
#endif
    }

    if (always_check_bounds != 0) {
        // _CRTDBG_CHECK_ALWAYS_DF means that every time memory is allocated/released,
        // the system will automatically call _CrtCheckMemory() to check for memory out of bounds
        dbg_flag |= _CRTDBG_CHECK_ALWAYS_DF;
    }

    // Setting CrtDbgFlag
    if (dbg_flag != 0)
        _CrtSetDbgFlag(dbg_flag);

#endif // _DEBUG
#endif // _MSC_VER

#endif // JSTD_USE_CRTDBG_CHECK
}

#ifdef __cplusplus
}
#endif

#endif // JSTD_SYSTEM_CRTDEG_ENV_H
