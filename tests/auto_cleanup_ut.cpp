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

pthread_mutex_t g_test_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_rwlock_t g_test_rwlock = PTHREAD_RWLOCK_INITIALIZER;

int do_auto_unlock()
{
    __isula_auto_pm_unlock pthread_mutex_t *local_mutex = nullptr;
    if (pthread_mutex_lock(&g_test_lock) != 0) {
        // if lock failed, do not do auto unlock
        return -1;
    }

    local_mutex = &g_test_lock;
    return 0;
}

TEST(autocleanup_testcase, test__isula_auto_pm_unlock)
{
    int ret;

    ret = do_auto_unlock();
    if (ret == -1) {
        return;
    }

    ret = pthread_mutex_lock(&g_test_lock);
    if (ret != 0) {
        ASSERT_NE(EBUSY, errno);
    }
    (void)pthread_mutex_unlock(&g_test_lock);
}

int do_prw_auto_unlock()
{
    __isula_auto_prw_unlock pthread_rwlock_t *local_rwlock = nullptr;
    if (pthread_rwlock_wrlock(&g_test_rwlock) != 0) {
        // if lock failed, do not do auto unlock
        return -1;
    }

    local_rwlock = &g_test_rwlock;
    return 0;
}

TEST(autocleanup_testcase, test__isula_auto_prw_unlock)
{
    int ret;

    ret = do_prw_auto_unlock();
    if (ret == -1) {
        return;
    }
    ret = pthread_rwlock_wrlock(&g_test_rwlock);
    if (ret != 0) {
        ASSERT_NE(EBUSY, errno);
    }
    (void)pthread_rwlock_unlock(&g_test_rwlock);

    ret = do_prw_auto_unlock();
    if (ret == -1) {
        return;
    }
    ret = pthread_rwlock_rdlock(&g_test_rwlock);
    if (ret != 0) {
        ASSERT_NE(EBUSY, errno);
    }
    (void)pthread_rwlock_unlock(&g_test_rwlock);
}

size_t do_auto_free()
{
    __isula_auto_free void *test = nullptr;
#if defined(__GLIBC__) && ((__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 33))
    struct mallinfo2 info = { 0 };

    // use 1024 * 1024 to ensure memory allo from mmap
    test = malloc(1024 * 1024);
    info = mallinfo2();
    return info.hblks;
#else
    struct mallinfo info = { 0 };

    // use 1024 * 1024 to ensure memory allo from mmap
    test = malloc(1024 * 1024);
    info = mallinfo();
    return info.hblks;
#endif
}

int *do_auto_free_and_transfer()
{
    __isula_auto_free int *test = nullptr;

    // use 1024 * 1024 to ensure memory allo from mmap
    test = static_cast<int *>(malloc(sizeof(int)));
    *test = 8;

    return isula_transfer_ptr(test);
}

TEST(autocleanup_testcase, test__isula_auto_free)
{
#if defined(__GLIBC__) && ((__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 33))
    struct mallinfo2 before;
    struct mallinfo2 after;
    size_t used;

    before = mallinfo2();
    used = do_auto_free();
    after = mallinfo2();
#else
    struct mallinfo before;
    struct mallinfo after;
    size_t used;

    before = mallinfo();
    used = do_auto_free();
    after = mallinfo();
#endif
    ASSERT_EQ(0, after.hblks);
    ASSERT_NE(used, after.hblks);
    ASSERT_NE(used, before.hblks);
    ASSERT_EQ(before.hblks, after.hblks);

    __isula_auto_free int *transfer_ptr = do_auto_free_and_transfer();
    ASSERT_NE(nullptr, transfer_ptr);
    ASSERT_EQ(8, *transfer_ptr);
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

int do_auto_close_and_transfer()
{
    __isula_auto_close int fd = -1;

    fd = open("/proc/self/cmdline", 0444);

    return isula_transfer_fd(fd);
}

TEST(autocleanup_testcase, test__isula_auto_close)
{
    int openfd, ret;
    size_t i;
    struct stat sbuf = { 0 };
    __isula_auto_close int transfer_fd = -1;

    transfer_fd = do_auto_close_and_transfer();
    ret = fstat(transfer_fd, &sbuf);
    ASSERT_EQ(0, ret);

    openfd = do_auto_close();
    ret = fstat(openfd, &sbuf);
    ASSERT_NE(0, ret);
    ASSERT_EQ(EBADF, errno);

    // test auto cleanup in for-loop
    for (i = 0; i < 10; i++) {
        ret = fstat(openfd, &sbuf);
        ASSERT_NE(0, ret);
        ASSERT_EQ(EBADF, errno);

        __isula_auto_close int inner_fd = -1;
        inner_fd = open("/proc/self/cmdline", 0444);
        openfd = inner_fd;
    }
}