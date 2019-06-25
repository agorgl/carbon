#include "ptime.h"
#include <string.h>
#include <assert.h>

/*
 *  Uses the following time measurement functions under the hood:
 *  Windows:        QueryPerformanceFrequency() / QueryPerformanceCounter()
 *  MacOS/iOS:      mach_absolute_time()
 *  Linux/others:   clock_gettime(CLOCK_MONOTONIC)
 */

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
typedef struct {
    uint32_t initialized;
    LARGE_INTEGER freq;
    LARGE_INTEGER start;
} _ptime_state_t;
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach/mach_time.h>
typedef struct {
    uint32_t initialized;
    mach_timebase_info_data_t timebase;
    uint64_t start;
} _ptime_state_t;
#else /* Anything else, this will need more care for non-Linux platforms */
#include <time.h>
typedef struct {
    uint32_t initialized;
    uint64_t start;
} _ptime_state_t;
#endif
static _ptime_state_t _ptm;

/* Prevent 64-bit overflow when computing relative timestamp */
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
static int64_t int64_muldiv(int64_t value, int64_t numer, int64_t denom)
{
    int64_t q = value / denom;
    int64_t r = value % denom;
    return q * numer + r * numer / denom;
}
#endif

void time_setup()
{
    memset(&_ptm, 0, sizeof(_ptm));
    _ptm.initialized = 0xABCDABCD;
    #if defined(_WIN32)
        QueryPerformanceFrequency(&_ptm.freq);
        QueryPerformanceCounter(&_ptm.start);
    #elif defined(__APPLE__) && defined(__MACH__)
        mach_timebase_info(&_ptm.timebase);
        _ptm.start = mach_absolute_time();
    #else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        _ptm.start = (uint64_t)ts.tv_sec*1000000000 + (uint64_t)ts.tv_nsec;
    #endif
}

uint64_t time_now()
{
    assert(_ptm.initialized == 0xABCDABCD);
    uint64_t now;
    #if defined(_WIN32)
        LARGE_INTEGER qpc_t;
        QueryPerformanceCounter(&qpc_t);
        now = int64_muldiv(qpc_t.QuadPart - _ptm.start.QuadPart, 1000000000, _ptm.freq.QuadPart);
    #elif defined(__APPLE__) && defined(__MACH__)
        const uint64_t mach_now = mach_absolute_time() - _ptm.start;
        now = int64_muldiv(mach_now, _ptm.timebase.numer, _ptm.timebase.denom);
    #else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        now = ((uint64_t)ts.tv_sec*1000000000 + (uint64_t)ts.tv_nsec) - _ptm.start;
    #endif
    return now;
}

uint64_t time_diff(uint64_t new_ticks, uint64_t old_ticks)
{
    if (new_ticks > old_ticks) {
        return new_ticks - old_ticks;
    } else {
        return 1;
    }
}

uint64_t time_since(uint64_t start_ticks)
{
    return time_diff(time_now(), start_ticks);
}

uint64_t time_laptime(uint64_t* last_time)
{
    assert(last_time);
    uint64_t dt = 0;
    uint64_t now = time_now();
    if (*last_time != 0) {
        dt = time_diff(now, *last_time);
    }
    *last_time = now;
    return dt;
}

double time_sec(uint64_t ticks)  { return (double)ticks / 1000000000.0; }
double time_msec(uint64_t ticks) { return (double)ticks / 1000000.0; }
double time_usec(uint64_t ticks) { return (double)ticks / 1000.0; }
double time_nsec(uint64_t ticks) { return (double)ticks; }

uint64_t time_ticks(double msecs) { return (uint64_t)(msecs * 1000000.0); }
