#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <endian.h>
#include <inttypes.h>

/*---------------------------------------------------------------
 * Examples
 *---------------------------------------------------------------*/
#if 0
#define NUM2PTR(x) ((void*)(uintptr_t)(x))

static void test_basic(void)
{
    hashmap_t* hmap;
    void* ret;

    hmap = hashmap_create(0, 0);
    assert(hmap != NULL);

    ret = hashmap_get(hmap, "test", 4);
    assert(ret == NULL);

    ret = hashmap_put(hmap, "test", 4, NUM2PTR(0x55));
    assert(ret == NUM2PTR(0x55));

    ret = hashmap_get(hmap, "test", 4);
    assert(ret == NUM2PTR(0x55));

    ret = hashmap_del(hmap, "test", 4);
    assert(ret == NUM2PTR(0x55));

    ret = hashmap_get(hmap, "test", 4);
    assert(ret == NULL);

    hashmap_destroy(hmap);
}

static void test_walk(void)
{
    const unsigned nitems = 17; // prime
    hashmap_t* hmap;
    void *ret, *key, *val;
    uint32_t bitmap = 0;
    uintmax_t iter;
    size_t klen;

    hmap = hashmap_create(0, 0);
    assert(hmap != NULL);

    iter = HM_WALK_BEGIN;
    key = hashmap_walk(hmap, &iter, &klen, &val);
    assert(key == NULL);

    for (unsigned i = 0; i < nitems; i++) {
        ret = hashmap_put(hmap, &i, sizeof(int), NUM2PTR(i));
        assert(ret == NUM2PTR(i));
    }

    iter = HM_WALK_BEGIN;
    while ((key = hashmap_walk(hmap, &iter, &klen, &val)) != NULL) {
        bitmap |= 1U << (uintptr_t)val;
    }
    assert(bitmap == 0x1ffff);

    hashmap_destroy(hmap);
}
#endif

/*---------------------------------------------------------------
 * Utils
 *---------------------------------------------------------------*/
/*
 * A regular assert (debug/diagnostic only).
 */
#if defined(DEBUG)
#define ASSERT assert
#else
#define ASSERT(x)
#endif

/*
 * Branch prediction macros.
 */
#ifndef __predict_true
#define __predict_true(x)  __builtin_expect((x) != 0, 1)
#define __predict_false(x) __builtin_expect((x) != 0, 0)
#endif

/*
 * Minimum, maximum and rounding macros.
 */
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

/*
 * DSO visibility attributes (for ELF targets).
 */
#if defined(__GNUC__) && __GNUC__ >= (4)
#define __dso_hidden __attribute__((__visibility__("hidden")))
#else
#define __dso_hidden
#endif

/*
 * Find first bit.
 */
#ifndef fls
static inline int fls(int x) { return x ? (sizeof(int) * CHAR_BIT) - __builtin_clz(x) : 0; }
#endif

/*---------------------------------------------------------------
 * Fast Div
 *---------------------------------------------------------------*/
/*
 * Fast 32bit division and remainder.
 *
 * Reference:
 *
 *  Torbjörn Granlund and Peter L. Montgomery, "Division by Invariant
 *  Integers Using Multiplication", ACM SIGPLAN Notices, Issue 6, Vol 29,
 *  http://gmplib.org/~tege/divcnst-pldi94.pdf, 61-72, June 1994.
 *
 * The following example computes q = a / b and r = a % b:
 *
 *  uint64_t divinfo = fast_div32_init(b);
 *  q = fast_div32(a, b, divinfo);
 *  r = fast_rem32(a, b, divinfo);
 */
static inline uint64_t fast_div32_init(uint32_t div)
{
    uint64_t mt;
    uint8_t s1, s2;
    int l;

    l  = fls(div - 1);
    mt = (uint64_t)(0x100000000ULL * ((1ULL << l) - div));
    s1 = (l > 1) ? 1U : (uint8_t)l;
    s2 = (l == 0) ? 0 : (uint8_t)(l - 1);
    return (uint64_t)(mt / div + 1) << 32 | (uint32_t)s1 << 8 | s2;
}

static inline uint32_t fast_div32(uint32_t v, uint32_t div, uint64_t divinfo)
{
    const uint32_t m  = divinfo >> 32;
    const unsigned s1 = (divinfo & 0x0000ff00) >> 8;
    const unsigned s2 = (divinfo & 0x000000ff);
    const uint32_t t  = (uint32_t)(((uint64_t)v * m) >> 32);
    (void)div; /* unused */
    return (t + ((v - t) >> s1)) >> s2;
}

