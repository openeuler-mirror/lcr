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
 * Description: provide container commander definition
 ******************************************************************************/
#ifndef __COMMANDER_H_
#define __COMMANDER_H_
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef COMMANDER_MAX_OPTIONS
#define COMMANDER_MAX_OPTIONS 64
#endif

typedef enum {
    /* no arguments */
    CMD_OPT_TYPE_BOOL,
    /* required arguments */
    CMD_OPT_TYPE_STRING,
    CMD_OPT_TYPE_STRING_DUP,
    CMD_OPT_TYPE_CALLBACK
} command_option_type_t;

struct _command;

struct command_option;

typedef int (*command_callback_t)(struct command_option *options, const char *arg);

typedef struct command_option {
    command_option_type_t type;
    bool hasdata;
    const char *large;
    int small;
    void *data;
    const char *description;
    command_callback_t cb;
} command_option_t;

typedef struct _command {
    const char *type;
    const char *usage;
    const char *description;
    const char *name;
    const char *version;
    int option_count;
    command_option_t *options;
    int argc;
    const char **argv;
} command_t;

void command_init(command_t *self, command_option_t *options, int options_len, int argc, const char **argv,
                  const char *description, const char *usage);

int compare_options(const void *s1, const void *s2);

void print_options(int options_len, const command_option_t *options);

void command_help(const command_t *self);

void command_option(command_t *self, command_option_type_t type, void *data, int small, const char *large,
                    const char *desc, command_callback_t cb);

int command_parse_args(command_t *self, int *argc, char * const **argv);

int command_append_array(command_option_t *option, const char *arg);

int command_append_array_with_space(command_option_t *option, const char *arg);

int command_convert_llong(command_option_t *option, const char *arg);

int command_convert_uint(command_option_t *option, const char *arg);

#endif /* COMMANDER_H */
