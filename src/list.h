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
#ifndef _LIST_H_
#define _LIST_H_

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef DEBUG
#define list_assert(cond, msg) assert(cond && msg)
#else
#define list_assert(cond, msg) (void)(0 && (cond))
#endif

struct list_head {
    struct list_head* prev;
    struct list_head* next;
};

void list_inithead(struct list_head* item);
void list_add(struct list_head* item, struct list_head* list);
void list_addtail(struct list_head* item, struct list_head* list);
void list_replace(struct list_head* from, struct list_head* to);
void list_del(struct list_head* item);
void list_delinit(struct list_head* item);
bool list_is_empty(const struct list_head* list);
bool list_is_singular(const struct list_head* list);
void list_splice(struct list_head* src, struct list_head* dst);
void list_splicetail(struct list_head* src, struct list_head* dst);
void list_validate(const struct list_head* list);
unsigned list_length(const struct list_head* list);

#define LIST_ENTRY(__type, __item, __field) \
    ((__type*)(((char*)(__item)) - offsetof(__type, __field)))

/**
 * Cast from a pointer to a member of a struct back to the containing struct.
 * 'sample' MUST be initialized, or else the result is undefined!
 */
#ifndef container_of
#define container_of(ptr, sample, member) \
    (void*)((char*)(ptr) \
         - ((char*)&(sample)->member - (char*)(sample)))
#endif

#define list_first_entry(ptr, type, member) \
        LIST_ENTRY(type, (ptr)->next, member)

#define list_last_entry(ptr, type, member) \
        LIST_ENTRY(type, (ptr)->prev, member)

#define LIST_FOR_EACH_ENTRY(pos, head, member)                       \
   for (pos = NULL, pos = container_of((head)->next, pos, member);   \
    &pos->member != (head);                                          \
    pos = container_of(pos->member.next, pos, member))

#define LIST_FOR_EACH_ENTRY_SAFE(pos, storage, head, member)         \
   for (pos = NULL, pos = container_of((head)->next, pos, member),   \
    storage = container_of(pos->member.next, pos, member);           \
    &pos->member != (head);                                          \
    pos = storage, storage = container_of(storage->member.next, storage, member))

#define LIST_FOR_EACH_ENTRY_SAFE_REV(pos, storage, head, member)     \
   for (pos = NULL, pos = container_of((head)->prev, pos, member),   \
    storage = container_of(pos->member.prev, pos, member);           \
    &pos->member != (head);                                          \
    pos = storage, storage = container_of(storage->member.prev, storage, member))

#define LIST_FOR_EACH_ENTRY_FROM(pos, start, head, member)           \
   for (pos = NULL, pos = container_of((start), pos, member);        \
    &pos->member != (head);                                          \
    pos = container_of(pos->member.next, pos, member))

#define LIST_FOR_EACH_ENTRY_FROM_REV(pos, start, head, member)       \
   for (pos = NULL, pos = container_of((start), pos, member);        \
    &pos->member != (head);                                          \
    pos = container_of(pos->member.prev, pos, member))

#define list_for_each_entry(type, pos, head, member)                 \
   for (type* pos = LIST_ENTRY(type, (head)->next, member),          \
         *__next = LIST_ENTRY(type, pos->member.next, member);       \
    &pos->member != (head);                                          \
    pos = LIST_ENTRY(type, pos->member.next, member),                \
    list_assert(pos == __next, "use _safe iterator"),                \
    __next = LIST_ENTRY(type, __next->member.next, member))

#define list_for_each_entry_safe(type, pos, head, member)            \
   for (type* pos = LIST_ENTRY(type, (head)->next, member),          \
         *__next = LIST_ENTRY(type, pos->member.next, member);       \
    &pos->member != (head);                                          \
    pos = __next,                                                    \
    __next = LIST_ENTRY(type, __next->member.next, member))

#define list_for_each_entry_rev(type, pos, head, member)             \
   for (type* pos = LIST_ENTRY(type, (head)->prev, member),          \
         *__prev = LIST_ENTRY(type, pos->member.prev, member);       \
    &pos->member != (head);                                          \
    pos = LIST_ENTRY(type, pos->member.prev, member),                \
    list_assert(pos == __prev, "use _safe iterator"),                \
    __prev = LIST_ENTRY(type, __prev->member.prev, member))

#define list_for_each_entry_safe_rev(type, pos, head, member)        \
   for (type* pos = LIST_ENTRY(type, (head)->prev, member),          \
         *__prev = LIST_ENTRY(type, pos->member.prev, member);       \
    &pos->member != (head);                                          \
    pos = __prev,                                                    \
        __prev = LIST_ENTRY(type, __prev->member.prev, member))

#define list_for_each_entry_from(type, pos, start, head, member)     \
   for (type* pos = LIST_ENTRY(type, (start), member);               \
    &pos->member != (head);                                          \
    pos = LIST_ENTRY(type, pos->member.next, member))

#define list_for_each_entry_from_rev(type, pos, start, head, member) \
   for (type* pos = LIST_ENTRY(type, (start), member);               \
    &pos->member != (head);                                          \
    pos = LIST_ENTRY(type, pos->member.prev, member))

#endif /* ! _LIST_H_ */
