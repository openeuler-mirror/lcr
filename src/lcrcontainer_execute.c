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

/*
 * liblcrapi
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#include "constants.h"
#include "lcrcontainer_execute.h"
#include "utils.h"
#include "log.h"
#include "error.h"
#include "oci_runtime_spec.h"
#include "lcrcontainer_extend.h"

// Cgroup v1 Item Definition
#define CGROUP_BLKIO_WEIGHT "blkio.weight"
#define CGROUP_CPU_SHARES "cpu.shares"
#define CGROUP_CPU_PERIOD "cpu.cfs_period_us"
#define CGROUP_CPU_QUOTA "cpu.cfs_quota_us"
#define CGROUP_CPU_RT_PERIOD "cpu.rt_period_us"
#define CGROUP_CPU_RT_RUNTIME "cpu.rt_runtime_us"
#define CGROUP_CPUSET_CPUS "cpuset.cpus"
#define CGROUP_CPUSET_MEMS "cpuset.mems"
#define CGROUP_MEMORY_LIMIT "memory.limit_in_bytes"
#define CGROUP_MEMORY_SWAP "memory.memsw.limit_in_bytes"
#define CGROUP_MEMORY_RESERVATION "memory.soft_limit_in_bytes"

// Cgroup v2 Item Definition
#define CGROUP2_IO_WEIGHT "io.weight"
#define CGROUP2_IO_BFQ_WEIGHT "io.bfq.weight"
#define CGROUP2_CPU_WEIGHT "cpu.weight"
#define CGROUP2_CPU_MAX "cpu.max"
#define CGROUP2_CPUSET_CPUS "cpuset.cpus"
#define CGROUP2_CPUSET_MEMS "cpuset.mems"
#define CGROUP2_MEMORY_MAX "memory.max"
#define CGROUP2_MEMORY_LOW "memory.low"
#define CGROUP2_MEMORY_SWAP_MAX "memory.swap.max"

#define REPORT_SET_CGROUP_ERROR(item, value)                                                          \
    do                                                                                                \
    {                                                                                                 \
        SYSERROR("Error updating cgroup %s to %s", (item), (value));                                  \
        lcr_set_error_message(LCR_ERR_RUNTIME, "Error updating cgroup %s to %s: %s", (item), (value), \
                              strerror(errno));                                                       \
    } while (0)

static inline void add_array_elem(char **array, size_t total, size_t *pos, const char *elem)
{
    if (*pos + 1 >= total - 1) {
        return;
    }
    array[*pos] = lcr_util_strdup_s(elem);
    *pos += 1;
}

static inline void add_array_kv(char **array, size_t total, size_t *pos, const char *k, const char *v)
{
    if (k == NULL || v == NULL) {
        return;
    }
    add_array_elem(array, total, pos, k);
    add_array_elem(array, total, pos, v);
}

static uint64_t stat_get_ull(struct lxc_container *c, const char *item)
{
    char buf[80] = {0};
    int len = 0;
    uint64_t val = 0;

    len = c->get_cgroup_item(c, item, buf, sizeof(buf));
    if (len <= 0) {
        DEBUG("unable to read cgroup item %s", item);
        return 0;
    }

    val = strtoull(buf, NULL, 0);
    return val;
}

static bool update_resources_cpuset_mems(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    bool ret = false;

    if (cr->cpuset_mems != NULL && strcmp(cr->cpuset_mems, "") != 0) {
        if (!c->set_cgroup_item(c, CGROUP_CPUSET_MEMS, cr->cpuset_mems)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_CPUSET_MEMS, cr->cpuset_mems);
            goto err_out;
        }
    }
    ret = true;
err_out:
    return ret;
}

static bool update_resources_cpuset(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    bool ret = false;
    if (cr->cpuset_cpus != NULL && strcmp(cr->cpuset_cpus, "") != 0) {
        if (!c->set_cgroup_item(c, CGROUP_CPUSET_CPUS, cr->cpuset_cpus)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_CPUSET_CPUS, cr->cpuset_cpus);
            goto err_out;
        }
    }

    ret = true;
err_out:
    return ret;
}

static int update_resources_cpuset_cpus_v2(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    if (cr->cpuset_cpus != NULL && strcmp(cr->cpuset_cpus, "") != 0) {
        if (!c->set_cgroup_item(c, CGROUP2_CPUSET_CPUS, cr->cpuset_cpus)) {
            REPORT_SET_CGROUP_ERROR(CGROUP2_CPUSET_CPUS, cr->cpuset_cpus);
            return -1;
        }
    }

    return 0;
}

static int update_resources_cpuset_mems_v2(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    if (cr->cpuset_mems != NULL && strcmp(cr->cpuset_mems, "") != 0) {
        if (!c->set_cgroup_item(c, CGROUP2_CPUSET_MEMS, cr->cpuset_mems)) {
            REPORT_SET_CGROUP_ERROR(CGROUP2_CPUSET_MEMS, cr->cpuset_mems);
            return -1;
        }
    }

    return 0;
}

static int update_resources_cpu_shares(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    int ret = 0;
    char numstr[128] = {0}; /* max buffer */

    if (cr->cpu_shares != 0) {
        int num = snprintf(numstr, sizeof(numstr), "%llu", (unsigned long long)(cr->cpu_shares));
        if (num < 0 || (size_t)num >= sizeof(numstr)) {
            ret = -1;
            goto out;
        }

        if (!c->set_cgroup_item(c, CGROUP_CPU_SHARES, numstr)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_CPU_SHARES, numstr);
            ret = -1;
            goto out;
        }
    }

