/******************************************************************************
 * lcr: utils library for iSula
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

#ifndef __LCR_LIST_H
#define __LCR_LIST_H


struct lcr_list {
    void *elem;
    struct lcr_list *next;
    struct lcr_list *prev;
};

/* Iterate through an lcr list. */
#define lcr_list_for_each(__iterator, __list) \
    for ((__iterator) = (__list)->next; \
         (__iterator) != (__list); \
         (__iterator) = (__iterator)->next)

/*
 * Iterate safely through an lcr list
 */
#define lcr_list_for_each_safe(__iterator, __list, __next) \
    for ((__iterator) = (__list)->next, (__next) = (__iterator)->next; \
         (__iterator) != (__list); \
         (__iterator) = (__next), (__next) = (__next)->next)

/* Initialize list. */
static inline void lcr_list_init(struct lcr_list *list)
{
    list->elem = NULL;
    list->next = list->prev = list;
}

/* Add an element to a list. See lcr_list_add() and lcr_list_add_tail() for an
 * idiom. */
static inline void lcr_list_add_elem(struct lcr_list *list, void *elem)
{
    list->elem = elem;
}

/* Retrieve first element of list. */
static inline void *lcr_list_first_elem(struct lcr_list *list)
{
    return list->next->elem;
}

/* Retrieve last element of list. */
static inline void *lcr_list_last_elem(struct lcr_list *list)
{
    return list->prev->elem;
}

/* Determine if list is empty. */
static inline int lcr_list_empty(const struct lcr_list *list)
{
    return list == list->next;
}

/* Workhorse to be called from lcr_list_add() and lcr_list_add_tail(). */
static inline void __lcr_list_add(struct lcr_list *new, struct lcr_list *prev, struct lcr_list *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/*
 * Idiom to add an element to the beginning of an lcr list
 */
static inline void lcr_list_add(struct lcr_list *head, struct lcr_list *list)
{
    __lcr_list_add(list, head, head->next);
}

/*
 * Idiom to add an element to the end of an lcr list
 */
static inline void lcr_list_add_tail(
    struct lcr_list *head, struct lcr_list *list)
{
    __lcr_list_add(list, head->prev, head);
}

/* Idiom to merge two lcr list */
static inline void lcr_list_merge(
    struct lcr_list *list1, struct lcr_list *list2)
{
    struct lcr_list *list1_tail, *list2_tail;
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
static inline void lcr_list_del(struct lcr_list *list)
{
    struct lcr_list *next, *prev;

    next = list->next;
    prev = list->prev;
    next->prev = prev;
    prev->next = next;
}

/* Return length of the list. */
static inline size_t lcr_list_len(struct lcr_list *list)
{
    size_t i = 0;
    struct lcr_list *iter;
    lcr_list_for_each(iter, list) {
        i++;
    }

    return i;
}

#endif
