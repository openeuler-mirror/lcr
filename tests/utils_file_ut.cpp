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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include "mock.h"
#include "utils_file.h"
#include "auto_cleanup.h"

#define FILE_PERMISSION_TEST 0755

TEST(utils_file_testcase, test_isula_dir_exists)
{
    const char *path = "/tmp/test";

    ASSERT_EQ(isula_path_remove(path), 0);
    ASSERT_EQ(isula_dir_exists(nullptr), false);

    ASSERT_EQ(isula_dir_exists(path), false);

    ASSERT_EQ(isula_dir_exists("/"), true);
    ASSERT_EQ(isula_dir_exists("/./././//"), true);
    ASSERT_EQ(isula_dir_recursive_mk(path, FILE_PERMISSION_TEST), 0);
    ASSERT_EQ(isula_dir_exists(path), true);
    ASSERT_EQ(isula_dir_exists("/tmp/../tmp/.//test/"), true);
    ASSERT_EQ(isula_path_remove(path), 0);
}

TEST(utils_file_testcase, test_isula_file_exists)
{
    std::string path = "/tmp/test";

    ASSERT_EQ(isula_path_remove(path.c_str()), 0);
    ASSERT_EQ(isula_file_exists(nullptr), false);

    ASSERT_EQ(isula_file_exists(path.c_str()), false);
    ASSERT_EQ(isula_file_exists("/tmp/./../tmp/.////test"), false);

    ASSERT_EQ(isula_dir_recursive_mk(path.c_str(), FILE_PERMISSION_TEST), 0);
    ASSERT_EQ(isula_file_exists(path.c_str()), true);
    ASSERT_EQ(isula_file_exists("/tmp/./../tmp/.////test"), true);
    ASSERT_EQ(isula_path_remove(path.c_str()), 0);
}

TEST(utils_file_testcase, test_isula_dir_build)
{
    ASSERT_EQ(isula_dir_build(nullptr), -1);

    std::string path = "/tmp/test/file";
    ASSERT_EQ(isula_dir_build(path.c_str()), 0);
    ASSERT_EQ(isula_file_exists("/tmp"), true);
    ASSERT_EQ(isula_file_exists("/tmp/test"), true);
    ASSERT_EQ(isula_path_remove(path.c_str()), 0);
}

TEST(utils_file_testcase, test_isula_dir_recursive_mk)
{
    ASSERT_EQ(isula_dir_recursive_mk(nullptr, FILE_PERMISSION_TEST), -1);
    ASSERT_EQ(isula_dir_recursive_mk("", FILE_PERMISSION_TEST), -1);

    std::string path = "/tmp/test";
    std::string path_link = "/tmp/test/link";
    std::string dept_dir = "/tmp/test/a/b/c";

    ASSERT_EQ(isula_dir_recursive_mk(path.c_str(), FILE_PERMISSION_TEST), 0);
    ASSERT_EQ(isula_dir_recursive_mk(path_link.c_str(), FILE_PERMISSION_TEST), 0);
    ASSERT_EQ(isula_dir_recursive_mk(dept_dir.c_str(), FILE_PERMISSION_TEST), 0);
    ASSERT_EQ(isula_dir_exists(path.c_str()), true);
    ASSERT_EQ(isula_dir_exists(path_link.c_str()), true);
    ASSERT_EQ(isula_dir_exists(dept_dir.c_str()), true);

    ASSERT_NE(isula_dir_recursive_remove(path.c_str(), ISULA_MAX_PATH_DEPTH-2), 0);
    ASSERT_EQ(isula_dir_exists(path.c_str()), true);
    ASSERT_EQ(isula_dir_exists(path_link.c_str()), false);
    ASSERT_EQ(isula_dir_exists(dept_dir.c_str()), true);
    ASSERT_EQ(isula_dir_recursive_remove(path.c_str(), 0), 0);
    ASSERT_EQ(isula_dir_exists(path.c_str()), false);
    ASSERT_EQ(isula_dir_exists(dept_dir.c_str()), false);
}

TEST(utils_file_testcase, test_isula_file_ensure_path)
{
    __isula_auto_free char *rpath = NULL;
    std::string path = "/tmp//././test";
    std::string errPath = "/tmp/x/x/test";

    ASSERT_EQ(isula_path_remove(path.c_str()), 0);
    ASSERT_EQ(isula_file_ensure_path(nullptr, path.c_str()), -1);
    ASSERT_EQ(isula_file_ensure_path(&rpath, nullptr), -1);
    ASSERT_EQ(rpath, nullptr);

    ASSERT_EQ(isula_file_exists(errPath.c_str()), false);
    ASSERT_EQ(isula_file_ensure_path(&rpath, errPath.c_str()), -1);
    ASSERT_EQ(rpath, nullptr);

    ASSERT_EQ(isula_file_exists(path.c_str()), false);
    ASSERT_EQ(isula_file_ensure_path(&rpath, path.c_str()), 0);
    ASSERT_NE(rpath, nullptr);
    ASSERT_STREQ(rpath, "/tmp/test");
    ASSERT_EQ(isula_file_exists(rpath), true);
    ASSERT_EQ(isula_path_remove(path.c_str()), 0);
    ASSERT_EQ(isula_file_exists(rpath), false);
}