out:
    return ret;
}

static int update_resources_cpu_weight_v2(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    char numstr[128] = {0}; /* max buffer */

    if (cr->cpu_shares == 0) {
        return 0;
    }

    // 252144 comes from linux kernel code "#define MAX_SHARES (1UL << 18)"
    if (cr->cpu_shares < 2 || cr->cpu_shares > 262144) {
        ERROR("invalid cpu shares %lld out of range [2-262144]", (long long)cr->cpu_shares);
        return -1;
    }

    int num = snprintf(numstr, sizeof(numstr), "%llu",
                       (unsigned long long)lcr_util_trans_cpushare_to_cpuweight(cr->cpu_shares));
    if (num < 0 || (size_t)num >= sizeof(numstr)) {
        return -1;
    }

    if (!c->set_cgroup_item(c, CGROUP2_CPU_WEIGHT, numstr)) {
        REPORT_SET_CGROUP_ERROR(CGROUP2_CPU_WEIGHT, numstr);
        return -1;
    }

    return 0;
}

static int update_resources_cpu_period(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    int ret = 0;
    char numstr[128] = {0}; /* max buffer */

    if (cr->cpu_period != 0) {
        int num = snprintf(numstr, sizeof(numstr), "%llu", (unsigned long long)(cr->cpu_period));
        if (num < 0 || (size_t)num >= sizeof(numstr)) {
            ret = -1;
            goto out;
        }

        if (!c->set_cgroup_item(c, CGROUP_CPU_PERIOD, numstr)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_CPU_PERIOD, numstr);
            ret = -1;
            goto out;
        }
    }

out:
    return ret;
}

static int update_resources_cpu_max_v2(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    int num = 0;
    uint64_t period = cr->cpu_period;
    uint64_t quota = cr->cpu_quota;
    char numstr[128] = {0}; /* max buffer */

    if (quota == 0 && period == 0) {
        return 0;
    }

    if (period == 0) {
        period = DEFAULT_CPU_PERIOD;
    }

    // format:
    // $MAX $PERIOD
    if ((int64_t) quota > 0) {
        num = snprintf(numstr, sizeof(numstr), "%llu %llu", (unsigned long long)quota, (unsigned long long)period);
    } else {
        num = snprintf(numstr, sizeof(numstr), "max %llu", (unsigned long long)period);
    }
    if (num < 0 || (size_t)num >= sizeof(numstr)) {
        return -1;
    }

    if (!c->set_cgroup_item(c, CGROUP2_CPU_MAX, numstr)) {
        REPORT_SET_CGROUP_ERROR(CGROUP2_CPU_MAX, numstr);
        return -1;
    }

    return 0;
}

