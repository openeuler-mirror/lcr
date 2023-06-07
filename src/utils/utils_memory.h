/******************************************************************************
 * isula: memory utils
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
#ifndef _ISULA_UTILS_UTILS_MEMORY_H
#define _ISULA_UTILS_UTILS_MEMORY_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void *lcr_util_smart_calloc_s(size_t unit_size, size_t count);
void *lcr_util_common_calloc_s(size_t size);
int lcr_mem_realloc(void **newptr, size_t newsize, void *oldptr, size_t oldsize);

char *lcr_util_strdup_s(const char *src);

#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_UTILS_MEMORY_H */