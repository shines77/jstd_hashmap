
#ifndef JSTD_TEST_CPU_WARMUP_H
#define JSTD_TEST_CPU_WARMUP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdint.h>

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(_WINDOWS_)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // WIN32_LEAN_AND_MEAN
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif // _WIN32

#if !defined(_MSC_VER) || (_MSC_VER >= 1800)
#include <chrono>
#endif

#include <atomic>

namespace jtest {
namespace CPU {

#if !defined(_MSC_VER) || (_MSC_VER >= 1800)

static
void warm_up(int delayMillsecs)
{
    using namespace std::chrono;
#if !defined(_DEBUG)
    double delayTimeLimit = (double)delayMillsecs / 1.0;
    volatile intptr_t sum = 0;

    printf("------------------------------------------\n\n");
    printf("CPU warm-up begin ...\n");
    //::fflush(stdout);
    high_resolution_clock::time_point startTime, endTime;
    duration<double, std::ratio<1, 1000>> elapsedTime;
    startTime = high_resolution_clock::now();
    do {
        for (intptr_t i = 0; i < 500; ++i) {
            sum += i;
            for (intptr_t j = 5000; j >= 0; --j) {
                sum -= j;
            }
        }
        endTime = high_resolution_clock::now();
        elapsedTime = endTime - startTime;
    } while (elapsedTime.count() < delayTimeLimit);

    printf("sum = %d, time: %0.3f ms\n", (int)sum, elapsedTime.count());
    printf("CPU warm-up end   ... \n\n");
    printf("------------------------------------------\n\n");
    //::fflush(stdout);
#endif // !_DEBUG
}

#else

static
void warm_up(DWORD delayMillsecs)
{
#if !defined(_DEBUG)
    volatile intptr_t sum = 0;

    printf("CPU warm-up begin ...\n");
    //::fflush(stdout);
    DWORD startTime, endTime;
    DWORD elapsedTime;
    startTime = ::timeGetTime();
    do {
        for (intptr_t i = 0; i < 500; ++i) {
            sum += i;
            for (intptr_t j = 5000; j >= 0; --j) {
                sum -= j;
            }
        }
        endTime = ::timeGetTime();
        elapsedTime = endTime - startTime;
    } while (elapsedTime < delayMillsecs);

    printf("sum = %d, time: %u ms\n", (int)sum, elapsedTime);
    printf("CPU warm-up end   ... \n\n");
    //::fflush(stdout);
#endif // !_DEBUG
}

#endif // _MSC_VER

struct JSTD_DLL WarmUp {
    WarmUp(unsigned int delayMillsecs = 1000) {
        //
        // See: https://stackoverflow.com/questions/40579342/is-there-any-compiler-barrier-which-is-equal-to-asm-memory-in-c11
        //
        std::atomic_signal_fence(std::memory_order_release);        // _compile_barrier()
        jtest::CPU::warm_up(delayMillsecs);
        std::atomic_signal_fence(std::memory_order_release);        // _compile_barrier()
    }
};

} // namespace CPU
} // namespace jtest

#endif // JSTD_TEST_CPU_WARMUP_H