TEST(utils_file_testcase, test_isula_dir_recursive_remove)
{
    std::string path = "/tmp/test";

    ASSERT_EQ(isula_dir_recursive_remove(nullptr, 0), -1);
    ASSERT_EQ(isula_dir_recursive_remove(path.c_str(), ISULA_MAX_PATH_DEPTH), -1);
    ASSERT_EQ(isula_dir_recursive_remove(path.c_str(), 0), 0);
}

TEST(utils_file_testcase, test_isula_file_open_and_remove)
{
    std::string path = "/tmp/test";
    // struct stat buf;
    // int nret;
    int fd;

    ASSERT_EQ(isula_file_open(nullptr, O_RDONLY, 0), -1);
    // ASSERT_NE(isula_file_open(path.c_str(), -1, 0), -1);
    ASSERT_EQ(isula_file_open("/tmp/xxxx/xx/test", O_RDONLY, 0), -1);

    ASSERT_EQ(isula_path_remove(nullptr), -1);
    ASSERT_EQ(isula_path_remove("/tmp/xxx/xx"), 0);
    ASSERT_EQ(isula_path_remove("/tmp/xxx/xx/"), 0);

    fd = isula_file_open(path.c_str(), O_RDONLY | O_CREAT, 0640);
    ASSERT_NE(fd, -1);
    // for some env, the umask set might lead the mode not to be 0644 
    // nret = stat(path.c_str(), &buf);
    // ASSERT_EQ(nret, 0);
    // ASSERT_EQ(buf.st_mode&0640, 0640);
    close(fd);
    ASSERT_EQ(isula_path_remove(path.c_str()), 0);
}

TEST(utils_file_testcase, test_isula_close_inherited_fds)
{
    std::string path = "/tmp/test";
    __isula_auto_close int fd;

    fd = isula_file_open(path.c_str(), O_RDONLY | O_CREAT, 0640);
    ASSERT_NE(fd, -1);

    pid_t pid = fork();
    if (pid < 0) {
        ASSERT_GE(pid, 0);
    }
    if (pid == 0) {
        // this is child
        int ret;
        char checkPath[PATH_MAX] = { 0 };

        ret = snprintf(checkPath, PATH_MAX, "/proc/self/fd/%d", fd);
        ASSERT_NE(ret, -1);
        ASSERT_EQ(isula_close_inherited_fds(true, fd), 0);
        ASSERT_EQ(isula_file_exists(checkPath), true);
        ASSERT_EQ(isula_close_inherited_fds(true, -1), 0);
        ASSERT_EQ(isula_file_exists(checkPath), false);
        exit(0);
    }

    ASSERT_EQ(isula_path_remove(path.c_str()), 0);
}

TEST(utils_file_testcase, test_isula_read_write_nointr)
{
    char buf[32] = { 0 };
    std::string test_file = "/tmp/test_read_nointr";
    std::string test_string = "hello";
    __isula_auto_close int fd_wr = -1;
    __isula_auto_close int fd_rd = -1;
    int nwrite = -1;
    int nread = -1;

    ASSERT_EQ(isula_file_read_nointr(-1, nullptr, 32), -1);
    ASSERT_EQ(isula_file_read_nointr(0, nullptr, 32), -1);
    ASSERT_EQ(isula_file_read_nointr(1, nullptr, 32), -1);

    fd_wr = isula_file_open(test_file.c_str(), O_CREAT | O_RDWR | O_APPEND | O_SYNC, 0640);
    ASSERT_GT(fd_wr, 0);
    nwrite = isula_file_write_nointr(fd_wr, test_string.c_str(), 5);
    ASSERT_EQ(nwrite, 5);
    fd_rd = open(test_file.c_str(), O_RDONLY);
    nread = isula_file_read_nointr(fd_rd, buf, 32);
    ASSERT_EQ(nread, 5);

    isula_path_remove(test_file.c_str());
}

TEST(utils_file_testcase, test_isula_set_non_block)
{
    ASSERT_EQ(isula_set_non_block(-1), -1);

    int pipefd[2];
    ASSERT_EQ(0, pipe(pipefd));
    ASSERT_EQ(0, isula_set_non_block(pipefd[0]));
    int flag = fcntl(pipefd[0], F_GETFL, 0);
    ASSERT_NE(-1, flag);
    EXPECT_TRUE(flag & O_NONBLOCK);
    close(pipefd[0]);
    close(pipefd[1]);

    int pipefd2[2];
    ASSERT_EQ(0, pipe(pipefd2));
    close(pipefd2[1]);
    ASSERT_EQ(-1, isula_set_non_block(pipefd2[1]));
    close(pipefd2[0]);
}

TEST(utils_file_testcase, test_util_validate_absolute_path)
{
    ASSERT_EQ(isula_validate_absolute_path("/etc/isulad"), 0);
    ASSERT_EQ(isula_validate_absolute_path("/isulad/"), 0);

    ASSERT_EQ(isula_validate_absolute_path(nullptr), -1);
    ASSERT_EQ(isula_validate_absolute_path("./isulad"), -1);
    ASSERT_EQ(isula_validate_absolute_path("isulad"), -1);
}