static int update_resources_cpu_rt_period(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    int ret = 0;
    char numstr[LCR_NUMSTRLEN64] = {0}; /* max buffer */

    if (cr->cpurt_period != 0) {
        int num = snprintf(numstr, sizeof(numstr), "%lld", (long long)(cr->cpurt_period));
        if (num < 0 || (size_t)num >= sizeof(numstr)) {
            ret = -1;
            goto out;
        }

        if (!c->set_cgroup_item(c, CGROUP_CPU_RT_PERIOD, numstr)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_CPU_RT_PERIOD, numstr);
            ret = -1;
            goto out;
        }
    }

out:
    return ret;
}

static int update_resources_cpu_rt_runtime(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    int ret = 0;
    char numstr[LCR_NUMSTRLEN64] = {0}; /* max buffer */

    if (cr->cpurt_runtime != 0) {
        int num = snprintf(numstr, sizeof(numstr), "%lld", (long long)(cr->cpurt_runtime));
        if (num < 0 || (size_t)num >= sizeof(numstr)) {
            ret = -1;
            goto out;
        }

        if (!c->set_cgroup_item(c, CGROUP_CPU_RT_RUNTIME, numstr)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_CPU_RT_RUNTIME, numstr);
            ret = -1;
            goto out;
        }
    }

out:
    return ret;
}

static int update_resources_cpu_quota(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    int ret = 0;
    char numstr[128] = {0}; /* max buffer */

    if (cr->cpu_quota != 0) {
        int num = snprintf(numstr, sizeof(numstr), "%llu", (unsigned long long)(cr->cpu_quota));
        if (num < 0 || (size_t)num >= sizeof(numstr)) {
            ret = -1;
            goto out;
        }

        if (!c->set_cgroup_item(c, CGROUP_CPU_QUOTA, numstr)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_CPU_QUOTA, numstr);
            ret = -1;
            goto out;
        }
    }

out:
    return ret;
}

static bool update_resources_cpu_v1(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    bool ret = false;

    if (update_resources_cpu_shares(c, cr) != 0) {
        goto err_out;
    }

    if (update_resources_cpu_period(c, cr) != 0) {
        goto err_out;
    }

    if (update_resources_cpu_quota(c, cr) != 0) {
        goto err_out;
    }

    if (!update_resources_cpuset(c, cr)) {
        goto err_out;
    }

    if (!update_resources_cpuset_mems(c, cr)) {
        goto err_out;
    }

    if (update_resources_cpu_rt_period(c, cr) != 0) {
        goto err_out;
    }
    if (update_resources_cpu_rt_runtime(c, cr) != 0) {
        goto err_out;
    }

    ret = true;
err_out:
    return ret;
}

static int update_resources_cpu_v2(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    if (update_resources_cpu_weight_v2(c, cr) != 0) {
        return -1;
    }

    if (update_resources_cpu_max_v2(c, cr) != 0) {
        return -1;
    }

    if (update_resources_cpuset_cpus_v2(c, cr) != 0) {
        return -1;
    }

    if (update_resources_cpuset_mems_v2(c, cr) != 0) {
        return -1;
    }

    return 0;
}

static int update_resources_memory_limit(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    int ret = 0;
    char numstr[128] = {0}; /* max buffer */

    if (cr->memory_limit != 0) {
        int num = snprintf(numstr, sizeof(numstr), "%llu", (unsigned long long)(cr->memory_limit));
        if (num < 0 || (size_t)num >= sizeof(numstr)) {
            ret = -1;
            goto out;
        }

        if (!c->set_cgroup_item(c, CGROUP_MEMORY_LIMIT, numstr)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_MEMORY_LIMIT, numstr);
            ret = -1;
            goto out;
        }
    }

