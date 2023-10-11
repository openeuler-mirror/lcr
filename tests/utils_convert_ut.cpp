/******************************************************************************
 * iSula-libutils: ut for utils_convert.c
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

#include <string.h>

#include "mock.h"
#include "utils_convert.h"
#include "utils_memory.h"

TEST(utils_convert_testcase, test_isula_safe_strto_bool)
{
    int ret;
    bool converted = false;
    ret = isula_safe_strto_bool("1", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(converted);

    ret = isula_safe_strto_bool("1", nullptr);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_bool("t", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(converted);

    ret = isula_safe_strto_bool("T", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(converted);

    ret = isula_safe_strto_bool("true", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(converted);

    ret = isula_safe_strto_bool("True", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(converted);

    ret = isula_safe_strto_bool("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_FALSE(converted);

    ret = isula_safe_strto_bool("f", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_FALSE(converted);

    ret = isula_safe_strto_bool("F", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_FALSE(converted);

    ret = isula_safe_strto_bool("false", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_FALSE(converted);

    ret = isula_safe_strto_bool("FALSE", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_FALSE(converted);

    ret = isula_safe_strto_bool("False", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_FALSE(converted);

    ret = isula_safe_strto_bool("x", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_bool("nullptr", &converted);
    ASSERT_NE(ret, 0);
}

TEST(utils_convert_testcase, test_isula_safe_strto_uint16)
{
    int ret;
    uint16_t converted;
    ret = isula_safe_strto_uint16("255", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 255);

    ret = isula_safe_strto_uint16("255", nullptr);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint16("-1", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint16("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = isula_safe_strto_uint16("1.23", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint16("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint16(std::to_string((long long)UINT16_MAX + 1).c_str(), &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint16("nullptr", &converted);
    ASSERT_NE(ret, 0);
}

TEST(utils_convert_testcase, test_isula_safe_strto_uint64)
{
    int ret;
    uint64_t converted;
    ret = isula_safe_strto_uint64("255", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 255);

    ret = isula_safe_strto_uint64("255", nullptr);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint64("-1", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, UINT64_MAX);

    ret = isula_safe_strto_uint64("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = isula_safe_strto_uint64("1.23", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint64("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint64("18446744073709551616", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint64("nullptr", &converted);
    ASSERT_NE(ret, 0);
}

TEST(utils_convert_testcase, test_isula_safe_strto_int)
{
    int ret;
    int converted;
    ret = isula_safe_strto_int("123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 123456);

    ret = isula_safe_strto_int("123456", nullptr);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_int("-123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, -123456);

    ret = isula_safe_strto_int("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = isula_safe_strto_int("1.23", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_int("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_int(std::to_string((long long)INT_MIN - 1).c_str(), &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_int(std::to_string((long long)INT_MAX + 1).c_str(), &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_int("nullptr", &converted);
    ASSERT_NE(ret, 0);
}

TEST(utils_convert_testcase, test_isula_safe_strto_uint)
{
    int ret;
    unsigned int converted;
    ret = isula_safe_strto_uint("123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 123456);

    ret = isula_safe_strto_uint("123456", nullptr);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint("-123456", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = isula_safe_strto_uint("1.23", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint(std::to_string((long long)UINT_MAX + 1).c_str(), &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_uint("nullptr", &converted);
    ASSERT_NE(ret, 0);
}

TEST(utils_convert_testcase, test_isula_safe_strto_llong)
{
    int ret;
    long long converted;
    ret = isula_safe_strto_llong("123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 123456);

    ret = isula_safe_strto_llong("123456", nullptr);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_llong("-123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, -123456);

    ret = isula_safe_strto_llong("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = isula_safe_strto_llong("1.23", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_llong("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_llong("-9223372036854775809", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_llong("9223372036854775808", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_llong("nullptr", &converted);
    ASSERT_NE(ret, 0);
}

TEST(utils_convert_testcase, test_isula_safe_strto_double)
{
    int ret;
    double converted;
    ret = isula_safe_strto_double("123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_DOUBLE_EQ(converted, 123456);

    ret = isula_safe_strto_double("123456", nullptr);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_double("-123456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_DOUBLE_EQ(converted, -123456);

    ret = isula_safe_strto_double("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_DOUBLE_EQ(converted, 0);

    ret = isula_safe_strto_double("123.456", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_DOUBLE_EQ(converted, 123.456);

    ret = isula_safe_strto_double("1x", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_safe_strto_double("nullptr", &converted);
    ASSERT_NE(ret, 0);
}

TEST(utils_convert_testcase, test_isula_parse_byte_size_string)
{
    int64_t converted = 0;
    int ret;

    ret = isula_parse_byte_size_string("10.9876B", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 10);

    ret = isula_parse_byte_size_string("2048.965kI", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 2098140);

    ret = isula_parse_byte_size_string("1.1GiB", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 1181116006);

    ret = isula_parse_byte_size_string("2.0tI", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 2199023255552);

    ret = isula_parse_byte_size_string("1024mB", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 1073741824);

    ret = isula_parse_byte_size_string("10.12a3PIb", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("1234.0a9", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("-10.123", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("-10.0B", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("-10.0GiB", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("-10kI", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("-10tI", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("-10Pib", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("-10.12a3mB", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("0.12345mB", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 129446);

    ret = isula_parse_byte_size_string("0.9876543210123456789tI", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 1085937410176);

    ret = isula_parse_byte_size_string("0.0kI", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = isula_parse_byte_size_string("0.0Pib", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = isula_parse_byte_size_string("0", &converted);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(converted, 0);

    ret = isula_parse_byte_size_string("0.123aB", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("0.123aGiB", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("9223372036854775808.123B", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("9007199254740992.0kI", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("8796093022208.0mB", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("8589934592GiB", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("8192PIb", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("8388608.1abtI", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("9223372036854775808.1a", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("123a456.123mB", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("6a1.123Pib", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("12a.0GiB", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("a1230.0", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("1&3B", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("a1tI", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("1a.a1kI", &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string(nullptr, &converted);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("1", nullptr);
    ASSERT_NE(ret, 0);

    ret = isula_parse_byte_size_string("", &converted);
    ASSERT_NE(ret, 0);
}
