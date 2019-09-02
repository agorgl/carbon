#include "texture_font.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <math.h>

#define FONT_STB
#if defined(FONT_FREETYPE)
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_LCD_FILTER_H
#include FT_ADVANCES_H
#elif defined(FONT_STB)
#include <stb_truetype.h>
#else
#error "Unsupported font backend"
#endif

#define HRES  64
#define HRESf 64.f
#define DPI   72

/*-----------------------------------------------------------------
 * Freetype error list
 *-----------------------------------------------------------------*/
#ifdef FONT_FREETYPE
#undef __FTERRORS_H__
#define FT_ERRORDEF(e, v, s)  {e, s},
#define FT_ERROR_START_LIST   {
#define FT_ERROR_END_LIST     {0, 0}};
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H
#endif

/*-----------------------------------------------------------------
 * Vector
 *-----------------------------------------------------------------*/
struct vector {
    /* Pointer to dynamically allocated items */
    void* items;
    /* Number of items that can be held in currently allocated storage */
    size_t capacity;
    /* Number of items */
    size_t size;
    /* Size (in bytes) of a single item */
    size_t item_size;
};

static struct vector* vector_new(size_t item_size)
{
    struct vector* self = (struct vector*)malloc(sizeof(struct vector));
    assert(item_size);
    if (!self) {
        /* No more memory for allocating data */
        return 0;
    }
    self->item_size = item_size;
    self->size = 0;
    self->capacity = 1;
    self->items = malloc(self->item_size * self->capacity);
    return self;
}

static void vector_delete(struct vector* self)
{
    assert(self);
    free(self->items);
    free(self);
}

static void vector_reserve(struct vector* self, const size_t size)
{
    assert(self);
    if (self->capacity < size) {
        self->items = realloc(self->items, size * self->item_size);
        self->capacity = size;
    }
}

static void vector_clear(struct vector* self)
{
    assert(self);
    self->size = 0;
}

static void vector_set(struct vector* self, const size_t index, const void* item)
{
    assert(self);
    assert(self->size);
    assert(index < self->size);
    memcpy((char*)(self->items) + index * self->item_size, item, self->item_size);
}

static void vector_insert(struct vector* self, const size_t index, const void* item)
{
    assert(self);
    assert(index <= self->size);
    if (self->capacity <= self->size) {
        vector_reserve(self, 2 * self->capacity);
    }
    if (index < self->size) {
        memmove((char*)(self->items) + (index + 1) * self->item_size,
                (char*)(self->items) + (index + 0) * self->item_size,
                (self->size - index) * self->item_size);
    }
    self->size++;
    vector_set(self, index, item);
}

static void vector_push_back(struct vector* self, const void* item)
{
    vector_insert(self, self->size, item);
}

static size_t vector_size(const struct vector* self)
{
    assert(self);
    return self->size;
}

static const void* vector_get(const struct vector* self, size_t index)
{
    assert(self);
    assert(self->size);
    assert(index < self->size);
    return (char*)(self->items) + index * self->item_size;
}

/*-----------------------------------------------------------------
 * UTF-8 utils
 *-----------------------------------------------------------------*/
/* Returns the size in bytes of a given UTF-8 encoded character surrogate */
size_t utf8_surrogate_len(const char* character)
{
    size_t result = 0;
    char test_char;

    if (!character)
        return 0;

    test_char = character[0];

    if ((test_char & 0x80) == 0)
        return 1;

    while (test_char & 0x80) {
        test_char <<= 1;
        result++;
    }

    return result;
}

/* Returns the length of the given UTF-8 encoded and NULL terminated string */
size_t utf8_strlen(const char* string)
{
    const char* ptr = string;
    size_t result = 0;
    while (*ptr) {
        ptr += utf8_surrogate_len(ptr);
        result++;
    }
    return result;
}

/* Converts a given UTF-8 encoded character to its UTF-32 LE equivalent */
uint32_t utf8_to_utf32(const char* character)
{
    uint32_t result = -1;
    if (!character)
        return result;
    if ((character[0] & 0x80) == 0x0)
        result = character[0];
    if ((character[0] & 0xC0) == 0xC0)
        result = ((character[0] & 0x3F) << 6) | (character[1] & 0x3F);
    if ((character[0] & 0xE0) == 0xE0)
        result = ((character[0] & 0x1F) << (6 + 6)) | ((character[1] & 0x3F) << 6) | (character[2] & 0x3F);
    if ((character[0] & 0xF0) == 0xF0)
        result = ((character[0] & 0x0F) << (6 + 6 + 6)) | ((character[1] & 0x3F) << (6 + 6)) |
                 ((character[2] & 0x3F) << 6) | (character[3] & 0x3F);
    if ((character[0] & 0xF8) == 0xF8)
        result = ((character[0] & 0x07) << (6 + 6 + 6 + 6)) | ((character[1] & 0x3F) << (6 + 6 + 6)) |
                 ((character[2] & 0x3F) << (6 + 6)) | ((character[3] & 0x3F) << 6) | (character[4] & 0x3F);
    return result;
}

/*-----------------------------------------------------------------
 * edtaa3
 *-----------------------------------------------------------------*/
