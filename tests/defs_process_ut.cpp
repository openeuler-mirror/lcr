/******************************************************************************
 * UT for parser and generator for JSON definition of process
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 *
 * Authors:
 * Xu Xuepeng <xuxuepeng1@huawei.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 ********************************************************************************/
#include <gtest/gtest.h>

#include <iostream>

#include <string.h>
#include <unistd.h>

#include "defs_process.h"

/* Test case for successful parsing */
TEST(defs_process_testcase, parse_file_success)
{
    const char *filename = "process.json";
    parser_error err = NULL;
    defs_process *ptr = defs_process_parse_file(filename, nullptr, &err);
    char *jstr = nullptr;

    /* Assertions */
    ASSERT_NE(ptr, nullptr);
    ASSERT_STREQ(ptr->cwd, "/home/user");
    ASSERT_EQ(ptr->args_len, 2);
    ASSERT_STREQ(ptr->args[0], "arg1");
    ASSERT_STREQ(ptr->args[1], "arg2");
    ASSERT_EQ(ptr->env_len, 2);
    ASSERT_STREQ(ptr->env[0], "key1=value1");
    ASSERT_STREQ(ptr->env[1], "key2=value2");
    ASSERT_EQ(ptr->console_size->height, 25);
    ASSERT_EQ(ptr->console_size->width, 80);
    ASSERT_EQ(ptr->terminal, true);
    ASSERT_STREQ(ptr->user->username, "user");
    ASSERT_EQ(ptr->user->uid, 1000);
    ASSERT_EQ(ptr->user->gid, 1000);
    ASSERT_EQ(ptr->user->additional_gids_len, 2);
    ASSERT_EQ(ptr->user->additional_gids[0], 1001);
    ASSERT_EQ(ptr->user->additional_gids[1], 1002);
    ASSERT_STREQ(ptr->apparmor_profile, "profile");
    ASSERT_EQ(ptr->oom_score_adj, 0);
    ASSERT_STREQ(ptr->selinux_label, "system_u:system_r:unconfined_t:s0");
    ASSERT_EQ(ptr->no_new_privileges, false);
    ASSERT_EQ(ptr->capabilities->bounding_len, 2);
    ASSERT_STREQ(ptr->capabilities->bounding[0], "CAP_CHOWN");
    ASSERT_STREQ(ptr->capabilities->bounding[1], "CAP_DAC_OVERRIDE");
    ASSERT_EQ(ptr->capabilities->permitted_len, 1);
    ASSERT_STREQ(ptr->capabilities->permitted[0], "CAP_NET_BIND_SERVICE");
    ASSERT_EQ(ptr->capabilities->effective_len, 1);
    ASSERT_STREQ(ptr->capabilities->effective[0], "CAP_SYS_ADMIN");
    ASSERT_EQ(ptr->capabilities->inheritable_len, 1);
    ASSERT_STREQ(ptr->capabilities->inheritable[0], "CAP_SYS_CHROOT");
    ASSERT_EQ(ptr->capabilities->ambient_len, 1);
    ASSERT_STREQ(ptr->capabilities->ambient[0], "CAP_NET_RAW");
    ASSERT_EQ(ptr->rlimits_len, 1);
    ASSERT_EQ(ptr->rlimits[0]->hard, 200);
    ASSERT_EQ(ptr->rlimits[0]->soft, 100);
    ASSERT_STREQ(ptr->rlimits[0]->type, "RLIMIT_NOFILE");
    ASSERT_EQ(err, nullptr);

    jstr = defs_process_generate_json(ptr, nullptr, &err);
    ASSERT_EQ(err, nullptr);
    ASSERT_NE(jstr, nullptr);

    /* Cleanup */
    free_defs_process(ptr);
    free(jstr);
    free(err);
}

/* Test case for file read failure */
TEST(defs_process_testcase, parse_file_file_read_fail) {
    const char *filename = "non_existing_file.txt";
    parser_error err = NULL;
    defs_process *ptr = defs_process_parse_file(filename, nullptr, &err);

    /* Assertions */
    ASSERT_EQ(ptr, nullptr);
    ASSERT_STREQ(err, "cannot read the file: non_existing_file.txt");

    /* Cleanup */
    free(err);
}
