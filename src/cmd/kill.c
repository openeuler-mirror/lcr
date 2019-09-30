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
 * Description: provide container kill functions
 ******************************************************************************/
#include "lcrcontainer.h"

#include "securec.h"
#include "arguments.h"
#include "kill.h"
#include "log.h"
#include "utils.h"


const char g_lcr_cmd_kill_desc[] = "Kill a container with the identifier NAME";
static const char g_lcr_cmd_kill_usage[] = "kill [command options] --name=NAME";

struct lcr_arguments g_lcr_cmd_kill_args;

static int parse_verify_signal(int *signo)
{
    *signo = util_sig_parse(g_lcr_cmd_kill_args.signal);
    if (*signo == -1) {
        fprintf(stderr, "Invalid signal: %s", g_lcr_cmd_kill_args.signal);
        return -1;
    }

    if (!util_valid_signal(*signo)) {
        fprintf(stderr, "The Linux daemon does not support signal %d", *signo);
        return -1;
    }

    return 0;
}

static inline int check_container_name()
{
    if (g_lcr_cmd_kill_args.name == NULL) {
        fprintf(stderr, "Missing container name, use -n,--name option");
        return -1;
    }

    return 0;
}
int cmd_kill_main(int argc, const char **argv)
{
    int signo;
    command_t cmd;
    struct command_option options[] = {
        KILL_OPTIONS(g_lcr_cmd_kill_args),
        COMMON_OPTIONS(g_lcr_cmd_kill_args)
    };

    lcr_arguments_init(&g_lcr_cmd_kill_args);
    g_lcr_cmd_kill_args.signal = "SIGKILL";
    command_init(&cmd, options, sizeof(options) / sizeof(options[0]), argc, (const char **)argv,
                 g_lcr_cmd_kill_desc, g_lcr_cmd_kill_usage);
    if (command_parse_args(&cmd, &g_lcr_cmd_kill_args.argc, &g_lcr_cmd_kill_args.argv)) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(g_lcr_cmd_kill_args.name, g_lcr_cmd_kill_args.log_file,
                     g_lcr_cmd_kill_args.log_priority,
                     g_lcr_cmd_kill_args.progname,
                     g_lcr_cmd_kill_args.quiet,
                     LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    if (check_container_name() != 0) {
        exit(EXIT_FAILURE);
    }

    if (parse_verify_signal(&signo) != 0) {
        exit(EXIT_FAILURE);
    }

    if (!lcr_kill(g_lcr_cmd_kill_args.name, g_lcr_cmd_kill_args.lcrpath, (uint32_t)signo)) {
        fprintf(stderr, "Container \"%s\" kill failed", g_lcr_cmd_kill_args.name);
        exit(EXIT_FAILURE);
    }

    printf("%s\n", g_lcr_cmd_kill_args.name);

    exit(EXIT_SUCCESS);
}
