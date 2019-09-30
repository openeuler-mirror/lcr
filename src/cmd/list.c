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
 * Description: provide container list functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "lcrcontainer.h"
#include "lcrcontainer_extend.h"

#include "arguments.h"
#include "list.h"
#include "utils.h"
#include "log.h"

/* keep track of field widths for printing. */
struct lcr_lens {
    unsigned int lcr_name_len;
    unsigned int lcr_state_len;
    unsigned int lcr_interface_len;
    unsigned int lcr_ipv4_len;
    unsigned int lcr_ipv6_len;
    unsigned int lcr_init_len;
    unsigned int lcr_ram_len;
    unsigned int lcr_swap_len;
};

const char g_lcr_cmd_list_desc[] = "lists containers";
static const char g_lcr_cmd_list_usage[] = "list [command options]";

static void info_field_width(const struct lcr_container_info *info, const size_t size, struct lcr_lens *l);
static void info_print_table(const struct lcr_container_info *info, size_t size, const struct lcr_lens *length,
                             const struct lcr_arguments *args);
static void info_print_quiet(const struct lcr_container_info *info, size_t size, const struct lcr_arguments *args);

struct lcr_arguments g_lcr_cmd_list_args = { 0 };

int cmd_list_main(int argc, const char **argv)
{
    command_t cmd;
    struct command_option options[] = { LIST_OPTIONS(g_lcr_cmd_list_args), COMMON_OPTIONS(g_lcr_cmd_list_args) };

    lcr_arguments_init(&g_lcr_cmd_list_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]), argc, (const char **)argv, g_lcr_cmd_list_desc,
                 g_lcr_cmd_list_usage);
    if (command_parse_args(&cmd, &g_lcr_cmd_list_args.argc, &g_lcr_cmd_list_args.argv)) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(NULL, g_lcr_cmd_list_args.log_file, g_lcr_cmd_list_args.log_priority, g_lcr_cmd_list_args.progname,
                     g_lcr_cmd_list_args.quiet, LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    struct lcr_lens max_len = {
        .lcr_name_len = 4, /* NAME */
        .lcr_state_len = 5, /* STATE */
        .lcr_interface_len = 9, /* INTERFACE */
        .lcr_ipv4_len = 4, /* IPV4 */
        .lcr_ipv6_len = 4, /* IPV6 */
        .lcr_init_len = 3, /* PID */
        .lcr_ram_len = 3, /* RAM */
        .lcr_swap_len = 4, /* SWAP */
    };

    struct lcr_container_info *info_arr = NULL;
    int num;

    DEBUG("Get the container information");
    if (g_lcr_cmd_list_args.list_active) {
        num = lcr_list_active_containers(g_lcr_cmd_list_args.lcrpath, &info_arr);
    } else {
        num = lcr_list_all_containers(g_lcr_cmd_list_args.lcrpath, &info_arr);
    }

    if (num == -1) {
        exit(EXIT_FAILURE);
    }

    if (g_lcr_cmd_list_args.list_quiet == true) {
        info_print_quiet(info_arr, (size_t)num, &g_lcr_cmd_list_args);
    } else {
        info_field_width(info_arr, (size_t)num, &max_len);
        info_print_table(info_arr, (size_t)num, &max_len, &g_lcr_cmd_list_args);
    }

    lcr_containers_info_free(&info_arr, (size_t)num);
    exit(EXIT_SUCCESS);
}

static bool should_skip_print_info(const struct lcr_arguments *args, const struct lcr_container_info *in)
{
    return (args->list_running == true && strncmp(in->state, "RUNNING", 7)) ||
           (args->list_stopped == true && strncmp(in->state, "STOPPED", 7));
}

static void info_print_quiet(const struct lcr_container_info *info, size_t size, const struct lcr_arguments *args)
{
    if (size == 0) {
        return;
    }
    const struct lcr_container_info *in = NULL;
    size_t i = 0;
    for (i = 0, in = info; i < size; i++, in++) {
        if (should_skip_print_info(args, in)) {
            continue;
        }
        printf("%s\n", in->name ? in->name : "-");
    }
}

static void info_print_table(const struct lcr_container_info *info, size_t size, const struct lcr_lens *length,
                             const struct lcr_arguments *args)
{
    if (size == 0) {
        return;
    }
    printf("%-*s ", (int)length->lcr_name_len, "NAME");
    printf("%-*s ", (int)length->lcr_state_len, "STATE");
    printf("%-*s ", (int)length->lcr_ipv4_len, "IPV4");
    printf("%-*s ", (int)length->lcr_ipv6_len, "IPV6");
    printf("\n");
    const struct lcr_container_info *in = NULL;
    size_t i = 0;
    for (i = 0, in = info; i < size; i++, in++) {
        if (should_skip_print_info(args, in)) {
            continue;
        }
        printf("%-*s ", (int)length->lcr_name_len, in->name ? in->name : "-");
        printf("%-*s ", (int)length->lcr_state_len, in->state ? in->state : "-");
        printf("%-*s ", (int)length->lcr_ipv4_len, in->ipv4 ? in->ipv4 : "-");
        printf("%-*s ", (int)length->lcr_ipv6_len, in->ipv6 ? in->ipv6 : "-");
        printf("\n");
    }
}

static void width_ip_and_memory(const struct lcr_container_info *in, struct lcr_lens *l)
{
    size_t len = 0;
    char buf[64];

    if (in->ipv4) {
        len = strlen(in->ipv4);
        if (len > l->lcr_ipv4_len) {
            l->lcr_ipv4_len = (unsigned int)len;
        }
    }
    if (in->ipv6) {
        len = strlen(in->ipv6);
        if (len > l->lcr_ipv6_len) {
            l->lcr_ipv6_len = (unsigned int)len;
        }
    }

    len = (size_t)sprintf_s(buf, sizeof(buf), "%.2f", in->ram);
    if (len > l->lcr_ram_len) {
        l->lcr_ram_len = (unsigned int)len;
    }

    len = (size_t)sprintf_s(buf, sizeof(buf), "%.2f", in->swap);
    if (len > l->lcr_swap_len) {
        l->lcr_swap_len = (unsigned int)len;
    }
}

static void set_info_field_max_width(const struct lcr_container_info *in, struct lcr_lens *l)
{
    size_t len;

    if (in->name) {
        len = strlen(in->name);
        if (len > l->lcr_name_len) {
            l->lcr_name_len = (unsigned int)len;
        }
    }
    if (in->state) {
        len = strlen(in->state);
        if (len > l->lcr_state_len) {
            l->lcr_state_len = (unsigned int)len;
        }
    }
    if (in->interface) {
        len = strlen(in->interface);
        if (len > l->lcr_interface_len) {
            l->lcr_interface_len = (unsigned int)len;
        }
    }

    if (in->init != -1) {
        char buf[64];
        int nret = sprintf_s(buf, sizeof(buf), "%d", in->init);
        if (nret > 0 && (size_t)nret > l->lcr_init_len) {
            l->lcr_init_len = (size_t)nret;
        }
    }

    width_ip_and_memory(in, l);
}

static void info_field_width(const struct lcr_container_info *info, const size_t size, struct lcr_lens *l)
{
    size_t i;
    const struct lcr_container_info *in = NULL;

    for (i = 0, in = info; i < size; i++, in++) {
        set_info_field_max_width(in, l);
    }
}
