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
 * Description: provide container arguments functions
 ******************************************************************************/
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include  <stdbool.h>

#include "arguments.h"
#include "commands.h"
#include "utils.h"
#include "securec.h"

/* lcr arguments init */
void lcr_arguments_init(struct lcr_arguments *args)
{
    args->name = NULL;
    args->log_file = NULL;
    args->log_priority = NULL;
    args->quiet = 0;
    args->lcrpath = NULL;
    args->create_rootfs = NULL;
    args->create_dist = NULL;
    args->spec_bundle = NULL;
    args->spec_translate = NULL;
    args->spec_dist = NULL;
    args->list_quiet = false;
    args->list_running = false;
    args->list_stopped = false;
    args->list_active = false;
    args->start_daemonize = true;
    args->start_pidfile = NULL;
    args->console_logpath = NULL;
    args->console_fifos[0] = NULL;
    args->console_fifos[1] = NULL;
    args->delete_force = false;
    args->argc = 0;
    args->argv = NULL;
    args->ociconfig = NULL;
}


/* print common help */
void print_common_help()
{
    size_t len;
    struct lcr_arguments cmd_common_args = {};
    struct command_option options[] = {
        COMMON_OPTIONS(cmd_common_args)
    };
    len = sizeof(options) / sizeof(options[0]);
    qsort(options, len, sizeof(options[0]), compare_options);
    fprintf(stdout, "COMMON OPTIONS :\n");
    print_options((int)len, options);
}
