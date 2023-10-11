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

#ifndef _ISULA_UTILS_BUFFER_H
#define _ISULA_UTILS_BUFFER_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct __isula_buffer;

/*
 * clear used data in buffer, and reset used_size to 0
 */
typedef void (*isula_buf_clear_op)(struct __isula_buffer *buf);

/*
* append string which length is length into buffer;
* length should >= strlen(formatted string), else will truncate format string;
* success, return 0;
* fail, return -1;
*/
typedef int (*isula_buf_nappend_op)(struct __isula_buffer *buf, size_t length, const char *format, ...);

/*
* append string into buffer;
* success, return 0;
* fail, return -1;
*/
typedef int (*isula_buf_append_op)(struct __isula_buffer *buf, const char *str);

/*
* transform buffer to string, and return to caller
*/
typedef char *(*isula_buf_to_str_op)(const struct __isula_buffer *buf);

/*
* get length of data in buffer
*/
typedef size_t (*isula_buf_length_op)(const struct __isula_buffer *buf);

struct __isula_buffer {
    char *contents;
    size_t bytes_used;
    size_t total_size;
    isula_buf_clear_op clear;
    isula_buf_nappend_op nappend;
    isula_buf_append_op append;
    isula_buf_to_str_op to_str;
    isula_buf_length_op length;
};
typedef struct __isula_buffer isula_buffer;

/*
* create isula buffer with initial size memory
* if initial_size == 0, will return NULL;
* if success, return a isula_buffer struct;
*/
isula_buffer *isula_buffer_alloc(size_t initial_size);

/*
* free isula_buffer struct and memory in it
*/
void isula_buffer_free(isula_buffer *buf);

#ifdef __cplusplus
}
#endif

#endif