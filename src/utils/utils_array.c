/******************************************************************************
 * isula: array utils
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
#include "utils_array.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "utils_memory.h"

/* lcr free array */
void isula_free_array(void **orig_array)
{
    void **p = NULL;
    for (p = orig_array; p && *p; p++) {
        free(*p);
        *p = NULL;
    }
    free((void *)orig_array);
}

/* lcr grow array */
int isula_grow_array(void ***orig_array, size_t *orig_capacity, size_t size, size_t increment)
{
    size_t add_capacity;
    void **add_array = NULL;

    if (orig_array == NULL || orig_capacity == NULL || size == SIZE_MAX) {
        return -1;
    }

    if (increment == 0) {
        return -1;
    }

    if ((*orig_array) == NULL || (*orig_capacity) == 0) {
        free(*orig_array);
        *orig_array = NULL;
        *orig_capacity = 0;
    }

    add_capacity = *orig_capacity;
    while (size + 1 > add_capacity) {
        if (add_capacity > SIZE_MAX - increment) {
            return -1;
        }
        add_capacity += increment;
    }
    if (add_capacity != *orig_capacity) {
        add_array = isula_smart_calloc_s(sizeof(void *), add_capacity);
        if (add_array == NULL) {
            return -1;
        }
        if (*orig_array) {
            (void)memcpy(add_array, *orig_array, *orig_capacity * sizeof(void *));
            free((void *)*orig_array);
        }

        *orig_array = add_array;
        *orig_capacity = add_capacity;
    }

    return 0;
}

/* lcr array len */
size_t isula_array_len(void **orig_array)
{
    void **p = NULL;
    size_t length = 0;

    for (p = orig_array; p && *p; p++) {
        length++;
    }

    return length;
}

int isula_array_append(void ***array, const void *element, clone_cb cb)
{
    size_t len;
    void **new_array = NULL;

    if (array == NULL || element == NULL || cb == NULL) {
        return -1;
    }

    // let new len to len + 2 for element and null
    len = isula_array_len(*array);

    new_array = isula_smart_calloc_s(sizeof(void *), (len + 2));
    if (new_array == NULL) {
        ERROR("Out of memory");
        return -1;
    }
    if (*array != NULL) {
        (void)memcpy(new_array, *array, len * sizeof(void *));
        free((void *)*array);
    }
    *array = new_array;

    new_array[len] = cb(element);

    return 0;
}
