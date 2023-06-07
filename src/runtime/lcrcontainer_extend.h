/******************************************************************************
 * lcr: utils library for iSula
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 *
 * Authors:
 * Haozi007 <liuhao27@huawei.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ********************************************************************************/

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
        (P) = lcr_util_common_calloc_s((size)); \
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
 * You should pass oci_filename or oci_spec to this function.
 * return: a linked list
 */
struct lcr_list *lcr_oci2lcr(const struct lxc_container *c, oci_runtime_spec *container,
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

bool translate_spec(const struct lxc_container *c, oci_runtime_spec *container);

#ifdef __cplusplus
}
#endif

#endif /* __LCR_CONTAINER_EXTEND_H */
