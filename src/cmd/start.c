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
 * Description: provide container start functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lcrcontainer.h"
#include "start.h"
#include "log.h"
#include "utils.h"

const char g_lcr_cmd_start_desc[] = "start container";
static const char g_lcr_cmd_start_usage[] = "start [command options] --name=NAME";

struct lcr_arguments g_lcr_cmd_start_args;
int callback_foreground(command_option_t *option, const char *arg)
{
    struct lcr_arguments *args = (struct lcr_arguments *)option->data;
    if (arg == NULL) {
        return 0;
    }
    if (!strcmp(arg, "true")) {
        args->start_daemonize = false;
    } else if (!strcmp(arg, "false")) {
        args->start_daemonize = true;
    } else {
        return -1;
    }
    return 0;
}
int cmd_start_main(int argc, const char **argv)
{
    command_t cmd;
    struct command_option options[] = {
        START_OPTIONS(g_lcr_cmd_start_args),
        COMMON_OPTIONS(g_lcr_cmd_start_args)
    };
    struct lcr_start_request request = {0};

    lcr_arguments_init(&g_lcr_cmd_start_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]), argc, (const char **)argv,
                 g_lcr_cmd_start_desc, g_lcr_cmd_start_usage);

    if (command_parse_args(&cmd, &g_lcr_cmd_start_args.argc, &g_lcr_cmd_start_args.argv) ||
        start_checker(&g_lcr_cmd_start_args)) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(g_lcr_cmd_start_args.name, g_lcr_cmd_start_args.log_file,
                     g_lcr_cmd_start_args.log_priority,
                     g_lcr_cmd_start_args.name,
                     g_lcr_cmd_start_args.quiet,
                     LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    if (g_lcr_cmd_start_args.name == NULL) {
        fprintf(stderr, "missing container name, use -n,--name option\n");
        exit(EXIT_FAILURE);
    }

    request.name = g_lcr_cmd_start_args.name;
    request.lcrpath = g_lcr_cmd_start_args.lcrpath;
    request.logpath = g_lcr_cmd_start_args.log_file;
    request.loglevel = g_lcr_cmd_start_args.log_priority;
    request.daemonize = g_lcr_cmd_start_args.start_daemonize;
    request.tty = true;
    request.open_stdin = true;
    request.pidfile = g_lcr_cmd_start_args.start_pidfile;
    request.console_fifos = g_lcr_cmd_start_args.console_fifos;
    request.console_logpath = g_lcr_cmd_start_args.console_logpath;

    if (!lcr_start(&request)) {
        ERROR("Failed to start container %s", g_lcr_cmd_start_args.name);
        exit(EXIT_FAILURE);
    }

    if (g_lcr_cmd_start_args.start_daemonize) {
        fprintf(stdout, "Container \"%s\" started\n", g_lcr_cmd_start_args.name);
    }
    exit(EXIT_SUCCESS);
}

static bool check_start_arguments(const struct lcr_arguments *args)
{
    return (args->console_fifos[0] && !args->console_fifos[1]) ||
           (!args->console_fifos[0] && args->console_fifos[1]);
}

int start_checker(const struct lcr_arguments *args)
{
    if (check_start_arguments(args)) {
        fprintf(stderr, "Should specify the input and output FIFOs at the same time\n");
        return -1;
    }

    return 0;
}
