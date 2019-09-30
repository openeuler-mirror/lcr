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
 * Description: provide container list definition
 ******************************************************************************/
#ifndef __CMD_LIST_H
#define __CMD_LIST_H

#include "arguments.h"
#include "commander.h"

#define LIST_OPTIONS(cmdargs)                                                                                   \
    { CMD_OPT_TYPE_BOOL, false, "quiet", 'q', &(cmdargs).list_quiet, "Display only container names", NULL },    \
    { CMD_OPT_TYPE_BOOL, false, "active", 0, &(cmdargs).list_active, "List only active containers", NULL },     \
    { CMD_OPT_TYPE_BOOL, false, "running", 0, &(cmdargs).list_running, "List only running containers", NULL },  \
    { CMD_OPT_TYPE_BOOL, false, "stopped", 0, &(cmdargs).list_stopped, "List only stopped containers", NULL }

extern const char g_lcr_cmd_list_desc[];
extern struct lcr_arguments g_lcr_cmd_list_args;
int cmd_list_main(int argc, const char **argv);

#endif /* __CMD_LIST_H */
