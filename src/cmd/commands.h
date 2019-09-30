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
 * Description: provide container commands definition
 ******************************************************************************/
#ifndef __COMMAND_H
#define __COMMAND_H

#include "arguments.h"
#include "commander.h"
#include "utils.h"

// A command is described by:
// @name: The name which should be passed as a second parameter
// @executor: The function that will be executed if the command
// matches. Receives the argc of the program minus two, and
// the rest os argv
// @description: Brief description, will show in help messages
// @longdesc: Long descripton to show when you run `help <command>`
struct command {
    const char * const name;
    int (*executor)(int, const char **);
    const char * const description;
    const char * const longdesc;
    struct lcr_arguments *args;
};

// Gets a pointer to a command, to allow implementing custom behavior
// returns null if not found.
//
// NOTE: Command arrays must end in a command with all member is NULL
const struct command *command_by_name(const struct command *commands, const char * const name);

// Default help command if implementation doesn't prvide one
int commmand_default_help(const char * const program_name, struct command *commands, int argc, char **argv);

// Tries to execute a command in the command list, The command name
// must be in argv[1]. Example usage
int run_command(struct command *commands, int argc, const char **argv);
#endif /* __COMMAND_H */
