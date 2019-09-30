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
 * Description: provide container pause functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "lcrcontainer.h"

#include "arguments.h"
#include "pause.h"
#include "log.h"
#include "utils.h"

const char g_lcr_cmd_pause_desc[] = "Pause container in specified container NAME";
static const char g_lcr_cmd_pause_usage[] = "pause [command options] --name=NAME";

struct lcr_arguments g_lcr_cmd_pause_args;

int cmd_pause_main(int argc, const char **argv)
{
    command_t cmd;
    struct command_option options[] = {
        PAUSE_OPTIONS(g_lcr_cmd_pause_args),
        COMMON_OPTIONS(g_lcr_cmd_pause_args)
    };

    lcr_arguments_init(&g_lcr_cmd_pause_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]),
                 argc,
                 (const char **)argv,
                 g_lcr_cmd_pause_desc, g_lcr_cmd_pause_usage);
    if (command_parse_args(&cmd, &g_lcr_cmd_pause_args.argc, &g_lcr_cmd_pause_args.argv)) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(g_lcr_cmd_pause_args.name, g_lcr_cmd_pause_args.log_file,
                     g_lcr_cmd_pause_args.log_priority,
                     g_lcr_cmd_pause_args.progname,
                     g_lcr_cmd_pause_args.quiet,
                     LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    if (g_lcr_cmd_pause_args.name == NULL) {
        fprintf(stderr, "missing container name, use -n,--name option\n");
        exit(EXIT_FAILURE);
    }

    if (!lcr_pause(g_lcr_cmd_pause_args.name, g_lcr_cmd_pause_args.lcrpath)) {
        fprintf(stderr, "Failed to pause container %s\n", g_lcr_cmd_pause_args.name);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Container \"%s\" paused\n", g_lcr_cmd_pause_args.name);
    exit(EXIT_SUCCESS);
}
