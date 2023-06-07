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

#ifndef _ISULA_UTILS_UTILS_H
#define _ISULA_UTILS_UTILS_H

#include "utils_memory.h"
#include "utils_convert.h"
#include "utils_cgroup.h"
#include "utils_array.h"
#include "utils_file.h"

#ifdef __cplusplus
extern "C" {
#endif

int lcr_wait_for_pid(pid_t pid);
int lcr_wait_for_pid_status(pid_t pid);

#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_UTILS_H */