static inline uint32_t fast_rem32(uint32_t v, uint32_t div, uint64_t divinfo)
{
    return v - div * fast_div32(v, div, divinfo);
}

/*
 * Common divisions (compiler will inline the constants and optimise).
 *
 * x = FAST_DIV32_10(1234);
 * y = FAST_REM32_10(1234);
 *
 * Will produce x = 123 and y = 4.
 */
#define FAST_DIV32_3(x)  fast_div32((x),  3, fast_div32_init(3))
#define FAST_DIV32_10(x) fast_div32((x), 10, fast_div32_init(10))
#define FAST_REM32_10(x) fast_rem32((x), 10, fast_div32_init(10))

/*---------------------------------------------------------------
 * MurmurHash3
 *---------------------------------------------------------------*/
/*
 * murmurhash3 -- from the original code:
 *
 * "MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain. The author hereby disclaims copyright to this source code."
 *
 * References:
 *  https://github.com/aappleby/smhasher/
 */
uint32_t murmurhash3(const void* key, size_t len, uint32_t seed)
{
    const uint8_t* data = key;
    const size_t orig_len = len;
    uint32_t h = seed;

    if (__predict_true(((uintptr_t)key & 3) == 0)) {
        while (len >= sizeof(uint32_t)) {
            uint32_t k = *(const uint32_t*)(const void*)data;

            k  = htole32(k);

            k *= 0xcc9e2d51;
            k  = (k << 15) | (k >> 17);
            k *= 0x1b873593;

            h ^= k;
            h  = (h << 13) | (h >> 19);
            h  = h * 5 + 0xe6546b64;

            data += sizeof(uint32_t);
            len  -= sizeof(uint32_t);
        }
    } else {
        while (len >= sizeof(uint32_t)) {
            uint32_t k;

            k  = data[0];
            k |= data[1] << 8;
            k |= data[2] << 16;
            k |= data[3] << 24;

            k *= 0xcc9e2d51;
            k  = (k << 15) | (k >> 17);
            k *= 0x1b873593;

            h ^= k;
            h  = (h << 13) | (h >> 19);
            h  = h * 5 + 0xe6546b64;

            data += sizeof(uint32_t);
            len  -= sizeof(uint32_t);
        }
    }

    /*
     * Handle the last few bytes of the input array.
     */
    uint32_t k = 0;

    switch (len) {
        case 3:
            k ^= data[2] << 16;
            /* FALLTHROUGH */
        case 2:
            k ^= data[1] << 8;
            /* FALLTHROUGH */
        case 1:
            k ^= data[0];
            k *= 0xcc9e2d51;
            k  = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
    }

    /*
     * Finalisation mix: force all bits of a hash block to avalanche.
     */
    h ^= orig_len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

/*---------------------------------------------------------------
 * Hashmap
 *---------------------------------------------------------------*/
#define MAX_GROWTH_STEP      (1024U * 1024)
#define APPROX_85_PERCENT(x) (((x)*870) >> 10)
#define APPROX_40_PERCENT(x) (((x)*409) >> 10)

typedef struct {
    void* key;
    void* val;
    uint64_t hash : 32;
    uint64_t psl  : 16;
    uint64_t len  : 16;
} rh_bucket_t;

struct hashmap {
    unsigned size;
    unsigned nitems;
    unsigned flags;
    unsigned minsize;
    uint64_t divinfo;
    rh_bucket_t* buckets;
    uint64_t hashkey;

    /*
     * Small optimisation for a single element case: allocate one
     * bucket together with the hashmap structure -- it will generally
     * fit within the same cache-line.
     */
    rh_bucket_t init_bucket;
};

static inline uint32_t compute_hash(const hashmap_t* hmap, const void* key, const size_t len)
{
    /*
     * Avoiding the use function pointers here; test and call relying
     * on branch predictors provides a better performance.
     */
    return murmurhash3(key, len, hmap->hashkey);
}

static int __attribute__((__unused__)) validate_psl_p(hashmap_t* hmap, const rh_bucket_t* bucket, unsigned i)
{
    unsigned base_i = fast_rem32(bucket->hash, hmap->size, hmap->divinfo);
    unsigned diff = (base_i > i) ? hmap->size - base_i + i : i - base_i;
    return bucket->key == NULL || diff == bucket->psl;
}

void* hashmap_get(hashmap_t* hmap, const void* key, size_t len)
{
    const uint32_t hash = compute_hash(hmap, key, len);
    unsigned n = 0, i = fast_rem32(hash, hmap->size, hmap->divinfo);
    rh_bucket_t* bucket;

    ASSERT(key != NULL);
    ASSERT(len != 0);

    /*
     * Lookup is a linear probe.
     */
probe:
    bucket = &hmap->buckets[i];
    ASSERT(validate_psl_p(hmap, bucket, i));

    if (bucket->hash == hash && bucket->len == len &&
        memcmp(bucket->key, key, len) == 0) {
        return bucket->val;
    }

    /*
     * Stop probing if we hit an empty bucket; also, if we hit a
     * bucket with PSL lower than the distance from the base location,
     * then it means that we found the "rich" bucket which should
     * have been captured, if the key was inserted -- see the central
     * point of the algorithm in the insertion function.
     */
    if (!bucket->key || n > bucket->psl) {
        return NULL;
    }
    n++;

    /* Continue to the next bucket. */
    i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
    goto probe;
}

/*
 * hashmap_insert: internal hashmap_put(), without the resize.
 */
static void* hashmap_insert(hashmap_t* hmap, const void* key, size_t len, void* val)
{
    const uint32_t hash = compute_hash(hmap, key, len);
    rh_bucket_t *bucket, entry;
    unsigned i;

    ASSERT(key != NULL);
    ASSERT(len != 0);

    /*
     * Setup the bucket entry.
     */
    if ((hmap->flags & HM_NOCOPY) == 0) {
        if ((entry.key = malloc(len)) == NULL) {
            return NULL;
        }
        memcpy(entry.key, key, len);
    } else {
        entry.key = (void*)(uintptr_t)key;
    }
    entry.hash = hash;
    entry.len  = len;
    entry.val  = val;
    entry.psl  = 0;

    /*
     * From the paper: "when inserting, if a record probes a location
     * that is already occupied, the record that has traveled longer
     * in its probe sequence keeps the location, and the other one
     * continues on its probe sequence" (page 12).
     *
     * Basically: if the probe sequence length (PSL) of the element
     * being inserted is greater than PSL of the element in the bucket,
     * then swap them and continue.
     */
    i = fast_rem32(hash, hmap->size, hmap->divinfo);
probe:
    bucket = &hmap->buckets[i];
    if (bucket->key) {
        ASSERT(validate_psl_p(hmap, bucket, i));

        /*
         * There is a key in the bucket.
         */
        if (bucket->hash == hash && bucket->len == len &&
            memcmp(bucket->key, key, len) == 0) {
            /* Duplicate key: return the current value. */
            if ((hmap->flags & HM_NOCOPY) == 0) {
                free(entry.key);
            }
            return bucket->val;
        }

        /*
         * We found a "rich" bucket.  Capture its location.
         */
        if (entry.psl > bucket->psl) {
            rh_bucket_t tmp;

            /*
             * Place our key-value pair by swapping the "rich"
             * bucket with our entry.  Copy the structures.
             */
            tmp     = entry;
            entry   = *bucket;
            *bucket = tmp;
        }
        entry.psl++;

        /* Continue to the next bucket. */
        ASSERT(validate_psl_p(hmap, bucket, i));
        i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
        goto probe;
    }

    /*
     * Found a free bucket: insert the entry.
     */
    *bucket = entry; // copy
    hmap->nitems++;

    ASSERT(validate_psl_p(hmap, bucket, i));
    return val;
}

static int hashmap_resize(hashmap_t* hmap, size_t newsize)
{
    const size_t len = newsize * sizeof(rh_bucket_t);
    rh_bucket_t* oldbuckets = hmap->buckets;
    const size_t oldsize = hmap->size;
    rh_bucket_t* newbuckets;

    ASSERT(newsize > 0);
    ASSERT(newsize > hmap->nitems);

    /*
     * Check for an overflow and allocate buckets.  Also, generate
     * a new hash key/seed every time we resize the hash table.
     */
    if (newsize == 1) {
        memset(&hmap->init_bucket, 0, sizeof(rh_bucket_t));
        newbuckets = &hmap->init_bucket;
    } else if (newsize > UINT_MAX) {
        return -1;
    } else if ((newbuckets = calloc(1, len)) == NULL) {
        return -1;
    }
    hmap->buckets = newbuckets;
    hmap->size    = newsize;
    hmap->nitems  = 0;

    hmap->divinfo  = fast_div32_init(newsize);
    hmap->hashkey ^= random() | (random() << 32);

    for (unsigned i = 0; i < oldsize; i++) {
        const rh_bucket_t* bucket = &oldbuckets[i];
        /* Skip the empty buckets. */
        if (!bucket->key) {
            continue;
        }
        hashmap_insert(hmap, bucket->key, bucket->len, bucket->val);
        if ((hmap->flags & HM_NOCOPY) == 0) {
            free(bucket->key);
        }
    }
    if (oldbuckets && oldbuckets != &hmap->init_bucket) {
        free(oldbuckets);
    }
    return 0;
}

void* hashmap_put(hashmap_t* hmap, const void* key, size_t len, void* val)
{
    const size_t threshold = APPROX_85_PERCENT(hmap->size);

    /*
     * If the load factor is more than the threshold, then resize.
     */
    if (__predict_false(hmap->nitems > threshold)) {
        /*
         * Grow the hash table by doubling its size, but with
         * a limit of MAX_GROWTH_STEP.
         */
        const size_t grow_limit = hmap->size + MAX_GROWTH_STEP;
        const size_t newsize = MIN(hmap->size << 1, grow_limit);
        if (hashmap_resize(hmap, newsize) != 0) {
            return NULL;
        }
    }

    return hashmap_insert(hmap, key, len, val);
}

void* hashmap_del(hashmap_t* hmap, const void* key, size_t len)
{
    const size_t threshold = APPROX_40_PERCENT(hmap->size);
    const uint32_t hash = compute_hash(hmap, key, len);
    unsigned n = 0, i = fast_rem32(hash, hmap->size, hmap->divinfo);
    rh_bucket_t* bucket;
    void* val;

    ASSERT(key != NULL);
    ASSERT(len != 0);
probe:
    /*
     * The same probing logic as in the lookup function.
     */
    bucket = &hmap->buckets[i];
    if (!bucket->key || n > bucket->psl) {
        return NULL;
    }
    ASSERT(validate_psl_p(hmap, bucket, i));

    if (bucket->hash != hash || bucket->len != len ||
        memcmp(bucket->key, key, len) != 0) {
        /* Continue to the next bucket. */
        i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
        n++;
        goto probe;
    }

    /*
     * Free the bucket.
     */
    if ((hmap->flags & HM_NOCOPY) == 0) {
        free(bucket->key);
    }
    val = bucket->val;
    hmap->nitems--;

    /*
     * The probe sequence must be preserved in the deletion case.
     * Use the backwards-shifting method to maintain low variance.
     */
    for (;;) {
        rh_bucket_t* nbucket;

        bucket->key = NULL;
        bucket->len = 0;

        i = fast_rem32(i + 1, hmap->size, hmap->divinfo);
        nbucket = &hmap->buckets[i];
        ASSERT(validate_psl_p(hmap, nbucket, i));

        /*
         * Stop if we reach an empty bucket or hit a key which
         * is in its base (original) location.
         */
        if (!nbucket->key || nbucket->psl == 0) {
            break;
        }

        nbucket->psl--;
        *bucket = *nbucket;
        bucket  = nbucket;
    }

    /*
     * If the load factor is less than threshold, then shrink by
     * halving the size, but not more than the minimum size.
     */
    if (hmap->nitems > hmap->minsize && hmap->nitems < threshold) {
        size_t newsize = MAX(hmap->size >> 1, hmap->minsize);
        (void)hashmap_resize(hmap, newsize);
    }
    return val;
}

void* hashmap_walk(hashmap_t* hmap, uintmax_t* iter, size_t* lenp, void** valp)
{
    const unsigned hmap_size = hmap->size;
    unsigned i = *iter;

    while (i < hmap_size) {
        rh_bucket_t* bucket = &hmap->buckets[i];

        i++; /* next */
        if (!bucket->key) {
            continue;
        }
        *iter = i;
        if (lenp) {
            *lenp = bucket->len;
        }
        if (valp) {
            *valp = bucket->val;
        }
        return bucket->key;
    }
    return NULL;
}

hashmap_t* hashmap_create(size_t size, unsigned flags)
{
    hashmap_t* hmap;

    hmap = calloc(1, sizeof(hashmap_t));
    if (!hmap) {
        return NULL;
    }
    hmap->flags = flags;
    hmap->minsize = MAX(size, 1);
    if (hashmap_resize(hmap, hmap->minsize) != 0) {
        free(hmap);
        return NULL;
    }
    ASSERT(hmap->buckets);
    ASSERT(hmap->size);
    return hmap;
}

void hashmap_destroy(hashmap_t* hmap)
{
    if ((hmap->flags & HM_NOCOPY) == 0) {
        for (unsigned i = 0; i < hmap->size; i++) {
            const rh_bucket_t* bucket = &hmap->buckets[i];

            if (bucket->key) {
                free(bucket->key);
            }
        }
    }
    if (hmap->buckets != &hmap->init_bucket) {
        free(hmap->buckets);
    }
    free(hmap);
}