#define SQRT2 1.4142136
/*
 * Compute the local gradient at edge pixels using convolution filters.
 * The gradient is computed only at edge pixels. At other places in the
 * image, it is never used, and it's mostly zero anyway.
 */
void computegradient(double* img, int w, int h, double* gx, double* gy)
{
    int i, j, k;
    double glength;
    for (i = 1; i < h - 1; i++) { /* Avoid edges where the kernels would spill over */
        for (j = 1; j < w - 1; j++) {
            k = i * w + j;
            if ((img[k] > 0.0) && (img[k] < 1.0)) { /* Compute gradient for edge pixels only */
                gx[k] = -img[k - w - 1] - SQRT2 * img[k - 1] - img[k + w - 1] + img[k - w + 1] + SQRT2 * img[k + 1] +
                        img[k + w + 1];
                gy[k] = -img[k - w - 1] - SQRT2 * img[k - w] - img[k - w + 1] + img[k + w - 1] + SQRT2 * img[k + w] +
                        img[k + w + 1];
                glength = gx[k] * gx[k] + gy[k] * gy[k];
                if (glength > 0.0) { /* Avoid division by zero */
                    glength = sqrt(glength);
                    gx[k] = gx[k] / glength;
                    gy[k] = gy[k] / glength;
                }
            }
        }
    }
    /* TODO: Compute reasonable values for gx, gy also around the image edges.
     * (These are zero now, which reduces the accuracy for a 1-pixel wide region
     * around the image edge.) 2x2 kernels would be suitable for this. */
}

/*
 * A somewhat tricky function to approximate the distance to an edge in a
 * certain pixel, with consideration to either the local gradient (gx,gy)
 * or the direction to the pixel (dx,dy) and the pixel greyscale value a.
 * The latter alternative, using (dx,dy), is the metric used by edtaa2().
 * Using a local estimate of the edge gradient (gx,gy) yields much better
 * accuracy at and near edges, and reduces the error even at distant pixels
 * provided that the gradient direction is accurately estimated.
 */
double edgedf(double gx, double gy, double a)
{
    double df, glength, temp, a1;
    if ((gx == 0) || (gy == 0)) { /* Either A) gu or gv are zero, or B) both */
        df = 0.5 - a;             /* Linear approximation is A) correct or B) a fair guess */
    } else {
        glength = sqrt(gx * gx + gy * gy);
        if (glength > 0) {
            gx = gx / glength;
            gy = gy / glength;
        }
        /* Everything is symmetric wrt sign and transposition,
         * so move to first octant (gx >= 0, gy >= 0, gx >= gy) to
         * avoid handling all possible edge directions. */
        gx = fabs(gx);
        gy = fabs(gy);
        if (gx < gy) {
            temp = gx;
            gx = gy;
            gy = temp;
        }
        a1 = 0.5 * gy / gx;
        if (a < a1) { /* 0 <= a < a1 */
            df = 0.5 * (gx + gy) - sqrt(2.0 * gx * gy * a);
        } else if (a < (1.0 - a1)) { /* a1 <= a <= 1-a1 */
            df = (0.5 - a) * gx;
        } else { /* 1-a1 < a <= 1 */
            df = -0.5 * (gx + gy) + sqrt(2.0 * gx * gy * (1.0 - a));
        }
    }
    return df;
}

double distaa3(double* img, double* gximg, double* gyimg, int w, int c, int xc, int yc, int xi, int yi)
{
    double di, df, dx, dy, gx, gy, a;
    int closest;

    closest = c - xc - yc * w; /* Index to the edge pixel pointed to from c */
    a = img[closest];          /* Grayscale value at the edge pixel */
    gx = gximg[closest];       /* X gradient component at the edge pixel */
    gy = gyimg[closest];       /* Y gradient component at the edge pixel */

    /* Clip grayscale values outside the range [0,1] */
    if (a > 1.0)
        a = 1.0;
    if (a < 0.0)
        a = 0.0;

    /* Not an object pixel, return "very far" ("don't know yet") */
    if (a == 0.0)
        return 1000000.0;

    dx = (double)xi;
    dy = (double)yi;
    di = sqrt(dx * dx + dy * dy); /* Length of integer vector, like a traditional EDT */
    if (di == 0) {                /* Use local gradient only at edges */
        /* Estimate based on local gradient only */
        df = edgedf(gx, gy, a);
    } else {
        /* Estimate gradient based on direction to edge (accurate for large di) */
        df = edgedf(dx, dy, a);
    }
    return di + df; /* Same metric as edtaa2, except at edges (where di=0) */
}

/* Shorthand macro: add ubiquitous parameters dist, gx, gy, img and w and call distaa3() */
#define DISTAA(c, xc, yc, xi, yi) (distaa3(img, gx, gy, w, c, xc, yc, xi, yi))

