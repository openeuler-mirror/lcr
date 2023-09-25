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

char *lcr_util_string_join(const char *sep, const char **parts, size_t len)
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

/* lcr shrink array */
static char **lcr_shrink_array(char **orig_array, size_t new_size)
{
    char **res_array = NULL;
    size_t i;

    if (new_size == 0) {
        return orig_array;
    }

    res_array = (char **)isula_smart_calloc_s(sizeof(char *), new_size);
    if (res_array == NULL) {
        return orig_array;
    }

    for (i = 0; i < new_size; i++) {
        res_array[i] = orig_array[i];
    }
    free(orig_array);
    return res_array;
}

/* lcr string split and trim */
char **lcr_string_split_and_trim(const char *orig_str, char _sep)
{
    char *token = NULL;
    __isula_auto_free char *str = NULL;
    char *reserve_ptr = NULL;
    char deli[2] = { _sep, '\0' };
    char **res_array = NULL;
    size_t capacity = 0;
    size_t count = 0;
    int r, tmp_errno;

    if (orig_str == NULL) {
        return calloc(1, sizeof(char *));
    }

    str = isula_strdup_s(orig_str);

    token = strtok_r(str, deli, &reserve_ptr);
    while (token != NULL) {
        while (token[0] == ' ' || token[0] == '\t') {
            token++;
        }
        size_t len = strlen(token);
        while (len > 0 && (token[len - 1] == ' ' || token[len - 1] == '\t')) {
            token[len - 1] = '\0';
            len--;
        }
        r = lcr_grow_array((void ***)&res_array, &capacity, count + 1, 16);
        if (r < 0) {
            goto error_out;
        }
        res_array[count] = isula_strdup_s(token);
        count++;
        token = strtok_r(NULL, deli, &reserve_ptr);
    }

    return lcr_shrink_array(res_array, count + 1);

error_out:
    tmp_errno = errno;
    lcr_free_array((void **)res_array);
    errno = tmp_errno;
    return NULL;
}

/* lcr free array */
void lcr_free_array(void **orig_array)
{
    void **p = NULL;
    for (p = orig_array; p && *p; p++) {
        free(*p);
        *p = NULL;
    }
    free((void *)orig_array);
}

/* lcr grow array */
int lcr_grow_array(void ***orig_array, size_t *orig_capacity, size_t size, size_t increment)
{
    size_t add_capacity;
    void **add_array = NULL;

    if (orig_array == NULL || orig_capacity == NULL || size == SIZE_MAX) {
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
size_t lcr_array_len(void **orig_array)
{
    void **p = NULL;
    size_t length = 0;

    for (p = orig_array; p && *p; p++) {
        length++;
    }

    return length;
}


/* util string replace one */
static ssize_t lcr_util_string_replace_one(const char *needle, const char *replace, const char *haystack, char **result)
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
char *lcr_util_string_replace(const char *needle, const char *replace, const char *haystack)
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
        length = lcr_util_string_replace_one(needle, replace, haystack, &res_string);
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
char *lcr_util_string_append(const char *post, const char *pre)
{
    char *res_string = NULL;
    size_t length = 0;

    if (post == NULL && pre == NULL) {
        return NULL;
    }
    if (pre == NULL) {
        return isula_strdup_s(post);
    }
    if (post == NULL) {
        return isula_strdup_s(pre);
    }
    if (strlen(post) > ((SIZE_MAX - strlen(pre)) - 1)) {
        ERROR("String is too long to be appended");
        return NULL;
    }
    length = strlen(post) + strlen(pre) + 1;
    res_string = isula_common_calloc_s(length);
    if (res_string == NULL) {
        return NULL;
    }
    (void)strcat(res_string, pre);
    (void)strcat(res_string, post);

    return res_string;
}

/* util string split prefix */
char *lcr_util_string_split_prefix(size_t prefix_len, const char *file)
{
    size_t file_len = 0;
    size_t len = 0;
    char *path = NULL;

    if (file == NULL) {
        return NULL;
    }

    file_len = strlen(file);
    if (file_len < prefix_len) {
        return NULL;
    }
    len = strlen(file) - prefix_len;
    if (len > SIZE_MAX / sizeof(char) - 1) {
        return NULL;
    }
    path = isula_smart_calloc_s(sizeof(char), (len + 1));
    if (path == NULL) {
        return NULL;
    }
    (void)strncpy(path, file + prefix_len, len);
    path[len] = '\0';

    return path;
}

/* util array len */
size_t lcr_util_array_len(char **array)
{
    char **pos = NULL;
    size_t len = 0;

    for (pos = array; pos && *pos; pos++) {
        len++;
    }

    return len;
}