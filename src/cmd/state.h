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
 * Description: provide container state definition
 ******************************************************************************/
#ifndef __CMD_STATE_H
#define __CMD_STATE_H

#include "arguments.h"
#include "commander.h"

#define STATE_OPTIONS(cmdargs) \
    { CMD_OPT_TYPE_STRING, false, "name", 'n', &(cmdargs).name, "Name of the container", NULL }

extern const char g_lcr_cmd_state_desc[];
extern struct lcr_arguments g_lcr_cmd_state_args;
int cmd_state_main(int argc, const char **argv);

#endif /* __CMD_STATE_H */
