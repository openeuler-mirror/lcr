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
 * Description: provide container resume functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "lcrcontainer.h"

#include "arguments.h"
#include "resume.h"
#include "utils.h"
#include "log.h"

const char g_lcr_cmd_resume_desc[] = "Resume container in specified container NAME";
static const char g_lcr_cmd_resume_usage[] = "resume [command options] --name=NAME";

struct lcr_arguments g_lcr_cmd_resume_args;

int cmd_resume_main(int argc, const char **argv)
{
    command_t cmd;
    struct command_option options[] = { RESUME_OPTIONS(g_lcr_cmd_resume_args), COMMON_OPTIONS(g_lcr_cmd_resume_args) };

    lcr_arguments_init(&g_lcr_cmd_resume_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]), argc, (const char **)argv, g_lcr_cmd_resume_desc,
                 g_lcr_cmd_resume_usage);
    if (command_parse_args(&cmd, &g_lcr_cmd_resume_args.argc, &g_lcr_cmd_resume_args.argv)) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(g_lcr_cmd_resume_args.name, g_lcr_cmd_resume_args.log_file, g_lcr_cmd_resume_args.log_priority,
                     g_lcr_cmd_resume_args.progname, g_lcr_cmd_resume_args.quiet, LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    if (g_lcr_cmd_resume_args.name == NULL) {
        fprintf(stderr, "missing container name, use -n,--name option\n");
        exit(EXIT_FAILURE);
    }

    if (!lcr_resume(g_lcr_cmd_resume_args.name, g_lcr_cmd_resume_args.lcrpath)) {
        fprintf(stderr, "Failed to resume container %s\n", g_lcr_cmd_resume_args.name);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Container \"%s\" resumed\n", g_lcr_cmd_resume_args.name);
    exit(EXIT_SUCCESS);
}
