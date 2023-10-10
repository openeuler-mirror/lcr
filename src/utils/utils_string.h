/******************************************************************************
 * isula: string utils
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
#ifndef _ISULA_UTILS_UTILS_STRING_H
#define _ISULA_UTILS_UTILS_STRING_H

#include <sys/types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Return copy of spe + parts[0] + parts[...];
 * if spe == null or parts == null or len == 0, will return NULL;
 * if error, return NULL;
 */
char *isula_string_join(const char *sep, const char **parts, size_t len);

/*
 * Return copy of pre + add_str
 * if pre is null, just return copy of add_str;
 * if add_str is null, just return copy of pre;
 * if error, return NULL;
 */
char *isula_string_append(const char *pre, const char *add_str);

/*
 * Replace 'needle' in string haystack with 'replacement';
*/
char *isula_string_replace(const char *needle, const char *replacement, const char *haystack);

struct __isula_string_array;

/*
 * Append new item into end of string array;
 * if success, return 0;
 * if failed, return -1;
 */
typedef int (*isula_arr_append_op)(struct __isula_string_array *, const char *);

/*
 * Append all items in second array into first;
 * if first == NULL, return -1;
 * if second == NULL, do nothing and return 0;
 * if success, return 0;
 * if failed, return -1;
 */
typedef int (*isula_arr_append_arr_op)(struct __isula_string_array *first, struct __isula_string_array *second);

/*
 * Check if array contains element item;
 * if success, return true;
 * if failed, return false;
 */
typedef bool (*isula_arr_contain_op)(const struct __isula_string_array *ptr, const char *item);

struct __isula_string_array {
    char **items;
    size_t len;
    size_t cap;

    isula_arr_append_op append;
    isula_arr_append_arr_op append_arr;
    isula_arr_contain_op contain;
};
typedef struct __isula_string_array isula_string_array;

/*
 * Create and init isula string array;
 * if req_init_cap == 0, will return struct with capablity 2;
 * if success, return struct;
 * if failed, return NULL;
 */
isula_string_array *isula_string_array_new(size_t req_init_cap);

/*
 * Free isula string array;
 * if ptr maybe used after call this function, you should do 'ptr == NULL;';
 */
void isula_string_array_free(isula_string_array *ptr);

isula_string_array *isula_string_split_to_multi(const char *src_str, char delim);

#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_UTILS_STRING_H */