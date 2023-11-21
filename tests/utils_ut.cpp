/******************************************************************************
 * iSula-libutils: utils library for iSula
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 *
 * Authors:
 * jikai <jikai11@huawei.com>
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

#include <limits.h>
#include <fcntl.h>

#include "utils.h"

char *test_read_text_file(const char *path)
{
    char *buf = NULL;
    long len = 0;
    int f_fd = -1;
    size_t readlen = 0;
    FILE *filp = NULL;
    char *rpath = NULL;
    const long max_size = 10 * 1024 * 1024; /* 10M */

    if (path == NULL) {
        return NULL;
    }

    if (lcr_util_ensure_path(&rpath, path) != 0) {
        return NULL;
    }

    f_fd = open(rpath, O_RDONLY | O_CLOEXEC, 0666);
    if (f_fd < 0) {
        goto err_out;
    }

    filp = fdopen(f_fd, "r");
    if (filp == NULL) {
        close(f_fd);
        goto err_out;
    }

    if (fseek(filp, 0, SEEK_END)) {
        goto err_out;
    }

    len = ftell(filp);
    if (len > max_size) {
        goto err_out;
    }

    if (fseek(filp, 0, SEEK_SET)) {
        goto err_out;
    }

    buf = (char *)lcr_util_common_calloc_s((size_t)(len + 1));
    if (buf == NULL) {
        goto err_out;
    }

    readlen = fread(buf, 1, (size_t)len, filp);
    if (((readlen < (size_t)len) && (!feof(filp))) || (readlen > (size_t)len)) {
        free(buf);
        buf = NULL;
        goto err_out;
    }

    buf[(size_t)len] = 0;

err_out:

    if (filp != NULL) {
        fclose(filp);
    }

    free(rpath);

    return buf;
}

TEST(utils_testcase, test_get_random_tmp_file)
{
#define RANDOM_TMP_PATH 10
    const char *fname = "/tmp/lcr-test/test";
    char *tmp_file = lcr_util_get_random_tmp_file(nullptr);
    const char *prefix = "/tmp/lcr-test/.tmp-test-";
    ASSERT_EQ(tmp_file, nullptr);

    tmp_file = lcr_util_get_random_tmp_file(fname);
    ASSERT_NE(tmp_file, nullptr);

    ASSERT_EQ(strlen(tmp_file), strlen("/tmp/lcr-test/.tmp-test-") + RANDOM_TMP_PATH);
    ASSERT_EQ(memcmp(tmp_file, prefix, strlen(prefix)), 0);
    free(tmp_file);
}

TEST(utils_testcase, test_atomic_write_file)
{
    const char *fname = "/tmp/lcr-test/test";
    const char *content = "line1\nline2\n";
    const char *new_content = "line1\nline2\nline3\n";
    char *readcontent = nullptr;

    ASSERT_EQ(lcr_util_atomic_write_file(NULL, content, strlen(content), 0644, false), -1);
    ASSERT_EQ(lcr_util_atomic_write_file(fname, NULL, 0, 0644, false), 0);

    ASSERT_EQ(lcr_util_build_dir(fname), 0);

    ASSERT_EQ(lcr_util_atomic_write_file(fname, content, strlen(content), 0644, false), 0);

    readcontent = test_read_text_file(fname);
    ASSERT_NE(readcontent, nullptr);
    ASSERT_STREQ(readcontent, content);
    free(readcontent);

    ASSERT_EQ(lcr_util_atomic_write_file(fname, new_content, strlen(new_content), 0644, false), 0);
    readcontent = test_read_text_file(fname);
    ASSERT_NE(readcontent, nullptr);
    ASSERT_STREQ(readcontent, new_content);
    free(readcontent);

    ASSERT_EQ(lcr_util_recursive_rmdir("/tmp/lcr-test/", 1), 0);
}
