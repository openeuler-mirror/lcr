/******************************************************************************
 * iSula-libutils: ut for utils_string.c
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
#include <stdlib.h>
#include <stdio.h>
#include <climits>
#include "mock.h"

#include <iostream>
#include <string.h>

#include "utils_string.h"
#include "utils_memory.h"

extern "C" {
    DECLARE_WRAPPER(calloc, void *, (size_t nmemb, size_t size));
    DEFINE_WRAPPER(calloc, void *, (size_t nmemb, size_t size), (nmemb, size));
}

TEST(utils_string_testcase, test_isula_string_join)
{
    const char *test[] = {
        "nosuid",
        "noexec",
        "mode=0620",
    };
    char *ret = nullptr;

    ret = isula_string_join(nullptr, test, 3);
    ASSERT_EQ(ret, nullptr);
    ret = isula_string_join(",", nullptr, 3);
    ASSERT_EQ(ret, nullptr);
    ret = isula_string_join(",", test, 0);
    ASSERT_EQ(ret, nullptr);
    ret = isula_string_join("123", test, SIZE_MAX);
    ASSERT_EQ(ret, nullptr);

    ret = isula_string_join("", test, 3);
    ASSERT_NE(ret, nullptr);
    ASSERT_STREQ(ret, "nosuidnoexecmode=0620");
    free(ret);

    ret = isula_string_join(",", test, 3);
    ASSERT_NE(ret, nullptr);
    ASSERT_STREQ(ret, "nosuid,noexec,mode=0620");
    free(ret);

    ret = isula_string_join("+-*/=", test, 3);
    ASSERT_NE(ret, nullptr);
    ASSERT_STREQ(ret, "nosuid+-*/=noexec+-*/=mode=0620");
    free(ret);
}

TEST(utils_string_testcase, test_isula_string_append)
{
    char *ret = nullptr;

    ret = isula_string_append(nullptr, nullptr);
    ASSERT_EQ(ret, nullptr);

    MOCK_SET(calloc, nullptr);
    ret = isula_string_append("hello", " world");
    ASSERT_EQ(ret, nullptr);
    MOCK_CLEAR(calloc);

    ret = isula_string_append("hello", nullptr);
    ASSERT_NE(ret, nullptr);
    ASSERT_STREQ(ret, "hello");
    free(ret);

    ret = isula_string_append(nullptr, " world");
    ASSERT_NE(ret, nullptr);
    ASSERT_STREQ(ret, " world");
    free(ret);

    ret = isula_string_append("hello", " world");
    ASSERT_NE(ret, nullptr);
    ASSERT_STREQ(ret, "hello world");
    free(ret);
}

TEST(utils_string_testcase, test_isula_string_replace)
{
    const char *space_magic_str =  "[#)";
    char *ret = nullptr;
    
    ret = isula_string_replace(nullptr, space_magic_str, "test");
    ASSERT_EQ(ret, nullptr);
    ret = isula_string_replace(" ", nullptr, "test");
    ASSERT_EQ(ret, nullptr);
    ret = isula_string_replace(" ", space_magic_str, nullptr);
    ASSERT_EQ(ret, nullptr);

    ret = isula_string_replace(" ", space_magic_str, "test");
    ASSERT_NE(ret, nullptr);
    ASSERT_STREQ(ret, "test");
    free(ret);

    ret = isula_string_replace(" ", space_magic_str, "hello world");
    ASSERT_NE(ret, nullptr);
    ASSERT_STREQ(ret, "hello[#)world");
    free(ret);

    ret = isula_string_replace(" ", space_magic_str, " hello world ");
    ASSERT_NE(ret, nullptr);
    ASSERT_STREQ(ret, "[#)hello[#)world[#)");
    free(ret);

    ret = isula_string_replace("#", space_magic_str, "#");
    ASSERT_NE(ret, nullptr);
    ASSERT_STREQ(ret, "[#)");
    free(ret);
}

TEST(utils_string_testcase, test_isula_string_array_new)
{
    isula_string_array *sarray = isula_string_array_new(4);
    ASSERT_NE(sarray, nullptr);
    ASSERT_EQ(sarray->cap, 4);
    ASSERT_EQ(sarray->len, 0);
    isula_string_array_free(sarray);
    sarray = nullptr;

    sarray = isula_string_array_new(0);
    ASSERT_NE(sarray, nullptr);
    ASSERT_EQ(sarray->cap, 2);
    ASSERT_EQ(sarray->len, 0);
    isula_string_array_free(sarray);
    sarray = nullptr;

    sarray = isula_string_array_new(SIZE_MAX);
    ASSERT_EQ(sarray, nullptr);
}

