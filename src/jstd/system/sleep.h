
#ifndef JSTD_SYSTEM_SLEEP_H
#define JSTD_SYSTEM_SLEEP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

__declspec(dllimport) int __stdcall SwitchToThread(void);

#elif defined(__GNUC__) || defined(__clang__) || defined(__linux__)

#include <unistd.h>     // For usleep()
#include <sched.h>      // For sched_yield()

#endif // _MSC_VER && !__clang__

//
// TODO: Thread::YieldTo()
//
// See: https://stackoverflow.com/questions/1413630/switchtothread-thread-yield-vs-thread-sleep0-vs-thead-sleep1
//

/***********************************************************************************************

// C# source code

public enum ThreadYieldTarget
{
    /// <summary>  Operation system will decide when to interrupt the thread </summary>
    None,

    /// <summary> Yield time slice to any other thread on same processor </summary>
    AnyThreadOnSameProcessor,

    /// <summary>  Yield time slice to other thread of same or higher piority on any processor </summary>
    SameOrHigherPriorityThreadOnAnyProcessor,

    /// <summary>  Yield time slice to any other thread on any processor </summary>
    AnyThreadOnAnyProcessor
}

public static class Thread
{
    [DllImport("kernel32.dll")]
    static extern bool SwitchToThread();

    [DllImport("winmm.dll")]
    internal static extern uint timeBeginPeriod(uint period);

    [DllImport("winmm.dll")]
    internal static extern uint timeEndPeriod(uint period);

    /// <summary>  yields time slice of current thread to specified target threads </summary>
    public static void YieldTo(ThreadYieldTarget threadYieldTarget)
    {
        switch (threadYieldTarget) {
            case ThreadYieldTarget.None:
                break;
            case ThreadYieldTarget.AnyThreadOnSameProcessor:
                SwitchToThread();
                break;
            case ThreadYieldTarget.SameOrHigherPriorityThreadOnAnyProcessor:
                System.Threading.Thread.Sleep(0);
                break;
            case ThreadYieldTarget.AnyThreadOnAnyProcessor:
                timeBeginPeriod(1); // reduce sleep to actually 1ms instead of
                                    // system time slice with is around 15ms
                System.Threading.Thread.Sleep(1);
                timeEndPeriod(1);   // undo
                break;
            default: throw new ArgumentOutOfRangeException("threadYieldTarget");
        }
    }
}

***********************************************************************************************/

namespace jstd {

#if defined(_MSC_VER) && !defined(__clang__)

/* Sleep for the platform */
static inline
void thread_sleep(unsigned int millisec)
{
    Sleep(millisec);
}

/* Sleep more tiny for the platform */
static inline
void thread_wsleep(unsigned int millisec)
{
    Sleep(millisec);
}

/************************************************************************************

  int SwitchToThread();

  The yield of execution is limited to the processor of the calling thread.
  The operating system will not switch execution to another processor,
  even if that processor is idle or is running a thread of lower priority.

************************************************************************************/

/* Switch to the other threads in the same CPU core. */
static inline
int thread_yield()
{
    return SwitchToThread();
}

//
// SwitchToThread vs Sleep(0), Sleep(1)
// https://stackoverflow.com/questions/1383943/switchtothread-vs-sleep1
//

//
// Sleep(0) will allow threads on other processors to run.
//

/* Switch to the other threads in the all CPU core. */
static inline
int cpu_yield()
{
    Sleep(0);
    return 0;
}

#elif defined(__GNUC__) || defined(__clang__) || defined(__linux__)

/* Sleep for the platform */
static inline
void thread_sleep(unsigned int millisec)
{
    usleep(millisec * 1000);
}

/* Sleep more tiny for the platform */
static inline
void thread_wsleep(unsigned int millisec)
{
    if (millisec != 0)
        usleep(millisec * 1000);
    else
        usleep(250);
}

/*****************************************************************************

  sched_yield()--Yield Processor to Another Thread

  https://www.ibm.com/docs/en/i/7.2?topic=ssw_ibm_i_72/apis/users_32.htm

  #include <sched.h>
  int sched_yield(void);

  The sched_yield() function yields the processor from the currently executing
  thread to another ready-to-run, active thread of equal or higher priority.

  If no threads of equal or higher priority are active and ready to run,
  sched_yield() returns immediately, and the calling thread continues to run
  until its time has expired.

*****************************************************************************/

/* Switch to the other threads in the same CPU core. */
static inline
int thread_yield()
{
    return sched_yield();
}

/* Switch to the other threads in the all CPU core. */
static inline
int cpu_yield()
{
    return sched_yield();
}

#else /* other unknown os */

static inline
int thread_sleep(unsigned int millisec)
{
    // Not implemented
    volatile int sum = 0;
    unsigned int i, j;
    for (i = 0; i < millisec; ++i) {
        sum += i;
        for (j = 50000; j >= 0; --j) {
            sum -= j;
        }
    }
    return sum;
}

static inline
int thread_wsleep(unsigned int millisec)
{
    // Not implemented
    if (millisec != 0) {
        return thread_sleep(millisec);
    } else {
        volatile unsigned int sum = 0;
        unsigned int i, j;
        for (i = 0; i < 1; ++i) {
            sum += i;
            for (j = 1000; j >= 0; --j) {
                sum -= j;
            }
        }
        return sum;
    }
}

static inline
int thread_yield()
{
    // Not implemented
    return 0;
}

static inline
int cpu_yield()
{
    // Not implemented
    return 0;
}

#endif // _MSC_VER && !__clang__

} // namespace jstd

#endif // JSTD_SYSTEM_SLEEP_H
