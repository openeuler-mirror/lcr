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
#include "utils_string.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "utils_memory.h"
#include "utils_array.h"
#include "auto_cleanup.h"

static char *do_string_join(const char *sep, const char **parts, size_t parts_len, size_t result_len)
{
    size_t iter;

    char *res_string = calloc(result_len + 1, 1);
    if (res_string == NULL) {
        return NULL;
    }
    for (iter = 0; iter < parts_len - 1; iter++) {
        (void)strcat(res_string, parts[iter]);
        (void)strcat(res_string, sep);
    }
    (void)strcat(res_string, parts[parts_len - 1]);
    return res_string;
}

char *isula_string_join(const char *sep, const char **parts, size_t len)
{
    size_t sep_len;
    size_t result_len;
    size_t iter;

    if (len == 0 || parts == NULL || sep == NULL) {
        return NULL;
    }

    sep_len = strlen(sep);

    if ((sep_len != 0) && (sep_len != 1) && (len > SIZE_MAX / sep_len + 1)) {
        return NULL;
    }
    result_len = (len - 1) * sep_len;
    for (iter = 0; iter < len; iter++) {
        if (parts[iter] == NULL || result_len >= SIZE_MAX - strlen(parts[iter])) {
            return NULL;
        }
        result_len += strlen(parts[iter]);
    }

    return do_string_join(sep, parts, len, result_len);
}

/* util string replace one */
static ssize_t do_isula_string_replace_one(const char *needle, const char *replace, const char *haystack, char **result)
{
    char *res_string = *result;
    char *p = NULL;
    char *next_p = NULL;
    ssize_t length = 0;
    size_t replace_len, nl_len, part_len;

    replace_len = strlen(replace);
    nl_len = strlen(needle);

    for (next_p = (char *)haystack, p = strstr(next_p, needle); p != NULL; next_p = p, p = strstr(next_p, needle)) {
        part_len = (size_t)(p - next_p);
        if ((res_string != NULL) && part_len > 0) {
            (void)memcpy(&res_string[length], next_p, part_len);
        }
        length += (ssize_t)part_len;
        if ((res_string != NULL) && replace_len > 0) {
            (void)memcpy(&res_string[length], replace, replace_len);
        }
        length += (ssize_t)replace_len;
        p += nl_len;
    }
    part_len = strlen(next_p);
    if ((res_string != NULL) && part_len > 0) {
        (void)memcpy(&res_string[length], next_p, part_len);
    }
    length += (ssize_t)part_len;
    return length;
}

/* util string replace */
char *isula_string_replace(const char *needle, const char *replace, const char *haystack)
{
    ssize_t length = -1;
    ssize_t reserve_len = -1;
    char *res_string = NULL;

    if ((needle == NULL) || (replace == NULL) || (haystack == NULL)) {
        ERROR("Invalid NULL pointer");
        return NULL;
    }

    while (length == -1 || res_string == NULL) {
        if (length != -1) {
            res_string = calloc(1, (size_t)length + 1);
            if (res_string == NULL) {
                return NULL;
            }
            reserve_len = length;
        }
        length = do_isula_string_replace_one(needle, replace, haystack, &res_string);
        if (length < 0) {
            free(res_string);
            return NULL;
        }
    }

    if (reserve_len != length) {
        free(res_string);
        return NULL;
    }
    if (res_string[length] != '\0') {
        free(res_string);
        return NULL;
    }

    return res_string;
}

/* util string append */
char *isula_string_append(const char *pre, const char *add_str)
{
    char *res_string = NULL;
    size_t length = 0;

    if (add_str == NULL && pre == NULL) {
        return NULL;
    }
    if (pre == NULL) {
        return isula_strdup_s(add_str);
    }
    if (add_str == NULL) {
        return isula_strdup_s(pre);
    }
    if (strlen(add_str) > ((SIZE_MAX - strlen(pre)) - 1)) {
        ERROR("String is too long to be appended");
        return NULL;
    }
    length = strlen(add_str) + strlen(pre) + 1;
    res_string = isula_common_calloc_s(length);
    if (res_string == NULL) {
        return NULL;
    }
    (void)strcat(res_string, pre);
    (void)strcat(res_string, add_str);

    return res_string;
}

isula_string_array *isula_string_split_to_multi(const char *src_str, char delim)
{
    char *token = NULL;
    char *cur = NULL;
    char deli[2] = { delim, '\0' };
    char *tmpstr = NULL;
    // for empty result, we should return ["", NULL]
    isula_string_array *result = isula_string_array_new(2);

    if (src_str == NULL) {
        return NULL;
    }

    if (src_str[0] == '\0') {
        result->append(result, "");
        return result;
    }

    tmpstr = isula_strdup_s(src_str);
    cur = tmpstr;
    token = strsep(&cur, deli);
    while (token != NULL) {
        if (result->append(result, token) != 0) {
            isula_string_array_free(result);
            return NULL;
        }
        token = strsep(&cur, deli);
    }

    free(tmpstr);
    return result;
}

