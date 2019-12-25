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
 * Description: provide container definition
 ******************************************************************************/
#ifndef __LCR_CONTAINER_EXTEND_H
#define __LCR_CONTAINER_EXTEND_H

#include <lxc/lxccontainer.h>

#include "oci_runtime_spec.h"

#ifdef __cplusplus
extern "C" {
#endif

/* defined in `lcr_list.h` */
struct lcr_list;

#define SAFE_MALLOC(P, size, ret)           \
    do {                                    \
        (P) = util_common_calloc_s((size)); \
        if ((P) == NULL) {                  \
            ERROR("Out of memory");         \
            (ret) = false;                  \
        }                                   \
    } while (0);

/*
 * Get a complete list of active containers for a given lcrpath.
 * return Number of containers, or -1 on error.
 */
int lcr_list_active_containers(const char *lcrpath, struct lcr_container_info **info_arr);

/*
 * Delete a container
 * param name		: container name, required.
 * param lcrpath	: container path, set to NULL if you want use default lcrpath.
 * param force		: force to delete container
 */
bool lcr_delete_with_force(const char *name, const char *lcrpath, bool force);

/*
 * Free lcr_conf
 */
void lcr_free_config(struct lcr_list *lcr_conf);

bool container_parse(const char *oci_filename, const char *oci_json_data, oci_runtime_spec **container);

/*
 * Translate oci specification to lcr configuration.
 *   You should pass oci_filename or oci_spec to this function.
 * param oci_filename	: oci spec filename, in json format
 * param oci_json_data	: json string of oci config data
 * param new_container	: return newest oci_runtime_spec struct
 * param seccomp	: return seccomp parsed from oci spec
 * return: a linked list
 */
struct lcr_list *lcr_oci2lcr(const struct lxc_container *c, const char *container_rootfs,
                             oci_runtime_spec *new_container,
                             char **seccomp);

/*
 * Create a new specification file
 * param name			: container name, required.
 * param lcrpath		: container path, set to NULL if you want use default lcrpath.
 * param lcr_conf		: generate specification according to lcr_conf list
 * param seccomp_conf	: seccomp_conf will be wrote into seccomp file, set it to NULL if you don't need
 */
bool lcr_save_spec(const char *name, const char *lcrpath, const struct lcr_list *lcr_conf, const char *seccomp_conf);

int lcr_containers_info_get(const char *lcrpath, struct lcr_container_info **info, size_t *size, char **containers,
                            int num);

char *lcr_get_bundle(const char *lcrpath, const char *name);


#ifdef __cplusplus
}
#endif

#endif /* __LCR_CONTAINER_EXTEND_H */
