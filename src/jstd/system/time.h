
#ifndef JSTD_SYSTEM_TIMER_H
#define JSTD_SYSTEM_TIMER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "jstd/basic/stddef.h"
//#include "jstd/basic/export.h"
#include "jstd/basic/assert.h"

#include <assert.h>

#ifndef JMC_INLINE
#ifdef _MSC_VER
#define JMC_INLINE  static __inline
#else
#define JMC_INLINE  static inline
#endif // _MSC_VER
#endif // JMC_INLINE

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <time.h>
#include <windows.h>
#elif defined(__linux__)
  #ifdef __cplusplus
    #include <ctime>
  #else
    #include <time.h>   // for clock_gettime()
  #endif
#else  /* generic Unix */
#include <sys/time.h>
#endif  /* (choice of OS) */

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t jmc_timestamp_t;
typedef double  jmc_timefloat_t;

////////////////////////////////////////////////////////////////////////////////

JMC_INLINE jmc_timestamp_t jmc_get_nanosec(void);
JMC_INLINE jmc_timestamp_t jmc_get_millisec(void);

JMC_INLINE jmc_timefloat_t jmc_get_second(void);
JMC_INLINE jmc_timefloat_t jmc_get_millisecf(void);

JMC_INLINE jmc_timestamp_t jmc_get_timestamp(void);

/////////////////////////////////////////////////////////////////////////

JMC_INLINE jmc_timestamp_t jmc_get_elapsedtime_ms (jmc_timestamp_t time_start);
JMC_INLINE jmc_timefloat_t jmc_get_elapsedtime_msf(jmc_timestamp_t time_start);
JMC_INLINE jmc_timefloat_t jmc_get_elapsedtime_sec(jmc_timestamp_t time_start);

JMC_INLINE jmc_timestamp_t jmc_get_interval_ms (jmc_timestamp_t time_interval);
JMC_INLINE jmc_timefloat_t jmc_get_interval_msf(jmc_timestamp_t time_interval);
JMC_INLINE jmc_timefloat_t jmc_get_interval_sec(jmc_timestamp_t time_interval);

////////////////////////////////////////////////////////////////////////////////

/* Unit: ns (nanosecond), return value: int64_t. */
JMC_INLINE
jmc_timestamp_t jmc_get_nanosec(void)
{
    jmc_timestamp_t result;

#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER qp_cnt, qp_freq;
    QueryPerformanceCounter(&qp_cnt);
    QueryPerformanceFrequency(&qp_freq);
    result = (jmc_timestamp_t)(((double)qp_cnt.QuadPart / (double)qp_freq.QuadPart) * 1000000000.0);
#elif defined(__linux__)
    struct timespec ts;
#if JIMIC_USE_ASSERT
    int status =
#endif  /* JIMIC_USE_ASSERT */
        clock_gettime(CLOCK_REALTIME, &ts);
    JIMIC_ASSERT_EX(status == 0, "CLOCK_REALTIME not supported");
    result = (jmc_timestamp_t)((int64_t)(1000000000UL) * (int64_t)(ts.tv_sec) + (int64_t)(ts.tv_nsec));
#else  /* generic Unix */
    struct timeval tv;
#if JIMIC_USE_ASSERT
    int status =
#endif  /* JIMIC_USE_ASSERT */
        gettimeofday(&tv, NULL);
    JIMIC_ASSERT_EX(status == 0, "gettimeofday failed");
    result = (jmc_timestamp_t)((int64_t)(1000000000UL) * (int64_t)(tv.tv_sec) + (int64_t)(1000UL) * (int64_t)(tv.tv_usec));
#endif  /*(choice of OS) */

    return result;
}

