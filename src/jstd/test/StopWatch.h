
#ifndef JSTD_TEST_STOPWATCH_H
#define JSTD_TEST_STOPWATCH_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <time.h>       // For ::clock()

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(_WINDOWS_)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock.h>    // For ::gettimeofday(...)
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#else
#include <sys/time.h>   // For ::gettimeofday(...)
#endif // _WIN32

#if (__cplusplus >= 201103L) || (defined(_MSC_VER) && (_MSC_VER >= 1800))
  #define HAVE_STD_CHRONO_H     1
#else
  #define HAVE_STD_CHRONO_H     0
#endif

#if HAVE_STD_CHRONO_H
#include <chrono>
#endif

#ifndef __COMPILER_BARRIER
#if defined(_MSC_VER) || defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(_WINDOWS_)
#include <intrin.h>
#pragma intrinsic(_ReadWriteBarrier)
#define __COMPILER_BARRIER()        _ReadWriteBarrier()
#else
#define __COMPILER_BARRIER()        __asm__ __volatile__ ("" : : : "memory")
#endif
#endif

namespace jtest {

template <typename T>
struct JSTD_DLL TimeRatio {
    typedef T   time_value_t;

    // 1 second = 1000 millisecond
    static time_value_t millisecs;      // static_cast<time_float_t>(1000.0);

    // 1 second = 1,000,000 microsec
    static time_value_t microsecs;      // static_cast<time_float_t>(1000000.0);

    // 1 second = 1,000,000,000 nanosec
    static time_value_t nanosecs;       // static_cast<time_float_t>(1000000000.0);
};

// 1 second = 1000 millisec
template <typename T>
typename TimeRatio<T>::time_value_t
TimeRatio<T>::millisecs = static_cast<typename TimeRatio<T>::time_value_t>(1000.0);

// 1 second = 1,000,000 microsec
template <typename T>
typename TimeRatio<T>::time_value_t
TimeRatio<T>::microsecs = static_cast<typename TimeRatio<T>::time_value_t>(1000000.0);

// 1 second = 1,000,000,000 nanosec
template <typename T>
typename TimeRatio<T>::time_value_t
TimeRatio<T>::nanosecs = static_cast<typename TimeRatio<T>::time_value_t>(1000000000.0);

namespace detail {

template <typename T>
class JSTD_DLL duration_time {
public:
    typedef T   time_value_t;

private:
    time_value_t duration_;

public:
    duration_time(time_value_t duration) : duration_(duration) {}
    duration_time(const duration_time<time_value_t> & src) : duration_(src.duration_) {}
    ~duration_time() {}

    time_value_t seconds() const {
        return this->duration_;
    }

    time_value_t millisecs() const {
        return (this->seconds() * TimeRatio<time_value_t>::millisecs);
    }

    time_value_t microsecs() const {
        return (this->seconds() * TimeRatio<time_value_t>::microsecs);
    }

    time_value_t nanosecs() const {
        return (this->seconds() * TimeRatio<time_value_t>::nanosecs);
    }
};

} // namespace detail

template <typename T>
class JSTD_DLL StopWatchBase {
public:
    typedef T                                   impl_type;
    typedef typename impl_type::time_float_t    time_float_t;
    typedef typename impl_type::time_stamp_t    time_stamp_t;
    typedef typename impl_type::time_point_t    time_point_t;
    typedef typename impl_type::duration_type   duration_type;
    typedef TimeRatio<time_float_t>             time_ratio;

private:
    time_point_t start_time_;
    time_point_t stop_time_;

    static time_point_t base_time_;

public:
    StopWatchBase() :
        start_time_(impl_type::now()), stop_time_(start_time_) {
    }
    StopWatchBase(StopWatchBase<T> const & src) :
        start_time_(src.start_time_), stop_time_(src.stop_time_) {
    }
    ~StopWatchBase() {}

    void reset() {
        this->restart();
    }

    void restart() {
        start_time_ = impl_type::now();
        stop_time_ = start_time_;
        __COMPILER_BARRIER();
    }

