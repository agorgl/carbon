#include "thread_pool.h"
#include "threads.h"

typedef enum {
    IMMEDIATE_SHUTDOWN = 1,
    GRACEFUL_SHUTDOWN  = 2
} threadpool_shutdown_t;

typedef struct {
    void (*function)(void*);
    void *argument;
} threadpool_task_t;

/**
 * The threadpool struct
 *
 * notify       Condition variable to notify worker threads.
 * threads      Array containing worker thread handles.
 * thread_count Number of threads
 * queue        Array containing the task queue.
 * queue_size   Size of the task queue.
 * head         Index of the first element.
 * tail         Index of the next element.
 * count        Number of pending tasks
 * shutdown     Flag indicating if the pool is shutting down
 * started      Number of started threads
 */
struct threadpool_t {
    mtx_t lock;
    cnd_t notify;
    thrd_t* threads;
    threadpool_task_t* queue;
    int thread_count;
    int queue_size;
    int head;
    int tail;
    int count;
    int shutdown;
    int started;
};

static int threadpool_thread(void* arg);
static int threadpool_free(threadpool_t* pool);

threadpool_t* threadpool_create(int thread_count, int queue_size)
{
    threadpool_t* pool;
    int i;

    if (thread_count <= 0
     || thread_count > THREAD_POOL_MAX_THREADS
     || queue_size <= 0
     || queue_size > THREAD_POOL_MAX_QUEUE)
        return 0;

    pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    if (!pool)
        goto err;

    /* Initialize */
    pool->thread_count = 0;
    pool->queue_size = queue_size;
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = pool->started = 0;

    /* Allocate thread and task queue */
    pool->threads = calloc(thread_count, sizeof(thrd_t));
    pool->queue = calloc(queue_size, sizeof(threadpool_task_t));

    /* Initialize mutex and conditional variable first */
    if ((mtx_init(&(pool->lock), mtx_plain) != 0)
     || (cnd_init(&(pool->notify)) != 0)
     || (pool->threads == 0)
     || (pool->queue == 0)) {
        goto err;
    }

    /* Start worker threads */
    for (i = 0; i < thread_count; i++) {
        if (thrd_create(&(pool->threads[i]), threadpool_thread, pool) != 0) {
            threadpool_destroy(pool, 0);
            return 0;
        }
        pool->thread_count++;
        pool->started++;
    }

    return pool;

err:
    if (pool) {
        threadpool_free(pool);
    }
    return NULL;
}

int threadpool_add(threadpool_t* pool, void (*function)(void*), void* argument)
{
    int next;
    int err = 0;

    if (pool == NULL || function == NULL)
        return THREADPOOL_INVALID;

    if (mtx_lock(&(pool->lock)) != 0)
        return THREADPOOL_LOCK_FAILURE;

    next = (pool->tail + 1) % pool->queue_size;

    do {
        /* Are we full ? */
        if (pool->count == pool->queue_size) {
            err = THREADPOOL_QUEUE_FULL;
            break;
        }

        /* Are we shutting down ? */
        if (pool->shutdown) {
            err = THREADPOOL_SHUTDOWN;
            break;
        }

        /* Add task to queue */
        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].argument = argument;
        pool->tail = next;
        pool->count += 1;

        /* cnd_broadcast */
        if (cnd_signal(&(pool->notify)) != 0) {
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }
    } while(0);

    if (mtx_unlock(&pool->lock) != 0) {
        err = THREADPOOL_LOCK_FAILURE;
    }

    return err;
}

int threadpool_destroy(threadpool_t* pool, int flags)
{
    int i, err = 0;

    if (pool == NULL)
        return THREADPOOL_INVALID;

    if (mtx_lock(&(pool->lock)) != 0)
        return THREADPOOL_LOCK_FAILURE;

    do {
        /* Already shutting down */
        if (pool->shutdown) {
            err = THREADPOOL_SHUTDOWN;
            break;
        }

        pool->shutdown = (flags & THREADPOOL_GRACEFUL)
            ? GRACEFUL_SHUTDOWN
            : IMMEDIATE_SHUTDOWN;

        /* Wake up all worker threads */
        if ((cnd_broadcast(&(pool->notify)) != 0)
         || (mtx_unlock(&(pool->lock)) != 0)) {
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }

        /* Join all worker thread */
        for (i = 0; i < pool->thread_count; i++) {
            if (thrd_join(pool->threads[i], NULL) != 0) {
                err = THREADPOOL_THREAD_FAILURE;
            }
        }
    } while(0);

    /* Only if everything went well do we deallocate the pool */
    if (!err)
        threadpool_free(pool);

    return err;
}

static int threadpool_free(threadpool_t* pool)
{
    if (pool == NULL || pool->started > 0)
        return -1;

    /* Did we manage to allocate ? */
    if (pool->threads) {
        free(pool->threads);
        free(pool->queue);

        /* Because we allocate pool->threads after initializing the
         * mutex and condition variable, we're sure they're
         * initialized. Let's lock the mutex just in case. */
        mtx_lock(&(pool->lock));
        mtx_destroy(&(pool->lock));
        cnd_destroy(&(pool->notify));
    }

    free(pool);
    return 0;
}

static int threadpool_thread(void* threadpool)
{
    threadpool_task_t task;
    threadpool_t* pool = (threadpool_t*)threadpool;

    for (;;) {
        /* Lock must be taken to wait on conditional variable */
        mtx_lock(&(pool->lock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from cnd_wait(), we own the lock. */
        while ((pool->count == 0) && (!pool->shutdown)) {
            cnd_wait(&(pool->notify), &(pool->lock));
        }

        if ((pool->shutdown == IMMEDIATE_SHUTDOWN)
         || ((pool->shutdown == GRACEFUL_SHUTDOWN) && (pool->count == 0))) {
            break;
        }

        /* Grab our task */
        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;

        /* Unlock */
        mtx_unlock(&(pool->lock));

        /* Get to work */
        (*(task.function))(task.argument);
    }

    pool->started--;
    mtx_unlock(&(pool->lock));
    thrd_exit(0);
    return 0;
}