/* Unit: ms (millisecond), return value: int64_t. */
JMC_INLINE
jmc_timestamp_t jmc_get_millisec(void)
{
    jmc_timestamp_t result;

#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER qp_cnt, qp_freq;
    QueryPerformanceCounter(&qp_cnt);
    QueryPerformanceFrequency(&qp_freq);
    result = (jmc_timestamp_t)(((double)qp_cnt.QuadPart / (double)qp_freq.QuadPart) * 1000.0);
#elif defined(__linux__)
    struct timespec ts;
#if JIMIC_USE_ASSERT
    int status =
#endif /* JIMIC_USE_ASSERT */
        clock_gettime(CLOCK_REALTIME, &ts);
    JIMIC_ASSERT_EX(status == 0, "CLOCK_REALTIME not supported");
    result = (jmc_timestamp_t)((int64_t)(1000UL) * (int64_t)(ts.tv_sec) + (int64_t)(ts.tv_nsec) / (int64_t)(1000000UL));
#else /* generic Unix */
    struct timeval tv;
#if JIMIC_USE_ASSERT
    int status =
#endif /* JIMIC_USE_ASSERT */
        gettimeofday(&tv, NULL);
    JIMIC_ASSERT_EX(status == 0, "gettimeofday failed");
    result = (jmc_timestamp_t)((int64_t)(1000UL) * (int64_t)(tv.tv_sec) + (int64_t)(tv.tv_usec) / (int64_t)(1000UL));
#endif /*(choice of OS) */

    return result;
}

/* Unit: s (second), return value: floating point */
JMC_INLINE
jmc_timefloat_t jmc_get_second(void)
{
    jmc_timefloat_t result;

    jmc_timestamp_t time_usecs;
    time_usecs = jmc_get_nanosec();

#if defined(_WIN32) || defined(_WIN64)
    result = (jmc_timefloat_t)time_usecs * 1E-9;
#elif defined(__linux__)
    result = (jmc_timefloat_t)time_usecs * 1E-9;
#else  /* generic Unix */
    result = (jmc_timefloat_t)time_usecs * 1E-6;
#endif  /*(choice of OS) */

    return result;
}

/* Unit: ms (millisecond), floating point */
JMC_INLINE
jmc_timefloat_t jmc_get_millisecf(void)
{
    jmc_timefloat_t result;

    jmc_timestamp_t time_usecs;
    time_usecs = jmc_get_nanosec();

#if defined(_WIN32) || defined(_WIN64)
    result = (jmc_timefloat_t)time_usecs * 1E-6;
#elif defined(__linux__)
    result = (jmc_timefloat_t)time_usecs * 1E-6;
#else  /* generic Unix */
    result = (jmc_timefloat_t)time_usecs * 1E-3;
#endif  /*(choice of OS) */

    return result;
}

// Minimum unit: Minimum time measurement unit for each operating system,
// Windows: CPU TSC count; Linux: ns (nsec); Unix: us (usec).
JMC_INLINE
jmc_timestamp_t jmc_get_timestamp(void)
{
    jmc_timestamp_t result;

#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER qp_cnt;
    QueryPerformanceCounter(&qp_cnt);
    result = (jmc_timestamp_t)qp_cnt.QuadPart;
#elif defined(__linux__)
    struct timespec ts;
#if JIMIC_USE_ASSERT
    int status =
#endif  /* JIMIC_USE_ASSERT */
        clock_gettime(CLOCK_REALTIME, &ts);
    JIMIC_ASSERT_EX(status == 0, "CLOCK_REALTIME not supported");
    result = (jmc_timestamp_t)((int64_t)(1000000000UL) * (int64_t)(ts.tv_sec) + (int64_t)(ts.tv_nsec));
#else  /* generic Unix */
    struct timeval tv;
#if JIMIC_USE_ASSERT
    int status =
#endif  /* JIMIC_USE_ASSERT */
        gettimeofday(&tv, NULL);
    JIMIC_ASSERT_EX(status == 0, "gettimeofday failed");
    result = (jmc_timestamp_t)((int64_t)(1000000UL) * (int64_t)(tv.tv_sec) + (int64_t)(tv.tv_usec));
#endif  /*(choice of OS) */

    return result;
}

