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

#include "read_file.h"
#include "oci_runtime_hooks.h"

TEST(libocispec_testcase, test_oci_runtime_spec_hooks)
{
    const char *fname = "./ocihook.json";
    oci_runtime_spec_hooks *hooks = nullptr;
    parser_error jerr = nullptr;
    char *jstr = nullptr;

    hooks = oci_runtime_spec_hooks_parse_file(fname, nullptr, &jerr);
    ASSERT_EQ(jerr, nullptr) << "parse hook failed: " << jerr;
    ASSERT_NE(hooks, nullptr);

    ASSERT_EQ(hooks->prestart_len, 1);
    ASSERT_NE(hooks->prestart, nullptr);
    EXPECT_STREQ(hooks->prestart[0]->path, "prestart.sh");
    ASSERT_EQ(hooks->prestart[0]->args_len, 1);
    EXPECT_STREQ(hooks->prestart[0]->args[0], "prestart.sh");
    ASSERT_EQ(hooks->prestart[0]->env_len, 2);
    EXPECT_STREQ(hooks->prestart[0]->env[0], "haozi=007");
    EXPECT_STREQ(hooks->prestart[0]->env[1], "type=prestart");
    ASSERT_EQ(hooks->prestart[0]->timeout, 1);

    ASSERT_EQ(hooks->poststart_len, 1);
    ASSERT_NE(hooks->poststart, nullptr);
    EXPECT_STREQ(hooks->poststart[0]->path, "poststart.sh");
    ASSERT_EQ(hooks->poststart[0]->args_len, 1);
    EXPECT_STREQ(hooks->poststart[0]->args[0], "poststart.sh");
    ASSERT_EQ(hooks->poststart[0]->env_len, 2);
    EXPECT_STREQ(hooks->poststart[0]->env[0], "haozi=008");
    EXPECT_STREQ(hooks->poststart[0]->env[1], "type=poststart");
    ASSERT_EQ(hooks->poststart[0]->timeout, 2);

    ASSERT_EQ(hooks->poststop_len, 1);
    ASSERT_NE(hooks->poststop, nullptr);
    EXPECT_STREQ(hooks->poststop[0]->path, "poststop.sh");
    ASSERT_EQ(hooks->poststop[0]->args_len, 1);
    EXPECT_STREQ(hooks->poststop[0]->args[0], "poststop.sh");
    ASSERT_EQ(hooks->poststop[0]->env_len, 2);
    EXPECT_STREQ(hooks->poststop[0]->env[0], "haozi=009");
    EXPECT_STREQ(hooks->poststop[0]->env[1], "type=poststop");
    ASSERT_EQ(hooks->poststop[0]->timeout, 3);

    jstr = oci_runtime_spec_hooks_generate_json(hooks, nullptr, &jerr);
    ASSERT_EQ(jerr, nullptr);
    ASSERT_NE(jstr, nullptr);

    free_oci_runtime_spec_hooks(hooks);
    free(jstr);
}

TEST(libocispec_testcase, test_json_readfile)
{
    const char *fname = "./ocihook.json";
    const char *not_exist = "/tmp/not_exist.json";
    char *jstr = nullptr;
    size_t len = 0;

    jstr = read_file(fname, &len);
    ASSERT_NE(jstr, nullptr);
    ASSERT_EQ(len, 527);
    free(jstr);
    len = 0;

    jstr = read_file(not_exist, &len);
    ASSERT_EQ(jstr, nullptr);
    ASSERT_EQ(len, 0);
    len = 0;

    jstr = read_file(nullptr, nullptr);
    ASSERT_EQ(jstr, nullptr);
}

