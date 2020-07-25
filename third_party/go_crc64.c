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

#include <stdlib.h>
#include <pthread.h>

#include "go_crc64.h"

pthread_mutex_t g_crc_lock = PTHREAD_MUTEX_INITIALIZER;
isula_crc_table_t g_iso_crc_table;

static void make_table(isula_crc_table_t *tab, uint64_t poly)
{
    uint64_t i, j;
    uint64_t t;

    for (i = 0; i < CRC_COL_LEN; i++) {
        t = i;
        for (j = 0; j < CRC_ROW_LEN; j++) {
            if ((t & 1) == 1) {
                t = (t >> 1) ^ poly;
            } else {
                t >>= 1;
            }
        }
        tab->table[0][i] = t;
    }
}

static void make_crc_table(isula_crc_table_t *tab)
{
    uint64_t i, j;
    uint64_t crc;

    for (i = 0; i < CRC_COL_LEN; i++) {
        crc = tab->table[0][i];
        for (j = 1; j < CRC_ROW_LEN; j++) {
            crc = tab->table[0][(crc&0xff)] ^ (crc >> 8);
            tab->table[j][i] = crc;
        }
    }
}

const isula_crc_table_t* new_isula_crc_table(uint64_t poly)
{
    isula_crc_table_t *ret = NULL;
    if (pthread_mutex_lock(&g_crc_lock) != 0) {
        return ret;
    }

    switch (poly) {
        case ISO_POLY:
            ret = &g_iso_crc_table;
            break;
        default:
            goto out;
    }
    // ret must be non-null
    if (ret->inited) {
        goto out;
    }
    ret->inited = true;
    make_table(ret, poly);
    make_crc_table(ret);

out:
    (void)pthread_mutex_unlock(&g_crc_lock);
    return ret;
}

bool isula_crc_update(const isula_crc_table_t *tab, uint64_t *crc, unsigned char *data, size_t data_len)
{
    size_t i = 0;
    uint64_t tcrc = 0;

    if (tab == NULL || !(tab->inited) || crc == NULL) {
        // invalid table, just return origin crc
        return false;
    }
    tcrc = ~(*crc);

    if (data_len >= 64) {
        for (; i + CRC_ROW_LEN < data_len; i = i + CRC_ROW_LEN) {
            tcrc ^= (uint64_t)data[i] | ((uint64_t)data[i+1] << 8) | ((uint64_t)data[i+2] << 16) |
                ((uint64_t)data[i+3] << 24) | ((uint64_t)data[i+4] << 32) | ((uint64_t)data[i+5] << 40) |
                ((uint64_t)data[i+6] << 48) | ((uint64_t)data[i+7] << 56);

            tcrc = tab->table[7][tcrc & 0xff] ^
                tab->table[6][(tcrc >> 8) & 0xff] ^
                tab->table[5][(tcrc >> 16) & 0xff] ^
                tab->table[4][(tcrc >> 24) & 0xff] ^
                tab->table[3][(tcrc >> 32) & 0xff] ^
                tab->table[2][(tcrc >> 40) & 0xff] ^
                tab->table[1][(tcrc >> 48) & 0xff] ^
                tab->table[0][(tcrc >> 56)];
        }
    }

    for (; i < data_len; i++) {
        tcrc = tab->table[0][((uint8_t)tcrc) ^ ((uint8_t)data[i])] ^ (tcrc >> 8);
    }

    *crc = ~tcrc;
    return true;
}

void isula_crc_sum(uint64_t crc, unsigned char data[8])
{
    data[0] = (unsigned char)(crc >> 56);
    data[1] = (unsigned char)(crc >> 48);
    data[2] = (unsigned char)(crc >> 40);
    data[3] = (unsigned char)(crc >> 32);
    data[4] = (unsigned char)(crc >> 24);
    data[5] = (unsigned char)(crc >> 16);
    data[6] = (unsigned char)(crc >> 8);
    data[7] = (unsigned char)(crc);
}