out:
    return ret;
}

static int trans_int64_to_numstr_with_max(int64_t value, char *numstr, size_t size)
{
    int num = 0;

    if (value == -1) {
        num = snprintf(numstr, size, "max");
    } else {
        num = snprintf(numstr, size, "%lld", (long long)value);
    }
    if (num < 0 || (size_t)num >= size) {
        return -1;
    }

    return 0;
}

static int update_resources_memory_limit_v2(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    char numstr[128] = {0}; /* max buffer */

    if (cr->memory_limit == 0) {
        return 0;
    }

    if (trans_int64_to_numstr_with_max((int64_t)cr->memory_limit, numstr, sizeof(numstr)) != 0) {
        return -1;
    }

    if (!c->set_cgroup_item(c, CGROUP2_MEMORY_MAX, numstr)) {
        REPORT_SET_CGROUP_ERROR(CGROUP2_MEMORY_MAX, numstr);
        return -1;
    }

    return 0;
}

static int update_resources_memory_swap(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    int ret = 0;
    char numstr[128] = {0}; /* max buffer */

    if (cr->memory_swap != 0) {
        int num = snprintf(numstr, sizeof(numstr), "%llu", (unsigned long long)(cr->memory_swap));
        if (num < 0 || (size_t)num >= sizeof(numstr)) {
            ret = -1;
            goto out;
        }

        if (!c->set_cgroup_item(c, CGROUP_MEMORY_SWAP, numstr)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_MEMORY_SWAP, numstr);
            ret = -1;
            goto out;
        }
    }

out:
    return ret;
}

static int update_resources_memory_swap_v2(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    char numstr[128] = {0}; /* max buffer */
    int64_t swap = 0;

    if (cr->memory_swap == 0) {
        return 0;
    }

    if (lcr_util_get_real_swap(cr->memory_limit, cr->memory_swap, &swap) != 0) {
        return -1;
    }

    if (trans_int64_to_numstr_with_max((int64_t)swap, numstr, sizeof(numstr)) != 0) {
        return -1;
    }

    if (!c->set_cgroup_item(c, CGROUP2_MEMORY_SWAP_MAX, numstr)) {
        REPORT_SET_CGROUP_ERROR(CGROUP2_MEMORY_SWAP_MAX, numstr);
        return -1;
    }

    return 0;
}

static int update_resources_memory_reservation(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    int ret = 0;
    char numstr[128] = {0}; /* max buffer */

    if (cr->memory_reservation != 0) {
        int num = snprintf(numstr, sizeof(numstr), "%llu", (unsigned long long)(cr->memory_reservation));
        if (num < 0 || (size_t)num >= sizeof(numstr)) {
            ret = -1;
            goto out;
        }

        if (!c->set_cgroup_item(c, CGROUP_MEMORY_RESERVATION, numstr)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_MEMORY_RESERVATION, numstr);
            ret = -1;
            goto out;
        }
    }

out:
    return ret;
}

static int update_resources_memory_reservation_v2(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    char numstr[128] = {0}; /* max buffer */

    if (cr->memory_reservation == 0) {
        return 0;
    }

    if (trans_int64_to_numstr_with_max((int64_t)cr->memory_reservation, numstr, sizeof(numstr)) != 0) {
        return -1;
    }

    if (!c->set_cgroup_item(c, CGROUP2_MEMORY_LOW, numstr)) {
        return -1;
    }

    return 0;
}

