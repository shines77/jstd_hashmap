
#include "jstd/system/crtdbg_env.h"

/***********************************************************************************

    Set the environment of CRTDBG (in debug mode, check for memory out-of-bounds
        and memory leak problems):

    Always_check_bounds non-zero means that every time memory is allocated/released,
    the system will automatically call _CrtCheckMemory() to check the memory
    out-of-bounds situation Use_vld non-zero means, use vld to check for memory overlap problems,
    instead of using CrtDbg's memory allocation detection.

 ************************************************************************************/
void jstd_set_crtdbg_env(int display_memory_leak, int always_check_bounds)
{
#if defined(JSTD_USE_CRTDBG_CHECK) && (JSTD_USE_CRTDBG_CHECK != 0)

#if defined(_MSC_VER)
#if defined(_DEBUG) || !defined(NDEBUG)
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
