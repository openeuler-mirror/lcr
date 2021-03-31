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

#ifndef __LCR_CONTAINER_H
#define __LCR_CONTAINER_H
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include <lxc/lxccontainer.h>

#ifdef __cplusplus
extern "C" {
#endif

/* define console log config */

struct lcr_console_config {
    char *log_path;
    unsigned int log_rotate;
    char *log_file_size;
};

/*
* Store lcr container info
*/
struct lcr_container_info {
    /* Name of container. */
    char *name;
    /* State of container. */
    char *state;
    /* Interface of container. */
    char *interface;
    char *ipv4;
    char *ipv6;
    pid_t init;
    double ram;
    double swap;
    bool running;
};

struct blkio_stats {
    uint64_t read;
    uint64_t write;
    uint64_t total;
};

/*
* Store lcr container state
*/
struct lcr_container_state {
    /* Name of container */
    char *name;
    /* State of container */
    char *state;
    /* The process ID of the init container */
    pid_t init;
    /* Current pids */
    uint64_t pids_current;
    /* CPU usage */
    uint64_t cpu_use_nanos;
    uint64_t cpu_use_user;
    uint64_t cpu_use_sys;
    /* BlkIO usage */
    struct blkio_stats io_service_bytes;
    struct blkio_stats io_serviced;
    /* Memory usage */
    uint64_t mem_used;
    uint64_t mem_limit;
    /* Kernel Memory usage */
    uint64_t kmem_used;
    uint64_t kmem_limit;
    /* Cache usage */
    uint64_t cache;
    uint64_t cache_total;
    uint64_t inactive_file_total;
};

typedef enum {
    lcr_msg_state,
    lcr_msg_priority,
    lcr_msg_exit_code,
} lcr_msg_type_t;

struct lcr_msg {
    lcr_msg_type_t type;
    char name[NAME_MAX + 1];
    int value;
    int pid;
};

struct lcr_cgroup_resources {
    uint64_t blkio_weight;
    uint64_t cpu_shares;
    uint64_t cpu_period;
    uint64_t cpu_quota;
    char *cpuset_cpus;
    char *cpuset_mems;
    uint64_t memory_limit;
    uint64_t memory_swap;
    uint64_t memory_reservation;
    uint64_t kernel_memory_limit;
    int64_t cpurt_period;
    int64_t cpurt_runtime;
};

/*
* Get one container info for a given lcrpath.
* return struct of container info, or NULL on error.
*/
struct lcr_container_info *lcr_container_info_get(const char *name, const char *lcrpath);

/*
* Free lcr_container_info returned lcr_container_info_get
*/
void lcr_container_info_free(struct lcr_container_info *info);

/*
* Get a complete list of all containers for a given lcrpath.
* return Number of containers, or -1 on error.
*/
int lcr_list_all_containers(const char *lcrpath, struct lcr_container_info **info_arr);

/*
* Free lcr_container_info array returned by lcr_list_{active,all}_containers
*/
void lcr_containers_info_free(struct lcr_container_info **info_arr, size_t size);

/*
* Create a container
* param name    : container name
* param lcrpath : container path
* param oci_json_data : json string of oci config data
*/
bool lcr_create_from_ocidata(const char *name, const char *lcrpath, const void *oci_json_data);

/*
* Create a container
* param name    : container name
* param lcrpath : container path
* param oci_config	: pointer of struct oci config
*/
bool lcr_create(const char *name, const char *lcrpath, void *oci_config);

/*
* Start a container
* param name		: container name, required.
* param lcrpath	: container path, set to NULL if you want use default lcrpath.
* param logpath	: log file path.
* param loglevel	: log level.
* param pidfile	: container pidfile path, set to NULL if you don't need.
* param daemonize	: daemonize the container.
* console_fifos[]	: path of the console fifos,[0]:input, [1]:output.used internal by iSulad
* console_logpath	:path of console log file,
*			 set to NULL if want to use the default configure(base on the config file)
		 set to PATH(for example "/home/XX/XX.log"), LXC will save the console to this file
* share_ns		: array of container's name or pid which want to share namespace with them
* start_timeout	: seconds for waiting on a container to start before it is killed
* container_pidfile	: container's pidfile
* param argv		: array of arguments to pass to init.
* uid : user to run container
* gid : user in which group
* additional_gids : Add additional groups to join
*/
struct lcr_start_request {
    const char *name;
    const char *lcrpath;

    const char *logpath;
    const char *loglevel;

    bool daemonize;
    bool tty;
    bool open_stdin;
    const char **console_fifos;
    uint32_t start_timeout;
    const char *container_pidfile;
    const char *exit_fifo;
};
bool lcr_start(const struct lcr_start_request *request);

/*
* Stop a container
* param name		: container name, required.
* param lcrpath	: container path, set to NULL if you want use default lcrpath.
* param signal		: signal to send to the container.
*/
bool lcr_kill(const char *name, const char *lcrpath, uint32_t signal);

/*
* Delete a container
* param name		: container name, required.
* param lcrpath	: container path, set to NULL if you want use default lcrpath.
* param force		: force to delete container
*/
bool lcr_delete(const char *name, const char *lcrpath);

bool lcr_clean(const char *name, const char *lcrpath, const char *logpath, const char *loglevel, pid_t pid);

/*
* Get state of the container
* param name		: container name, required.
* param lcrpath	: container path, set to NULL if you want use default lcrpath.
* param lcs		: returned contaiener state
*/
bool lcr_state(const char *name, const char *lcrpath, struct lcr_container_state *lcs);

/*
* Pause a container
* param name		: container name, required.
* param lcrpath	: container path, set to NULL if you want use default lcrpath.
*/
bool lcr_pause(const char *name, const char *lcrpath);

/*
* Resume a container
* param name		: container name, required.
* param lcrpath	: container path, set to NULL if you want use default lcrpath.
*/
bool lcr_resume(const char *name, const char *lcrpath);

/*
* Free lcr_container_state returned by lcr_state
*/
void lcr_container_state_free(struct lcr_container_state *lcs);

/*
* console function
* param name    	: name of container
* param lcrpath	: container path, set to NULL if you want use default lcrpath.
* param in_fifo	: fifo names of input FIFO
* param out_fifo	: fifo names of output FIFO
* Returns false if the console FIFOs add failed, true if success
*/
bool lcr_console(const char *name, const char *lcrpath, const char *in_fifo, const char *out_fifo,
                 const char *err_fifo);

/*
* get container console configs
* param name		: name of container
* param lcrpath	: container path, set to NULL if you want use default lcrpath.
* param config		: use to store container console configs, cannot be NULL
*/
bool lcr_get_console_config(const char *name, const char *lcrpath, struct lcr_console_config *config);

void lcr_free_console_config(struct lcr_console_config *config);

int lcr_log_init(const char *name, const char *file, const char *priority,
                 const char *prefix, int quiet, const char *lcrpath);

struct lcr_exec_request {
    const char *name;
    const char *lcrpath;

    const char *logpath;
    const char *loglevel;

    const char **console_fifos;

    const char *user;

    const char **env;
    size_t env_len;
    const char **args;
    size_t args_len;

    int64_t timeout;

    const char *suffix;

    bool tty;
    bool open_stdin;
    char *workdir;
};
/*
* Execute process inside a container
*/
bool lcr_exec(const struct lcr_exec_request *request, int *exit_code);

bool lcr_update(const char *name, const char *lcrpath, const struct lcr_cgroup_resources *cr);

const char *lcr_get_errmsg();

void lcr_free_errmsg();

bool lcr_get_container_pids(const char *name, const char *lcrpath, pid_t **pids, size_t *pids_len);

bool lcr_resize(const char *name, const char *lcrpath, unsigned int height, unsigned int width);
bool lcr_exec_resize(const char *name, const char *lcrpath, const char *suffix, unsigned int height,
                     unsigned int width);
#ifdef __cplusplus
}
#endif

#endif /* __LCR_CONTAINER_H */