// According to the timestamp interval, get the millisecond value of the elapsed time,
// Unit: ms (millisecond), return value: int64_t.
JMC_INLINE
jmc_timestamp_t jmc_get_elapsedtime_ms(jmc_timestamp_t time_start)
{
    jmc_timestamp_t result, time_end;

#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER qp_cnt, qp_freq;
    QueryPerformanceCounter(&qp_cnt);
    time_end = (jmc_timestamp_t)qp_cnt.QuadPart;

    QueryPerformanceFrequency(&qp_freq);
    //result = (jmc_timestamp_t)((double)((time_end - time_start) * 1000LL) / (double)qp_freq.QuadPart);
    result = (jmc_timestamp_t)(((double)(time_end - time_start) / (double)qp_freq.QuadPart) * 1000.0);
#elif defined(__linux__)
    struct timespec ts;
#if JIMIC_USE_ASSERT
    int status =
#endif  /* JIMIC_USE_ASSERT */
        clock_gettime(CLOCK_REALTIME, &ts);
    JIMIC_ASSERT_EX(status == 0, "CLOCK_REALTIME not supported");
    time_end = (jmc_timestamp_t)((int64_t)(1000000000UL) * (int64_t)(ts.tv_sec) + (int64_t)(ts.tv_nsec));

    result = (jmc_timestamp_t)(time_end - time_start) / 1000000LL;
#else  /* generic Unix */
    struct timeval tv;
#if JIMIC_USE_ASSERT
    int status =
#endif  /* JIMIC_USE_ASSERT */
        gettimeofday(&tv, NULL);
    JIMIC_ASSERT_EX(status == 0, "gettimeofday failed");
    time_end = (jmc_timestamp_t)((int64_t)(1000000UL) * (int64_t)(tv.tv_sec) + (int64_t)(tv.tv_usec));

    result = (jmc_timestamp_t)(time_end - time_start) / 1000LL;
#endif  /*(choice of OS) */

    return result;
}

// According to the timestamp interval, get the millisecond value of the elapsed time,
// Unit: ms (millisecond), return value: floating point.
JMC_INLINE
jmc_timefloat_t jmc_get_elapsedtime_msf(jmc_timestamp_t time_start)
{
    jmc_timefloat_t result;
    jmc_timestamp_t time_end;

#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER qp_cnt, qp_freq;
    QueryPerformanceCounter(&qp_cnt);
    time_end = (jmc_timestamp_t)qp_cnt.QuadPart;

    QueryPerformanceFrequency(&qp_freq);
    //result = (jmc_timefloat_t)((double)((time_end - time_start) * 1000LL) / (double)qp_freq.QuadPart);
    result = (jmc_timefloat_t)(((double)(time_end - time_start) / (double)qp_freq.QuadPart) * 1000.0);
#elif defined(__linux__)
    struct timespec ts;
#if JIMIC_USE_ASSERT
    int status =
#endif  /* JIMIC_USE_ASSERT */
        clock_gettime(CLOCK_REALTIME, &ts);
    JIMIC_ASSERT_EX(status == 0, "CLOCK_REALTIME not supported");
    time_end = (jmc_timestamp_t)((int64_t)(1000000000UL) * (int64_t)(ts.tv_sec) + (int64_t)(ts.tv_nsec));

    result = (jmc_timefloat_t)(time_end - time_start)  * 1E-6;
#else  /* generic Unix */
    struct timeval tv;
#if JIMIC_USE_ASSERT
    int status =
#endif  /* JIMIC_USE_ASSERT */
        gettimeofday(&tv, NULL);
    JIMIC_ASSERT_EX(status == 0, "gettimeofday failed");
    time_end = (jmc_timestamp_t)((int64_t)(1000000UL) * (int64_t)(tv.tv_sec) + (int64_t)(tv.tv_usec));

    result = (jmc_timefloat_t)(time_end - time_start)  * 1E-3;
#endif  /*(choice of OS) */

    return result;
}

