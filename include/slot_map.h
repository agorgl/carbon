/*********************************************************************************************************************/
/*                                                  /===-_---~~~~~~~~~------____                                     */
/*                                                 |===-~___                _,-'                                     */
/*                  -==\\                         `//~\\   ~~~~`---.___.-~~                                          */
/*              ______-==|                         | |  \\           _-~`                                            */
/*        __--~~~  ,-/-==\\                        | |   `\        ,'                                                */
/*     _-~       /'    |  \\                      / /      \      /                                                  */
/*   .'        /       |   \\                   /' /        \   /'                                                   */
/*  /  ____  /         |    \`\.__/-~~ ~ \ _ _/'  /          \/'                                                     */
/* /-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`                                                      */
/*                   \_|      /        _)   ;  ),   __--~~                                                           */
/*                     '~~--_/      _-~/-  / \   '-~ \                                                               */
/*                    {\__--_/}    / \\_>- )<__\      \                                                              */
/*                    /'   (_/  _-~  | |__>--<__|      |                                                             */
/*                   |0  0 _/) )-~     | |__>--<__|     |                                                            */
/*                   / /~ ,_/       / /__>---<__/      |                                                             */
/*                  o o _//        /-~_>---<__-~      /                                                              */
/*                  (^(~          /~_>---<__-      _-~                                                               */
/*                 ,/|           /__>--<__/     _-~                                                                  */
/*              ,//('(          |__>--<__|     /                  .----_                                             */
/*             ( ( '))          |__>--<__|    |                 /' _---_~\                                           */
/*          `-)) )) (           |__>--<__|    |               /'  /     ~\`\                                         */
/*         ,/,'//( (             \__>--<__\    \            /'  //        ||                                         */
/*       ,( ( ((, ))              ~-__>--<_~-_  ~--____---~' _/'/        /'                                          */
/*     `~/  )` ) ,/|                 ~-_~>--<_/-__       __-~ _/                                                     */
/*   ._-~//( )/ )) `                    ~~-'_/_/ /~~~~~~~__--~                                                       */
/*    ;'( ')/ ,)(                              ~~~~~~~~~~                                                            */
/*   ' ') '( (/                                                                                                      */
/*     '   '  `                                                                                                      */
/*********************************************************************************************************************/
#ifndef _SLOT_MAP_H_
#define _SLOT_MAP_H_

/*
 * Slot Map data structure
 *
 * An unordered, associative container with:
 *  - O(1) lookup, erase and insert
 *  - Dense contiguous element storage
 *  - Insert calls return a unique key
 *  - Keys to erased values won't work [1], even if the slot is reused [2]
 *  - Consistent performance
 *
 * Problems it solves:
 *  - Items that need unique identifiers
 *  - Avoids ABA-like problems
 *
 * Alternatives:
 *  * Hashmap
 *    - Lookup cost is inconsistent due to hashing and probing
 *    - Hashing has high overhead
 *    - User must make keys
 *    - Prone to ABA-like problems
 *    - Non contiguous
 *  * Array/Vector
 *    - Can be used with indices as "keys"
 *    - Removing an element invalidates other "keys"
 *    - Prone to ABA-like problems
 *
 * How it works:
 *     When inserting a new value entry in the slot_map a key is generated
 *  and returned to the user that can be used to lookup or remove that entry.
 *  The key consists of two parts, an index and a generation value. The first
 *  is used as an index, to reference a slot into the slot array. The second
 *  is compared to the generation value of the referenced slot to see if the
 *  key is still valid for the data that the slot points to (has not been
 *  removed or replaced). If the generation values match, the slot holds a
 *  data index that is then used to access the entry in the dense data array.
 *
 * Notes:
 *  [1]: Until integer overflow occurs in generation field
 *  [2]: Each key is unique for the data that was associated with initially
 */

#include <stdlib.h>
#include <stdint.h>

/*
 * A unique identifier for each data entry in the slot_map
 * Created by the slot_map when inserting a data entry.
 * Decomposable pair of integer-like types:
 *  - Index into the slot table
 *  - Generation counter (version) for the slot
 * Gets data for slot in 'index' position of the slot table,
 * if key and slot 'generation' values match
 */

typedef uint64_t sm_index_t;
typedef uint32_t sm_generation_t;

