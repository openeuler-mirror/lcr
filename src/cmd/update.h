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
 * Description: provide container update definition
 ******************************************************************************/
#ifndef __CMD_UPDATE_H
#define __CMD_UPDATE_H

#include "arguments.h"
#include "commander.h"

#define UPDATE_OPTIONS(cmdargs) \
    { CMD_OPT_TYPE_STRING, false, "name", 'n', &(cmdargs).name, "Name of the container", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "blkio-weight", 0, &(cmdargs).cr.blkio_weight, \
        "Block IO (relative weight), between 10 and 1000", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "cpu-shares", 0, &(cmdargs).cr.cpu_shares, "CPU shares (relative weight)", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "cpu-period", 0, &(cmdargs).cr.cpu_period, \
        "Limit CPU CFS (Completely Fair Scheduler) period", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "cpu-quota", 0, &(cmdargs).cr.cpu_quota, \
        "Limit CPU CFS (Completely Fair Scheduler) quota", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "cpuset-cpus", 0, &(cmdargs).cr.cpuset_cpus, \
        "CPUs in which to allow execution (0-3, 0,1)", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "cpuset-mems", 0, &(cmdargs).cr.cpuset_mems, \
        "MEMs in which to allow execution (0-3, 0,1)", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "kernel-memory", 0, &(cmdargs).cr.kernel_memory_limit, \
        "Kernel memory limit", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "memory", 'm', &(cmdargs).cr.memory_limit, "Memory limit", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "memory-reservation", 0, &(cmdargs).cr.memory_reservation, \
        "Memory soft limit", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "memory-swap", 0, &(cmdargs).cr.memory_swap, \
        "Swap limit equal to memory plus swap: '-1' to enable unlimited swap", NULL }

extern const char g_lcr_cmd_update_desc[];
extern struct lcr_arguments g_lcr_cmd_update_args;
int cmd_update_main(int argc, const char **argv);

#endif /* __CMD_UPDATE_H */
