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