#define SLOT_MAP_TOTAL_BITS (sizeof(sm_index_t) * 8)
#define SLOT_MAP_INDEX_BITS (SLOT_MAP_TOTAL_BITS * 3 / 4)
#define SLOT_MAP_GENERATION_BITS (SLOT_MAP_TOTAL_BITS - SLOT_MAP_INDEX_BITS)
#define SM_INVALID_KEY ((sm_key){.index = ~0l, .generation = ~0})

/* Check for sane bit sizes */
#define SLOT_MAP_INDEX_BITS_MIN (SLOT_MAP_TOTAL_BITS - 8)
#define STATIC_ASSERT(COND, MSG) typedef char static_assertion_##MSG[(COND)?1:-1]
STATIC_ASSERT(SLOT_MAP_INDEX_BITS < SLOT_MAP_INDEX_BITS_MIN, Insane);

typedef struct {
    sm_index_t index           : SLOT_MAP_INDEX_BITS;
    sm_generation_t generation : SLOT_MAP_GENERATION_BITS;
} sm_key;

struct slot_map {
    /*
     * Array of slots (sparse)
     */
    struct sm_slot {
        /*
         * Generation value is the "version" of the current slot
         * so that a key with index to this slot created for old data
         * can be invalidated
         */
        sm_generation_t generation : SLOT_MAP_GENERATION_BITS;
        /*
         * Index points to an entry in the data table
         * This indirection allows data array to be dense,
         * while slots are sparsely distributed
         */
        size_t data_idx;
        /*
         * In-place free list to achieve O(1) insert
         * We reuse slots by getting keys of the head of the free list,
         * and if the free list is empty we allocate a new key
         */
        size_t free_list_next, free_list_prev;
    }* slots;

    /* Number/Capacity of slots */
    size_t num_slots, cap_slots;

    /* Free list head, tail */
    size_t free_list_head, free_list_tail;

    /* Size of each entry */
    size_t esz;

    /* Number/Capacity of entries */
    size_t size, capacity;

    /* Array of indexes to do reverse mapping */
    size_t* data_to_slot;

    /* Array of data (dense) */
    void* data;
};

/*
 * slot_map_init - initialize the slot_map
 * @sm: the slot map to initialize
 * @esz: each element's size
 */
void slot_map_init(struct slot_map* sm, size_t esz);

/*
 * slot_map_destroy - free the slot_map
 * @sm: the slot map to free
 */
void slot_map_destroy(struct slot_map* sm);

/*
 * slot_map_keys_equal - checks that pair of keys are the same
 * @k1: the first key to check
 * @k2: the second key to check
 */
int slot_map_keys_equal(sm_key k1, sm_key k2);

/*
 * slot_map_key_valid - checks that key is not the invalid key constant
 * @k: the key to check
 */
int slot_map_key_valid(sm_key k);

/*
 * slot_map_insert - insert an element in the slot_map
 * @sm: the slot map
 * @data: the element's data to insert (or just allocate space if null)
 */
sm_key slot_map_insert(struct slot_map* sm, void* data);

/*
 * slot_map_foreign_add - insert an element in the slot_map with the given key
 * @sm: the slot map
 * @k: the key to try and insert
 * @data: the element's data to insert (or just allocate space if null)
 */
void* slot_map_foreign_add(struct slot_map* sm, sm_key k, void* data);

/*
 * slot_map_lookup - lookup the element in the slot_map
 * @sm: the slot map
 * @k: the key that references the element we want to lookup
 */
void* slot_map_lookup(struct slot_map* sm, sm_key k);

/*
 * slot_map_data - fetch the data in index
 * @sm: the slot map
 * @idx: the index to use for the data array
 */
void* slot_map_data(struct slot_map* sm, size_t idx);

/*
 * slot_map_data_to_key - retrieve key corresponding to given data index
 * @sm: the slot map
 * @idx: the index to use for the data array
 */
sm_key slot_map_data_to_key(struct slot_map* sm, size_t idx);

/*
 * slot_map_remove - remove the element in the slot_map
 * @sm: the slot map
 * @k: the key that references the element we want to remove
 */
int slot_map_remove(struct slot_map* sm, sm_key k);

#endif /* ! _SLOT_MAP_H_ */
