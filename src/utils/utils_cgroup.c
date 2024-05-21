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
#include "utils_cgroup.h"

#include <sys/vfs.h>

#include "log.h"

/* swap in oci is memoy+swap, so here we need to get real swap */
int lcr_util_get_real_swap(int64_t memory, int64_t memory_swap, int64_t *swap)
{
    if (swap == NULL) {
        ERROR("empty swap pointer");
        return -1;
    }

    if (memory == -1 && memory_swap == 0) {
        *swap = -1; // -1 is max
        return 0;
    }

    if (memory_swap == -1 || memory_swap == 0) {
        *swap = memory_swap; // keep max or unset
        return 0;
    }

    if (memory == -1 || memory == 0) {
        ERROR("unable to set swap limit without memory limit");
        return -1;
    }

    if (memory < 0) {
        ERROR("invalid memory");
        return -1;
    }

    if (memory > memory_swap) {
        ERROR("memory+swap must >= memory");
        return -1;
    }

    *swap = memory_swap - memory;
    return 0;
}

int lcr_util_trans_cpushare_to_cpuweight(int64_t cpu_share)
{
    /* map from range [2-262144] to [1-10000] */
    return 1 + ((cpu_share - 2) * 9999) / 262142;
}

uint64_t lcr_util_trans_blkio_weight_to_io_weight(int weight)
{
    // map from [10-1000] to [1-10000]
    return (uint64_t)(1 + ((uint64_t)weight - 10) * 9999 / 990);
}

uint64_t lcr_util_trans_blkio_weight_to_io_bfq_weight(int weight)
{
    // map from [10-1000] to [1-1000]
    return (uint64_t)(1 + ((uint64_t)weight - 10) * 999 / 990);
}

int lcr_util_get_cgroup_version(void)
{
    struct statfs fs = {0};

    if (statfs(CGROUP_MOUNTPOINT, &fs) != 0) {
        SYSERROR("failed to statfs %s", CGROUP_MOUNTPOINT);
        return -1;
    }

    if (fs.f_type == CGROUP2_SUPER_MAGIC) {
        return CGROUP_VERSION_2;
    } else {
        return CGROUP_VERSION_1;
    }
}