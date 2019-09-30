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
 * Description: provide lcr container functions
 ******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "create.h"
#include "delete.h"
#include "spec.h"
#include "list.h"
#include "start.h"
#include "clean.h"
#include "exec.h"
#include "state.h"
#include "pause.h"
#include "resume.h"
#include "update.h"
#include "help.h"
#include "kill.h"


// The list of our supported commands
struct command g_commands[] = {
    {
        // `create` sub-command
        "create",
        cmd_create_main,
        g_lcr_cmd_create_desc,
        NULL,
        &g_lcr_cmd_create_args
    },
    {
        // `delete` sub-command
        "delete",
        cmd_delete_main,
        g_lcr_cmd_delete_desc,
        NULL,
        &g_lcr_cmd_delete_args
    },
    {
        // `spec` sub-command
        "spec",
        cmd_spec_main,
        g_lcr_cmd_spec_desc,
        NULL,
        &g_lcr_cmd_spec_args
    },
    {
        // `list` sub-command
        "list",
        cmd_list_main,
        g_lcr_cmd_list_desc,
        NULL,
        &g_lcr_cmd_list_args
    },
    {
        // `start` sub-command
        "start",
        cmd_start_main,
        g_lcr_cmd_start_desc,
        NULL,
        &g_lcr_cmd_start_args
    },
    {
        // `kill` sub-command
        "kill",
        cmd_kill_main,
        g_lcr_cmd_kill_desc,
        NULL,
        &g_lcr_cmd_kill_args
    },

    {
        // `clean` sub-command
        "clean",
        cmd_clean_main,
        g_lcr_cmd_clean_desc,
        NULL,
        &g_lcr_cmd_clean_args
    },
    {
        // `exec` sub-command
        "exec",
        cmd_exec_main,
        g_lcr_cmd_exec_desc,
        NULL,
        &g_lcr_cmd_exec_args
    },
    {
        // `state` sub-command
        "state",
        cmd_state_main,
        g_lcr_cmd_state_desc,
        NULL,
        &g_lcr_cmd_state_args
    },
    {
        // `pause` sub-command
        "pause",
        cmd_pause_main,
        g_lcr_cmd_pause_desc,
        NULL,
        &g_lcr_cmd_pause_args
    },
    {
        // `resume` sub-command
        "resume",
        cmd_resume_main,
        g_lcr_cmd_resume_desc,
        NULL,
        &g_lcr_cmd_resume_args
    },
    {
        // `update` sub-command
        "update",
        cmd_update_main,
        g_lcr_cmd_update_desc,
        NULL,
        &g_lcr_cmd_update_args
    },
    {
        // `help` sub-command
        "help",
        NULL,
        g_lcr_cmd_help_desc,
        g_lcr_cmd_help_long_desc,
        NULL
    },
    { NULL, NULL, NULL, NULL, NULL } // End of the list
};

int main(int argc, const char **argv)
{
    return run_command(g_commands, argc, argv);
}
