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
 * Description: provide container delete functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "lcrcontainer.h"
#include "lcrcontainer_extend.h"
#include "delete.h"
#include "arguments.h"
#include "log.h"
#include "utils.h"

const char g_lcr_cmd_delete_desc[] = "Delete a container";
static const char g_lcr_cmd_delete_usage[] = "delete --name=NAME";

struct lcr_arguments g_lcr_cmd_delete_args;

static int delete_cmd_init(int argc, const char **argv)
{
    int ret = 0;
    command_t cmd;
    struct command_option options[] = { DELETE_OPTIONS(g_lcr_cmd_delete_args), COMMON_OPTIONS(g_lcr_cmd_delete_args) };

    lcr_arguments_init(&g_lcr_cmd_delete_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]), argc, (const char **)argv, g_lcr_cmd_delete_desc,
                 g_lcr_cmd_delete_usage);

    if (command_parse_args(&cmd, &g_lcr_cmd_delete_args.argc, &g_lcr_cmd_delete_args.argv)) {
        ret = -1;
    }

    return ret;
}

int cmd_delete_main(int argc, const char **argv)
{
    if (delete_cmd_init(argc, argv) != 0) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(g_lcr_cmd_delete_args.name, g_lcr_cmd_delete_args.log_file, g_lcr_cmd_delete_args.log_priority,
                     g_lcr_cmd_delete_args.progname, g_lcr_cmd_delete_args.quiet, LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    if (g_lcr_cmd_delete_args.name == NULL) {
        fprintf(stderr, "missing --name,-n option\n");
        exit(EXIT_FAILURE);
    }

    if (!lcr_delete_with_force(g_lcr_cmd_delete_args.name, g_lcr_cmd_delete_args.lcrpath,
                               g_lcr_cmd_delete_args.delete_force)) {
        fprintf(stderr, "Error deleteing container %s\n", g_lcr_cmd_delete_args.name);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Container \"%s\" deleted\n", g_lcr_cmd_delete_args.name);
    exit(EXIT_SUCCESS);
}
