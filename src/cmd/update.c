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
 * Description: provide container update functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <securec.h>

#include "lcrcontainer.h"
#include "update.h"
#include "arguments.h"
#include "log.h"
#include "utils.h"

#define BLKIOWEIGHT 1
#define CPUSHARES 2
#define CPUPERIOD 3
#define CPUQUOTA 4
#define CPUSETCPUS 5
#define CPUSETMEMS 6
#define KERNELMEMORY 7
#define MEMORYRESERV 8
#define MEMORYSWAP 9

const char g_lcr_cmd_update_desc[] = "Update configuration of a container";
static const char g_lcr_cmd_update_usage[] = "update --name=NAME";

struct lcr_arguments g_lcr_cmd_update_args;

static void to_cgroup_cpu_resources(const struct lcr_arguments *args, struct lcr_cgroup_resources *cr)
{
    if (args->cr.blkio_weight) {
        cr->blkio_weight = strtoull(args->cr.blkio_weight, NULL, 10);
    }
    if (args->cr.cpu_shares) {
        cr->cpu_shares = strtoull(args->cr.cpu_shares, NULL, 10);
    }
    if (args->cr.cpu_period) {
        cr->cpu_period = strtoull(args->cr.cpu_period, NULL, 10);
    }
    if (args->cr.cpu_quota) {
        cr->cpu_quota = strtoull(args->cr.cpu_quota, NULL, 10);
    }
    if (args->cr.cpuset_cpus) {
        cr->cpuset_cpus = args->cr.cpuset_cpus;
    }
    if (args->cr.cpuset_mems) {
        cr->cpuset_mems = args->cr.cpuset_mems;
    }
}

static void to_cgroup_mem_resources(const struct lcr_arguments *args, struct lcr_cgroup_resources *cr)
{
    if (args->cr.kernel_memory_limit) {
        cr->kernel_memory_limit = strtoull(args->cr.kernel_memory_limit, NULL, 10);
    }
    if (args->cr.memory_reservation) {
        cr->memory_reservation = strtoull(args->cr.memory_reservation, NULL, 10);
    }
    if (args->cr.memory_limit) {
        cr->memory_limit = strtoull(args->cr.memory_limit, NULL, 10);
    }
    if (args->cr.memory_swap) {
        cr->memory_swap = strtoull(args->cr.memory_swap, NULL, 10);
    }
}

static void to_cgroup_resources(const struct lcr_arguments *args, struct lcr_cgroup_resources *cr)
{
    if (args == NULL || cr == NULL) {
        return;
    }

    to_cgroup_cpu_resources(args, cr);

    to_cgroup_mem_resources(args, cr);
}

int cmd_update_main(int argc, const char **argv)
{
    command_t cmd;
    struct lcr_cgroup_resources cr = { 0 };
    struct command_option options[] = { UPDATE_OPTIONS(g_lcr_cmd_update_args), COMMON_OPTIONS(g_lcr_cmd_update_args) };

    lcr_arguments_init(&g_lcr_cmd_update_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]), argc, (const char **)argv, g_lcr_cmd_update_desc,
                 g_lcr_cmd_update_usage);

    if (command_parse_args(&cmd, &g_lcr_cmd_update_args.argc, &g_lcr_cmd_update_args.argv)) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(g_lcr_cmd_update_args.name, g_lcr_cmd_update_args.log_file, g_lcr_cmd_update_args.log_priority,
                     g_lcr_cmd_update_args.progname, g_lcr_cmd_update_args.quiet, LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    if (g_lcr_cmd_update_args.name == NULL) {
        fprintf(stderr, "missing --name,-n option\n");
        exit(EXIT_FAILURE);
    }

    to_cgroup_resources(&g_lcr_cmd_update_args, &cr);
    if (!lcr_update(g_lcr_cmd_update_args.name, g_lcr_cmd_update_args.lcrpath, &cr)) {
        fprintf(stderr, "Error update container %s\n", g_lcr_cmd_update_args.name);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