static size_t get_string_array_scale_size(size_t old_size)
{
#define DOUBLE_THRESHOLD 1024
    const size_t max_threshold = ISULA_MAX_MEMORY_SIZE / sizeof(char *);
    if (old_size == 0) {
        return 1;
    }

    if (old_size < DOUBLE_THRESHOLD) {
        return old_size << 1;
    }

    // new_size = old_size + old_size / 4
    if (old_size > max_threshold - (old_size >> 2)) {
        return max_threshold;
    }

    return old_size + (old_size >> 2);
}

static bool do_expand_array(isula_string_array *array, size_t new_cap)
{
    char **new_items = NULL;

    // array capability sure less than ISULA_MAX_MEMORY_SIZE
    // so we need to check Overflow:
    if (new_cap <= array->cap) {
        ERROR("Too large string array, overflow memory");
        return false;
    }

    // new_size * sizeof(*new_items) and list->len * sizeof(*list->items)
    if (isula_mem_realloc((void **)&new_items, new_cap * sizeof(char *), (void *)&(array->items),
                         array->len * sizeof(char *)) != 0) {
        ERROR("Out of memory");
        return false;
    }
    array->items = new_items;
    array->cap = new_cap;

    return true;
}

static int isula_string_array_append(struct __isula_string_array *ptr, const char *elem)
{
    if (ptr == NULL) {
        ERROR("invalid string array");
        return -1;
    }

    if (elem == NULL) {
        DEBUG("empty new item, just ignore it");
        return 0;
    }

    if (ptr->len >= ptr->cap) {
        // expand string array
        size_t new_cap = get_string_array_scale_size(ptr->cap);
        if (!do_expand_array(ptr, new_cap)) {
            return -1;
        }
    }

    ptr->items[ptr->len] = isula_strdup_s(elem);
    ptr->len += 1;
    return 0;
}

static int isula_string_array_append_array(struct __isula_string_array *dst, struct __isula_string_array *src)
{
    size_t i;
    if (dst == NULL) {
        ERROR("invalid string array");
        return -1;
    }

    if (src == NULL) {
        return 0;
    }

    if (dst->len + src->len >= dst->cap) {
        const size_t max_threshold = ISULA_MAX_MEMORY_SIZE / sizeof(char *);
        size_t new_cap = dst->cap + src->cap;
        if (new_cap > max_threshold) {
            return -1;
        }
        // expand string array
        if (!do_expand_array(dst, new_cap)) {
            return -1;
        }
    }

    for (i = 0; i < src->len; i++) {
        dst->items[dst->len] = isula_strdup_s(src->items[i]);
        dst->len += 1;
    }
    return 0;
}

static bool isula_string_array_contain(const struct __isula_string_array *ptr, const char *item)
{
    size_t i;

    if (ptr == NULL || item == NULL) {
        return false;
    }

    for (i = 0; i < ptr->len; i++) {
        // if container NULL item, just skip it
        if (ptr->items[i] == NULL) {
            continue;
        }
        if (strcmp(ptr->items[i], item) == 0) {
            return true;
        }
    }

    return false;
}

void isula_string_array_free(isula_string_array *ptr)
{
    size_t i;

    if (ptr == NULL) {
        return;
    }

    for (i = 0; i < ptr->len; i++) {
        free(ptr->items[i]);
        ptr->items[i] = NULL;
    }
    free(ptr->items);
    ptr->items = NULL;
    ptr->len = 0;
    ptr->cap = 0;

    free(ptr);
}

isula_string_array *isula_string_array_new(size_t req_init_cap)
{
    isula_string_array *ptr = NULL;
    // default length of string array is 2
    size_t init_cap = 2;

    if (req_init_cap != 0) {
        init_cap = req_init_cap;
    }

    ptr = (isula_string_array *)isula_common_calloc_s(sizeof(isula_string_array));
    if (ptr == NULL) {
        ERROR("Out of memory");
        return NULL;
    }

    ptr->items = (char **)isula_smart_calloc_s(sizeof(char *), init_cap);
    if (ptr->items == NULL) {
        ERROR("Out of memory");
        free(ptr);
        return NULL;
    }

    ptr->len = 0;
    ptr->cap = init_cap;
    ptr->contain = isula_string_array_contain;
    ptr->append = isula_string_array_append;
    ptr->append_arr = isula_string_array_append_array;

    return ptr;
}