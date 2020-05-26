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

#ifndef LCR_BUFFER_H
#define LCR_BUFFER_H

#include <stdlib.h>
#include <strings.h>
#include <stdarg.h>

typedef struct Buffer {
    char *contents;
    size_t bytes_used;
    size_t total_size;
} Buffer;

Buffer *buffer_alloc(size_t initial_size);
void buffer_free(Buffer *buf);
int buffer_nappendf(Buffer *buf, size_t length, const char *format, ...);
char *buffer_to_s(const Buffer *buf);
#endif
