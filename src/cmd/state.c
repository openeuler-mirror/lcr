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
 * Description: provide container state functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <securec.h>

#include "lcrcontainer.h"
#include "state.h"
#include "arguments.h"
#include "log.h"
#include "utils.h"

const char g_lcr_cmd_state_desc[] = "Output the state of a container";
static const char g_lcr_cmd_state_usage[] = "state --name=NAME";

struct lcr_arguments g_lcr_cmd_state_args;

static uint64_t read_memory_info(void)
{
    uint64_t sysmem_limit = 0;
    size_t len = 0;
    char *line = NULL;
    char *p = NULL;

    FILE *fp = util_fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        ERROR("Failed to open /proc/meminfo");
        return sysmem_limit;
    }

    while (getline(&line, &len, fp) != -1) {
        p = strchr(line, ' ');
        if (p == NULL) {
            goto out;
        }
        *p = '\0';
        p++;
        if (strcmp(line, "MemTotal:") == 0) {
            while (*p != '\0' && (*p == ' ' || *p == '\t')) {
                p++;
            }
            if (*p == '\0') {
                goto out;
            }
            sysmem_limit = strtoull(p, NULL, 0);
            break;
        }
    }

out:
    fclose(fp);
    free(line);
    return sysmem_limit * SIZE_KB;
}

static void size_humanize(unsigned long long val, char *buf, size_t bufsz)
{
    errno_t ret;

    if (val > 1 << 30) {
        ret = sprintf_s(buf, bufsz, "%u.%2.2u GiB",
                        (unsigned int)(val >> 30),
                        (unsigned int)((val & ((1 << 30) - 1)) * 100) >> 30);
    } else if (val > 1 << 20) {
        unsigned long long x = val + 5243;  /* for rounding */
        ret = sprintf_s(buf, bufsz, "%u.%2.2u MiB",
                        (unsigned int)(x >> 20), (unsigned int)(((x & ((1 << 20) - 1)) * 100) >> 20));
    } else if (val > 1 << 10) {
        unsigned long long x = val + 5;  /* for rounding */
        ret = sprintf_s(buf, bufsz, "%u.%2.2u KiB",
                        (unsigned int)(x >> 10), (unsigned int)(((x & ((1 << 10) - 1)) * 100) >> 10));
    } else {
        ret = sprintf_s(buf, bufsz, "%u bytes", (unsigned int)val);
    }

    if (ret < 0) {
        ERROR("Failed to sprintf string");
        return;
    }
}

static void print_state(const struct lcr_container_state *lcs)
{
    char buf[BUFSIZE];

    fprintf(stdout, "%-15s %s\n", "Name:", lcs->name);
    fprintf(stdout, "%-15s %s\n", "State:", lcs->state);
    if (strcmp(lcs->state, "RUNNING") != 0) {
        return;
    }

    fprintf(stdout, "%-15s %d\n", "PID:", lcs->init);
    fprintf(stdout, "%-15s %.2f seconds\n", "CPU use:",
            (double)lcs->cpu_use_nanos / 1000000000.0);

    fprintf(stdout, "%-15s %llu\n", "Pids current:",
            (unsigned long long)lcs->pids_current);

    size_humanize(lcs->mem_used, buf, sizeof(buf));
    fprintf(stdout, "%-15s %s\n", "Memory use:", buf);

    size_humanize(lcs->mem_limit, buf, sizeof(buf));
    fprintf(stdout, "%-15s %s\n", "Memory limit:", buf);

    size_humanize(lcs->kmem_used, buf, sizeof(buf));
    fprintf(stdout, "%-15s %s\n", "KMem use:", buf);

    size_humanize(lcs->kmem_limit, buf, sizeof(buf));
    fprintf(stdout, "%-15s %s\n", "KMem limit:", buf);

    size_humanize(lcs->io_service_bytes.read, buf, sizeof(buf));
    fprintf(stdout, "%-15s %s\n", "Blkio read:", buf);

    size_humanize(lcs->io_service_bytes.write, buf, sizeof(buf));
    fprintf(stdout, "%-15s %s\n", "Blkio write:", buf);
}

static inline int check_container_name()
{
    if (g_lcr_cmd_state_args.name == NULL) {
        fprintf(stderr, "missing --name,-n option\n");
        return -1;
    }

    return 0;
}

static void set_sysmem_limit(struct lcr_container_state *state)
{
    uint64_t sysmem_limit = 0;

    sysmem_limit = read_memory_info();
    if (sysmem_limit > 0 && state->mem_limit > sysmem_limit) {
        state->mem_limit = sysmem_limit;
    }
    if (sysmem_limit > 0 && state->kmem_limit > sysmem_limit) {
        state->kmem_limit = sysmem_limit;
    }
}

int cmd_state_main(int argc, const char **argv)
{
    struct lcr_container_state state = { 0 };
    command_t cmd;
    struct command_option options[] = {
        STATE_OPTIONS(g_lcr_cmd_state_args),
        COMMON_OPTIONS(g_lcr_cmd_state_args)
    };

    lcr_arguments_init(&g_lcr_cmd_state_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]), argc, (const char **)argv,
                 g_lcr_cmd_state_desc, g_lcr_cmd_state_usage);
    if (command_parse_args(&cmd, &g_lcr_cmd_state_args.argc, &g_lcr_cmd_state_args.argv)) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(g_lcr_cmd_state_args.name,
                     g_lcr_cmd_state_args.log_file,
                     g_lcr_cmd_state_args.log_priority,
                     g_lcr_cmd_state_args.progname,
                     g_lcr_cmd_state_args.quiet,
                     LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    if (check_container_name() != 0) {
        exit(EXIT_FAILURE);
    }

    if (!lcr_state(g_lcr_cmd_state_args.name, g_lcr_cmd_state_args.lcrpath, &state)) {
        fprintf(stderr, "Error get the container \"%s\"'s state\n", g_lcr_cmd_state_args.name);
        exit(EXIT_FAILURE);
    }

    set_sysmem_limit(&state);

    print_state(&state);
    lcr_container_state_free(&state);
    exit(EXIT_SUCCESS);
}