void edtaa3(double* img, double* gx, double* gy, int w, int h, short* distx, short* disty, double* dist)
{
    int x, y, i, c;
    int offset_u, offset_ur, offset_r, offset_rd, offset_d, offset_dl, offset_l, offset_lu;
    double olddist, newdist;
    int cdistx, cdisty, newdistx, newdisty;
    int changed;
    double epsilon = 1e-3;

    /* Initialize index offsets for the current image width */
    offset_u = -w;
    offset_ur = -w + 1;
    offset_r = 1;
    offset_rd = w + 1;
    offset_d = w;
    offset_dl = w - 1;
    offset_l = -1;
    offset_lu = -w - 1;

    /* Initialize the distance images */
    for (i = 0; i < w * h; i++) {
        distx[i] = 0; /* At first, all pixels point to */
        disty[i] = 0; /* themselves as the closest known. */
        if (img[i] <= 0.0) {
            dist[i] = 1000000.0; /* Big value, means "not set yet" */
        } else if (img[i] < 1.0) {
            dist[i] = edgedf(gx[i], gy[i], img[i]); /* Gradient-assisted estimate */
        } else {
            dist[i] = 0.0; /* Inside the object */
        }
    }

    /* Perform the transformation */
    do {
        changed = 0;

        /* Scan rows, except first row */
        for (y = 1; y < h; y++) {
            /* Move index to leftmost pixel of current row */
            i = y * w;
            /* Scan right, propagate distances from above & left */
            /* Leftmost pixel is special, has no left neighbors */
            olddist = dist[i];
            if (olddist > 0) { /* If non-zero distance or not set yet */
                c = i + offset_u; /* Index of candidate for testing */
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx;
                newdisty = cdisty + 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_ur;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx - 1;
                newdisty = cdisty + 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    changed = 1;
                }
            }
            i++;

            /* Middle pixels have all neighbors */
            for (x = 1; x < w - 1; x++, i++) {
                olddist = dist[i];
                if (olddist <= 0)
                    continue; /* No need to update further */

                c = i + offset_l;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx + 1;
                newdisty = cdisty;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_lu;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx + 1;
                newdisty = cdisty + 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_u;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx;
                newdisty = cdisty + 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_ur;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx - 1;
                newdisty = cdisty + 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    changed = 1;
                }
            }

            /* Rightmost pixel of row is special, has no right neighbors */
            olddist = dist[i];
            if (olddist > 0) { /* If not already zero distance */
                c = i + offset_l;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx + 1;
                newdisty = cdisty;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_lu;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx + 1;
                newdisty = cdisty + 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_u;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx;
                newdisty = cdisty + 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    changed = 1;
                }
            }

            /* Move index to second rightmost pixel of current row. */
            /* Rightmost pixel is skipped, it has no right neighbor. */
            i = y * w + w - 2;

            /* Scan left, propagate distance from right */
            for (x = w - 2; x >= 0; x--, i--) {
                olddist = dist[i];
                if (olddist <= 0)
                    continue; /* Already zero distance */

                c = i + offset_r;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx - 1;
                newdisty = cdisty;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    changed = 1;
                }
            }
        }

        /* Scan rows in reverse order, except last row */
        for (y = h - 2; y >= 0; y--) {
            /* Move index to rightmost pixel of current row */
            i = y * w + w - 1;

            /* Scan left, propagate distances from below & right */
            /* Rightmost pixel is special, has no right neighbors */
            olddist = dist[i];
            if (olddist > 0) { /* If not already zero distance */
                c = i + offset_d;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx;
                newdisty = cdisty - 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_dl;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx + 1;
                newdisty = cdisty - 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    changed = 1;
                }
            }
            i--;

            /* Middle pixels have all neighbors */
            for (x = w - 2; x > 0; x--, i--) {
                olddist = dist[i];
                if (olddist <= 0)
                    continue; /* Already zero distance */

                c = i + offset_r;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx - 1;
                newdisty = cdisty;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_rd;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx - 1;
                newdisty = cdisty - 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_d;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx;
                newdisty = cdisty - 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_dl;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx + 1;
                newdisty = cdisty - 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    changed = 1;
                }
            }
            /* Leftmost pixel is special, has no left neighbors */
            olddist = dist[i];
            if (olddist > 0) { /* If not already zero distance */
                c = i + offset_r;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx - 1;
                newdisty = cdisty;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_rd;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx - 1;
                newdisty = cdisty - 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    olddist = newdist;
                    changed = 1;
                }

                c = i + offset_d;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx;
                newdisty = cdisty - 1;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    changed = 1;
                }
            }

            /* Move index to second leftmost pixel of current row. */
            /* Leftmost pixel is skipped, it has no left neighbor. */
            i = y * w + 1;
            for (x = 1; x < w; x++, i++) {
                /* Scan right, propagate distance from left */
                olddist = dist[i];
                if (olddist <= 0)
                    continue; /* Already zero distance */

                c = i + offset_l;
                cdistx = distx[c];
                cdisty = disty[c];
                newdistx = cdistx + 1;
                newdisty = cdisty;
                newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
                if (newdist < olddist - epsilon) {
                    distx[i] = newdistx;
                    disty[i] = newdisty;
                    dist[i] = newdist;
                    changed = 1;
                }
            }
        }
    } while (changed); /* Sweep until no more updates are made */
    /* The transformation is completed. */
}

/*-----------------------------------------------------------------
 * Distance field
 *-----------------------------------------------------------------*/
/* Create a distance map from the given grayscale image.
 * Returns a newly allocated distance field.
 * This image must be freed after usage. */
