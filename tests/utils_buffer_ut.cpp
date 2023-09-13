/******************************************************************************
 * iSula-libutils: ut for utils_buffer.c
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

#include <gtest/gtest.h>

#include <iostream>
#include <string.h>

#include "utils_buffer.h"

TEST(isula_buffer_testcase, test__isula_buffer_alloc)
{
    isula_buffer *ib = nullptr;

    ib = isula_buffer_alloc(0);
    ASSERT_EQ(ib, nullptr);

    ib = isula_buffer_alloc(SIZE_MAX);
    ASSERT_EQ(ib, nullptr);

    ib = isula_buffer_alloc(1);
    ASSERT_NE(ib, nullptr);
    ASSERT_NE(ib->contents, nullptr);
    ASSERT_EQ(ib->bytes_used, 0);
    ASSERT_EQ(ib->total_size, 1);

    isula_buffer_free(ib);
    ib = nullptr;

    // check nullptr input, donot coredump
    isula_buffer_free(ib);
}

TEST(isula_buffer_testcase, test__isula_buffer_nappend)
{
    isula_buffer *ib = nullptr;
    int ret;
    const size_t initSize = 8;

    ib = isula_buffer_alloc(initSize);
    ASSERT_NE(ib, nullptr);
    ASSERT_NE(ib->contents, nullptr);
    ASSERT_EQ(ib->bytes_used, 0);
    ASSERT_EQ(ib->total_size, initSize);

    ret = ib->nappend(nullptr, 1, "hello");
    ASSERT_NE(ret, 0);
    ret = ib->nappend(ib, SIZE_MAX, "hello");
    ASSERT_NE(ret, 0);
    ret = ib->nappend(ib, 0, "hello");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 0);
    ret = ib->nappend(ib, 1, nullptr);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 0);
    ret = ib->nappend(ib, 6, "hello");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 5);
    ASSERT_EQ(ib->total_size, initSize);
    ASSERT_STREQ(ib->contents, "hello");
    ret = ib->nappend(ib, 7, " world");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 11);
    ASSERT_EQ(ib->total_size, initSize * 2);
    ASSERT_STREQ(ib->contents, "hello world");

    ib->clear(ib);
    ASSERT_EQ(ib->bytes_used, 0);
    
    ret = ib->nappend(ib, 10, "hello %s", "world");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 10);
    ASSERT_EQ(ib->total_size, initSize * 2);
    ASSERT_STREQ(ib->contents, "hello worl");

    isula_buffer_free(ib);
    ib = nullptr;
}


TEST(isula_buffer_testcase, test__isula_buffer_append)
{
    isula_buffer *ib = nullptr;
    int ret;
    const size_t initSize = 8;

    ib = isula_buffer_alloc(initSize);
    ASSERT_NE(ib, nullptr);
    ASSERT_NE(ib->contents, nullptr);
    ASSERT_EQ(ib->bytes_used, 0);
    ASSERT_EQ(ib->total_size, initSize);

    ret = ib->append(nullptr, "hello");
    ASSERT_NE(ret, 0);
    ret = ib->append(ib, nullptr);
    ASSERT_EQ(ret, 0);
    ret = ib->append(ib, "");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 0);
    ret = ib->append(ib, "hello");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 5);
    ASSERT_EQ(ib->total_size, initSize);
    ASSERT_STREQ(ib->contents, "hello");
    ret = ib->append(ib, " world");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 11);
    ASSERT_EQ(ib->total_size, initSize * 2);
    ASSERT_STREQ(ib->contents, "hello world");

    isula_buffer_free(ib);
    ib = nullptr;
}

TEST(isula_buffer_testcase, test__isula_buffer_to_str)
{
    isula_buffer *ib = nullptr;
    int ret;
    const size_t initSize = 8;
    char *tmpStr = nullptr;

    ib = isula_buffer_alloc(initSize);
    ASSERT_NE(ib, nullptr);
    ASSERT_NE(ib->contents, nullptr);
    ASSERT_EQ(ib->bytes_used, 0);
    ASSERT_EQ(ib->total_size, initSize);

    tmpStr = ib->to_str(nullptr);
    ASSERT_EQ(tmpStr, nullptr);
    tmpStr = ib->to_str(ib);
    ASSERT_NE(tmpStr, nullptr);
    ASSERT_STREQ(tmpStr, "");
    free(tmpStr);

    ret = ib->nappend(ib, 5, "hello");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 5);
    ASSERT_EQ(ib->total_size, initSize);
    ASSERT_STREQ(ib->contents, "hello");
    tmpStr = ib->to_str(ib);
    ASSERT_NE(tmpStr, nullptr);
    ASSERT_STREQ(tmpStr, "hello");
    free(tmpStr);

    ret = ib->nappend(ib, 6, " world");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 11);
    ASSERT_EQ(ib->total_size, initSize * 2);
    ASSERT_STREQ(ib->contents, "hello world");
    tmpStr = ib->to_str(ib);
    ASSERT_NE(tmpStr, nullptr);
    ASSERT_STREQ(tmpStr, "hello world");
    free(tmpStr);

    isula_buffer_free(ib);
    ib = nullptr;
}

TEST(isula_buffer_testcase, test__isula_buffer_clear)
{
    isula_buffer *ib = nullptr;
    int ret;
    const size_t initSize = 8;
    char *tmpStr = nullptr;

    ib = isula_buffer_alloc(initSize);
    ASSERT_NE(ib, nullptr);
    ASSERT_NE(ib->contents, nullptr);
    ASSERT_EQ(ib->bytes_used, 0);
    ASSERT_EQ(ib->total_size, initSize);

    ret = ib->nappend(ib, 5, "hello");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(ib->bytes_used, 5);
    ASSERT_EQ(ib->length(ib), 5);
    ASSERT_EQ(ib->total_size, initSize);
    ASSERT_STREQ(ib->contents, "hello");
    tmpStr = ib->to_str(ib);
    ASSERT_NE(tmpStr, nullptr);
    ASSERT_STREQ(tmpStr, "hello");
    free(tmpStr);
    ib->clear(ib);
    ASSERT_NE(ib, nullptr);
    ASSERT_NE(ib->contents, nullptr);
    ASSERT_EQ(ib->length(ib), 0);
    ASSERT_EQ(ib->total_size, initSize);

    ib->clear(nullptr);
    ASSERT_EQ(ib->length(nullptr), 0);

    isula_buffer_free(ib);
    ib = nullptr;
}