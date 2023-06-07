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
#ifndef _ISULA_UTILS_UTILS_ARRAY_H
#define _ISULA_UTILS_UTILS_ARRAY_H

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void lcr_free_array(void **array);
int lcr_grow_array(void ***array, size_t *capacity, size_t new_size, size_t capacity_increment);
size_t lcr_array_len(void **array);
size_t lcr_util_array_len(char **array);

char *lcr_util_string_join(const char *sep, const char **parts, size_t len);
char **lcr_string_split_and_trim(const char *str, char _sep);
char *lcr_util_string_replace(const char *needle, const char *replacement, const char *haystack);
char *lcr_util_string_append(const char *post, const char *pre);
char *lcr_util_string_split_prefix(size_t prefix_len, const char *file);


#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_UTILS_ARRAY_H */