/******************************************************************************
 * iSula-libutils: utils library for iSula
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

#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "auto_cleanup.h"

size_t do_auto_free()
{
    __isula_auto_free void *test = nullptr;
    struct mallinfo info = { 0 };

    // use 1024 * 1024 to ensure memory allo from mmap
    test = malloc(1024 * 1024);
    info = mallinfo();
    return info.hblks;
}

TEST(autocleanup_testcase, test__isula_auto_free)
{
    struct mallinfo before;
    struct mallinfo after;
    size_t used;

    before = mallinfo();
    used = do_auto_free();
    after = mallinfo();
    ASSERT_EQ(0, after.hblks);
    ASSERT_NE(used, after.hblks);
    ASSERT_NE(used, before.hblks);
    ASSERT_EQ(before.hblks, after.hblks);
}

int do_auto_file()
{
    __isula_auto_file FILE *fp = nullptr;

    fp = fopen("/proc/self/cmdline", "r");
    return fileno(fp);
}

TEST(autocleanup_testcase, test__isula_auto_file)
{
    int openfd, ret;
    struct stat sbuf = { 0 };

    openfd = do_auto_file();

    ret = fstat(openfd, &sbuf);

    ASSERT_NE(0, ret);
    ASSERT_EQ(EBADF, errno);
}

int do_auto_dir()
{
    __isula_auto_dir DIR *fp = nullptr;

    fp = opendir("/proc/self/fd/");
    return dirfd(fp);
}

TEST(autocleanup_testcase, test__isula_auto_dir)
{
    int openfd, ret;
    struct stat sbuf = { 0 };

    openfd = do_auto_dir();

    ret = fstat(openfd, &sbuf);

    ASSERT_NE(0, ret);
    ASSERT_EQ(EBADF, errno);
}

int do_auto_close()
{
    __isula_auto_close int fd = -1;

    fd = open("/proc/self/cmdline", 0444);

    return fd;
}

TEST(autocleanup_testcase, test__isula_auto_close)
{
    int openfd, ret;
    struct stat sbuf = { 0 };

    openfd = do_auto_close();

    ret = fstat(openfd, &sbuf);

    ASSERT_NE(0, ret);
    ASSERT_EQ(EBADF, errno);
}