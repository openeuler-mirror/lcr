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
 * Description: provide container commands functions
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "securec.h"
#include "utils.h"
#include "log.h"
#include "commands.h"
#include "arguments.h"
#include "config.h" // For VERSION

const char cmd_option_desc[] = "Options";
const char cmd_option_usage[] = "[OPTIONS] COMMAND [arg...]";
struct lcr_arguments g_lcr_cmd_args = {};

/* print version */
static void print_version()
{
    printf("Version %s, commit %s\n", VERSION, LCR_GIT_COMMIT);
}

/*command by name*/
const struct command *command_by_name(const struct command *commands, const char * const name)
{
    size_t i = 0;

    if (commands == NULL) {
        return NULL;
    }

    while (1) {
        if (!commands[i].name) {
            return NULL;
        }

        if (strcmp(commands[i].name, name) == 0) {
            return commands + i;
        }

        ++i;
    }
}

/* compare commands */
int compare_commands(const void *s1, const void *s2)
{
    return strcmp((*(const struct command *)s1).name, (*(const struct command *)s2).name);
}
// Default help command if implementation doesn't provide one
int command_default_help(const char * const program_name, struct command *commands, int argc, const char **argv)
{
    const struct command *command = NULL;

    if (commands == NULL) {
        return 1;
    }

    if (argc == 0) {
        size_t i = 0;
        size_t max_size = 0;

        printf("USAGE:\n");
        printf("\t%s [OPTIONS] COMMAND [args...]\n", program_name);
        printf("\n");
        printf("COMMANDS:\n");

        for (i = 0; commands[i].name != NULL; i++) {
            size_t cmd_size = strlen(commands[i].name);
            if (cmd_size > max_size) {
                max_size = cmd_size;
            }
        }
        qsort(commands, i, sizeof(commands[0]), compare_commands);
        for (i = 0; commands[i].name != NULL; i++) {
            printf("\t%*s\t%s\n", -(int)max_size, commands[i].name, commands[i].description);
        }

        printf("\n");
        print_common_help();
        return 0;
    } else if (argc > 1) {
        printf("%s: unrecognized argument: \"%s\"\n", program_name, argv[1]);
        return 1;
    }

    command = command_by_name(commands, argv[0]);

    if (command == NULL) {
        printf("%s: sub-command \"%s\" not found\n", program_name, argv[0]);
        printf("run `lcr --help` for a list of sub-commonds\n");
        return 1;
    }

    if (command->longdesc != NULL) {
        printf("%s\n", command->longdesc);
    }
    return 0;
}

/* option command init */
void option_command_init(command_t *self, command_option_t *options, int options_len, int argc, const char **argv,
                         const char *description, const char *usage)
{
    if (memset_s(self, sizeof(command_t), 0, sizeof(command_t)) != EOK) {
        COMMAND_ERROR("Failed to set memory");
        return;
    }
    self->name = argv[0];
    self->argc = argc - 1;
    self->argv = argv + 1;
    self->usage = usage;
    self->description = description;
    self->options = options;
    self->option_count = options_len;
}
// Tries to execute a command in the command list. The command name
// must be in argv[1]. Example usage:
//
int run_command(struct command *commands, int argc, const char **argv)
{
    const struct command *command = NULL;

    if (argc == 1) {
        return command_default_help(argv[0], commands, argc - 1, argv + 1);
    }

    if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        return command_default_help(argv[0], commands, argc - 2, argv + 2);
    }

    if (strcmp(argv[1], "--version") == 0) {
        print_version();
        return 0;
    }

    command = command_by_name(commands, argv[1]);
    if (command != NULL) {
        return command->executor(argc, argv);
    }

    printf("%s: command \"%s\" not found\n", argv[0], argv[1]);
    printf("run `%s --help` or `run -h` for a list of sub-commonds\n", argv[0]);
    return 1;
}
