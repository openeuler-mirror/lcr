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
 * Description: provide container spec definition
 ******************************************************************************/
#ifndef __CMD_SPEC_H
#define __CMD_SPEC_H

#include "arguments.h"
#include "commander.h"

#define SPEC_OPTIONS(cmdargs) \
    { CMD_OPT_TYPE_STRING, false, "bundle", 'b', &(cmdargs).spec_bundle, \
        "Path to the root of the bundle directory, default is current directory", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "translate", 't', &(cmdargs).spec_translate, \
        "Translate oci specification (in json format) to lcr configuration", NULL }, \
    { CMD_OPT_TYPE_STRING, false, "dist", 0, &(cmdargs).spec_dist, \
        "Generate distribution specification, now support: ubuntu", NULL }

extern const char g_lcr_cmd_spec_desc[];
extern struct lcr_arguments g_lcr_cmd_spec_args;
int cmd_spec_main(int argc, const char **argv);

#endif /* __CMD_SPEC_H */
