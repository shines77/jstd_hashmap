
#ifndef JSTD_SYSTEM_SLEEP_H
#define JSTD_SYSTEM_SLEEP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_MSC_VER) || defined(_ICL) || defined(__INTEL_COMPILER) || defined(__MINGW32__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

__declspec(dllimport) int __stdcall SwitchToThread(void);

#endif // _MSC_VER

#ifdef __cplusplus
extern "C" {
#endif

/* Sleep for the platform */
void jstd_sleep(unsigned int millisec);

/* Sleep for Windows or MinGW */
void jstd_wsleep(unsigned int millisec);

/* Yield(): Switch to the other threads in the same CPU core. */
void jstd_yield();

#ifdef __cplusplus
}
#endif

#endif // JSTD_SYSTEM_SLEEP_H
