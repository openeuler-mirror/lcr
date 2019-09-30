/******************************************************************************
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * lcr licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 * Author: wujing
 * Create: 2018-11-08
 * Description: provide container clean functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "clean.h"
#include "lcrcontainer.h"
#include "arguments.h"
#include "log.h"
#include "utils.h"

const char g_lcr_cmd_clean_desc[] =
    "Delete any resources held by the container often used with detached container NAME";
static const char g_lcr_cmd_clean_usage[] = "clean [command options] --name=NAME";

struct lcr_arguments g_lcr_cmd_clean_args;

int cmd_clean_main(int argc, const char **argv)
{
    command_t cmd;
    struct command_option options[] = {
        CLEAN_OPTIONS(g_lcr_cmd_clean_args),
        COMMON_OPTIONS(g_lcr_cmd_clean_args)
    };

    lcr_arguments_init(&g_lcr_cmd_clean_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]),
                 argc, (const char **)argv,
                 g_lcr_cmd_clean_desc, g_lcr_cmd_clean_usage);
    if (command_parse_args(&cmd, &(g_lcr_cmd_clean_args.argc), &(g_lcr_cmd_clean_args.argv))) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(g_lcr_cmd_clean_args.name, g_lcr_cmd_clean_args.log_file,
                     g_lcr_cmd_clean_args.log_priority,
                     g_lcr_cmd_clean_args.progname,
                     g_lcr_cmd_clean_args.quiet,
                     LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    if (g_lcr_cmd_clean_args.name == NULL) {
        fprintf(stderr, "missing container name, use -n,--name option\n");
        exit(EXIT_FAILURE);
    }

    if (!lcr_clean(g_lcr_cmd_clean_args.name,
                   g_lcr_cmd_clean_args.lcrpath,
                   g_lcr_cmd_clean_args.log_file,
                   g_lcr_cmd_clean_args.log_priority,
                   0)) {
        fprintf(stderr, "Failed to clean container %s\n", g_lcr_cmd_clean_args.name);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Container \"%s\" Cleaned\n", g_lcr_cmd_clean_args.name);
    exit(EXIT_SUCCESS);
}
