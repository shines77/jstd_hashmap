
#include "jstd/system/sleep.h"

#if defined(__MINGW32__) || defined(__CYGWIN32__)
#include <unistd.h>     // For usleep()
#elif defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER)) || defined(__linux__)
#include <unistd.h>     // For usleep()
#include <sched.h>      // For sched_yield()
#endif //__MINGW32__

#if defined(_MSC_VER) || defined(_ICL) || defined(__INTEL_COMPILER) || defined(__MINGW32__) || defined(__CYGWIN32__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>    // For Sleep(), SwitchToThread()
#endif // _MSC_VER

#if defined(__MINGW32__) || defined(__CYGWIN32__)

void jstd_sleep(unsigned int millisec)
{
    usleep(millisec * 1000);
}

void jstd_wsleep(unsigned int millisec)
{
    Sleep(millisec);
}

void jstd_yield()
{
    SwitchToThread();
}

#elif defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER)) || defined(__linux__)

void jstd_sleep(unsigned int millisec)
{
    usleep(millisec * 1000);
}

void jstd_wsleep(unsigned int millisec)
{
    if (millisec != 0)
        jstd_sleep(millisec);
    else
        usleep(250);
}

void jstd_yield()
{
    sched_yield();
}

#elif defined(_MSC_VER) || defined(_ICL) || defined(__INTEL_COMPILER)

void jstd_sleep(unsigned int millisec)
{
    Sleep(millisec);
}

void jstd_wsleep(unsigned int millisec)
{
    jstd_sleep(millisec);
}

void jstd_yield()
{
    SwitchToThread();
}

#else  /* other unknown os */

void jstd_sleep(unsigned int millisec)
{
    // Do nothing !!
    volatile unsigned int sum = 0;
    unsigned int i, j;
    for (i = 0; i < millisec; ++i) {
        sum += i;
        for (j = 50000; j >= 0; --j) {
            sum -= j;
        }
    }
}

void jstd_wsleep(unsigned int millisec)
{
    // Not implemented
}

void jstd_yield()
{
    // Not implemented
}

#endif // __linux__
