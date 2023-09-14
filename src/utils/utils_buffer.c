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

#include "utils_buffer.h"

#include "utils_memory.h"
#include "log.h"
#include "auto_cleanup.h"

void isula_buffer_clear(isula_buffer *buf)
{
    if (buf == NULL) {
        return;
    }
    (void)memset(buf->contents, 0, buf->total_size);
    buf->bytes_used = 0;
}

static bool buffer_has_space(const isula_buffer *buf, size_t desired_length)
{
    size_t bytes_remaining;

    if (buf->total_size < buf->bytes_used) {
        return false;
    }
    bytes_remaining = buf->total_size - buf->bytes_used;

    return desired_length <= bytes_remaining;
}

static int buffer_grow(isula_buffer *buf, size_t minimum_size)
{
    size_t factor, new_size;
    char *tmp = NULL;

    factor = buf->total_size;
    if (factor < minimum_size) {
        factor = minimum_size;
    }

    if (factor == 0) {
        ERROR("Invalid grow size");
        return -1;
    }

    tmp = lcr_util_smart_calloc_s(factor, 2);
    if (tmp == NULL) {
        ERROR("Out of memory");
        return -1;
    }
    new_size = factor * 2;

    (void)memcpy(tmp, buf->contents, buf->total_size);

    free(buf->contents);
    buf->contents = tmp;
    buf->total_size = new_size;

    return 0;
}

static void buffer_cat(isula_buffer *buf, const char *append, size_t length)
{
    size_t i;

    for (i = 0; i < length; i++) {
        if (append[i] == '\0') {
            break;
        }

        *(buf->contents + buf->bytes_used + i) = append[i];
    }

    buf->bytes_used += i;
    *(buf->contents + buf->bytes_used) = '\0';
}

static int buffer_append(isula_buffer *buf, const char *append, size_t length)
{
    size_t desired_length = 0;

    if (append == NULL) {
        return 0;
    }

    desired_length = length + 1;
    if (!buffer_has_space(buf, desired_length)) {
        if (buffer_grow(buf, desired_length) != 0) {
            return -1;
        }
    }

    buffer_cat(buf, append, length);

    return 0;
}

int isula_buffer_nappend(isula_buffer *buf, size_t length, const char *format, ...)
{
    int status = 0;
    size_t printf_length;
    __isula_auto_free char *tmp = NULL;
    va_list argp;

    if (buf == NULL) {
        DEBUG("Empty buffer.");
        return -1;
    }

    if (format == NULL || length == 0) {
        return 0;
    }

    if (length > SIZE_MAX - 1) {
        ERROR("Too large append string");
        return -1;
    }
    printf_length = length + 1;
    tmp = lcr_util_smart_calloc_s(sizeof(char), printf_length);
    if (tmp == NULL) {
        ERROR("Out of memory");
        return -1;
    }

    va_start(argp, format);
    status = vsnprintf(tmp, printf_length, format, argp);
    va_end(argp);
    if (status < 0) {
        SYSERROR("Sprintf error");
        return -1;
    }

    return buffer_append(buf, tmp, length);
}

int isula_buffer_append(isula_buffer *buf, const char *str)
{
    if (buf == NULL) {
        DEBUG("Empty buffer.");
        return -1;
    }

    if (str == NULL || strlen(str) == 0) {
        return 0;
    }

    return buffer_append(buf, str, strlen(str));
}

char *isula_buffer_to_str(const isula_buffer *buf)
{
    size_t len;
    char *result = NULL;

    if (buf == NULL) {
        DEBUG("Empty argument.");
        return NULL;
    }

    len = buf->bytes_used;
    if (len == SIZE_MAX) {
        ERROR("Too large buffer data");
        return NULL;
    }

    result = lcr_util_smart_calloc_s(1, len + 1);
    if (result == NULL) {
        ERROR("Out of memory");
        return NULL;
    }
    (void)strncpy(result, buf->contents, len);

    return result;
}

size_t isula_buffer_strlen(const isula_buffer *buf)
{
    return buf == NULL ? 0 : buf->bytes_used;
}

isula_buffer *isula_buffer_alloc(size_t initial_size)
{
    isula_buffer *buf = NULL;
    char *tmp = NULL;

    if (initial_size == 0) {
        return NULL;
    }

    buf = lcr_util_common_calloc_s(sizeof(isula_buffer));
    if (buf == NULL) {
        ERROR("Out of memory");
        return NULL;
    }

    tmp = lcr_util_smart_calloc_s(1, initial_size);
    if (tmp == NULL) {
        ERROR("Out of memory");
        free(buf);
        return NULL;
    }

    buf->contents = tmp;
    buf->bytes_used = 0;
    buf->total_size = initial_size;

    buf->clear = isula_buffer_clear;
    buf->nappend = isula_buffer_nappend;
    buf->append = isula_buffer_append;
    buf->to_str = isula_buffer_to_str;
    buf->length = isula_buffer_strlen;

    return buf;
}

void isula_buffer_free(isula_buffer *buf)
{
    if (buf == NULL) {
        return;
    }
    free(buf->contents);
    buf->contents = NULL;
    buf->bytes_used = 0;
    buf->total_size = 0;
    free(buf);
}