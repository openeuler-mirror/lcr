/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * lcr licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 * Description: utils_string llt
 * Author: tanyifeng
 * Create: 2019-07-08
 */

#include <stdlib.h>
#include <stdio.h>
#include <securec.h>
#include <gtest/gtest.h>
#include "mock.h"
#include "utils.h"

extern "C" {
    DECLARE_WRAPPER(util_strdup_s, char *, (const char *str));
    DEFINE_WRAPPER(util_strdup_s, char *, (const char *str), (str));

    DECLARE_WRAPPER(calloc, void *, (size_t nmemb, size_t size));
    DEFINE_WRAPPER(calloc, void *, (size_t nmemb, size_t size), (nmemb, size));

    DECLARE_WRAPPER_V(strcat_s, errno_t, (char *strDest, size_t destMax, const char *strSrc));
    DEFINE_WRAPPER_V(strcat_s, errno_t, (char *strDest, size_t destMax, const char *strSrc),
                     (strDest, destMax, strSrc));
}

static int g_strcat_s_cnt = 0;

static errno_t strcat_s_fail(char *strDest, size_t destMax, const char *strSrc)
{
    return (errno_t)EINVAL;
}

static errno_t strcat_s_second_fail(char *strDest, size_t destMax, const char *strSrc)
{
    g_strcat_s_cnt++;
    if (g_strcat_s_cnt == 1) {
        return __real_strcat_s(strDest, destMax, strSrc);
    }
    return (errno_t)EINVAL;
}

TEST(utils_string_llt, test_str_skip_str)
{
    const char *str = "abcdefghij1234567890";
    const char *substr = "abcdefgh";
    const char *result = nullptr;

    result = str_skip_str(str, substr);
    ASSERT_STREQ(result, "ij1234567890");

    result = str_skip_str(str, "habc");
    ASSERT_STREQ(result, nullptr);

    result = str_skip_str(str, "");
    ASSERT_STREQ(result, str);

    result = str_skip_str(str, nullptr);
    ASSERT_STREQ(result, nullptr);

    result = str_skip_str("a", "a");
    ASSERT_STREQ(result, "");

    result = str_skip_str("", "");
    ASSERT_STREQ(result, "");

    result = str_skip_str(nullptr, "");
    ASSERT_STREQ(result, nullptr);
}

TEST(utils_string_llt, test_util_string_join)
{
    const char *array_long[] = { "abcd", "1234", "5678", "", "&^%abc" };
    size_t array_long_len = sizeof(array_long) / sizeof(array_long[0]);

    const char *array_short[] = { "abcd" };
    size_t array_short_len = sizeof(array_short) / sizeof(array_short[0]);

    const char *array_nullptr[] = { nullptr };
    size_t array_nullptr_len = sizeof(array_nullptr) / sizeof(array_nullptr[0]);

    char *result = nullptr;

    result = util_string_join("   ", array_long, array_long_len);
    ASSERT_STREQ(result, "abcd   1234   5678      &^%abc");
    free(result);

    result = util_string_join("   ", array_short, array_short_len);
    ASSERT_STREQ(result, "abcd");
    free(result);

    result = util_string_join("   ", array_nullptr, array_nullptr_len);
    ASSERT_EQ(result, nullptr);

    result = util_string_join("   ", nullptr, 0);
    ASSERT_EQ(result, nullptr);

    result = util_string_join("", array_long, array_long_len);
    ASSERT_STREQ(result, "abcd12345678&^%abc");
    free(result);

    result = util_string_join(nullptr, array_long, array_long_len);
    ASSERT_STREQ(result, nullptr);

    MOCK_SET_V(strcat_s, strcat_s_fail);
    result = util_string_join("   ", array_short, array_short_len);
    ASSERT_STREQ(result, nullptr);
    MOCK_CLEAR(strcat_s);

    MOCK_SET_V(strcat_s, strcat_s_fail);
    result = util_string_join("   ", array_long, array_long_len);
    ASSERT_STREQ(result, nullptr);
    MOCK_CLEAR(strcat_s);

    g_strcat_s_cnt = 0;
    MOCK_SET_V(strcat_s, strcat_s_second_fail);
    result = util_string_join("   ", array_long, array_long_len);
    ASSERT_STREQ(result, nullptr);
    MOCK_CLEAR(strcat_s);
}

TEST(utils_string_llt, test_util_string_append)
{
    char *result = nullptr;

    result = util_string_append("abc", "123");
    ASSERT_STREQ(result, "123abc");
    free(result);

    result = util_string_append("abc", "");
    ASSERT_STREQ(result, "abc");
    free(result);

    result = util_string_append("abc", nullptr);
    ASSERT_STREQ(result, "abc");
    free(result);

    result = util_string_append("", "123");
    ASSERT_STREQ(result, "123");
    free(result);

    result = util_string_append("", "");
    ASSERT_STREQ(result, "");
    free(result);

    result = util_string_append("", nullptr);
    ASSERT_STREQ(result, "");
    free(result);

    result = util_string_append(nullptr, "123");
    ASSERT_STREQ(result, "123");
    free(result);

    result = util_string_append(nullptr, "");
    ASSERT_STREQ(result, "");
    free(result);

    result = util_string_append(nullptr, nullptr);
    ASSERT_STREQ(result, nullptr);

    MOCK_SET(calloc, nullptr);
    result = util_string_append("abc", "123");
    ASSERT_STREQ(result, nullptr);
    MOCK_CLEAR(calloc);

    MOCK_SET_V(strcat_s, strcat_s_fail);
    result = util_string_append("abc", "123");
    ASSERT_STREQ(result, nullptr);
    MOCK_CLEAR(strcat_s);

    g_strcat_s_cnt = 0;
    MOCK_SET_V(strcat_s, strcat_s_second_fail);
    result = util_string_append("abc", "123");
    ASSERT_STREQ(result, nullptr);
    MOCK_CLEAR(strcat_s);
}

