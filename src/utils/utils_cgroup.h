/******************************************************************************
 * isula: cgroup utils
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
#ifndef _ISULA_UTILS_UTILS_CGROUP_H
#define _ISULA_UTILS_UTILS_CGROUP_H

#include <sys/types.h>
#include <linux/magic.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CGROUP2_WEIGHT_MIN 1
#define CGROUP2_WEIGHT_MAX 10000
#define CGROUP2_BFQ_WEIGHT_MIN 1
#define CGROUP2_BFQ_WEIGHT_MAX 1000

#define DEFAULT_CPU_PERIOD 100000
#define CGROUP_MOUNTPOINT "/sys/fs/cgroup"

#ifndef CGROUP2_SUPER_MAGIC
#define CGROUP2_SUPER_MAGIC 0x63677270
#endif

#ifndef CGROUP_SUPER_MAGIC
#define CGROUP_SUPER_MAGIC 0x27e0eb
#endif

#define CGROUP_VERSION_1 1
#define CGROUP_VERSION_2 2

int lcr_util_get_real_swap(int64_t memory, int64_t memory_swap, int64_t *swap);
int lcr_util_trans_cpushare_to_cpuweight(int64_t cpu_share);
uint64_t lcr_util_trans_blkio_weight_to_io_weight(int weight);
uint64_t lcr_util_trans_blkio_weight_to_io_bfq_weight(int weight);
int lcr_util_get_cgroup_version();

#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_UTILS_CGROUP_H */