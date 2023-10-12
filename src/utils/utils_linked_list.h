/******************************************************************************
 * libisula: utils linked list
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 *
 * Authors:
 * Haozi007 <liuhao27@huawei.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ********************************************************************************/
#ifndef __UTIL_UTIL_LINKED_LIST_H
#define __UTIL_UTIL_LINKED_LIST_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct isula_linked_list {
    void *elem;
    struct isula_linked_list *next;
    struct isula_linked_list *prev;
};

/* Iterate through an list. */
#define isula_linked_list_for_each(__iterator, __list) \
    for ((__iterator) = (__list)->next; \
         (__iterator) != (__list); \
         (__iterator) = (__iterator)->next)

/*
 * Iterate safely through an linked list
 */
#define isula_linked_list_for_each_safe(__iterator, __list, __next) \
    for ((__iterator) = (__list)->next, (__next) = (__iterator)->next; \
         (__iterator) != (__list); \
         (__iterator) = (__next), (__next) = (__next)->next)

/* Initialize list. */
static inline void isula_linked_list_init(struct isula_linked_list *list)
{
    list->elem = NULL;
    list->next = list->prev = list;
}

/* Add an element to a list. See isula_linked_list_add() and isula_linked_list_add_tail() for an
 * idiom. */
static inline void isula_linked_list_add_elem(struct isula_linked_list *list, void *elem)
{
    list->elem = elem;
}

/* Retrieve first element of list. */
static inline void *isula_linked_list_first_elem(struct isula_linked_list *list)
{
    return list->next->elem;
}

/* Retrieve last element of list. */
static inline void *isula_linked_list_last_elem(struct isula_linked_list *list)
{
    return list->prev->elem;
}

/* Determine if list is empty. */
static inline bool isula_linked_list_empty(const struct isula_linked_list *list)
{
    return list == list->next;
}

/* Workhorse to be called from isula_linked_list_add() and isula_linked_list_add_tail(). */
static inline void __isula_linked_list_add(struct isula_linked_list *new_node, struct isula_linked_list *prev, struct isula_linked_list *next)
{
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}

/*
 * Idiom to add an element to the beginning of an lcr list
 */
static inline void isula_linked_list_add(struct isula_linked_list *head, struct isula_linked_list *list)
{
    __isula_linked_list_add(list, head, head->next);
}

/*
 * Idiom to add an element to the end of an lcr list
 */
static inline void isula_linked_list_add_tail(
    struct isula_linked_list *head, struct isula_linked_list *list)
{
    __isula_linked_list_add(list, head->prev, head);
}

/*
 * Merge list2 into tail of list1
 * Notes: include head of list2 will merge into list1
*/
static inline void isula_linked_list_merge(
    struct isula_linked_list *list1, struct isula_linked_list *list2)
{
    struct isula_linked_list *list1_tail, *list2_tail;
    list1_tail = list1->prev;
    list2_tail = list2->prev;
    // to merge two list, we need:
    // 1. update list1's prev (next is not needed) to list2's tail
    list1->prev = list2_tail;
    // 2. update list1's tail's next to list2
    list1_tail->next = list2;
    // 3. update list2's prev to list1's tail
    list2->prev = list1_tail;
    // 4. update list2's tail's next to list1
    list2_tail->next = list1;
}

/* Idiom to free an lcr list */
static inline void isula_linked_list_del(struct isula_linked_list *list)
{
    struct isula_linked_list *next, *prev;

    next = list->next;
    prev = list->prev;
    next->prev = prev;
    prev->next = next;
}

/* Return length of the list. */
static inline size_t isula_linked_list_len(struct isula_linked_list *list)
{
    size_t i = 0;
    struct isula_linked_list *iter;
    isula_linked_list_for_each(iter, list) {
        i++;
    }

    return i;
}

#ifdef __cplusplus
}
#endif

#endif
