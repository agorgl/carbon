#include "list.h"

void list_inithead(struct list_head* item)
{
    item->prev = item;
    item->next = item;
}

void list_add(struct list_head* item, struct list_head* list)
{
    item->prev = list;
    item->next = list->next;
    list->next->prev = item;
    list->next = item;
}

void list_addtail(struct list_head* item, struct list_head* list)
{
    item->next = list;
    item->prev = list->prev;
    list->prev->next = item;
    list->prev = item;
}

void list_replace(struct list_head* from, struct list_head* to)
{
    if (list_is_empty(from)) {
        list_inithead(to);
    } else {
        to->prev = from->prev;
        to->next = from->next;
        from->next->prev = to;
        from->prev->next = to;
    }
}

void list_del(struct list_head* item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
    item->prev = item->next = NULL;
}

void list_delinit(struct list_head* item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
    item->next = item;
    item->prev = item;
}

bool list_is_empty(const struct list_head* list)
{
   return list->next == list;
}

/**
 * Returns whether the list has exactly one element.
 */
bool list_is_singular(const struct list_head* list)
{
   return list->next != NULL && list->next != list && list->next->next == list;
}

unsigned list_length(const struct list_head* list)
{
   struct list_head* node;
   unsigned length = 0;
   for (node = list->next; node != list; node = node->next)
      length++;
   return length;
}

void list_splice(struct list_head* src, struct list_head* dst)
{
   if (list_is_empty(src))
      return;

   src->next->prev = dst;
   src->prev->next = dst->next;
   dst->next->prev = src->prev;
   dst->next = src->next;
}

void list_splicetail(struct list_head* src, struct list_head* dst)
{
   if (list_is_empty(src))
      return;

   src->prev->next = dst;
   src->next->prev = dst->prev;
   dst->prev->next = src->next;
   dst->prev = src->prev;
}

void list_validate(const struct list_head* list)
{
   struct list_head* node;
   assert(list->next->prev == list && list->prev->next == list);
   for (node = list->next; node != list; node = node->next)
      assert(node->next->prev == node && node->prev->next == node);
}