    void start() {
        start_time_ = impl_type::now();
        __COMPILER_BARRIER();
    }

    void stop() {
        __COMPILER_BARRIER();
        stop_time_ = impl_type::now();
    }

    static time_point_t now() {
        __COMPILER_BARRIER();
        time_point_t _now = impl_type::now();
        __COMPILER_BARRIER();
        return _now;
    }

    static time_stamp_t timestamp() {
        __COMPILER_BARRIER();
        time_stamp_t _timestamp = impl_type::timestamp(impl_type::now(), base_time_);
        __COMPILER_BARRIER();
        return _timestamp;
    }

    template <typename U>
    static detail::duration_time<time_float_t> duration(U now, U old) {
        detail::duration_time<time_float_t> _duration(impl_type::duration_time(now, old));
        return _duration;
    }

    time_float_t peekElapsedTime() const {
        __COMPILER_BARRIER();
        time_float_t elapsed_time = impl_type::duration_time(impl_type::now(), start_time_);
        return elapsed_time;
    }

    time_float_t peekElapsedNanosec() const {
        return (this->peekElapsedTime() * time_ratio::nanosecs);
    }

    time_float_t peekElapsedMicrosec() const {
        return (this->peekElapsedTime() * time_ratio::microsecs);
    }

    time_float_t peekElapsedMillisec() const {
        return (this->peekElapsedTime() * time_ratio::millisecs);
    }

    time_float_t peekElapsedSecond() const {
        return this->peekElapsedTime();
    }

    time_float_t currentTimeMillis() {
        return this->peekElapsedMillisec();
    }

    time_float_t getElapsedTime() {
        __COMPILER_BARRIER();
        time_float_t elapsed_time = impl_type::duration_time(stop_time_, start_time_);
        return elapsed_time;
    }

    time_float_t getElapsedNanosec() {
        return (this->getElapsedSecond() * time_ratio::nanosecs);
    }

    time_float_t getElapsedMicrosec() {
        return (this->getElapsedSecond() * time_ratio::microsecs);
    }

    time_float_t getElapsedMillisec() {
        return (this->getElapsedSecond() * time_ratio::millisecs);
    }

    time_float_t getElapsedSecond() {
        return this->getElapsedTime();
    }
};

template <typename T>
typename StopWatchBase<T>::time_point_t
StopWatchBase<T>::base_time_ = StopWatchBase<T>::impl_type::now();

template <typename T>
class JSTD_DLL StopWatchExBase {
public:
    typedef T                                   impl_type;
    typedef StopWatchExBase<T>                  this_type;
    typedef typename impl_type::time_float_t    time_float_t;
    typedef typename impl_type::time_stamp_t    time_stamp_t;
    typedef typename impl_type::time_point_t    time_point_t;
    typedef typename impl_type::duration_type   duration_type;
    typedef TimeRatio<time_float_t>             time_ratio;

private:
    time_point_t start_time_;
    time_point_t stop_time_;
    time_float_t elapsed_time_;
    time_float_t total_elapsed_time_;
    bool         running_;

    static time_point_t base_time_;

    // The zero value time.
    static time_float_t kTimeZero;      // static_cast<time_float_t>(0.0);
    // 1 second = 1,000,000 microsec
    static time_float_t kMicrosecCoff;  // static_cast<time_float_t>(1000000.0);
    // 1 second = 1,000 millisec
    static time_float_t kMillisecCoff;  // static_cast<time_float_t>(1000.0);

public:
    StopWatchExBase() : elapsed_time_(kTimeZero),
        total_elapsed_time_(kTimeZero), running_(false) {
        start_time_ = impl_type::now();
    }
    StopWatchExBase(StopWatchExBase<T> const & src) :
        start_time_(src.start_time_), stop_time_(src.stop_time_),
        elapsed_time_(src.elapsed_time_), total_elapsed_time_(src.total_elapsed_time_),
        running_(src.running_) {
    }
    ~StopWatchExBase() {}

