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

#ifndef __LCR_CONTAINER_EXECUTE_H
#define __LCR_CONTAINER_EXECUTE_H

#include "lcrcontainer.h"

#ifdef __cplusplus
extern "C" {
#endif

bool do_update(struct lxc_container *c, const char *name, const char *lcrpath, struct lcr_cgroup_resources *cr);

void do_lcr_state(struct lxc_container *c, struct lcr_container_state *lcs);

bool do_attach(const char *name, const char *path, const struct lcr_exec_request *request, int *exit_code);

void execute_lxc_start(const char *name, const char *path, const struct lcr_start_request *request);

#ifdef __cplusplus
}
#endif

#endif /* __LCR_CONTAINER_EXECUTE_H */
