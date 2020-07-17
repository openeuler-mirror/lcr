/******************************************************************************
 * ported from golang hash/crc64.go
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

#ifndef __CRC64_UTIL_H
#define __CRC64_UTIL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRC_ROW_LEN 8
#define CRC_COL_LEN 256
#define ISO_POLY 0xD800000000000000

typedef struct __isula_crc_table_t {
    bool inited;
    uint64_t table[CRC_ROW_LEN][CRC_COL_LEN];
} isula_crc_table_t;

const isula_crc_table_t* new_isula_crc_table(uint64_t poly);

bool isula_crc_update(const isula_crc_table_t *tab, uint64_t *crc, unsigned char *data, size_t data_len);

void isula_crc_sum(uint64_t crc, unsigned char data[8]);

#ifdef __cplusplus
}
#endif

#endif
