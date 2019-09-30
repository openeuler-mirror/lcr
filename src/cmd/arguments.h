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
 * Description: provide container arguments definition
 ******************************************************************************/
#ifndef __LCR_ARGUMENTS_H
#define __LCR_ARGUMENTS_H

#include <stdbool.h>
#include <getopt.h>
#include <stdio.h>

struct lcr_arguments;

struct args_cgroup_resources {
    char *blkio_weight;
    char *cpu_shares;
    char *cpu_period;
    char *cpu_quota;
    char *cpuset_cpus;
    char *cpuset_mems;
    char *memory_limit;
    char *memory_swap;
    char *memory_reservation;
    char *kernel_memory_limit;
};

struct lcr_arguments {
    const char *progname; /* sub command name */

    // For common options
    char *name; /*container name */
    char *log_file;
    char *log_priority;
    int quiet;

    char *lcrpath;

    // lcr create
    char *create_rootfs;
    char *create_dist;
    char *ociconfig;

    // lcr run
    // lcr spec
    char *spec_bundle;
    char *spec_translate;
    char *spec_dist;
    // lcr list
    bool list_quiet;
    bool list_active;
    bool list_running;
    bool list_stopped;
    // lcr start
    bool start_daemonize;
    char *start_pidfile;
    char *console_logpath;
    const char *console_fifos[2];

    // lcr kill
    char *signal;

    // lcr delete
    bool delete_force;
    // lcr update
    struct args_cgroup_resources cr;

    // remaining arguments
    char * const * argv;
    int argc;
};

#define COMMON_OPTIONS(cmdargs) \
    { CMD_OPT_TYPE_STRING, false, "logfile", 'o', &(cmdargs).log_file, \
        "Set the log file path wherer debug information is written", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "logpriority", 'l', &(cmdargs).log_priority, "Set log priority to LEVEL", NULL }, \
    { CMD_OPT_TYPE_BOOL, false, "silence", 's', &(cmdargs).quiet, "Don't produce any output to stderr", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "lcrpath", 'P', &(cmdargs).lcrpath, "Use specified container path", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "help", 0, NULL, "Show help", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "version", 0, NULL, "Print the version", NULL }


extern void print_common_help();

extern void lcr_arguments_init(struct lcr_arguments *args);

#define lcr_print_error(arg, fmt, args...) \
    fprintf(stderr, "%s: " fmt "\n", (arg)->progname, ## args)

#endif /*__LCR_ARGUMENTS_H*/