static bool update_resources_mem_v1(struct lxc_container *c, struct lcr_cgroup_resources *cr)
{
    bool ret = false;

    // If the memory update is set to -1 we should also set swap to -1, it means unlimited memory.
    if (cr->memory_limit == -1) {
        cr->memory_swap = -1;
    }

    if (cr->memory_limit != 0 && cr->memory_swap != 0) {
        uint64_t cur_mem_limit = stat_get_ull(c, "memory.limit_in_bytes");
        if (cr->memory_swap == -1 || cur_mem_limit < cr->memory_swap) {
            if (update_resources_memory_swap(c, cr) != 0) {
                goto err_out;
            }
            if (update_resources_memory_limit(c, cr) != 0) {
                goto err_out;
            }
        } else {
            if (update_resources_memory_limit(c, cr) != 0) {
                goto err_out;
            }
            if (update_resources_memory_swap(c, cr) != 0) {
                goto err_out;
            }
        }
    } else {
        if (update_resources_memory_limit(c, cr) != 0) {
            goto err_out;
        }
        if (update_resources_memory_swap(c, cr) != 0) {
            goto err_out;
        }
    }

    if (update_resources_memory_reservation(c, cr) != 0) {
        goto err_out;
    }

    ret = true;
err_out:
    return ret;
}

static int update_resources_mem_v2(struct lxc_container *c, struct lcr_cgroup_resources *cr)
{
    if (update_resources_memory_limit_v2(c, cr) != 0) {
        return -1;
    }

    if (update_resources_memory_reservation_v2(c, cr) != 0) {
        return -1;
    }

    if (update_resources_memory_swap_v2(c, cr) != 0) {
        return -1;
    }

    return 0;
}

static int update_resources_blkio_weight_v1(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    int ret = 0;
    char numstr[128] = {0}; /* max buffer */

    if (cr->blkio_weight != 0) {
        int num = snprintf(numstr, sizeof(numstr), "%llu", (unsigned long long)(cr->blkio_weight));
        if (num < 0 || (size_t)num >= sizeof(numstr)) {
            ret = -1;
            goto out;
        }

        if (!c->set_cgroup_item(c, CGROUP_BLKIO_WEIGHT, numstr)) {
            REPORT_SET_CGROUP_ERROR(CGROUP_BLKIO_WEIGHT, numstr);
            ret = -1;
            goto out;
        }
    }

out:
    return ret;
}

static int update_resources_io_weight_v2(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    uint64_t weight = 0;
    char numstr[128] = {0}; /* max buffer */

    if (cr->blkio_weight == 0) {
        return 0;
    }

    weight = lcr_util_trans_blkio_weight_to_io_weight(cr->blkio_weight);
    if (weight < CGROUP2_WEIGHT_MIN || weight > CGROUP2_WEIGHT_MAX) {
        ERROR("invalid io weight cased by invalid blockio weight %llu", (unsigned long long) cr->blkio_weight);
        return -1;
    }

    int num = snprintf(numstr, sizeof(numstr), "%llu", (unsigned long long)weight);
    if (num < 0 || (size_t)num >= sizeof(numstr)) {
        return -1;
    }

    if (!c->set_cgroup_item(c, CGROUP2_IO_WEIGHT, numstr)) {
        REPORT_SET_CGROUP_ERROR(CGROUP2_IO_WEIGHT, numstr);
        return -1;
    }

    return 0;
}

static int update_resources_io_bfq_weight_v2(struct lxc_container *c, const struct lcr_cgroup_resources *cr)
{
    uint64_t weight = 0;
    char numstr[128] = {0}; /* max buffer */

    if (cr->blkio_weight == 0) {
        return 0;
    }

    weight = lcr_util_trans_blkio_weight_to_io_bfq_weight(cr->blkio_weight);
    if (weight < CGROUP2_BFQ_WEIGHT_MIN || weight > CGROUP2_BFQ_WEIGHT_MAX) {
        ERROR("invalid io weight cased by invalid blockio weight %llu", (unsigned long long) cr->blkio_weight);
        return -1;
    }

    int num = snprintf(numstr, sizeof(numstr), "%llu", (unsigned long long)weight);
    if (num < 0 || (size_t)num >= sizeof(numstr)) {
        return -1;
    }

    if (!c->set_cgroup_item(c, CGROUP2_IO_BFQ_WEIGHT, numstr)) {
        REPORT_SET_CGROUP_ERROR(CGROUP2_IO_BFQ_WEIGHT, numstr);
        return -1;
    }

    return 0;
}

