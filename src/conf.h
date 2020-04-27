/******************************************************************************
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * lcr licensed under the Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Author: wujing
 * Create: 2018-11-08
 * Description: provide container conf function
 ******************************************************************************/
#ifndef __LCR_CONF_H
#define __LCR_CONF_H

#include "oci_runtime_spec.h"
#include "lcr_list.h"

#define INVALID_INT 0

/*
 * Store lcr configuration
 */
typedef struct {
    char *name;
    char *value;
} lcr_config_item_t;

/*
 * return value:
 *  0 is valid item
 *  1 will skip this
 *  -1 is invalid item
 * */
typedef int(*lcr_check_item_t)(const char *);
/*
 * Store annotations configuration
 * */
typedef struct {
    char *name;
    char *lxc_item_name;
    lcr_check_item_t checker;
} lcr_annotation_item_t;

/*
 * seccomp trans arch
 * */
typedef struct {
    char *arch;
    char *value;
} lcr_arch_value;

/*
 * get host arch
 * */
typedef struct {
    char *arch;
    char *value;
    int num;
} lcr_host_arch;

/*
 * Create a lcr_list node, and initialize the elem to a lcr_config_item_t with
 * key and value
 */
struct lcr_list *create_lcr_list_node(const char *key, const char *value);

/*
 * Free a lcr_list node
 */
void free_lcr_list_node(struct lcr_list *node);

/*
 * Translate oci hostname to lcr config
 */
struct lcr_list *trans_oci_hostname(const char *hostname);

/*
 * Translate oci process struct to lcr config
 */
struct lcr_list *trans_oci_process(const oci_runtime_spec_process *proc);

/*
 * Translate oci root struct to lcr config
 */
struct lcr_list *trans_oci_root(const oci_runtime_spec_root *root, const oci_runtime_config_linux *linux);
/*
 * Translate oci mounts struct to lcr config
 */
struct lcr_list *trans_oci_mounts(const oci_runtime_spec *c);

/*
 * Translate oci linux struct to lcr config
 */
struct lcr_list *trans_oci_linux(const oci_runtime_config_linux *l, char **seccomp_conf);

/*
 * Translate oci annotations to lcr config
 * This is not supported in standard oci runtime-spec
 */
struct lcr_list *trans_annotations(const json_map_string_string *anno);

/*
 * Get other lxc needed configurations
 */
struct lcr_list *get_needed_lxc_conf();


bool is_system_container(const oci_runtime_spec *container);

#endif /*__LCR_CONF_H*/
