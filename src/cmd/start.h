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
 * Description: provide container start definition
 ******************************************************************************/
#ifndef __CMD_START_H
#define __CMD_START_H

#include "arguments.h"
#include "commander.h"

#define START_OPTIONS(cmdargs) \
    { CMD_OPT_TYPE_STRING, false, "name", 'n', &(cmdargs).name, "Name of the container", NULL }, \
    { CMD_OPT_TYPE_BOOL, false, "daemon", 'd', &(cmdargs).start_daemonize, \
        "Daemonize the container (default)", NULL }, \
    { CMD_OPT_TYPE_CALLBACK, false, "foreground", 'F', &(cmdargs).start_daemonize, \
        "Start with the current tty attached to /dev/console", callback_foreground }, \
    { CMD_OPT_TYPE_STRING, false, "pidfile", 0, &(cmdargs).start_pidfile, "Create a file with the process id", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "console_file", 'L', &(cmdargs).console_logpath, \
        "Save the console output to the file", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "in-fifo", 0, &(cmdargs).console_fifos[0], "console fifo", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "out-fifo", 0, &(cmdargs).console_fifos[1], "console fifo", NULL }

extern const char g_lcr_cmd_start_desc[];
extern struct lcr_arguments g_lcr_cmd_start_args;
int callback_foreground(command_option_t *option, const char *arg);
int start_checker(const struct lcr_arguments *args);
int cmd_start_main(int argc, const char **argv);

#endif /* __CMD_START_H */
