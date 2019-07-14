#include "slot_map.h"
#include <string.h>
#include <assert.h>

#define POISON_POINTER ((void *)(0xDEAD000000000000UL))
#define INITIAL_CAPACITY 4
#define INVALID_IDX ~0lu

static inline void free_list_push(struct slot_map* sm, size_t idx)
{
    sm->slots[idx].free_list_next = INVALID_IDX;
    if (sm->free_list_head == INVALID_IDX) {
        sm->free_list_head = sm->free_list_tail = idx;
    } else {
        sm->slots[sm->free_list_tail].free_list_next = idx;
        sm->slots[idx].free_list_prev = sm->free_list_tail;
        sm->free_list_tail = idx;
    }
}

static inline void free_list_remove(struct slot_map* sm, size_t idx)
{
    struct sm_slot* s = &sm->slots[idx];
    if (s->free_list_prev != INVALID_IDX)
        sm->slots[s->free_list_prev].free_list_next = s->free_list_next;
    else
        sm->free_list_head = s->free_list_next;
    if (s->free_list_next != INVALID_IDX)
        sm->slots[s->free_list_next].free_list_prev = s->free_list_prev;
    else
        sm->free_list_tail = s->free_list_prev;
    s->free_list_next = s->free_list_prev = INVALID_IDX;
}

static inline void slots_resize(struct slot_map* sm, size_t ncap)
{
    assert(ncap > sm->cap_slots);
    size_t prev_cap = sm->cap_slots;
    sm->cap_slots   = ncap;
    sm->slots       = realloc(sm->slots, sm->cap_slots * sizeof(struct sm_slot));
    for (size_t i = prev_cap; i < sm->cap_slots; ++i) {
        struct sm_slot* s = &sm->slots[i];
        s->generation     = 0;
        s->data_idx       = INVALID_IDX;
        s->free_list_next = INVALID_IDX;
        s->free_list_prev = INVALID_IDX;
        free_list_push(sm, i);
    }
}

static inline void data_resize(struct slot_map* sm, size_t ncap)
{
    size_t prev_cap = sm->capacity;
    sm->capacity = ncap;
    sm->data = realloc(sm->data, sm->capacity * sm->esz);
    sm->data_to_slot = realloc(sm->data_to_slot, sm->capacity * sizeof(size_t));
    for (size_t i = prev_cap; i < sm->capacity; ++i)
        sm->data_to_slot[i] = INVALID_IDX;
}

void slot_map_init(struct slot_map* sm, size_t esz)
{
    sm->esz            = esz;
    sm->num_slots      = 0;
    sm->size           = 0;
    sm->cap_slots      = 0;
    sm->capacity       = 0;
    sm->slots          = 0;
    sm->data_to_slot   = 0;
    sm->data           = 0;
    sm->free_list_head = INVALID_IDX;
    sm->free_list_tail = INVALID_IDX;
    slots_resize(sm, INITIAL_CAPACITY);
    data_resize(sm, INITIAL_CAPACITY);
}

void slot_map_destroy(struct slot_map* sm)
{
    free(sm->data_to_slot);
    free(sm->slots);
    free(sm->data);
    sm->data_to_slot = POISON_POINTER;
    sm->slots = POISON_POINTER;
    sm->data  = POISON_POINTER;
}

int slot_map_keys_equal(sm_key k1, sm_key k2)
{
    return k1.index      == k2.index
        && k1.generation == k2.generation;
}

int slot_map_key_valid(sm_key k)
{
    return !slot_map_keys_equal(k, SM_INVALID_KEY);
}

static inline size_t data_append(struct slot_map* sm, void* data)
{
    if (sm->capacity - sm->size < 1)
        data_resize(sm, sm->capacity * 2);
    if (data)
        memcpy(sm->data + sm->size * sm->esz, data, sm->esz);
    return sm->size++;
}

static sm_key slot_map_next_key(struct slot_map* sm)
{
    sm_key k;
    /* Allocate new indexes */
    if (sm->cap_slots - sm->num_slots < 1)
        slots_resize(sm, sm->cap_slots * 2);
    assert(sm->free_list_head != INVALID_IDX);
    /* Next index */
    k.index = sm->free_list_head;
    k.generation = sm->slots[k.index].generation;
    free_list_remove(sm, sm->free_list_head);
    sm->num_slots++;
    return k;
}

sm_key slot_map_insert(struct slot_map* sm, void* data)
{
    sm_key k = slot_map_next_key(sm);
    size_t ndata_idx = data_append(sm, data);
    sm->slots[k.index].data_idx = ndata_idx;
    sm->data_to_slot[ndata_idx] = k.index;
    return k;
}

void* slot_map_foreign_add(struct slot_map* sm, sm_key k, void* data)
{
    assert(slot_map_key_valid(k));
    if (!(k.index < sm->cap_slots && sm->slots[k.index].data_idx != INVALID_IDX)) {
        if (k.index >= sm->cap_slots)
            slots_resize(sm, k.index + 1);
        size_t ndata_idx = data_append(sm, data);
        free_list_remove(sm, k.index);
        sm->slots[k.index].generation = k.generation;
        sm->slots[k.index].data_idx = ndata_idx;
        sm->data_to_slot[ndata_idx] = k.index;
        sm->num_slots++;
        return slot_map_data(sm, ndata_idx);
    }
    return 0;
}

void* slot_map_lookup(struct slot_map* sm, sm_key k)
{
    struct sm_slot* s = &sm->slots[k.index];
    if (k.generation == s->generation) {
        void* data = sm->data + s->data_idx * sm->esz;
        return data;
    }
    return 0;
}

void* slot_map_data(struct slot_map* sm, size_t idx)
{
    assert(idx < sm->size);
    return sm->data + idx * sm->esz;
}

sm_key slot_map_data_to_key(struct slot_map* sm, size_t idx)
{
    assert(idx < sm->size);
    sm_key k;
    k.index = sm->data_to_slot[idx];
    k.generation = sm->slots[k.index].generation;
    return k;
}

static inline void swap_with_last(struct slot_map* sm, size_t data_idx)
{
    memcpy(sm->data + data_idx * sm->esz, sm->data + (sm->size - 1) * sm->esz, sm->esz);
    sm->data_to_slot[data_idx] = sm->data_to_slot[sm->size - 1];
    sm->slots[sm->data_to_slot[sm->size - 1]].data_idx = data_idx;
}

int slot_map_remove(struct slot_map* sm, sm_key k)
{
    struct sm_slot* s = &sm->slots[k.index];
    if (k.generation == s->generation) {
        ++(s->generation);
        free_list_push(sm, k.index);
        if (s->data_idx != sm->size - 1)
            swap_with_last(sm, s->data_idx);
        s->data_idx = INVALID_IDX;
        --sm->size;
        --sm->num_slots;
        return 1;
    }
    return 0;
}