static bool update_resources(struct lxc_container *c, struct lcr_cgroup_resources *cr)
{
    bool ret = false;
    int cgroup_version = 0;

    if (c == NULL || cr == NULL) {
        return false;
    }

    cgroup_version = lcr_util_get_cgroup_version();
    if (cgroup_version < 0) {
        return false;
    }

    if (cgroup_version == CGROUP_VERSION_2) {
        if (update_resources_io_weight_v2(c, cr) != 0) {
            goto err_out;
        }
        if (update_resources_io_bfq_weight_v2(c, cr) != 0) {
            goto err_out;
        }

        if (update_resources_cpu_v2(c, cr) != 0) {
            goto err_out;
        }
        if (update_resources_mem_v2(c, cr) != 0) {
            goto err_out;
        }
    } else {
        if (update_resources_blkio_weight_v1(c, cr) != 0) {
            goto err_out;
        }

        if (!update_resources_cpu_v1(c, cr)) {
            goto err_out;
        }
        if (!update_resources_mem_v1(c, cr)) {
            goto err_out;
        }
    }

    ret = true;
err_out:
    return ret;
}

bool do_update(struct lxc_container *c, const char *name, const char *lcrpath, struct lcr_cgroup_resources *cr)
{
    bool bret = false;

    // If container is not running, update config file is enough,
    // resources will be updated when the container is started again.
    // If container is running (including paused), we need to update configs
    // to the real world.
    if (c->is_running(c)) {
        if (!update_resources(c, cr) && c->is_running(c)) {
            ERROR("Filed to update cgroup resources");
            goto out_free;
        }
    }

    bret = true;

out_free:
    if (bret) {
        clear_error_message(&g_lcr_error);
    }
    return bret;
}

void do_lcr_state(struct lxc_container *c, struct lcr_container_state *lcs)
{
    struct lxc_container_metrics lxc_metrics = { 0 };

    clear_error_message(&g_lcr_error);
    (void)memset(lcs, 0x00, sizeof(struct lcr_container_state));

    lcs->name = lcr_util_strdup_s(c->name);
    lcs->init = -1;// init to -1

    if (!c->get_container_metrics(c, &lxc_metrics)) {
        DEBUG("Failed to get container %s metrics", c->name);
        return;
    }

    lcs->state = lcr_util_strdup_s(lxc_metrics.state);
    lcs->init = lxc_metrics.init;

    lcs->cpu_use_nanos = lxc_metrics.cpu_use_nanos;
    lcs->pids_current = lxc_metrics.pids_current;

    lcs->cpu_use_user = lxc_metrics.cpu_use_user;
    lcs->cpu_use_sys = lxc_metrics.cpu_use_sys;

    lcs->io_serviced.read = lxc_metrics.io_serviced.read;
    lcs->io_serviced.write = lxc_metrics.io_serviced.write;
    lcs->io_serviced.total = lxc_metrics.io_serviced.total;

    lcs->io_service_bytes.read = lxc_metrics.io_service_bytes.read;
    lcs->io_service_bytes.write = lxc_metrics.io_service_bytes.write;
    lcs->io_service_bytes.total = lxc_metrics.io_service_bytes.total;

    lcs->mem_used = lxc_metrics.mem_used;
    lcs->mem_limit = lxc_metrics.mem_limit;
    lcs->kmem_used = lxc_metrics.kmem_used;
    lcs->kmem_limit = lxc_metrics.kmem_limit;

    lcs->cache = lxc_metrics.cache;
    lcs->cache_total = lxc_metrics.cache_total;
    lcs->inactive_file_total = lxc_metrics.inactive_file_total;
}

#define ExitSignalOffset 128

