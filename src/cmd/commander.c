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
 * Description: provide container commander functions
 ******************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"
#include "commander.h"
#include "securec.h"
#include "log.h"

int compare_options(const void *s1, const void *s2)
{
    return strcmp((*(const command_option_t *)s1).large, (*(const command_option_t *)s2).large);
}

static int get_max_option_length(int options_len, const command_option_t *options)
{
    int i;
    int max_option_len = 0;

    for (i = 0; i < options_len; i++) {
        command_option_t option = options[i];
        // -s
        int len = 2;
        // -s, --large
        if (option.large != NULL) {
            len = (int)(strlen(option.large) + 6);
        }
        if (len > max_option_len) {
            max_option_len = len;
        }
    }

    return max_option_len;
}

static void do_print_options(int options_len, const command_option_t *options, int max_option_len)
{
    int i;

    for (i = 0; i < options_len; i++) {
        command_option_t option = options[i];
        int curindex;
        int space_left = 0;

        curindex = fprintf(stdout, "  ");
        if (option.small) {
            curindex += fprintf(stdout, "-%c", (char)(option.small));
        }

        if (option.large != NULL) {
            if (option.small) {
                curindex += fprintf(stdout, ", --%s", option.large);
            } else {
                curindex += fprintf(stdout, "    --%s", option.large);
            }
        }

        if (curindex <= max_option_len) {
            space_left = max_option_len - curindex;
        }

        fprintf(stdout, "%*s%s\n", space_left, "", option.description);
    }
}

void print_options(int options_len, const command_option_t *options)
{
    int max_option_len = 0;

    max_option_len = get_max_option_length(options_len, options);

    // format: "  -s, --large    description"
    max_option_len += 6;

    do_print_options(options_len, options, max_option_len);

    fputc('\n', stdout);
}

void command_help(const command_t *self)
{
    const char *progname = strrchr(self->name, '/');
    if (progname == NULL) {
        progname = self->name;
    } else {
        progname++;
    }

    fprintf(stderr, "\nUsage:  %s [options] %s\n\n", progname, self->usage);
    fprintf(stderr, "%s\n\n", self->description);
    qsort(self->options, (size_t)(self->option_count), sizeof(self->options[0]), compare_options);
    print_options(self->option_count, self->options);
}

void command_init(command_t *self, command_option_t *options, int options_len, int argc, const char **argv,
                  const char *description, const char *usage)
{
    if (memset_s(self, sizeof(command_t), 0, sizeof(command_t)) != EOK) {
        COMMAND_ERROR("Failed to set memory");
        return;
    }
    self->name = argv[0];
    self->argc = argc - 2;
    self->argv = argv + 2;
    self->usage = usage;
    self->description = description;
    self->options = options;
    self->option_count = options_len;
}

void command_option(command_t *self, command_option_type_t type, void *data, int small, const char *large,
                    const char *desc, command_callback_t cb)
{
    if (self->option_count == COMMANDER_MAX_OPTIONS) {
        COMMAND_ERROR("Maximum option definitions exceeded\n");
        exit(EINVALIDARGS);
    }
    int n = self->option_count++;
    command_option_t *option = &(self->options[n]);
    option->type = type;
    option->data = data;
    option->cb = cb;
    option->small = small;
    option->description = desc;
    option->large = large;
}

static int read_option_arg(command_t *self, command_option_t *option, const char **opt_arg, const char **readed)
{
    if ((self == NULL) || (option == NULL) || (opt_arg == NULL)) {
        return -1;
    }
    if (option->hasdata) {
        *readed = *opt_arg;
        *opt_arg = NULL;
    }
    if (!option->hasdata && self->argc > 1) {
        option->hasdata = true;
        *readed = *++(self->argv);
        self->argc--;
    }
    if (!option->hasdata) {
        COMMAND_ERROR("Flag needs an argument: --%s", option->large);
        return -1;
    }
    return 0;
}

static int handle_option_type_bool(const command_option_t *option, const char **opt_arg)
{
    if (option->hasdata && strcmp(*opt_arg, "true") && strcmp(*opt_arg, "false")) {
        COMMAND_ERROR("Invalid boolean value \"%s\" for flag --%s", *opt_arg, option->large);
        return -1;
    }
    if (option->hasdata) {
        if (!strcmp(*opt_arg, "true")) {
            *(bool *)(option->data) = true;
        } else {
            *(bool *)(option->data) = false;
        }
        *opt_arg = NULL;
    } else {
        *(bool *)option->data = true;
    }
    return 0;
}

static int handle_option_type_string(command_t *self, command_option_t *option, const char **opt_arg)
{
    if (read_option_arg(self, option, opt_arg, (const char **)(option->data))) {
        return -1;
    }
    if (option->cb != NULL) {
        return option->cb(option, *(char **)(option->data));
    }
    return 0;
}

static int handle_option_type_string_dup(command_t *self, command_option_t *option, const char **opt_arg)
{
    const char *readed = NULL;
    if (read_option_arg(self, option, opt_arg, &readed)) {
        return -1;
    }

    free(*(char **)(option->data));

    *(char **)option->data = util_strdup_s(readed);

    if (option->cb != NULL) {
        return option->cb(option, readed);
    }
    return 0;
}

static int handle_option_type_callback(command_t *self, command_option_t *option, const char **opt_arg)
{
    const char *readed = NULL;
    if (read_option_arg(self, option, opt_arg, &readed)) {
        return -1;
    }
    if (option->cb == NULL) {
        COMMAND_ERROR("Must specify callback for type array");
        return -1;
    }
    return option->cb(option, readed);
}

