/******************************************************************************
 * iSula-libutils: ut for utils_mainloop.c
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

#include "utils_mainloop.h"

TEST(utils_mainloop_testcase, test_isula_mainloop)
{
    isula_epoll_descr_t descr = { 0 };

    ASSERT_NE(isula_epoll_open(nullptr), 0);
    ASSERT_EQ(isula_epoll_add_handler(&descr, -1, nullptr, nullptr), 0);
    ASSERT_NE(isula_epoll_add_handler(nullptr, 111, nullptr, nullptr), 0);
    ASSERT_NE(isula_epoll_loop(nullptr, -1), 0);
    ASSERT_NE(isula_epoll_remove_handler(nullptr, 111), 0);
    ASSERT_EQ(isula_epoll_close(nullptr), 0);

    ASSERT_EQ(isula_epoll_open(&descr), 0);
    ASSERT_NE(isula_epoll_add_handler(&descr, 111, nullptr, nullptr), 0);
    ASSERT_EQ(isula_epoll_loop(&descr, 10), 0);
    ASSERT_NE(isula_epoll_remove_handler(&descr, 111), 0);
    ASSERT_EQ(isula_epoll_close(&descr), 0);
}