static void execute_lxc_attach(const char *name, const char *path, const struct lcr_exec_request *request)
{
    // should check the size of params when add new params.
    char **params = NULL;
    size_t i = 0;
    size_t j = 0;
    size_t args_len = PARAM_NUM;

    if (lcr_util_check_inherited(true, -1) != 0) {
        COMMAND_ERROR("Close inherited fds failed");
        exit(EXIT_FAILURE);
    }

    args_len = args_len + request->args_len + request->env_len;

    if (args_len > (SIZE_MAX / sizeof(char *))) {
        exit(EXIT_FAILURE);
    }

    params = lcr_util_common_calloc_s(args_len * sizeof(char *));
    if (params == NULL) {
        COMMAND_ERROR("Out of memory");
        exit(EXIT_FAILURE);
    }
    add_array_elem(params, args_len, &i, "lxc-attach");
    add_array_elem(params, args_len, &i, "-n");
    add_array_elem(params, args_len, &i, name);
    add_array_elem(params, args_len, &i, "-P");
    add_array_elem(params, args_len, &i, path);
    add_array_elem(params, args_len, &i, "--clear-env");
    add_array_elem(params, args_len, &i, "--quiet");
    if (request->workdir != NULL) {
        add_array_kv(params, args_len, &i, "--workdir", request->workdir);
    }
    add_array_kv(params, args_len, &i, "--logfile", request->logpath);
    add_array_kv(params, args_len, &i, "-l", request->loglevel);
    add_array_kv(params, args_len, &i, "--in-fifo", request->console_fifos[0]);
    add_array_kv(params, args_len, &i, "--out-fifo", request->console_fifos[1]);
    add_array_kv(params, args_len, &i, "--err-fifo", request->console_fifos[2]);
    for (j = 0; j < request->env_len; j++) {
        add_array_elem(params, args_len, &i, "-v");
        add_array_elem(params, args_len, &i, request->env[j]);
    }

    if (request->timeout != 0) {
        char timeout_str[LCR_NUMSTRLEN64] = {0};
        add_array_elem(params, args_len, &i, "--timeout");
        int num = snprintf(timeout_str, LCR_NUMSTRLEN64, "%lld", (long long)request->timeout);
        if (num < 0 || num >= LCR_NUMSTRLEN64) {
            COMMAND_ERROR("Invaild attach timeout value :%lld", (long long)request->timeout);
            free(params);
            exit(EXIT_FAILURE);
        }
        add_array_elem(params, args_len, &i, timeout_str);
    }

    if (request->user != NULL) {
        add_array_elem(params, args_len, &i, "-u");
        add_array_elem(params, args_len, &i, request->user);
    }

    add_array_kv(params, args_len, &i, "--suffix", request->suffix);

    if (!request->tty) {
        add_array_elem(params, args_len, &i, "--disable-pty");
    }
    if (request->open_stdin) {
        add_array_elem(params, args_len, &i, "--open-stdin");
    }

    add_array_elem(params, args_len, &i, "--");
    for (j = 0; j < request->args_len; j++) {
        add_array_elem(params, args_len, &i, request->args[j]);
    }

    execvp("lxc-attach", params);

    COMMAND_ERROR("Failed to exec lxc-attach: %s", strerror(errno));
    free(params);
    exit(EXIT_FAILURE);
}

static int do_attach_get_exit_code(int status)
{
    int exit_code = 0;

    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    } else {
        exit_code = -1;
    }

    if (WIFSIGNALED(status)) {
        int signal;
        signal = WTERMSIG(status);
        exit_code = ExitSignalOffset + signal;
    }
    return exit_code;
}

