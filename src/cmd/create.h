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
 * Description: provide container create definition
 ******************************************************************************/
#ifndef __CMD_CREATE_H
#define __CMD_CREATE_H

#include "arguments.h"
#include "commander.h"

#define CREATE_OPTIONS(cmdargs) \
    {CMD_OPT_TYPE_STRING, false, "dist", 0, &(cmdargs).create_dist, \
        "Generate distribution specification, now support: `ubuntu`, `app`," \
        " `none`\n\t\t\tthe default dist is `ubuntu`\n\t\t\tNOTE: if the dist is `none`," \
        " it will not create default spec.", NULL}, \
    {CMD_OPT_TYPE_STRING, false, "name", 'n', &(cmdargs).name, "Name of the container", NULL}, \
    {CMD_OPT_TYPE_STRING, false, "ociconfig", 'c', &(cmdargs).ociconfig, \
        "File containing oci configuration (in json format)", NULL}, \
    {CMD_OPT_TYPE_STRING, false, "rootfs", 0, &(cmdargs).create_rootfs, \
        "Specify the rootfs for the container, dir or block device", NULL}

extern const char g_lcr_cmd_create_desc[];
extern struct lcr_arguments g_lcr_cmd_create_args;
int cmd_create_main(int argc, const char **argv);

#endif /* __CMD_CREATE_H */
