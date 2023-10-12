/******************************************************************************
 * iSula-libutils: ut for utils_file.c
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
#include "mock.h"

#include <iostream>
#include <string.h>
#include <chrono>

#include "utils.h"


TEST(utils_utils_testcase, test_isula_usleep_nointerupt)
{
    auto start_time = std::chrono::high_resolution_clock::now();
    isula_usleep_nointerupt(500);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    ASSERT_GT(elapsed_time.count(), 400);
    ASSERT_LT(elapsed_time.count(), 600);

    
    start_time = std::chrono::high_resolution_clock::now();
    isula_usleep_nointerupt(1000);
    end_time = std::chrono::high_resolution_clock::now();
    elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    ASSERT_GT(elapsed_time.count(), 900);
    ASSERT_LT(elapsed_time.count(), 1100);
}