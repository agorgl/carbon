#include "texture_atlas.h"
#include <string.h>
#include <assert.h>
#include <limits.h>

/*-----------------------------------------------------------------
 * Generic vector structure
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
                (char*)(self->items) + (index + 0) * self->item_size, (self->size - index) * self->item_size);
    }
    self->size++;
    vector_set(self, index, item);
}

static void vector_erase_range(struct vector* self, const size_t first, const size_t last)
{
    assert(self);
    assert(first < self->size);
    assert(last < self->size + 1);
    assert(first < last);

    memmove((char*)(self->items) + first * self->item_size,
            (char*)(self->items) + last * self->item_size,
            (self->size - last) * self->item_size);
    self->size -= (last - first);
}

static void vector_erase(struct vector* self, const size_t index)
{
    assert(self);
    assert(index < self->size);
    vector_erase_range(self, index, index + 1);
}

static void vector_push_back(struct vector* self, const void* item)
{
    vector_insert(self, self->size, item);
}

static const void* vector_get(const struct vector* self, size_t index)
{
    assert(self);
    assert(self->size);
    assert(index < self->size);
    return (char*)(self->items) + index * self->item_size;
}

/*-----------------------------------------------------------------
 * Texture Atlas
 *-----------------------------------------------------------------*/
texture_atlas* texture_atlas_new(const size_t width, const size_t height, const size_t depth)
{
    texture_atlas* self = (texture_atlas*)malloc(sizeof(texture_atlas));

    /* We want a one pixel border around the whole atlas
     * to avoid any artifact when sampling texture */
    ivec3 node = {{1, 1, width - 2}};

    assert((depth == 1) || (depth == 3) || (depth == 4));
    if (self == NULL) {
        /* No more memory for allocating data */
        return 0;
    }
    self->nodes = vector_new(sizeof(ivec3));
    self->used = 0;
    self->width = width;
    self->height = height;
    self->depth = depth;

    vector_push_back(self->nodes, &node);
    self->data = (unsigned char*)calloc(width * height * depth, sizeof(unsigned char));

    if (self->data == NULL) {
        /* No more memory for allocating data */
        return 0;
    }

    return self;
}

void texture_atlas_delete(texture_atlas* self)
{
    assert(self);
    vector_delete(self->nodes);
    if (self->data)
        free(self->data);
    free(self);
}

void texture_atlas_set_region(texture_atlas* self, const size_t x, const size_t y, const size_t width,
                              const size_t height, const unsigned char* data, const size_t stride)
{
    size_t i;
    size_t depth;
    size_t charsize;

    assert(self);
    assert(x > 0);
    assert(y > 0);
    assert(x < (self->width - 1));
    assert((x + width) <= (self->width - 1));
    assert(y < (self->height - 1));
    assert((y + height) <= (self->height - 1));

    /* Prevent copying data from undefined position
     * and prevent memcpy's undefined behavior when count is zero */
    assert(height == 0 || (data != NULL && width > 0));

    depth = self->depth;
    charsize = sizeof(char);
    for (i = 0; i < height; ++i) {
        memcpy(self->data + ((y + i) * self->width + x) * charsize * depth,
               data + (i * stride) * charsize,
               width * charsize * depth);
    }
}

int texture_atlas_fit(texture_atlas* self, const size_t index, const size_t width, const size_t height)
{
    ivec3* node;
    int x, y, width_left;
    size_t i;
    assert(self);

    node = (ivec3*)(vector_get(self->nodes, index));
    x = node->x;
    y = node->y;
    width_left = width;
    i = index;

    if ((x + width) > (self->width - 1))
        return -1;

    y = node->y;
    while (width_left > 0) {
        node = (ivec3*)(vector_get(self->nodes, i));
        if (node->y > y)
            y = node->y;
        if ((y + height) > (self->height - 1))
            return -1;
        width_left -= node->z;
        ++i;
    }
    return y;
}

void texture_atlas_merge(texture_atlas* self)
{
    ivec3 *node, *next;
    size_t i;
    assert(self);

    for (i = 0; i < self->nodes->size - 1; ++i) {
        node = (ivec3*)(vector_get(self->nodes, i));
        next = (ivec3*)(vector_get(self->nodes, i + 1));
        if (node->y == next->y) {
            node->z += next->z;
            vector_erase(self->nodes, i + 1);
            --i;
        }
    }
}

ivec4 texture_atlas_get_region(texture_atlas* self, const size_t width, const size_t height)
{
    int y, best_index;
    size_t best_height, best_width;
    ivec3 *node, *prev;
    ivec4 region = {{0, 0, width, height}};
    size_t i;
    assert(self);

    best_height = UINT_MAX;
    best_index = -1;
    best_width = UINT_MAX;
    for (i = 0; i < self->nodes->size; ++i) {
        y = texture_atlas_fit(self, i, width, height);
        if (y >= 0) {
            node = (ivec3*)vector_get(self->nodes, i);
            if (((y + height) < best_height) ||
                (((y + height) == best_height) && (node->z > 0 && (size_t)node->z < best_width))) {
                best_height = y + height;
                best_index = i;
                best_width = node->z;
                region.x = node->x;
                region.y = y;
            }
        }
    }

    if (best_index == -1) {
        region.x = -1;
        region.y = -1;
        region.width = 0;
        region.height = 0;
        return region;
    }

    node = (ivec3*)malloc(sizeof(ivec3));
    node->x = region.x;
    node->y = region.y + height;
    node->z = width;
    vector_insert(self->nodes, best_index, node);
    free(node);

    for (i = best_index + 1; i < self->nodes->size; ++i) {
        node = (ivec3*)vector_get(self->nodes, i);
        prev = (ivec3*)vector_get(self->nodes, i - 1);

        if (node->x < (prev->x + prev->z)) {
            int shrink = prev->x + prev->z - node->x;
            node->x += shrink;
            node->z -= shrink;
            if (node->z <= 0) {
                vector_erase(self->nodes, i);
                --i;
            } else {
                break;
            }
        } else {
            break;
        }
    }

    texture_atlas_merge(self);
    self->used += width * height;
    return region;
}

void texture_atlas_clear(texture_atlas* self)
{
    ivec3 node = {{1, 1, 1}};
    assert(self);
    assert(self->data);

    vector_clear(self->nodes);
    self->used = 0;

    /* We want a one pixel border around the whole atlas
       to avoid any artifact when sampling texture */
    node.z = self->width - 2;
    vector_push_back(self->nodes, &node);
    memset(self->data, 0, self->width * self->height * self->depth);
}
