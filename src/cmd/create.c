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
 * Description: provide container create functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "lcrcontainer.h"
#include "create.h"
#include "arguments.h"
#include "log.h"
#include "utils.h"
#include "read_file.h"

const char g_lcr_cmd_create_desc[] = "create a container";
static const char g_lcr_cmd_create_usage[] = "create --name=NAME --rootfs=<dir|blkdev>";

struct lcr_arguments g_lcr_cmd_create_args = { 0 };

static int check_create_args(struct lcr_arguments *lcr_cmd_create_args)
{
    if (lcr_cmd_create_args->name == NULL) {
        ERROR("Missing --name,-n option\n");
        return -1;
    }

    if (lcr_cmd_create_args->create_rootfs == NULL) {
        ERROR("Missing --rootfs option\n");
        return -1;
    }

    if (lcr_cmd_create_args->create_dist == NULL) {
        lcr_cmd_create_args->create_dist = "ubuntu";
    }

    if (!file_exists(lcr_cmd_create_args->create_rootfs)) {
        ERROR("Rootfs dir \"%s\" does not exist\n", lcr_cmd_create_args->create_rootfs);
        return -1;
    }

    return 0;
}

static int read_oci_json_data(char **oci_json_data, size_t *filesize)
{
    if (g_lcr_cmd_create_args.ociconfig == NULL) {
        return 0;
    }

    *oci_json_data = read_file(g_lcr_cmd_create_args.ociconfig, filesize);
    if (*oci_json_data == NULL) {
        ERROR("Can not read the file \"%s\"\n", g_lcr_cmd_create_args.ociconfig);
        return -1;
    }

    return 0;
}

int cmd_create_main(int argc, const char **argv)
{
    char *oci_json_data = NULL;
    size_t filesize;
    command_t cmd;
    struct command_option options[] = {
        CREATE_OPTIONS(g_lcr_cmd_create_args),
        COMMON_OPTIONS(g_lcr_cmd_create_args)
    };

    lcr_arguments_init(&g_lcr_cmd_create_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]), argc, (const char **)argv,
                 g_lcr_cmd_create_desc, g_lcr_cmd_create_usage);

    if (command_parse_args(&cmd, &g_lcr_cmd_create_args.argc, &g_lcr_cmd_create_args.argv)) {
        exit(EINVALIDARGS);
    }

    if (check_create_args(&g_lcr_cmd_create_args)) {
        exit(EXIT_FAILURE);
    }

    if (read_oci_json_data(&oci_json_data, &filesize) != 0) {
        exit(EXIT_FAILURE);
    }

    if (lcr_log_init(g_lcr_cmd_create_args.name,
                     g_lcr_cmd_create_args.log_file,
                     g_lcr_cmd_create_args.log_priority,
                     g_lcr_cmd_create_args.progname,
                     g_lcr_cmd_create_args.quiet,
                     LOGPATH)) {
        free(oci_json_data);
        oci_json_data = NULL;
        exit(EXIT_FAILURE);
    }

    if (!lcr_create(g_lcr_cmd_create_args.name,
                    g_lcr_cmd_create_args.lcrpath,
                    g_lcr_cmd_create_args.create_rootfs,
                    g_lcr_cmd_create_args.create_dist,
                    oci_json_data)) {
        ERROR("Error creating container %s", g_lcr_cmd_create_args.name);
        free(oci_json_data);
        oci_json_data = NULL;
        exit(EXIT_FAILURE);
    }

    INFO("Container \"%s\" created\n", g_lcr_cmd_create_args.name);
    free(oci_json_data);
    oci_json_data = NULL;
    exit(EXIT_SUCCESS);
}
