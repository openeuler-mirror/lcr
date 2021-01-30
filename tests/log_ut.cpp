/******************************************************************************
 * iSula-libutils: utils library for iSula
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
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
#include <unistd.h>

#include "log.h"
#include "utils.h"

TEST(log_testcases, test_isula_libutils_default_log_config)
{
    struct isula_libutils_log_config tconf = {0};
    const char *name = "test";

    // quiet configs check
    tconf.quiet = true;
    isula_libutils_default_log_config(name, &tconf);

    ASSERT_EQ(tconf.file, nullptr);
    EXPECT_STREQ(tconf.driver, ISULA_LOG_DRIVER_STDOUT);
    EXPECT_STREQ(name, tconf.name);
    EXPECT_STREQ("DEBUG", tconf.priority);

    // not quiet configs check
    tconf.quiet = false;
    isula_libutils_default_log_config(name, &tconf);

    ASSERT_EQ(tconf.file, nullptr);
    EXPECT_STREQ(tconf.driver, ISULA_LOG_DRIVER_STDOUT);
    EXPECT_STREQ(name, tconf.name);
    EXPECT_STREQ("DEBUG", tconf.priority);
}

static void check_log(int fd, bool exist, bool subcheck, const char *target)
{
    char buf[LXC_LOG_BUFFER_SIZE] = {0};
    ssize_t rlen;
    char *found = nullptr;

    rlen = lcr_util_read_nointr(fd, buf, LXC_LOG_BUFFER_SIZE);
    if (exist) {
        ASSERT_GT(rlen, 0);
    } else {
        ASSERT_LT(rlen, 0);
    }
    found = strstr(buf, target);
    if (subcheck) {
        ASSERT_NE(found, nullptr);
    } else {
        ASSERT_EQ(found, nullptr);
    }
}

TEST(log_testcases, test_isula_libutils_log_enable)
{
    struct isula_libutils_log_config tconf = {0};
    const char *prefix = "fake";
    const char *prio = "INFO";
    const char *invalid_prio = "INVALID";
    const char *fname = "/tmp/fake.fifo";
    int fd = -1;
    int ret = 0;

    ret = isula_libutils_log_enable(nullptr);
    ASSERT_NE(ret, 0);
    fd = isula_libutils_get_log_fd();
    ASSERT_EQ(fd, -1);

    tconf.driver = ISULA_LOG_DRIVER_FIFO;
    tconf.prefix = prefix;
    tconf.priority = prio;
    tconf.file = nullptr;
    ret = isula_libutils_log_enable(&tconf);
    ASSERT_NE(ret, 0);
    fd = isula_libutils_get_log_fd();
    ASSERT_EQ(fd, -1);

    tconf.driver = nullptr;
    tconf.prefix = prefix;
    tconf.priority = prio;
    tconf.file = fname;
    ret = isula_libutils_log_enable(&tconf);
    ASSERT_EQ(ret, 0);
    fd = isula_libutils_get_log_fd();
    ASSERT_EQ(fd, -1);

    tconf.driver = ISULA_LOG_DRIVER_STDOUT;
    tconf.prefix = prefix;
    tconf.priority = prio;
    tconf.file = fname;
    ret = isula_libutils_log_enable(&tconf);
    ASSERT_NE(ret, 0);
    isula_libutils_log_disable();

    tconf.driver = ISULA_LOG_DRIVER_STDOUT;
    tconf.prefix = prefix;
    tconf.priority = prio;
    tconf.file = nullptr;
    ret = isula_libutils_log_enable(&tconf);
    ASSERT_EQ(ret, 0);
    TRACE("trace log");
    DEBUG("debug log");
    INFO("info log");
    NOTICE("notice log");
    WARN("warn log");
    ERROR("error log");
    EVENT("event log");
    CRIT("crit log");
    FATAL("fatal log");
    isula_libutils_log_disable();

    tconf.driver = ISULA_LOG_DRIVER_FIFO;
    tconf.prefix = prefix;
    tconf.priority = invalid_prio;
    tconf.file = fname;
    ret = isula_libutils_log_enable(&tconf);
    ASSERT_EQ(ret, 0);
    fd = isula_libutils_get_log_fd();
    ASSERT_GE(fd, 0);
    DEBUG("debug log");
    check_log(fd, false, false, "debug log");
    TRACE("trace log");
    DEBUG("debug log");
    INFO("info log");
    NOTICE("notice log");
    WARN("warn log");
    ERROR("error log");
    EVENT("event log");
    CRIT("crit log");
    FATAL("fatal log");
    isula_libutils_log_disable();

    tconf.driver = ISULA_LOG_DRIVER_FIFO;
    tconf.prefix = prefix;
    tconf.priority = prio;
    tconf.file = fname;
    ret = isula_libutils_log_enable(&tconf);
    ASSERT_EQ(ret, 0);
    fd = isula_libutils_get_log_fd();
    ASSERT_GE(fd, 0);
    INFO("info log");
    check_log(fd, true, true, "info log");
    DEBUG("debug log");
    check_log(fd, false, false, "debug log");
    isula_libutils_log_disable();
}

TEST(log_testcases, test_isula_libutils_log_prefix)
{
    struct isula_libutils_log_config tconf = {0};
    const char *default_prefix = "iSula";
    const char *prefix = "prefix";
    const char *prio = "INFO";
    const char *fname = "/tmp/fake.fifo";
    int fd = -1;

    tconf.driver = ISULA_LOG_DRIVER_FIFO;
    tconf.priority = prio;
    tconf.file = fname;
    isula_libutils_log_enable(&tconf);

    fd = isula_libutils_get_log_fd();
    ASSERT_GE(fd, 0);

    isula_libutils_set_log_prefix(prefix);
    INFO("fake log");
    check_log(fd, true, true, prefix);

    isula_libutils_free_log_prefix();
    INFO("fake log");
    check_log(fd, true, false, prefix);
    INFO("fake log");
    check_log(fd, true, true, default_prefix);

    isula_libutils_set_log_prefix(nullptr);
    INFO("fake log");
    check_log(fd, true, true, default_prefix);

    isula_libutils_set_log_prefix("");
    INFO("fake log");
    check_log(fd, true, true, default_prefix);
}

