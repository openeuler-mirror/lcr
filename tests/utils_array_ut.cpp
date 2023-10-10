/******************************************************************************
 * iSula-libutils: ut for utils_array.c
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

#include "utils_array.h"
#include "utils_memory.h"

extern "C" {
    DECLARE_WRAPPER(calloc, void *, (size_t nmemb, size_t size));
    DEFINE_WRAPPER(calloc, void *, (size_t nmemb, size_t size), (nmemb, size));
}

TEST(utils_array_testcase, test_isula_array_len)
{
    const char *array_long[] = { "abcd", "1234", "a1b", nullptr };

    ASSERT_EQ(isula_array_len(nullptr), 0);

    ASSERT_EQ(isula_array_len((void **)array_long), 3);
}

TEST(utils_array_testcase, test_isula_free_array)
{
    char **array = nullptr;

    array = (char **)isula_common_calloc_s(4 * sizeof(char *));
    ASSERT_NE(array, nullptr);
    array[0] = isula_strdup_s("test1");
    array[1] = isula_strdup_s("test2");
    array[2] = isula_strdup_s("test3");
    array[3] = nullptr;

    isula_free_array(nullptr);
    isula_free_array((void **)array);
}

TEST(utils_array_testcase, test_isula_grow_array)
{
    char **array = nullptr;
    size_t capacity = 0;
    int ret;

    capacity = 1;
    array = (char **)isula_common_calloc_s(sizeof(char *));
    ASSERT_NE(array, nullptr);
    ret = isula_grow_array((void ***)&array, &capacity, 1, 1);
    ASSERT_EQ(ret, 0);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array[0], nullptr);
    ASSERT_EQ(array[1], nullptr);
    ASSERT_EQ(capacity, 2);
    isula_free_array((void **)array);
    array = nullptr;
    capacity = 0;

    capacity = 1;
    array = (char **)isula_common_calloc_s(capacity * sizeof(char *));
    ASSERT_NE(array, nullptr);
    ret = isula_grow_array((void ***)&array, &capacity, 1, 2);
    ASSERT_EQ(ret, 0);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array[0], nullptr);
    ASSERT_EQ(array[1], nullptr);
    ASSERT_EQ(array[2], nullptr);
    ASSERT_EQ(capacity, 3);
    isula_free_array((void **)array);
    array = nullptr;
    capacity = 0;

    capacity = 1;
    array = (char **)isula_common_calloc_s(capacity * sizeof(char *));
    ASSERT_NE(array, nullptr);
    ret = isula_grow_array((void ***)&array, &capacity, 1, 4);
    ASSERT_EQ(ret, 0);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array[0], nullptr);
    ASSERT_EQ(array[1], nullptr);
    ASSERT_EQ(array[2], nullptr);
    ASSERT_EQ(array[3], nullptr);
    ASSERT_EQ(array[4], nullptr);
    ASSERT_EQ(capacity, 5);
    isula_free_array((void **)array);
    array = nullptr;
    capacity = 0;

    capacity = 1;
    array = (char **)isula_common_calloc_s(capacity * sizeof(char *));
    ASSERT_NE(array, nullptr);
    ret = isula_grow_array((void ***)&array, &capacity, 1, 0);
    ASSERT_NE(ret, 0);
    isula_free_array((void **)array);
    array = nullptr;
    capacity = 0;

    capacity = 1;
    array = (char **)isula_common_calloc_s(capacity * sizeof(char *));
    ASSERT_NE(array, nullptr);
    ret = isula_grow_array((void ***)&array, &capacity, 4, 1);
    ASSERT_EQ(ret, 0);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array[0], nullptr);
    ASSERT_EQ(array[1], nullptr);
    ASSERT_EQ(array[2], nullptr);
    ASSERT_EQ(array[3], nullptr);
    ASSERT_EQ(array[4], nullptr);
    ASSERT_EQ(capacity, 5);
    isula_free_array((void **)array);
    array = nullptr;
    capacity = 0;

    capacity = 1;
    array = (char **)isula_common_calloc_s(capacity * sizeof(char *));
    ASSERT_NE(array, nullptr);
    ret = isula_grow_array((void ***)&array, &capacity, 0, 1);
    ASSERT_EQ(ret, 0);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array[0], nullptr);
    ASSERT_EQ(capacity, 1);
    isula_free_array((void **)array);
    array = nullptr;
    capacity = 0;

    capacity = 1;
    array = (char **)isula_common_calloc_s(capacity * sizeof(char *));
    ASSERT_NE(array, nullptr);
    ret = isula_grow_array((void ***)&array, nullptr, 1, 1);
    ASSERT_NE(ret, 0);
    isula_free_array((void **)array);
    array = nullptr;
    capacity = 0;

    capacity = 1;
    array = (char **)isula_common_calloc_s(capacity * sizeof(char *));
    ASSERT_NE(array, nullptr);
    ret = isula_grow_array(nullptr, &capacity, 1, 1);
    ASSERT_NE(ret, 0);
    isula_free_array((void **)array);
    array = nullptr;
    capacity = 0;
}

void *string_clone(const void *src)
{
    return (void *)isula_strdup_s((const char *)src);
}

TEST(utils_array_testcase, test_isula_array_append)
{
    char **array = nullptr;
    char **array_three = nullptr;
    int ret;

    ret = isula_array_append((void ***)&array, "1234567890", string_clone);
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(array[0], "1234567890");
    ASSERT_EQ(array[1], nullptr);
    isula_free_array((void **)array);
    array = nullptr;

    ret = isula_array_append((void ***)&array, "", string_clone);
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(array[0], "");
    ASSERT_EQ(array[1], nullptr);
    isula_free_array((void **)array);
    array = nullptr;

    ret = isula_array_append((void ***)&array, nullptr, string_clone);
    ASSERT_NE(ret, 0);

    array_three = (char **)isula_common_calloc_s(4 * sizeof(char *));
    ASSERT_NE(array_three, nullptr);
    array_three[0] = isula_strdup_s("test1");
    array_three[1] = isula_strdup_s("test2");
    array_three[2] = isula_strdup_s("test3");
    array_three[3] = nullptr;
    ret = isula_array_append((void ***)&array_three, "1234567890", string_clone);
    ASSERT_EQ(ret, 0);
    ASSERT_STREQ(array_three[0], "test1");
    ASSERT_STREQ(array_three[1], "test2");
    ASSERT_STREQ(array_three[2], "test3");
    ASSERT_STREQ(array_three[3], "1234567890");
    ASSERT_EQ(array_three[4], nullptr);
    isula_free_array((void **)array_three);
    array_three = nullptr;

    ret = isula_array_append((void ***)&array_three, "1234567890", nullptr);
    ASSERT_NE(ret, 0);

    ret = isula_array_append(nullptr, "1234567890", string_clone);
    ASSERT_NE(ret, 0);

    MOCK_SET(calloc, nullptr);
    ret = isula_array_append((void ***)&array, "", string_clone);
    ASSERT_NE(ret, 0);
    MOCK_CLEAR(calloc);
    isula_free_array((void **)array);
    array = nullptr;
}