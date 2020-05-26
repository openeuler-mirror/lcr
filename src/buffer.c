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

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>

#include "buffer.h"
#include "log.h"
#include "utils.h"

/* buffer allocate */
Buffer *buffer_alloc(size_t initial_size)
{
    Buffer *buf = NULL;
    char *tmp = NULL;

    if (initial_size == 0) {
        return NULL;
    }

    buf = lcr_util_common_calloc_s(sizeof(Buffer));
    if (buf == NULL) {
        return NULL;
    }

    tmp = calloc(1, initial_size);
    if (tmp == NULL) {
        free(buf);
        return NULL;
    }

    buf->contents = tmp;
    buf->bytes_used = 0;
    buf->total_size = initial_size;

    return buf;
}

/* buffer strlen */
static size_t buffer_strlen(const Buffer *buf)
{
    return buf == NULL ? 0 : buf->bytes_used;
}

/* buffer free */
void buffer_free(Buffer *buf)
{
    if (buf == NULL) {
        return;
    }
    free(buf->contents);
    buf->contents = NULL;
    free(buf);
}

/* buffer has space */
static bool buffer_has_space(const Buffer *buf, size_t desired_length)
{
    size_t bytes_remaining = 0;

    if (buf == NULL) {
        return false;
    }
    bytes_remaining = buf->total_size - buf->bytes_used;

    return desired_length <= bytes_remaining;
}

/* buffer grow */
static int buffer_grow(Buffer *buf, size_t minimum_size)
{
    size_t factor = 0;
    size_t new_size = 0;
    char *tmp = NULL;

    if (buf == NULL) {
        return -1;
    }

    factor = buf->total_size;
    if (factor < minimum_size) {
        factor = minimum_size;
    }

    if (factor > SIZE_MAX / 2) {
        return -1;
    }

    new_size = factor * 2;
    if (new_size == 0) {
        return -1;
    }

    tmp = lcr_util_common_calloc_s(new_size);
    if (tmp == NULL) {
        ERROR("Out of memory");
        return -1;
    }

    (void)memcpy(tmp, buf->contents, buf->total_size);

    free(buf->contents);
    buf->contents = tmp;
    buf->total_size = new_size;

    return 0;
}

/* buffer cat */
static void buffer_cat(Buffer *buf, const char *append, size_t length)
{
    size_t i = 0;

    if (buf == NULL) {
        return;
    }

    for (i = 0; i < length; i++) {
        if (append[i] == '\0') {
            break;
        }

        *(buf->contents + buf->bytes_used + i) = append[i];
    }

    buf->bytes_used += i;
    *(buf->contents + buf->bytes_used) = '\0';
}

/* buffer append */
static int buffer_append(Buffer *buf, const char *append, size_t length)
{
    size_t desired_length = 0;

    if (buf == NULL) {
        return -1;
    }

    desired_length = length + 1;
    if (!buffer_has_space(buf, desired_length)) {
        if (buffer_grow(buf, desired_length) != 0) {
            goto error;
        }
    }

    buffer_cat(buf, append, length);

    return 0;
error:
    return -1;
}

/* buffer nappendf */
int buffer_nappendf(Buffer *buf, size_t length, const char *format, ...)
{
    int status = 0;
    size_t printf_length = 0;
    char *tmp = NULL;
    va_list argp;

    if (buf == NULL) {
        return -1;
    }

    if (length > SIZE_MAX / sizeof(char) - 1) {
        return -1;
    }
    printf_length = length + 1;
    tmp = calloc(1, printf_length * sizeof(char));
    if (tmp == NULL) {
        return -1;
    }

    va_start(argp, format);
    status = vsprintf(tmp, format, argp);
    va_end(argp);
    if (status < 0) {
        goto error;
    }

    status = buffer_append(buf, tmp, length);
    if (status != 0) {
        goto error;
    }

    free(tmp);
    return 0;
error:
    free(tmp);
    return -1;
}

/* buffer to string */
char *buffer_to_s(const Buffer *buf)
{
    size_t len;
    char *result = NULL;

    if (buf == NULL) {
        return NULL;
    }

    len = buffer_strlen(buf);
    if (len == SIZE_MAX) {
        return NULL;
    }

    result = calloc(1, len + 1);
    if (result == NULL) {
        return NULL;
    }
    (void)strncpy(result, buf->contents, len);

    return result;
}