bool do_attach(const char *name, const char *path, const struct lcr_exec_request *request, int *exit_code)
{
    bool ret = false;
    pid_t pid = 0;
    ssize_t size_read = 0;
    char buffer[BUFSIZ] = {0};
    int pipefd[2] = {-1, -1};
    int status = 0;

    if (pipe(pipefd) != 0) {
        ERROR("Failed to create pipe\n");
        return false;
    }

    pid = fork();
    if (pid == (pid_t) -1) {
        ERROR("Failed to fork()\n");
        close(pipefd[0]);
        close(pipefd[1]);
        goto out;
    }

    if (pid == (pid_t)0) {
        (void)unsetenv("NOTIFY_SOCKET");
        if (lcr_util_null_stdfds() < 0) {
            COMMAND_ERROR("Failed to close fds");
            exit(EXIT_FAILURE);
        }
        setsid();

        // child process, dup2 pipefd[1] to stderr
        close(pipefd[0]);
        dup2(pipefd[1], 2);

        execute_lxc_attach(name, path, request);
    }

    close(pipefd[1]);

    status = lcr_wait_for_pid_status(pid);
    if (status < 0) {
        ERROR("Failed to wait lxc-attach");
        goto close_out;
    }

    *exit_code = do_attach_get_exit_code(status);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        size_read = read(pipefd[0], buffer, BUFSIZ);
        /* if we read errmsg means the runtime failed to exec */
        if (size_read > 0) {
            ERROR("Runtime error: %s", buffer);
            lcr_set_error_message(LCR_ERR_RUNTIME, "runtime error: %s", buffer);
            goto close_out;
        }
    }
    ret = true;

close_out:
    close(pipefd[0]);
out:
    return ret;
}

void execute_lxc_start(const char *name, const char *path, const struct lcr_start_request *request)
{
    // should check the size of params when add new params.
    char *params[PARAM_NUM] = {NULL};
    size_t i = 0;

    if (lcr_util_check_inherited(true, -1) != 0) {
        COMMAND_ERROR("Close inherited fds failed");
    }

    add_array_elem(params, PARAM_NUM, &i, "lxc-start");
    add_array_elem(params, PARAM_NUM, &i, "-n");
    add_array_elem(params, PARAM_NUM, &i, name);
    add_array_elem(params, PARAM_NUM, &i, "-P");
    add_array_elem(params, PARAM_NUM, &i, path);
    add_array_elem(params, PARAM_NUM, &i, "--quiet");
    add_array_kv(params, PARAM_NUM, &i, "--logfile", request->logpath);
    add_array_kv(params, PARAM_NUM, &i, "-l", request->loglevel);
    add_array_kv(params, PARAM_NUM, &i, "--in-fifo", request->console_fifos[0]);
    add_array_kv(params, PARAM_NUM, &i, "--out-fifo", request->console_fifos[1]);
    add_array_kv(params, PARAM_NUM, &i, "--err-fifo", request->console_fifos[2]);
    if (!request->tty) {
        add_array_elem(params, PARAM_NUM, &i, "--disable-pty");
    }
    if (request->open_stdin) {
        add_array_elem(params, PARAM_NUM, &i, "--open-stdin");
    }
    add_array_elem(params, PARAM_NUM, &i, request->daemonize ? "-d" : "-F");
    add_array_kv(params, PARAM_NUM, &i, "--container-pidfile", request->container_pidfile);
    add_array_kv(params, PARAM_NUM, &i, "--exit-fifo", request->exit_fifo);

    if (request->start_timeout != 0) {
        char start_timeout_str[LCR_NUMSTRLEN64] = {0};
        add_array_elem(params, PARAM_NUM, &i, "--start-timeout");
        int num = snprintf(start_timeout_str, LCR_NUMSTRLEN64, "%u", request->start_timeout);
        if (num < 0 || num >= LCR_NUMSTRLEN64) {
            COMMAND_ERROR("Invaild start timeout value: %u", request->start_timeout);
            exit(EXIT_FAILURE);
        }
        add_array_elem(params, PARAM_NUM, &i, start_timeout_str);
    }

    execvp("lxc-start", params);

    COMMAND_ERROR("Failed to exec lxc-start\n");
    exit(EXIT_FAILURE);
}
