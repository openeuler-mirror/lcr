/******************************************************************************
 * iSula-libutils: utils library for iSula
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
#include <gtest/gtest.h>

#include <iostream>

#include <string.h>
#include <unistd.h>

#include "go_crc64.h"

TEST(go_crc64_testcase, test_new_isula_crc_table)
{
    const isula_crc_table_t* ctab = new_isula_crc_table(ISO_POLY);
    ASSERT_NE(ctab, nullptr);
    ASSERT_EQ(ctab->inited, true);

    ctab = new_isula_crc_table(0x88888);
    ASSERT_EQ(ctab, nullptr);
}

TEST(go_crc64_testcase, test_isula_crc_update)
{
    const isula_crc_table_t* ctab = new_isula_crc_table(ISO_POLY);
    ASSERT_NE(ctab, nullptr);
    ASSERT_EQ(ctab->inited, true);

    uint64_t crc = 0;
    bool ret;

    unsigned char buf1[] = "haozi";
    ret = isula_crc_update(ctab, &crc, buf1, sizeof(buf1) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0x39804c3418100000);

    crc = 0;
    unsigned char buf2[] = "haozixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    ret = isula_crc_update(ctab, &crc, buf2, sizeof(buf2) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0xb8a4e0d5cfd9d3d6);

    crc = 0;
    unsigned char buf3[] = "haozixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxyyyy";
    ret = isula_crc_update(ctab, &crc, buf3, sizeof(buf3) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0x9fb11ff8b8a4e0d5);

    // check update with before crc
    crc = 0;
    ret = isula_crc_update(ctab, &crc, buf1, sizeof(buf1) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0x39804c3418100000);

    ret = isula_crc_update(ctab, &crc, buf2, sizeof(buf2) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0x8c862c945a1dbbfa);

    ret = isula_crc_update(ctab, &crc, buf3, sizeof(buf3) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0xaff9fc095bb587c6);
}

static bool check_sum(uint8_t sums[8], uint64_t crc)
{
    uint64_t tmp = 0;
    size_t i;

    for (i = 0; i < 8; i++) {
        tmp |= sums[i];
        if (i == 7) {
            break;
        }
        tmp <<= 8;
    }

    return (tmp == crc);
}

TEST(go_crc64_testcase, test_isula_crc_sum)
{
    const isula_crc_table_t* ctab = new_isula_crc_table(ISO_POLY);
    ASSERT_NE(ctab, nullptr);
    ASSERT_EQ(ctab->inited, true);

    uint64_t crc = 0;
    bool ret;
    uint8_t sums[8] = {0};

    unsigned char buf1[] = "haozi";
    ret = isula_crc_update(ctab, &crc, buf1, sizeof(buf1) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0x39804c3418100000);

    isula_crc_sum(crc, sums);
    ret = check_sum(sums, crc);
    ASSERT_EQ(ret, true);

    crc = 0;
    unsigned char buf2[] = "haozixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    ret = isula_crc_update(ctab, &crc, buf2, sizeof(buf2) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0xb8a4e0d5cfd9d3d6);

    isula_crc_sum(crc, sums);
    ret = check_sum(sums, crc);
    ASSERT_EQ(ret, true);

    crc = 0;
    unsigned char buf3[] = "haozixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxyyyy";
    ret = isula_crc_update(ctab, &crc, buf3, sizeof(buf3) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0x9fb11ff8b8a4e0d5);

    isula_crc_sum(crc, sums);
    ret = check_sum(sums, crc);
    ASSERT_EQ(ret, true);

    // check update with before crc
    crc = 0;
    ret = isula_crc_update(ctab, &crc, buf1, sizeof(buf1) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0x39804c3418100000);

    isula_crc_sum(crc, sums);
    ret = check_sum(sums, crc);
    ASSERT_EQ(ret, true);

    ret = isula_crc_update(ctab, &crc, buf2, sizeof(buf2) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0x8c862c945a1dbbfa);

    isula_crc_sum(crc, sums);
    ret = check_sum(sums, crc);
    ASSERT_EQ(ret, true);

    ret = isula_crc_update(ctab, &crc, buf3, sizeof(buf3) - 1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(crc, 0xaff9fc095bb587c6);

    isula_crc_sum(crc, sums);
    ret = check_sum(sums, crc);
    ASSERT_EQ(ret, true);
}

