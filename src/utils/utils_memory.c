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
#include "utils_memory.h"
#include <string.h>

#include "auto_cleanup.h"

#if __WORDSIZE == 64
// current max user memory for 64-machine is 2^47 B
#define MAX_MEMORY_SIZE ((size_t)1 << 47)
#else
// current max user memory for 32-machine is 2^31 B
#define MAX_MEMORY_SIZE ((size_t)1 << 31)
#endif

/*
* Be careful, if count == 0;
* some OS maybe not return NULL, you should manual free it
*/
void *isula_smart_calloc_s(size_t unit_size, size_t count)
{
    if (unit_size == 0) {
        return NULL;
    }

    if (count > (MAX_MEMORY_SIZE / unit_size)) {
        return NULL;
    }

    return calloc(count, unit_size);
}

/* util common malloc s */
void *isula_common_calloc_s(size_t size)
{
    if (size == 0 || size > MAX_MEMORY_SIZE) {
        return NULL;
    }

    return calloc((size_t)1, size);
}

void isula_free_s(void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    free(ptr);
}

int isula_mem_realloc(void **newptr, size_t newsize, void **oldptr, size_t oldsize)
{
    void *addr = NULL;

    if (newptr == NULL) {
        return -1;
    }

    if (oldsize >= newsize || newsize == 0) {
        return -1;
    }

    addr = isula_common_calloc_s(newsize);
    if (addr == NULL) {
        return -1;
    }

    if (oldptr != NULL && *oldptr != NULL) {
        (void)memcpy(addr, *oldptr, oldsize);
        free(*oldptr);
        *oldptr = NULL;
    }

    *newptr = addr;
    return 0;
}

/**
 * Return: copy string, you should free it.
 * Notice: If out of memory, will abort process!
*/
char *isula_strdup_s(const char *src)
{
    char *dst = NULL;

    if (src == NULL) {
        return NULL;
    }

    dst = strdup(src);
    if (dst == NULL) {
        abort();
    }

    return dst;
}