double* make_distance_mapd(double* data, unsigned int width, unsigned int height)
{
    short* xdist    = (short*)malloc(width * height * sizeof(short));
    short* ydist    = (short*)malloc(width * height * sizeof(short));
    double* gx      = (double*)calloc(width * height, sizeof(double));
    double* gy      = (double*)calloc(width * height, sizeof(double));
    double* outside = (double*)calloc(width * height, sizeof(double));
    double* inside  = (double*)calloc(width * height, sizeof(double));
    double vmin     = DBL_MAX;
    unsigned int i;

    /* Compute outside = edtaa3(bitmap); % Transform background (0's) */
    computegradient(data, width, height, gx, gy);
    edtaa3(data, gx, gy, width, height, xdist, ydist, outside);
    for (i = 0; i < width * height; ++i)
        if (outside[i] < 0.0)
            outside[i] = 0.0;

    /* Compute inside = edtaa3(1-bitmap); % Transform foreground (1's) */
    memset(gx, 0, sizeof(double) * width * height);
    memset(gy, 0, sizeof(double) * width * height);
    for (i = 0; i < width * height; ++i)
        data[i] = 1 - data[i];
    computegradient(data, width, height, gx, gy);
    edtaa3(data, gx, gy, width, height, xdist, ydist, inside);
    for (i = 0; i < width * height; ++i)
        if (inside[i] < 0)
            inside[i] = 0.0;

    /* distmap = outside - inside; % Bipolar distance field */
    for (i = 0; i < width * height; ++i) {
        outside[i] -= inside[i];
        if (outside[i] < vmin)
            vmin = outside[i];
    }

    vmin = fabs(vmin);

    for (i = 0; i < width * height; ++i) {
        double v = outside[i];
        if (v < -vmin)
            outside[i] = -vmin;
        else if (v > +vmin)
            outside[i] = +vmin;
        data[i] = (outside[i] + vmin) / (2 * vmin);
    }

    free(xdist);
    free(ydist);
    free(gx);
    free(gy);
    free(outside);
    free(inside);
    return data;
}

unsigned char* make_distance_mapb(unsigned char* img, unsigned int width, unsigned int height)
{
    double* data = (double*)calloc(width * height, sizeof(double));
    unsigned char* out = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    unsigned int i;

     /* Find minimimum and maximum values */
    double img_min = DBL_MAX, img_max = DBL_MIN;
    for (i = 0; i < width * height; ++i) {
        double v = img[i];
        data[i] = v;
        if (v > img_max)
            img_max = v;
        if (v < img_min)
            img_min = v;
    }

     /* Map values from 0 - 255 to 0.0 - 1.0 */
    for (i = 0; i < width * height; ++i)
        data[i] = (img[i] - img_min) / img_max;

    data = make_distance_mapd(data, width, height);

     /* Map values from 0.0 - 1.0 to 0 - 255 */
    for (i = 0; i < width * height; ++i)
        out[i] = (unsigned char)(255 * (1 - data[i]));

    free(data);
    return out;
}

/*-----------------------------------------------------------------
 * Texture glyph
 *-----------------------------------------------------------------*/
texture_glyph* texture_glyph_new()
{
    texture_glyph* self = (texture_glyph*)malloc(sizeof(texture_glyph));
    if (self == NULL) {
        /* No more memory for allocating data */
        return 0;
    }

    self->codepoint         = -1;
    self->width             = 0;
    self->height            = 0;
    self->rendermode        = GLYPH_RENDER_NORMAL;
    self->outline_thickness = 0.0;
    self->offset_x          = 0;
    self->offset_y          = 0;
    self->advance_x         = 0.0;
    self->advance_y         = 0.0;
    self->s0                = 0.0;
    self->t0                = 0.0;
    self->s1                = 0.0;
    self->t1                = 0.0;
    self->kerning           = vector_new(sizeof(kerning_pair));
    return self;
}

void texture_glyph_delete(texture_glyph* self)
{
    assert(self);
    vector_delete(self->kerning);
    free(self);
}

float texture_glyph_get_kerning(const texture_glyph* self, const char* codepoint)
{
    assert(self);
    uint32_t ucodepoint = utf8_to_utf32(codepoint);

    for (size_t i = 0; i < vector_size(self->kerning); ++i) {
        kerning_pair* kerning = (kerning_pair*)vector_get(self->kerning, i);
        if (kerning->codepoint == ucodepoint) {
            return kerning->kerning;
        }
    }
    return 0;
}

/*-----------------------------------------------------------------
 * Texture font
 *-----------------------------------------------------------------*/
