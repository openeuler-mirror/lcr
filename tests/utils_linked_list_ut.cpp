/******************************************************************************
 * iSula-libutils: ut for utils_linked_list.c
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
#include <chrono>

#include "utils_linked_list.h"
#include "utils_memory.h"

TEST(utils_linked_list_testcase, test_linked_list_ops)
{
    std::vector<std::string> testData = {
        "a", "b", "c", "d"
    };
    size_t i;

    auto *head = (struct isula_linked_list *)isula_common_calloc_s(sizeof(struct isula_linked_list));
    ASSERT_NE(head, nullptr);

    isula_linked_list_init(head);
    ASSERT_EQ(isula_linked_list_len(head), 0);
    ASSERT_EQ(isula_linked_list_empty(head), true);

    auto *elem = (struct isula_linked_list *)isula_common_calloc_s(sizeof(struct isula_linked_list));
    char *val = isula_strdup_s(testData[0].c_str());
    isula_linked_list_add_elem(elem, val);
    isula_linked_list_add(head, elem);
    ASSERT_EQ(isula_linked_list_len(head), 1);
    ASSERT_EQ(isula_linked_list_empty(head), false);
    ASSERT_STREQ(static_cast<char *>(isula_linked_list_first_elem(head)), testData[0].c_str());
    ASSERT_STREQ(static_cast<char *>(isula_linked_list_last_elem(head)), testData[0].c_str());
    
    for (i = 1; i < testData.size(); i++)
    {
        auto *elem = (struct isula_linked_list *)isula_common_calloc_s(sizeof(struct isula_linked_list));
        char *val = isula_strdup_s(testData[i].c_str());
        isula_linked_list_add_elem(elem, val);
        isula_linked_list_add_tail(head, elem);
    }
    ASSERT_EQ(isula_linked_list_len(head), testData.size());
    ASSERT_EQ(isula_linked_list_empty(head), false);
    ASSERT_STREQ(static_cast<char *>(isula_linked_list_first_elem(head)), testData[0].c_str());
    ASSERT_STREQ(static_cast<char *>(isula_linked_list_last_elem(head)), testData[3].c_str());

    struct isula_linked_list *iter;
    i = 0;
    isula_linked_list_for_each(iter, head) {
        char *tval = static_cast<char *>(iter->elem);
        ASSERT_STREQ(testData[i].c_str(), tval);
        i++;
    }

    i = 0;
    struct isula_linked_list *next = nullptr;
    isula_linked_list_for_each_safe(iter, head, next) {
        char *tval = static_cast<char *>(iter->elem);
        ASSERT_STREQ(testData[i].c_str(), tval);
        i++;
        isula_linked_list_del(iter);
        free(iter->elem);
        free(iter);
        if (i == 2) {
            break;
        }
    }
    ASSERT_EQ(isula_linked_list_len(head), 2);
    i = 2;
    isula_linked_list_for_each(iter, head) {
        char *tval = static_cast<char *>(iter->elem);
        ASSERT_STREQ(testData[i].c_str(), tval);
        i++;
    }

    std::vector<std::string> testData2 = {
        "g", "f"
    };
    auto *list2 = (struct isula_linked_list *)isula_common_calloc_s(sizeof(struct isula_linked_list));
    ASSERT_NE(list2, nullptr);

    isula_linked_list_init(list2);
    ASSERT_EQ(isula_linked_list_len(list2), 0);
    ASSERT_EQ(isula_linked_list_empty(list2), true);

    for (i = 0; i < testData2.size(); i++)
    {
        auto *elem = (struct isula_linked_list *)isula_common_calloc_s(sizeof(struct isula_linked_list));
        char *val = isula_strdup_s(testData2[i].c_str());
        isula_linked_list_add_elem(elem, val);
        isula_linked_list_add(list2, elem);
    }
    ASSERT_EQ(isula_linked_list_len(list2), testData2.size());
    ASSERT_EQ(isula_linked_list_empty(list2), false);
    ASSERT_STREQ(static_cast<char *>(isula_linked_list_first_elem(list2)), testData2[1].c_str());
    ASSERT_STREQ(static_cast<char *>(isula_linked_list_last_elem(list2)), testData2[0].c_str());

    isula_linked_list_add_elem(list2, isula_strdup_s("head"));
    isula_linked_list_merge(head, list2);
    // 5 = 2 node of head + 3 node of list2
    ASSERT_EQ(isula_linked_list_len(head), 5);
    ASSERT_EQ(isula_linked_list_empty(head), false);
    i = 0;
    std::vector<std::string> checkData = {
        "c", "d", "head", "f", "g" 
    };
    next = nullptr;
    isula_linked_list_for_each_safe(iter, head, next) {
        char *tval = static_cast<char *>(iter->elem);
        ASSERT_STREQ(checkData[i].c_str(), tval);
        i++;
        isula_linked_list_del(iter);
        free(iter->elem);
        free(iter);
    }

    free(head);
}