static int command_get_option_data(command_t *self, command_option_t *option, const char **opt_arg)
{
    if (option == NULL) {
        return -1;
    }
    switch (option->type) {
        case CMD_OPT_TYPE_BOOL:
            return handle_option_type_bool(option, opt_arg);
        case CMD_OPT_TYPE_STRING:
            return handle_option_type_string(self, option, opt_arg);
        case CMD_OPT_TYPE_STRING_DUP:
            return handle_option_type_string_dup(self, option, opt_arg);
        case CMD_OPT_TYPE_CALLBACK:
            return handle_option_type_callback(self, option, opt_arg);
        default:
            COMMAND_ERROR("Unkown command option type:%d", option->type);
            return -1;
    }
}
static int have_short_options(const command_t *self, char arg)
{
    int i;
    for (i = 0; i < self->option_count; i++) {
        if (self->options[i].small == arg) {
            return 0;
        }
    }
    return -1;
}

static void set_option_argument_when_match_flag(const char **opt_arg, command_option_t *option, bool *found)
{
    *found = true;
    if ((*opt_arg)[1] != '\0') {
        if ((*opt_arg)[1] == '=') {
            *opt_arg = *opt_arg + 2;
            option->hasdata = true;
        } else {
            *opt_arg = *opt_arg + 1;
        }
    } else {
        *opt_arg = NULL;
    }
}

static int command_parse_short_arg(command_t *self, const char *arg)
{
    const char *opt_arg = arg;
    bool found = true;
    int j;

    do {
        found = false;
        if (opt_arg[0] == 'h' && have_short_options(self, 'h') < 0) {
            command_help(self);
            exit(0);
        }
        for (j = 0; j < self->option_count; ++j) {
            command_option_t *option = &(self->options[j]);
            option->hasdata = false;

            if (option->small != opt_arg[0]) {
                continue;
            }

            // match flag
            set_option_argument_when_match_flag(&opt_arg, option, &found);

            if (command_get_option_data(self, option, &opt_arg)) {
                return -1;
            }
            break;
        }
    } while (found && (opt_arg != NULL));

    if (opt_arg != NULL) {
        COMMAND_ERROR("Unkown flag found:'%c'", opt_arg[0]);
        exit(EINVALIDARGS);
    }
    return 0;
}

static int command_parse_long_arg(command_t *self, const char *arg)
{
    int j;

    if (!strcmp(arg, "help")) {
        command_help(self);
        exit(0);
    }

    for (j = 0; j < self->option_count; ++j) {
        command_option_t *option = &(self->options[j]);
        const char *opt_arg = NULL;
        option->hasdata = false;

        if (option->large == NULL) {
            continue;
        }

        opt_arg = str_skip_str(arg, option->large);
        if (opt_arg == NULL) {
            continue;
        }

        if (opt_arg[0]) {
            if (opt_arg[0] != '=') {
                continue;
            }
            opt_arg = opt_arg + 1;
            option->hasdata = true;
        } else {
            opt_arg = NULL;
        }
        if (command_get_option_data(self, option, &opt_arg)) {
            return -1;
        }
        return 0;
    }
    COMMAND_ERROR("Unkown flag found:'--%s'\n", arg);
    exit(EINVALIDARGS);
}

int command_parse_args(command_t *self, int *argc, char * const **argv)
{
    int ret = 0;
    for (; self->argc; self->argc--, self->argv++) {
        const char *arg = self->argv[0];

        if (arg[0] != '-' || !arg[1]) {
            break;
        }

        // short option
        if (arg[1] != '-') {
            arg = arg + 1;
            ret = command_parse_short_arg(self, arg);
            if (!ret) {
                continue;
            }
            break;
        }

        // --
        if (!arg[2]) {
            self->argc--;
            self->argv++;
            break;
        }

        // long option
        arg = arg + 2;
        ret = command_parse_long_arg(self, arg);
        if (!ret) {
            continue;
        }
        break;
    }
    if (self->argc > 0) {
        *argc = self->argc;
        *argv = (char * const *)(self->argv);
    }
    return ret;
}

int command_append_array_with_space(command_option_t *option, const char *arg)
{
    int ret = 0;
    char *narg = util_string_replace(SPACE_MAGIC_STR, " ", arg);
    if (narg == NULL) {
        COMMAND_ERROR("Memory allocation error");
        return -1;
    }
    ret = util_array_append(option->data, narg);
    free(narg);
    return ret;
}

int command_append_array(command_option_t *option, const char *arg)
{
    if (option == NULL) {
        return -1;
    }
    char ***array = option->data;
    return util_array_append(array, arg);
}

int command_convert_llong(command_option_t *option, const char *arg)
{
    if (option == NULL) {
        return -1;
    }
    if (util_safe_llong(arg, option->data)) {
        COMMAND_ERROR("Invalid value \"%s\" for flag --%s", arg, option->large);
        return EINVALIDARGS;
    }
    return 0;
}

int command_convert_uint(command_option_t *option, const char *arg)
{
    if (option == NULL) {
        return -1;
    }
    if (util_safe_uint(arg, option->data)) {
        COMMAND_ERROR("Invalid value \"%s\" for flag --%s", arg, option->large);
        return EINVALIDARGS;
    }
    return 0;
}
