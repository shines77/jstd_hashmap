
#ifndef JSTD_MEMORY_MEOMORY_BARRIER_H
#define JSTD_MEMORY_MEOMORY_BARRIER_H

#pragma once

#if defined(_MSC_VER)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#define __COMPILER_BARRIER()    _ReadWriteBarrier()
#else
#define __COMPILER_BARRIER()    __asm volatile ("" : : : "memory")
#endif

#endif // JSTD_MEMORY_MEOMORY_BARRIER_H
