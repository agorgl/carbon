#include "threads.h"
#include <pthread.h>

/*
 * Configuration macro:
 *   EMULATED_THREADS_USE_NATIVE_TIMEDLOCK
 *     Use pthread_mutex_timedlock() for `mtx_timedlock()'
 *     Otherwise use mtx_trylock() + *busy loop* emulation.
 */
#if !defined(__CYGWIN__) && !defined(__APPLE__) && !defined(__NetBSD__)
#define EMULATED_THREADS_USE_NATIVE_TIMEDLOCK
#endif

/*
 * Implementation limits:
 *   - Conditionally emulation for "mutex with timeout"
 *     (see EMULATED_THREADS_USE_NATIVE_TIMEDLOCK macro)
 */
struct impl_thrd_param {
    thrd_start_t func;
    void* arg;
};

static inline void* impl_thrd_routine(void* p)
{
    struct impl_thrd_param pack = *((struct impl_thrd_param*)p);
    free(p);
    return (void*)(intptr_t)pack.func(pack.arg);
}

void call_once(once_flag* flag, void(*func)(void))
{
    pthread_once(flag, func);
}

int cnd_broadcast(cnd_t* cond)
{
    assert(cond != NULL);
    return (pthread_cond_broadcast(cond) == 0) ? thrd_success : thrd_error;
}

void cnd_destroy(cnd_t* cond)
{
    assert(cond);
    pthread_cond_destroy(cond);
}

int cnd_init(cnd_t* cond)
{
    assert(cond != NULL);
    return (pthread_cond_init(cond, NULL) == 0) ? thrd_success : thrd_error;
}

int cnd_signal(cnd_t* cond)
{
    assert(cond != NULL);
    return (pthread_cond_signal(cond) == 0) ? thrd_success : thrd_error;
}

int cnd_timedwait(cnd_t* cond, mtx_t* mtx, const struct timespec* abs_time)
{
    int rt;

    assert(mtx != NULL);
    assert(cond != NULL);
    assert(abs_time != NULL);

    rt = pthread_cond_timedwait(cond, mtx, abs_time);
    if (rt == ETIMEDOUT)
        return thrd_busy;
    return (rt == 0) ? thrd_success : thrd_error;
}

int cnd_wait(cnd_t* cond, mtx_t* mtx)
{
    assert(mtx != NULL);
    assert(cond != NULL);
    return (pthread_cond_wait(cond, mtx) == 0) ? thrd_success : thrd_error;
}

void mtx_destroy(mtx_t* mtx)
{
    assert(mtx != NULL);
    pthread_mutex_destroy(mtx);
}

/*
 * XXX: Workaround when building with -O0 and without pthreads link.
 *
 * In such cases constant folding and dead code elimination won't be
 * available, thus the compiler will always add the pthread_mutexattr*
 * functions into the binary. As we try to link, we'll fail as the
 * symbols are unresolved.
 *
 * Ideally we'll enable the optimisations locally, yet that does not
 * seem to work.
 *
 * So the alternative workaround is to annotate the symbols as weak.
 * Thus the linker will be happy and things don't clash when building
 * with -O1 or greater.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_WEAK) && !defined(__CYGWIN__)
__attribute__((weak))
int pthread_mutexattr_init(pthread_mutexattr_t* attr);

__attribute__((weak))
int pthread_mutexattr_settype(pthread_mutexattr_t* attr, int type);

__attribute__((weak))
int pthread_mutexattr_destroy(pthread_mutexattr_t* attr);
#endif

int mtx_init(mtx_t* mtx, int type)
{
    pthread_mutexattr_t attr;
    assert(mtx != NULL);
    if (type != mtx_plain
     && type != mtx_timed
     && type != mtx_try
     && type != (mtx_plain|mtx_recursive)
     && type != (mtx_timed|mtx_recursive)
     && type != (mtx_try|mtx_recursive))
        return thrd_error;

    if ((type & mtx_recursive) == 0) {
        pthread_mutex_init(mtx, NULL);
        return thrd_success;
    }

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return thrd_success;
}

int mtx_lock(mtx_t* mtx)
{
    assert(mtx != NULL);
    return (pthread_mutex_lock(mtx) == 0) ? thrd_success : thrd_error;
}

int mtx_timedlock(mtx_t* mtx, const struct timespec* ts)
{
    assert(mtx != NULL);
    assert(ts != NULL);

    {
#ifdef EMULATED_THREADS_USE_NATIVE_TIMEDLOCK
    int rt;
    rt = pthread_mutex_timedlock(mtx, ts);
    if (rt == 0)
        return thrd_success;
    return (rt == ETIMEDOUT) ? thrd_busy : thrd_error;
#else
    time_t expire = time(NULL);
    expire += ts->tv_sec;
    while (mtx_trylock(mtx) != thrd_success) {
        time_t now = time(NULL);
        if (expire < now)
            return thrd_busy;
        // busy loop!
        thrd_yield();
    }
    return thrd_success;
#endif
    }
}

int mtx_trylock(mtx_t* mtx)
{
    assert(mtx != NULL);
    return (pthread_mutex_trylock(mtx) == 0) ? thrd_success : thrd_busy;
}

int mtx_unlock(mtx_t* mtx)
{
    assert(mtx != NULL);
    return (pthread_mutex_unlock(mtx) == 0) ? thrd_success : thrd_error;
}

int thrd_create(thrd_t* thr, thrd_start_t func, void* arg)
{
    struct impl_thrd_param* pack;
    assert(thr != NULL);
    pack = (struct impl_thrd_param*)malloc(sizeof(struct impl_thrd_param));
    if (!pack) return thrd_nomem;
    pack->func = func;
    pack->arg = arg;
    if (pthread_create(thr, NULL, impl_thrd_routine, pack) != 0) {
        free(pack);
        return thrd_error;
    }
    return thrd_success;
}

thrd_t thrd_current(void)
{
    return pthread_self();
}

int thrd_detach(thrd_t thr)
{
    return (pthread_detach(thr) == 0) ? thrd_success : thrd_error;
}

int thrd_equal(thrd_t thr0, thrd_t thr1)
{
    return pthread_equal(thr0, thr1);
}

void thrd_exit(int res)
{
    pthread_exit((void*)(intptr_t)res);
}

int thrd_join(thrd_t thr, int* res)
{
    void* code;
    if (pthread_join(thr, &code) != 0)
        return thrd_error;
    if (res)
        *res = (int)(intptr_t)code;
    return thrd_success;
}

void thrd_sleep(const struct timespec* time_point, struct timespec* remaining)
{
    assert(time_point != NULL);
    nanosleep(time_point, remaining);
}

void thrd_yield(void)
{
    sched_yield();
}

int tss_create(tss_t* key, tss_dtor_t dtor)
{
    assert(key != NULL);
    return (pthread_key_create(key, dtor) == 0) ? thrd_success : thrd_error;
}

void tss_delete(tss_t key)
{
    pthread_key_delete(key);
}

void* tss_get(tss_t key)
{
    return pthread_getspecific(key);
}

int tss_set(tss_t key, void* val)
{
    return (pthread_setspecific(key, val) == 0) ? thrd_success : thrd_error;
}
