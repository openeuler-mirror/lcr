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
 * Description: provide container exec functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "lcrcontainer.h"
#include "exec.h"
#include "arguments.h"
#include "log.h"
#include "utils.h"

const char g_lcr_cmd_exec_desc[] = "execute new process inside the container";
static const char g_lcr_cmd_exec_usage[] = "exec --name=NAME [-- COMMAND]";

struct lcr_arguments g_lcr_cmd_exec_args;

static inline int check_container_name()
{
    if (g_lcr_cmd_exec_args.name == NULL) {
        fprintf(stderr, "missing --name,-n option\n");
        return -1;
    }

    return 0;
}

int cmd_exec_main(int argc, const char **argv)
{
    pid_t pid = 0;
    int ret;
    command_t cmd;
    struct command_option options[] = {
        EXEC_OPTIONS(g_lcr_cmd_exec_args),
        COMMON_OPTIONS(g_lcr_cmd_exec_args)
    };

    lcr_arguments_init(&g_lcr_cmd_exec_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]), argc, (const char **)argv,
                 g_lcr_cmd_exec_desc, g_lcr_cmd_exec_usage);
    if (command_parse_args(&cmd, &g_lcr_cmd_exec_args.argc, &g_lcr_cmd_exec_args.argv)) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(g_lcr_cmd_exec_args.name,
                     g_lcr_cmd_exec_args.log_file,
                     g_lcr_cmd_exec_args.log_priority,
                     g_lcr_cmd_exec_args.progname,
                     g_lcr_cmd_exec_args.quiet,
                     LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    if (check_container_name() != 0) {
        exit(EXIT_FAILURE);
    }

    if (!lcr_exec(g_lcr_cmd_exec_args.name,
                  g_lcr_cmd_exec_args.lcrpath,
                  g_lcr_cmd_exec_args.argc,
                  g_lcr_cmd_exec_args.argv,
                  &pid)) {
        fprintf(stderr, "Error execute new process inside container \"%s\"\n",
                g_lcr_cmd_exec_args.name);
        exit(EXIT_FAILURE);
    }

    ret = wait_for_pid(pid) < 0;
    if (ret < 0) {
        fprintf(stderr, "exec success bug got bad return %d\n", ret);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