    void reset() {
        elapsed_time_ = kTimeZero;
        total_elapsed_time_ = kTimeZero;
        start_time_ = impl_type::now();
        running_ = false;
        __COMPILER_BARRIER();
    }

    void restart() {
        running_ = false;
        elapsed_time_ = kTimeZero;
        total_elapsed_time_ = kTimeZero;
        start_time_ = impl_type::now();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void start() {
        if (!running_) {
            elapsed_time_ = kTimeZero;
            start_time_ = impl_type::now();
            running_ = true;
        }
        __COMPILER_BARRIER();
    }

    void stop() {
        __COMPILER_BARRIER();
        if (running_) {
            stop_time_ = impl_type::now();
            running_ = false;
            __COMPILER_BARRIER();
            elapsed_time_ = this->getDurationTime();
        }
    }

    void mark_start() {
        start_time_ = impl_type::now();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void mark_stop() {
        __COMPILER_BARRIER();
        stop_time_ = impl_type::now();
        running_ = false;
    }

    void resume() {
        this->start();
    }

    void pause() {
        __COMPILER_BARRIER();
        if (running_) {
            stop_time_ = impl_type::now();
            running_ = false;
            __COMPILER_BARRIER();
            elapsed_time_ = this->getDurationTime();
            total_elapsed_time_ += elapsed_time_;
        }
        __COMPILER_BARRIER();
    }

    void again() {
        stop();
        __COMPILER_BARRIER();
        if (elapsed_time_ != kTimeZero) {
            total_elapsed_time_ += elapsed_time_;
            elapsed_time_ = kTimeZero;
        }
    }

    static time_point_t now() {
        __COMPILER_BARRIER();
        time_point_t _now = impl_type::now();
        __COMPILER_BARRIER();
        return _now;
    }

    static time_stamp_t timestamp() {
        __COMPILER_BARRIER();
        time_stamp_t _timestamp = impl_type::timestamp(impl_type::now(), this_type::base_time_);
        __COMPILER_BARRIER();
        return _timestamp;
    }

    template <typename U>
    static detail::duration_time<time_float_t> duration(U now, U old) {
        detail::duration_time<time_float_t> _duration(impl_type::duration_time(now, old));
        return _duration;
    }

    time_float_t getDurationTime() const {
        detail::duration_time<time_float_t> _duration_time = impl_type::duration_time(stop_time_, start_time_);
        return _duration_time;
    }

    time_float_t getDurationMicrosec() {
        return (this->getDurationSecond() * time_ratio::microsecs);
    }

    time_float_t getDurationMillisec() {
        return (this->getDurationSecond() * time_ratio::millisecs);
    }

    time_float_t getDurationNanosec() {
        return (this->getDurationSecond() * time_ratio::nanosecs);
    }

    time_float_t getDurationSecond() {
        __COMPILER_BARRIER();
        if (!running_) {
            this->elapsed_time_ = this->getDurationTime();
        }
        return this->elapsed_time_;
    }

    time_float_t peekElapsedTime() const {
        __COMPILER_BARRIER();
        time_float_t elapsed_time = impl_type::duration_time(impl_type::now(), start_time_);
        return elapsed_time;
    }

    time_float_t peekElapsedMicrosec() const {
        return (this->peekElapsedTime() * time_ratio::microsecs);
    }

    time_float_t peekElapsedMillisec() const {
        return (this->peekElapsedTime() * time_ratio::millisecs);
    }

    time_float_t peekElapsedNanosec() const {
        return (this->peekElapsedTime() * time_ratio::nanosecs);
    }

    time_float_t peekElapsedSecond() const {
        return this->peekElapsedTime();
    }

    time_float_t currentTimeMillis() {
        return this->peekElapsedMillisec();
    }

    time_float_t getElapsedTime() {
        stop();
        return this->elapsed_time_;
    }

    time_float_t getElapsedMicrosec() {
        return (this->getElapsedTime() * time_ratio::microsecs);
    }

    time_float_t getElapsedMillisec() {
        return (this->getElapsedTime() * time_ratio::millisecs);
    }

    time_float_t getElapsedNanosec() {
        return (this->getElapsedTime() * time_ratio::nanosecs);
    }

    time_float_t getElapsedSecond() {
        return this->getElapsedTime();
    }

    time_float_t getTotalElapsedTime() const {
        __COMPILER_BARRIER();
        return this->total_elapsed_time_;
    }

    time_float_t getTotalMicrosec() const {
        return (this->getTotalElapsedTime() * time_ratio::microsecs);
    }

    time_float_t getTotalMillisec() const {
        return (this->getTotalElapsedTime() * time_ratio::millisecs);
    }

    time_float_t getTotalNanosec() const {
        return (this->getTotalElapsedTime() * time_ratio::nanosecs);
    }

    time_float_t getTotalSecond() const {
        return this->getTotalElapsedTime();
    }
};

template <typename T>
typename StopWatchExBase<T>::time_point_t
StopWatchExBase<T>::base_time_ = StopWatchExBase<T>::impl_type::now();

// The zero value time.
template <typename T>
typename StopWatchExBase<T>::time_float_t
StopWatchExBase<T>::kTimeZero = static_cast<typename StopWatchExBase<T>::time_float_t>(0.0);

template <typename TimeFloatTy>
class JSTD_DLL clockStopWatchImpl {
public:
    typedef TimeFloatTy                                     time_float_t;
    typedef clock_t                                         time_stamp_t;
    typedef clock_t                                         time_point_t;
    typedef time_float_t                                    duration_type;
    typedef clockStopWatchImpl<TimeFloatTy>                 this_type;

public:
    clockStopWatchImpl() {}
    ~clockStopWatchImpl() {}

    static time_stamp_t interval(time_point_t now_time, time_point_t old_time) {
        return static_cast<time_stamp_t>(now_time - old_time);
    }

    static time_point_t now() {
        time_point_t now_time = ::clock();
        return now_time;
    }

    static time_float_t duration_time(time_point_t now_time, time_point_t old_time) {
        return (this_type::interval(now_time, old_time) / static_cast<time_float_t>(CLOCKS_PER_SEC));
    }

    static time_stamp_t timestamp(time_point_t now_time, time_point_t base_time) {
        return static_cast<time_stamp_t>(now_time);
    }
};

typedef StopWatchBase< clockStopWatchImpl<double> >         clockStopWatch;
typedef StopWatchExBase< clockStopWatchImpl<double> >       clockStopWatchEx;

template <typename TimeFloatTy>
class JSTD_DLL defaultStopWatchImpl {
public:
    typedef TimeFloatTy                                     time_float_t;
    typedef int64_t                                         time_stamp_t;
    typedef timeval                                         time_point_t;
    typedef time_float_t                                    duration_type;
    typedef defaultStopWatchImpl<TimeFloatTy>               this_type;

public:
    defaultStopWatchImpl() {}
    ~defaultStopWatchImpl() {}

    static time_stamp_t to_timestamp_us(time_point_t * time) {
        return static_cast<time_stamp_t>(int64_t(time->tv_sec) * TimeRatio<int64_t>::microsecs + time->tv_usec);
    }

    static time_stamp_t interval(time_point_t now_time, time_point_t old_time) {
        return static_cast<time_stamp_t>(to_timestamp_us(&now_time) - to_timestamp_us(&old_time));
    }

    static time_point_t now() {
        time_point_t now_time;
        ::gettimeofday(&now_time, nullptr);
        return now_time;
    }

    static time_float_t duration_time(time_point_t now_time, time_point_t old_time) {
        return static_cast<time_float_t>(this_type::interval(now_time, old_time));
    }

    static time_stamp_t timestamp(time_point_t now_time, time_point_t base_time) {
        return this_type::interval(now_time, base_time);
    }
};

#if HAVE_STD_CHRONO_H

template <typename TimeFloatTy>
class JSTD_DLL StdStopWatchImpl {
public:
    typedef TimeFloatTy                                     time_float_t;
    typedef std::uint64_t                                   time_stamp_t;
    typedef std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds>
                                                            time_point_t;
    typedef std::chrono::duration<time_float_t>             duration_type;

public:
    StdStopWatchImpl() {}
    ~StdStopWatchImpl() {}

    static time_point_t now() {
        return std::chrono::high_resolution_clock::now();
    }

    static time_float_t duration_time(time_stamp_t now_time, time_stamp_t old_time) {
        return static_cast<time_float_t>(now_time - old_time);
    }

    static time_float_t duration_time(time_point_t now_time, time_point_t old_time) {
        duration_type _duration_time = std::chrono::duration_cast<duration_type>(now_time - old_time);
        return _duration_time.count();
    }

    static time_stamp_t timestamp(time_point_t now_time, time_point_t base_time) {
        std::chrono::duration<time_stamp_t> _duration_time =
            std::chrono::duration_cast<std::chrono::duration<time_stamp_t>>(now_time - base_time);
        return _duration_time.count();
    }
};

typedef StopWatchBase< StdStopWatchImpl<double> >           StopWatch;
typedef StopWatchExBase< StdStopWatchImpl<double> >         StopWatchEx;

typedef StopWatchBase< StdStopWatchImpl<double> >           defaultStopWatch;
typedef StopWatchExBase< StdStopWatchImpl<double> >         defaultStopWatchEx;

#else

typedef StopWatchBase< defaultStopWatchImpl<double> >       defaultStopWatch;
typedef StopWatchExBase< defaultStopWatchImpl<double> >     defaultStopWatchEx;

#endif // HAVE_STD_CHRONO_H

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(_WINDOWS_)

template <typename TimeFloatTy>
class JSTD_DLL timeGetTimeImpl {
public:
    typedef TimeFloatTy     time_float_t;
    typedef DWORD           time_stamp_t;
    typedef DWORD           time_point_t;
    typedef time_float_t    duration_type;

public:
    timeGetTimeImpl() {}
    ~timeGetTimeImpl() {}

    static time_point_t now() {
        return ::timeGetTime();
    }

    static time_float_t duration_time(time_point_t now_time, time_point_t old_time) {
        time_point_t _duration_time = now_time - old_time;
        return (_duration_time / static_cast<time_float_t>(1000));
    }

    static time_stamp_t timestamp(time_point_t now_time, time_point_t base_time) {
        return now_time;
    }
};

typedef StopWatchBase< timeGetTimeImpl<double> >    timeGetTimeStopWatch;
typedef StopWatchExBase< timeGetTimeImpl<double> >  timeGetTimeStopWatchEx;

#else

typedef StopWatchBase< defaultStopWatchImpl<double> >   timeGetTimeStopWatch;
typedef StopWatchExBase< defaultStopWatchImpl<double> > timeGetTimeStopWatchEx;

#endif // _WIN32

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(_WINDOWS_)

template <typename TimeFloatTy>
class JSTD_DLL getTickCountImpl {
public:
    typedef TimeFloatTy     time_float_t;
    typedef DWORD           time_stamp_t;
    typedef DWORD           time_point_t;
    typedef time_float_t    duration_type;

public:
    getTickCountImpl() {}
    ~getTickCountImpl() {}

    static time_point_t now() {
        return ::GetTickCount();
    }

    static time_float_t duration_time(time_point_t now_time, time_point_t old_time) {
        time_point_t _duration_time = now_time - old_time;
        return (_duration_time / static_cast<time_float_t>(1000));
    }

    static time_stamp_t timestamp(time_point_t now_time, time_point_t base_time) {
        return now_time;
    }
};

typedef StopWatchBase< getTickCountImpl<double> >   getTickCountStopWatch;
typedef StopWatchExBase< getTickCountImpl<double> > getTickCountStopWatchEx;

#else

typedef StopWatchBase< defaultStopWatchImpl<double> >   getTickCountStopWatch;
typedef StopWatchExBase< defaultStopWatchImpl<double> > getTickCountStopWatchEx;

#endif // _WIN32

} // namespace jtest

#endif // JSTD_TEST_STOPWATCH_H
