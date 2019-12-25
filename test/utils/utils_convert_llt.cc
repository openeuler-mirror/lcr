/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * lcr licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 * Description: utils_convert llt
 * Author: tanyifeng
 * Create: 2019-07-08
 */

#include <stdlib.h>
#include <stdio.h>
#include <climits>
#include <securec.h>
#include <gtest/gtest.h>
#include "mock.h"
#include "utils.h"

TEST(utils_convert, test_util_safe_int)
{
    int ret;
    int converted;
    ret = util_safe_int("123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 123456);

    ret = util_safe_int("123456", NULL);
    ASSERT_NE(ret, 0);

    ret = util_safe_int("-123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, -123456);

    ret = util_safe_int("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = util_safe_int("1.23", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_int("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_int(std::to_string((long long)INT_MIN - 1).c_str(), &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_int(std::to_string((long long)INT_MAX + 1).c_str(), &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_int("NULL", &converted);
    ASSERT_NE(ret, 0);
}

TEST(utils_convert, test_util_safe_uint)
{
    int ret;
    unsigned int converted;
    ret = util_safe_uint("123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 123456);

    ret = util_safe_uint("123456", NULL);
    ASSERT_NE(ret, 0);

    ret = util_safe_uint("-123456", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_uint("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = util_safe_uint("1.23", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_uint("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_uint(std::to_string((long long)UINT_MAX + 1).c_str(), &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_uint("NULL", &converted);
    ASSERT_NE(ret, 0);
}

TEST(utils_convert, test_util_safe_llong)
{
    int ret;
    long long converted;
    ret = util_safe_llong("123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 123456);

    ret = util_safe_llong("123456", NULL);
    ASSERT_NE(ret, 0);

    ret = util_safe_llong("-123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, -123456);

    ret = util_safe_llong("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = util_safe_llong("1.23", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_llong("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_llong("-9223372036854775809", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_llong("9223372036854775808", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_llong("NULL", &converted);
    ASSERT_NE(ret, 0);
}

TEST(utils_convert, test_util_safe_ullong)
{
    int ret;
    unsigned long long converted;
    ret = util_safe_ullong("123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 123456);

    ret = util_safe_ullong("123456", NULL);
    ASSERT_NE(ret, 0);

    ret = util_safe_ullong("-123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, -123456);

    ret = util_safe_ullong("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = util_safe_ullong("1.23", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_ullong("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_ullong("18446744073709551616", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_ullong("NULL", &converted);
    ASSERT_NE(ret, 0);
}


TEST(utils_convert, test_util_safe_strtod)
{
    int ret;
    double converted;
    ret = util_safe_strtod("123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_DOUBLE_EQ(converted, 123456);

    ret = util_safe_strtod("123456", NULL);
    ASSERT_NE(ret, 0);

    ret = util_safe_strtod("-123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_DOUBLE_EQ(converted, -123456);

    ret = util_safe_strtod("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_DOUBLE_EQ(converted, 0);

    ret = util_safe_strtod("123.456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_DOUBLE_EQ(converted, 123.456);

    ret = util_safe_strtod("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = util_safe_strtod("NULL", &converted);
    ASSERT_NE(ret, 0);
}