#ifdef FONT_FREETYPE
static int texture_font_load_face(texture_font* self, float size, FT_Library* library, FT_Face* face)
{
    FT_Error error;
    FT_Matrix matrix = {
        (int)((1.0/HRES) * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((1.0)      * 0x10000L)};

    assert(library);
    assert(size);

    /* Initialize library */
    error = FT_Init_FreeType(library);
    if (error) {
        fprintf(stderr, "FT_Error (0x%02x) : %s\n", FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup;
    }

    /* Load face */
    switch (self->location) {
        case TEXTURE_FONT_FILE:
            error = FT_New_Face(*library, self->filename, 0, face);
            break;
        case TEXTURE_FONT_MEMORY:
            error = FT_New_Memory_Face(*library, self->memory.base, self->memory.size, 0, face);
            break;
    }

    if (error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup_library;
    }

    /* Select charmap */
    error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
    if (error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup_face;
    }

    /* Set char size */
    error = FT_Set_Char_Size(*face, (int)(size * HRES), 0, DPI * HRES, DPI);

    if (error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup_face;
    }

    /* Set transform matrix */
    FT_Set_Transform(*face, &matrix, NULL);

    return 1;

cleanup_face:
    FT_Done_Face(*face);
cleanup_library:
    FT_Done_FreeType(*library);
cleanup:
    return 0;
}
#endif

#ifdef FONT_STB
static int texture_font_load_face(texture_font* self, stbtt_fontinfo* stb_fi)
{
    assert(self->location == TEXTURE_FONT_MEMORY); /* Unimplemented */

    int font_offset = stbtt_GetFontOffsetForIndex(self->memory.base, 0);
    if (font_offset == -1)
        return 0;

    int success = stbtt_InitFont(stb_fi, self->memory.base, font_offset);
    if (!success)
        return 0;

    return 1;
}
#endif

#if defined(FONT_FREETYPE)
void texture_font_generate_kerning(texture_font* self, FT_Face* face)
{
    FT_UInt glyph_index, prev_index;
    FT_Vector kerning;
#elif defined(FONT_STB)
void texture_font_generate_kerning(texture_font* self, stbtt_fontinfo* stb_fi)
{
    int glyph_index, prev_index;
    struct {long x, y;} kerning;
    float scale = stbtt_ScaleForMappingEmToPixels(stb_fi, self->size);
#endif
    texture_glyph *glyph, *prev_glyph;
    assert(self);

    /* For each glyph couple combination, check if kerning is necessary */
    /* Starts at index 1 since 0 is for the special backgroudn glyph */
    for (size_t i = 1; i < self->glyphs->size; ++i) {
        glyph = *(texture_glyph**)vector_get(self->glyphs, i);
#if defined(FONT_FREETYPE)
        glyph_index = FT_Get_Char_Index(*face, glyph->codepoint);
#elif defined(FONT_STB)
        glyph_index = stbtt_FindGlyphIndex(stb_fi, glyph->codepoint);
#endif
        vector_clear(glyph->kerning);

        for (size_t j = 1; j < self->glyphs->size; ++j) {
            prev_glyph = *(texture_glyph**)vector_get(self->glyphs, j);
#if defined(FONT_FREETYPE)
            prev_index = FT_Get_Char_Index(*face, prev_glyph->codepoint);
            FT_Get_Kerning(*face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning);
#elif defined(FONT_STB)
            prev_index = stbtt_FindGlyphIndex(stb_fi, prev_glyph->codepoint);
            kerning.x  = stbtt_GetGlyphKernAdvance(stb_fi, prev_index, glyph_index) * scale;
            kerning.x *= (float)(HRESf * HRESf);
#endif
            /* printf("%c(%d)-%c(%d): %ld\n",
             *       prev_glyph->codepoint, prev_glyph->codepoint,
             *       glyph_index, glyph_index, kerning.x); */
            if (kerning.x) {
                kerning_pair k = {prev_glyph->codepoint, kerning.x / (float)(HRESf * HRESf)};
                vector_push_back(glyph->kerning, &k);
            }
        }
    }
}

static int texture_font_init(texture_font* self)
{
    assert(self->atlas);
    assert(self->size > 0);
    assert((self->location == TEXTURE_FONT_FILE && self->filename) ||
           (self->location == TEXTURE_FONT_MEMORY && self->memory.base && self->memory.size));

    self->glyphs = vector_new(sizeof(texture_glyph*));
    self->height = 0;
    self->ascender = 0;
    self->descender = 0;
    self->rendermode = GLYPH_RENDER_NORMAL;
    self->outline_thickness = 0.0;
    self->hinting = 1;
    self->kerning = 1;
    self->filtering = 1;

    /* FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00) */
    /* FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10) */
    self->lcd_weights[0] = 0x10;
    self->lcd_weights[1] = 0x40;
    self->lcd_weights[2] = 0x70;
    self->lcd_weights[3] = 0x40;
    self->lcd_weights[4] = 0x10;

#if defined(FONT_FREETYPE)
    FT_Library library;
    FT_Face face;
    if (!texture_font_load_face(self, self->size * 100.f, &library, &face))
        return -1;
#elif defined(FONT_STB)
    struct stbtt_fontinfo stb_fi;
    if (!texture_font_load_face(self, &stb_fi))
        return -1;
#endif

#if defined(FONT_FREETYPE)
    self->underline_position = face->underline_position / (float)(HRESf * HRESf) * self->size;
    self->underline_position = roundf(self->underline_position);
    if (self->underline_position > -2) {
        self->underline_position = -2.0;
    }

    self->underline_thickness = face->underline_thickness / (float)(HRESf * HRESf) * self->size;
    self->underline_thickness = roundf(self->underline_thickness);
    if (self->underline_thickness < 1) {
        self->underline_thickness = 1.0;
    }
#endif

#if defined(FONT_FREETYPE)
    FT_Size_Metrics metrics;
    metrics = face->size->metrics;
    self->ascender  = (metrics.ascender >> 6) / 100.0;
    self->descender = (metrics.descender >> 6) / 100.0;
    self->height    = (metrics.height >> 6) / 100.0;
    self->linegap   = self->height - self->ascender + self->descender;
#elif defined(FONT_STB)
    int ascender, descender, line_gap;
    stbtt_GetFontVMetrics(&stb_fi, &ascender, &descender, &line_gap);
    float scale = stbtt_ScaleForMappingEmToPixels(&stb_fi, self->size);
    self->ascender  = scale * ascender;
    self->descender = scale * descender;
    self->linegap   = scale * line_gap;
    self->height    = self->ascender - self->descender + self->linegap;
#endif

#if defined(FONT_FREETYPE)
    FT_Done_Face(face);
    FT_Done_FreeType(library);
#endif

    /* NULL is a special glyph */
    texture_font_get_glyph(self, NULL);
    return 0;
}

texture_font* texture_font_new_from_file(texture_atlas* atlas, const float pt_size, const char* filename)
{
    assert(filename);
    texture_font* self = calloc(1, sizeof(*self));
    if (!self) {
        /* No more memory for allocating data */
        return 0;
    }

    self->atlas    = atlas;
    self->size     = pt_size;
    self->location = TEXTURE_FONT_FILE;
    self->filename = strdup(filename);

    if (texture_font_init(self)) {
        texture_font_delete(self);
        return 0;
    }
    return self;
}

texture_font* texture_font_new_from_memory(texture_atlas* atlas, float pt_size, const void* memory_base, size_t memory_size)
{
    assert(memory_base);
    assert(memory_size);
    texture_font* self = calloc(1, sizeof(*self));
    if (!self) {
        /* No more memory for allocating data */
        return 0;
    }

    self->atlas       = atlas;
    self->size        = pt_size;
    self->location    = TEXTURE_FONT_MEMORY;
    self->memory.base = memory_base;
    self->memory.size = memory_size;

    if (texture_font_init(self)) {
        texture_font_delete(self);
        return 0;
    }
    return self;
}

void texture_font_delete(texture_font* self)
{
    assert(self);
    if (self->location == TEXTURE_FONT_FILE && self->filename)
        free(self->filename);
    for (size_t i = 0; i < vector_size(self->glyphs); ++i) {
        texture_glyph* glyph = *(texture_glyph**)vector_get(self->glyphs, i);
        texture_glyph_delete(glyph);
    }
    vector_delete(self->glyphs);
    free(self);
}

texture_glyph* texture_font_find_glyph(texture_font* self, const char* codepoint)
{
    uint32_t ucodepoint = utf8_to_utf32(codepoint);
    for (size_t i = 0; i < self->glyphs->size; ++i) {
        texture_glyph* glyph = *(texture_glyph**)vector_get(self->glyphs, i);
        /* If codepoint is -1, we don't care about outline type or thickness */
        if ((glyph->codepoint == ucodepoint) &&
            ((ucodepoint == (uint32_t)(-1)) ||
             ((glyph->rendermode == self->rendermode) &&
              (glyph->outline_thickness == self->outline_thickness)))) {
            return glyph;
        }
    }
    return 0;
}

int texture_font_load_glyph(texture_font* self, const char* codepoint)
{
    int rval = 0;
#if defined (FONT_FREETYPE)
    FT_Library library;
    FT_Face face;
    if (!texture_font_load_face(self, self->size, &library, &face))
        return 0;
#elif defined (FONT_STB)
    struct stbtt_fontinfo stb_fi;
    if (!texture_font_load_face(self, &stb_fi))
        return 0;
#endif

    /* Check if codepoint has been already loaded */
    if (texture_font_find_glyph(self, codepoint)) {
        rval = 1;
        goto cleanup;
    }

    /* Codepoint NULL is special: it is used for line drawing
     * (overline, underline, strikethrough) and background */
    if (!codepoint) {
        ivec4 region = texture_atlas_get_region(self->atlas, 5, 5);
        texture_glyph* glyph = texture_glyph_new();
        static unsigned char data[4 * 4 * 3] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
        if (region.x < 0) {
            fprintf(stderr, "Texture atlas is full (line %d)\n", __LINE__);
            rval = 0;
            goto cleanup;
        }
        texture_atlas_set_region(self->atlas, region.x, region.y, 4, 4, data, 0);
        glyph->codepoint = -1;
        glyph->s0 = (region.x + 2) / (float)self->atlas->width;
        glyph->t0 = (region.y + 2) / (float)self->atlas->height;
        glyph->s1 = (region.x + 3) / (float)self->atlas->width;
        glyph->t1 = (region.y + 3) / (float)self->atlas->height;
        vector_push_back(self->glyphs, &glyph);

        rval = 1;
        goto cleanup;
    }

    struct {
        unsigned int rows;
        unsigned int width;
        int pitch, xoffs, yoffs;
        unsigned char* buffer;
    } src_bitmap;

#if defined (FONT_FREETYPE)
    int ft_glyph_top = 0;
    int ft_glyph_left = 0;
    FT_UInt glyph_index = FT_Get_Char_Index(face, (FT_ULong)utf8_to_utf32(codepoint));
    FT_Int32 flags = 0;

    /* WARNING: We use texture-atlas depth to guess if user wants LCD subpixel rendering */
    if (self->rendermode != GLYPH_RENDER_NORMAL && self->rendermode != GLYPH_RENDER_SIGNED_DISTANCE_FIELD) {
        flags |= FT_LOAD_NO_BITMAP;
    } else {
        flags |= FT_LOAD_RENDER;
    }

    if (!self->hinting) {
        flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
    } else {
        flags |= FT_LOAD_FORCE_AUTOHINT;
    }

    if (self->atlas->depth == 3) {
        FT_Library_SetLcdFilter(library, FT_LCD_FILTER_LIGHT);
        flags |= FT_LOAD_TARGET_LCD;

        if (self->filtering) {
            FT_Library_SetLcdFilterWeights(library, self->lcd_weights);
        }
    }

    FT_Error error;
    error = FT_Load_Glyph(face, glyph_index, flags);
    if (error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        rval = 0;
        goto cleanup;
    }

    FT_Glyph ft_glyph;
    FT_Bitmap ft_bitmap;
    if (self->rendermode == GLYPH_RENDER_NORMAL || self->rendermode == GLYPH_RENDER_SIGNED_DISTANCE_FIELD) {
        FT_GlyphSlot slot = face->glyph;
        ft_bitmap = slot->bitmap;
        ft_glyph_top = slot->bitmap_top;
        ft_glyph_left = slot->bitmap_left;
    } else {
        FT_Stroker stroker;
        FT_BitmapGlyph ft_bitmap_glyph;
        error = FT_Stroker_New(library, &stroker);

        if (error) {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n", FT_Errors[error].code, FT_Errors[error].message);
            goto cleanup_stroker;
        }

        FT_Stroker_Set(stroker, (int)(self->outline_thickness * HRES), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
        error = FT_Get_Glyph(face->glyph, &ft_glyph);

        if (error) {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n", FT_Errors[error].code, FT_Errors[error].message);
            goto cleanup_stroker;
        }

        if (self->rendermode == GLYPH_RENDER_OUTLINE_EDGE)
            error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
        else if (self->rendermode == GLYPH_RENDER_OUTLINE_POSITIVE)
            error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 0, 1);
        else if (self->rendermode == GLYPH_RENDER_OUTLINE_NEGATIVE)
            error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 1, 1);

        if (error) {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n", FT_Errors[error].code, FT_Errors[error].message);
            goto cleanup_stroker;
        }

        if (self->atlas->depth == 1)
            error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
        else
            error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_LCD, 0, 1);

        if (error) {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n", FT_Errors[error].code, FT_Errors[error].message);
            goto cleanup_stroker;
        }

        ft_bitmap_glyph = (FT_BitmapGlyph)ft_glyph;
        ft_bitmap = ft_bitmap_glyph->bitmap;
        ft_glyph_top = ft_bitmap_glyph->top;
        ft_glyph_left = ft_bitmap_glyph->left;

    cleanup_stroker:
        FT_Stroker_Done(stroker);

        if (error) {
            rval = 0;
            goto cleanup;
        }
    }
    src_bitmap.width  = ft_bitmap.width;
    src_bitmap.rows   = ft_bitmap.rows;
    src_bitmap.pitch  = ft_bitmap.pitch;
    src_bitmap.buffer = ft_bitmap.buffer;
    src_bitmap.xoffs  = ft_glyph_left;
    src_bitmap.yoffs  = ft_glyph_top;

#elif defined(FONT_STB)
    assert(self->atlas->depth == 1);
    int glyph_index = stbtt_FindGlyphIndex(&stb_fi, utf8_to_utf32(codepoint));
    float scale = stbtt_ScaleForMappingEmToPixels(&stb_fi, self->size);
    int width, height, xoffs, yoffs;
    unsigned char* data = stbtt_GetGlyphBitmap(&stb_fi, scale, scale, glyph_index, &width, &height, &xoffs, &yoffs);

    src_bitmap.width  = width;
    src_bitmap.rows   = height;
    src_bitmap.pitch  = width;
    src_bitmap.buffer = data;
    src_bitmap.xoffs  = xoffs;
    src_bitmap.yoffs  = -yoffs;
#endif

    struct {
        int left;
        int top;
        int right;
        int bottom;
    } padding = {0, 0, 1, 1};

    if (self->rendermode == GLYPH_RENDER_SIGNED_DISTANCE_FIELD) {
        padding.top = 1;
        padding.left = 1;
    }

    if (self->padding != 0) {
        padding.top += self->padding;
        padding.left += self->padding;
        padding.right += self->padding;
        padding.bottom += self->padding;
    }

    size_t src_w = src_bitmap.width / self->atlas->depth;
    size_t src_h = src_bitmap.rows;
    size_t tgt_w = src_w + padding.left + padding.right;
    size_t tgt_h = src_h + padding.top + padding.bottom;

    ivec4 region = texture_atlas_get_region(self->atlas, tgt_w, tgt_h);

    if (region.x < 0) {
        fprintf(stderr, "Texture atlas is full (line %d)\n", __LINE__);
        rval = 0;
        goto cleanup;
    }

    size_t x = region.x;
    size_t y = region.y;

    unsigned char* buffer = calloc(tgt_w * tgt_h * self->atlas->depth, sizeof(unsigned char));
    unsigned char* dst_ptr = buffer + (padding.top * tgt_w + padding.left) * self->atlas->depth;
    unsigned char* src_ptr = src_bitmap.buffer;
    for (size_t i = 0; i < src_h; i++) {
        /* Difference between width and pitch:
         * https://www.freetype.org/freetype2/docs/reference/ft2-basic_types.html#FT_Bitmap */
        memcpy(dst_ptr, src_ptr, src_bitmap.width);
        dst_ptr += tgt_w * self->atlas->depth;
        src_ptr += src_bitmap.pitch;
    }

#if defined(FONT_STB)
    free(src_bitmap.buffer);
#endif

    if (self->rendermode == GLYPH_RENDER_SIGNED_DISTANCE_FIELD) {
        unsigned char* sdf = make_distance_mapb(buffer, tgt_w, tgt_h);
        free(buffer);
        buffer = sdf;
    }

    texture_atlas_set_region(self->atlas, x, y, tgt_w, tgt_h, buffer, tgt_w * self->atlas->depth);
    free(buffer);

    texture_glyph* glyph     = texture_glyph_new();
    glyph->codepoint         = utf8_to_utf32(codepoint);
    glyph->width             = tgt_w;
    glyph->height            = tgt_h;
    glyph->rendermode        = self->rendermode;
    glyph->outline_thickness = self->outline_thickness;
    glyph->offset_x          = src_bitmap.xoffs;
    glyph->offset_y          = src_bitmap.yoffs;
    glyph->s0                = x / (float)self->atlas->width;
    glyph->t0                = y / (float)self->atlas->height;
    glyph->s1                = (x + glyph->width) / (float)self->atlas->width;
    glyph->t1                = (y + glyph->height) / (float)self->atlas->height;

    /* Discard hinting to get advance */
#if defined(FONT_FREETYPE)
    FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
    FT_GlyphSlot slot = face->glyph;
    glyph->advance_x = slot->advance.x / HRESf;
    glyph->advance_y = slot->advance.y / HRESf;

    if (self->rendermode != GLYPH_RENDER_NORMAL && self->rendermode != GLYPH_RENDER_SIGNED_DISTANCE_FIELD)
        FT_Done_Glyph(ft_glyph);

    texture_font_generate_kerning(self, &face);
#elif defined(FONT_STB)
    int advance, left_bearing;
    stbtt_GetGlyphHMetrics(&stb_fi, glyph_index, &advance, &left_bearing);
    advance *= scale; left_bearing *= scale;
    glyph->advance_x = advance;
    glyph->advance_y = 0.0f;

    texture_font_generate_kerning(self, &stb_fi);
#endif

    /* Store glyph */
    vector_push_back(self->glyphs, &glyph);
    rval = 1;

cleanup:
#ifdef FONT_FREETYPE
    FT_Done_Face(face);
    FT_Done_FreeType(library);
#endif
    return rval;
}

size_t texture_font_load_glyphs(texture_font* self, const char* codepoints)
{
    /* Load each glyph */
    for (size_t i = 0; i < strlen(codepoints); i += utf8_surrogate_len(codepoints + i)) {
        if (!texture_font_load_glyph(self, codepoints + i))
            return utf8_strlen(codepoints + i);
    }
    return 0;
}

texture_glyph* texture_font_get_glyph(texture_font* self, const char* codepoint)
{
    texture_glyph* glyph;

    assert(self);
    assert(self->filename);
    assert(self->atlas);

    /* Check if codepoint has been already loaded */
    if ((glyph = texture_font_find_glyph(self, codepoint)))
        return glyph;

    /* Glyph has not been already loaded */
    if (texture_font_load_glyph(self, codepoint))
        return texture_font_find_glyph(self, codepoint);

    return 0;
}

void texture_font_enlarge_atlas(texture_font* self, size_t width_new, size_t height_new)
{
    assert(self);
    assert(self->atlas);

    /* Ensure size increased */
    assert(width_new >= self->atlas->width);
    assert(height_new >= self->atlas->height);
    assert(width_new + height_new > self->atlas->width + self->atlas->height);
    texture_atlas* ta = self->atlas;
    size_t width_old = ta->width;
    size_t height_old = ta->height;

    /* Allocate new buffer */
    unsigned char* data_old = ta->data;
    ta->data = calloc(1, width_new * height_new * sizeof(char) * ta->depth);

    /* Update atlas size */
    ta->width = width_new;
    ta->height = height_new;

    /* Add node reflecting the gained space on the right */
    if (width_new > width_old) {
        ivec3 node;
        node.x = width_old - 1;
        node.y = 1;
        node.z = width_new - width_old;
        vector_push_back(ta->nodes, &node);
    }

    /* Copy over data from the old buffer, skipping first row and column because of the margin */
    size_t pixel_size = sizeof(char) * ta->depth;
    size_t old_row_size = width_old * pixel_size;
    texture_atlas_set_region(ta, 1, 1, width_old - 2, height_old - 2, data_old + old_row_size + pixel_size, old_row_size);
    free(data_old);

    /* Change uv coordinates of existing glyphs to reflect size change */
    float mulw = (float)width_old / width_new;
    float mulh = (float)height_old / height_new;
    for (size_t i = 0; i < vector_size(self->glyphs); i++) {
        texture_glyph* g = *(texture_glyph**)vector_get(self->glyphs, i);
        g->s0 *= mulw;
        g->s1 *= mulw;
        g->t0 *= mulh;
        g->t1 *= mulh;
    }
}