TEST(utils_string_testcase, test_isula_string_array_append)
{
    isula_string_array *sarray = isula_string_array_new(4);
    ASSERT_NE(sarray, nullptr);
    ASSERT_EQ(sarray->cap, 4);
    ASSERT_EQ(sarray->len, 0);
    int ret;

    ret = sarray->append(sarray, "1234567890");
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(sarray->items[0], "1234567890");
    ASSERT_EQ(sarray->items[1], nullptr);
    ASSERT_EQ(sarray->len, 1);

    ret = sarray->append(sarray, "abc");
    ASSERT_EQ(ret, 0);
    ret = sarray->append(sarray, "bcd");
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(sarray->items[1], "abc");
    ASSERT_STREQ(sarray->items[2], "bcd");
    ASSERT_EQ(sarray->len, 3);

    isula_string_array_free(sarray);
    sarray = nullptr;
}

TEST(utils_string_testcase, test_isula_string_array_append_array)
{
    isula_string_array *sarray = isula_string_array_new(4);
    ASSERT_NE(sarray, nullptr);
    ASSERT_EQ(sarray->cap, 4);
    ASSERT_EQ(sarray->len, 0);
    isula_string_array *sarray2 = isula_string_array_new(0);
    ASSERT_NE(sarray2, nullptr);
    ASSERT_EQ(sarray2->cap, 2);
    ASSERT_EQ(sarray2->len, 0);
    int ret;

    ret = sarray->append(sarray, "1234567890");
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(sarray->items[0], "1234567890");
    ASSERT_EQ(sarray->items[1], nullptr);
    ASSERT_EQ(sarray->len, 1);

    ret = sarray->append(sarray, "abc");
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(sarray->items[1], "abc");
    ret = sarray->append(sarray, "bcd");
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(sarray->items[2], "bcd");
    ASSERT_EQ(sarray->len, 3);

    ret = sarray->append_arr(sarray, nullptr);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(sarray->len, 3);

    ret = sarray2->append(sarray2, "hello");
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(sarray2->items[0], "hello");
    ret = sarray2->append(sarray2, "world");
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(sarray2->items[1], "world");

    ret = sarray->append_arr(nullptr, sarray2);
    ASSERT_EQ(ret, -1);

    ret = sarray->append_arr(sarray, sarray2);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(sarray->len, 5);
    ASSERT_STREQ(sarray->items[3], "hello");
    ASSERT_STREQ(sarray->items[4], "world");

    isula_string_array_free(sarray);
    sarray = nullptr;
    isula_string_array_free(sarray2);
}

TEST(utils_string_testcase, test_isula_string_array_contain)
{
    isula_string_array *sarray = isula_string_array_new(0);
    ASSERT_NE(sarray, nullptr);
    ASSERT_EQ(sarray->cap, 2);
    ASSERT_EQ(sarray->len, 0);
    int ret;
    bool bret = false;

    ret = sarray->append(sarray, "1234567890");
    ASSERT_EQ(ret, 0);
    ret = sarray->append(sarray, "abc");
    ASSERT_EQ(ret, 0);
    ret = sarray->append(sarray, "bcd");
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(sarray->len, 3);

    bret = sarray->contain(sarray, "axxx");
    ASSERT_EQ(bret, false);
    bret = sarray->contain(sarray, "abc");
    ASSERT_EQ(bret, true);

    isula_string_array_free(sarray);
    sarray = nullptr;
}

TEST(utils_string_testcase, test_isula_string_split_to_multi)
{
    isula_string_array *ret = nullptr;
    const char *test1 = "123 456";
    const char *test2 = "a,b,c,d";

    ret = isula_string_split_to_multi(nullptr, ',');
    ASSERT_EQ(ret, nullptr);

    ret = isula_string_split_to_multi("", ',');
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->len, 1);
    ASSERT_STREQ(ret->items[0], "");
    isula_string_array_free(ret);

    ret = isula_string_split_to_multi(test1, ' ');
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->len, 2);
    ASSERT_STREQ(ret->items[0], "123");
    ASSERT_STREQ(ret->items[1], "456");
    isula_string_array_free(ret);

    ret = isula_string_split_to_multi(test2, ',');
    ASSERT_NE(ret, nullptr);
    ASSERT_EQ(ret->len, 4);
    ASSERT_STREQ(ret->items[0], "a");
    ASSERT_STREQ(ret->items[1], "b");
    ASSERT_STREQ(ret->items[2], "c");
    ASSERT_STREQ(ret->items[3], "d");
    isula_string_array_free(ret);
}