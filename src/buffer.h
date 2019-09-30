/******************************************************************************
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * lcr licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 * Author: wujing
 * Create: 2018-11-08
 * Description: provide container buffer definition
 ******************************************************************************/
#ifndef LCRD_BUFFER_H
#define LCRD_BUFFER_H

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
