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
 * Description: provide container spec functions
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lcrcontainer.h"
#include "lcrcontainer_extend.h"
#include "oci_runtime_spec.h"

#include "arguments.h"
#include "spec.h"
#include "log.h"
#include "utils.h"

const char g_lcr_cmd_spec_desc[] =
    "Create a new specification file.";
static const char g_lcr_cmd_spec_usage[] = "spec [command options]";

struct lcr_arguments g_lcr_cmd_spec_args;

static void determine_distribution(const struct lcr_arguments *spec_args,
                                   struct lcr_list **lcr_conf, char **seccomp_conf)
{
    char *distribution = NULL;

    if (spec_args->spec_dist != NULL) {
        distribution = util_strdup_s(spec_args->spec_dist);
    } else {
        distribution = util_strdup_s("app");
    }
    *lcr_conf = lcr_dist2spec(distribution, seccomp_conf);

    free(distribution);
}

static int check_lcr_config(const struct lcr_arguments *spec_args, const struct lcr_list *lcr_conf)
{
    if (lcr_conf == NULL) {
        if (spec_args->spec_dist != NULL) {
            ERROR("Create distribution specific configuration failed");
        } else {
            ERROR("Translate oci specification to lcr configuration failed");
        }
        return -1;
    }

    return 0;
}

static int get_lcr_conf(const struct lcr_arguments *spec_args,
                        struct lcr_list **lcr_conf, char **seccomp_conf)
{
    int ret = -1;
    oci_runtime_spec *container = NULL;

    if (spec_args->spec_translate == NULL) {
        determine_distribution(spec_args, lcr_conf, seccomp_conf);
    } else {
        if (!container_parse(spec_args->spec_translate, NULL, &container)) {
            ERROR("Failed to parse container!");
            goto out;
        }
        *lcr_conf = lcr_oci2lcr(NULL, NULL, container, seccomp_conf);
    }

    if (check_lcr_config(spec_args, *lcr_conf) != 0) {
        goto out;
    }

    ret = 0;

out:
    free_oci_runtime_spec(container);
    return ret;
}

static int get_lcr_fake_path_name(const char *bundle, char **fake_path,
                                  char **fake_name)
{
    int ret = -1;
    size_t len = 0;
    size_t slash_index = 0;

    len = strlen(bundle);
    for (slash_index = len - 1; slash_index > 0;
         slash_index--) {
        if (bundle[slash_index] == '/') {
            break;
        }
    }
    *fake_path = util_common_calloc_s(slash_index + 1);
    if (*fake_path == NULL) {
        goto out;
    }

    if (strncpy_s(*fake_path, slash_index + 1, bundle, slash_index) != EOK) {
        ERROR("Failed to copy string!");
        goto out;
    }
    (*fake_path)[slash_index] = '\0';

    *fake_name = util_common_calloc_s((len - slash_index) + 1);
    if (*fake_name == NULL) {
        goto out;
    }

    if (strncpy_s(*fake_name, (len - slash_index) + 1, &(bundle[slash_index + 1]),
                  len - slash_index) != EOK) {
        ERROR("Failed to copy string!");
        goto out;
    }
    (*fake_name)[(len - slash_index) - 1] = '\0';

    ret = 0;
out:
    return ret;
}

static int check_spec_dist_and_translate()
{
    if ((g_lcr_cmd_spec_args.spec_dist != NULL) && (g_lcr_cmd_spec_args.spec_translate != NULL)) {
        ERROR("-t can't be used with --dist");
        return -1;
    }

    return 0;
}

static int get_spec_bundle(char *bundle, size_t len)
{
    if (g_lcr_cmd_spec_args.spec_bundle == NULL) {
        if (getcwd(bundle, len) == NULL) {
            ERROR("getcwd failed");
            return -1;
        }
    } else if (strlen(g_lcr_cmd_spec_args.spec_bundle) >= PATH_MAX ||
               realpath(g_lcr_cmd_spec_args.spec_bundle, bundle) == NULL) {
        ERROR("failed to get absolute path '%s'\n", g_lcr_cmd_spec_args.spec_bundle);
        return -1;
    }

    return 0;
}

int cmd_spec_main(int argc, const char **argv)
{
    int ret = -1;
    char *fakepath = NULL;
    char *fakename = NULL;
    char *seccomp = NULL;
    char bundle[PATH_MAX] = { 0 };
    struct lcr_list *lcr_conf = NULL;
    command_t cmd;
    struct command_option options[] = {
        SPEC_OPTIONS(g_lcr_cmd_spec_args),
        COMMON_OPTIONS(g_lcr_cmd_spec_args)
    };

    lcr_arguments_init(&g_lcr_cmd_spec_args);

    command_init(&cmd, options, sizeof(options) / sizeof(options[0]), argc,
                 (const char **)argv, g_lcr_cmd_spec_desc, g_lcr_cmd_spec_usage);
    if (command_parse_args(&cmd, &g_lcr_cmd_spec_args.argc, &g_lcr_cmd_spec_args.argv)) {
        exit(EINVALIDARGS);
    }

    if (lcr_log_init(NULL, g_lcr_cmd_spec_args.log_file, g_lcr_cmd_spec_args.log_priority,
                     g_lcr_cmd_spec_args.progname, g_lcr_cmd_spec_args.quiet, LOGPATH)) {
        exit(EXIT_FAILURE);
    }

    if (check_spec_dist_and_translate() != 0) {
        exit(EXIT_FAILURE);
    }

    if (get_spec_bundle(bundle, PATH_MAX) != 0) {
        exit(EXIT_FAILURE);
    }

    if (get_lcr_fake_path_name(bundle, &fakepath, &fakename) ||
        get_lcr_conf(&g_lcr_cmd_spec_args, &lcr_conf, &seccomp)) {
        goto out;
    }

    if (!lcr_save_spec(fakename, fakepath, lcr_conf, seccomp)) {
        goto out;
    }
    ret = 0;
out:
    lcr_free_config(lcr_conf);
    free(lcr_conf);
    free(seccomp);
    free(fakepath);
    free(fakename);
    return ret;
}