// According to the timestamp interval, get the second value of the elapsed time,
// Unit: s (second), return value: floating point.
JMC_INLINE
jmc_timefloat_t jmc_get_elapsedtime_sec(jmc_timestamp_t time_start)
{
    jmc_timefloat_t result;
    jmc_timestamp_t time_end;

#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER qp_cnt, qp_freq;
    QueryPerformanceCounter(&qp_cnt);
    time_end = (jmc_timestamp_t)qp_cnt.QuadPart;

    QueryPerformanceFrequency(&qp_freq);
    result = (jmc_timefloat_t)(((double)(time_end - time_start) / (double)qp_freq.QuadPart));
#elif defined(__linux__)
    struct timespec ts;
#if JIMIC_USE_ASSERT
    int status =
#endif  /* JIMIC_USE_ASSERT */
        clock_gettime(CLOCK_REALTIME, &ts);
    JIMIC_ASSERT_EX(status == 0, "CLOCK_REALTIME not supported");
    time_end = (jmc_timestamp_t)((int64_t)(1000000000UL) * (int64_t)(ts.tv_sec) + (int64_t)(ts.tv_nsec));

    result = (jmc_timefloat_t)(time_end - time_start)  * 1E-9;
#else  /* generic Unix */
    struct timeval tv;
#if JIMIC_USE_ASSERT
    int status =
#endif  /* JIMIC_USE_ASSERT */
        gettimeofday(&tv, NULL);
    JIMIC_ASSERT_EX(status == 0, "gettimeofday failed");
    time_end = (jmc_timestamp_t)((int64_t)(1000000UL) * (int64_t)(tv.tv_sec) + (int64_t)(tv.tv_usec));

    result = (jmc_timefloat_t)(time_end - time_start)  * 1E-6;
#endif  /*(choice of OS) */

    return result;
}

// According to the timestamp interval, get the millisecond value of the elapsed time,
// Unit: ms (millisecond), return value: int64_t.
JMC_INLINE
jmc_timestamp_t jmc_get_interval_ms(jmc_timestamp_t time_interval)
{
    jmc_timestamp_t result;

#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER qp_freq;
    QueryPerformanceFrequency(&qp_freq);
    result = (jmc_timestamp_t)(((double)time_interval / (double)qp_freq.QuadPart) * 1000.0);
#elif defined(__linux__)
    result = (jmc_timestamp_t)time_interval / 1000000LL;
#else  /* generic Unix */
    result = (jmc_timestamp_t)time_interval / 1000LL;
#endif  /*(choice of OS) */

    return result;
}

// According to the timestamp interval, get the millisecond value of the elapsed time,
// Unit: ms (millisecond), return value: floating point.
JMC_INLINE
jmc_timefloat_t jmc_get_interval_msf(jmc_timestamp_t time_interval)
{
    jmc_timefloat_t result;

#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER qp_freq;
    QueryPerformanceFrequency(&qp_freq);
    result = (jmc_timefloat_t)(((double)time_interval / (double)qp_freq.QuadPart) * 1000.0);
#elif defined(__linux__)
    result = (jmc_timefloat_t)time_interval * 1E-6;
#else  /* generic Unix */
    result = (jmc_timefloat_t)time_interval * 1E-3;
#endif  /*(choice of OS) */

    return result;
}

// According to the timestamp interval, get the second value of the elapsed time,
// Unit: s (second), return value: floating point.
JMC_INLINE
jmc_timefloat_t jmc_get_interval_sec(jmc_timestamp_t time_interval)
{
    jmc_timefloat_t result;

#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER qp_freq;
    QueryPerformanceFrequency(&qp_freq);
    result = (jmc_timefloat_t)(((double)time_interval / (double)qp_freq.QuadPart));
#elif defined(__linux__)
    result = (jmc_timefloat_t)time_interval * 1E-9;
#else  /* generic Unix */
    result = (jmc_timefloat_t)time_interval * 1E-6;
#endif  /*(choice of OS) */

    return result;
}

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif // JSTD_SYSTEM_TIMER_H
