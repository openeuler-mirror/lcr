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

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __WORDSIZE == 64
// current max user memory for 64-machine is 2^47 B
#define ISULA_MAX_MEMORY_SIZE ((size_t)1 << 47)
#else
// current max user memory for 32-machine is 2^31 B
#define ISULA_MAX_MEMORY_SIZE ((size_t)1 << 31)
#endif

/*
* allocate dynamic memory which size = (unit_size * count)
* if (unit_size * count) >= 2^47B on 64bit machine, will failed and return NULL;
* if unit_size == 0, will failed and return NULL;
*/
void *isula_smart_calloc_s(size_t unit_size, size_t count);

/*
* allocate dynamic memory
* if size >= 2^47B on 64bit machine, will failed and return NULL;
* if size == 0, will failed and return NULL;
*/
void *isula_common_calloc_s(size_t size);

/*
* free memory which allocate by isula_smart_calloc_s,
* isula_common_calloc_s, isula_mem_realloc or isula_strdup_s
*/
void isula_free_s(void *ptr);

/*
* reallocate dynamic memory
* if success, will copy data in oldptr into newptr, and free memory of oldptr
* if newptr == NULL, will failed;
* if (oldsize > newsize) or newsize == 0, will failed;
* if oldptr == NULL, isula_mem_realloc equal to isula_common_calloc_s;
*/
int isula_mem_realloc(void **newptr, size_t newsize, void **oldptr, size_t oldsize);

/*
* copy src and return to caller
* if src == NULL, will return NULL
* if strdup failed, will cause to abort process
*/
char *isula_strdup_s(const char *src);

#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_UTILS_MEMORY_H */