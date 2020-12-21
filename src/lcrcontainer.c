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

#include <lxc/lxccontainer.h>

#include "constants.h"
#include "error.h"
#include "lcrcontainer.h"
#include "lcrcontainer_execute.h"
#include "lcrcontainer_extend.h"
#include "log.h"
#include "utils.h"
#include "oci_runtime_spec.h"

/*
 * Free lcr_container_info array returned by lcr_list_{active,all}_containers
 */
void lcr_containers_info_free(struct lcr_container_info **info_arr, size_t size)
{
    size_t i = 0;
    struct lcr_container_info *in = NULL;

    if (info_arr == NULL) {
        return;
    }
    if (size == 0) {
        return;
    }
    for (i = 0, in = *info_arr; i < size; i++, in++) {
        free(in->interface);
        free(in->ipv4);
        free(in->ipv6);
        free(in->name);
        free(in->state);
    }
    free(*info_arr);
    *info_arr = NULL;
}

/*
 * Free lcr_container_info returned lcr_container_info_get
 */
void lcr_container_info_free(struct lcr_container_info *info)
{
    if (info == NULL) {
        return;
    }
    free(info->interface);
    info->interface = NULL;
    free(info->ipv4);
    info->ipv4 = NULL;
    free(info->ipv6);
    info->ipv6 = NULL;
    free(info->name);
    info->name = NULL;
    free(info->state);
    info->state = NULL;
    free(info);
}

static inline bool is_container_exists(struct lxc_container *c)
{
    return c->is_defined(c);
}

static inline bool is_container_can_control(struct lxc_container *c)
{
    return c->may_control(c);
}

/*
 * Get one container info for a given name and lcrpath.
 * return struct of container info, or NULL on error.
 */
struct lcr_container_info *lcr_container_info_get(const char *name, const char *lcrpath)
{
    int nret = -1;
    struct lcr_container_info *info = NULL;
    const char *st = NULL;
    bool run_flag = false;

    struct lxc_container *c = lxc_container_without_config_new(name, lcrpath);
    if (c == NULL) {
        return NULL;
    }

    if (!is_container_exists(c)) {
        goto put_and_finish;
    }

    st = c->state(c);
    if (st == NULL) {
        st = "UNKNOWN";
    }
    run_flag = (strcmp(st, "STOPPED") != 0);

    /* Now it makes sense to allocate memory */
    info = lcr_util_common_calloc_s(sizeof(*info));
    if (info == NULL) {
        nret = -1;
        goto put_and_finish;
    }
    info->init = -1;
    info->running = run_flag;
    info->name = lcr_util_strdup_s(name);
    info->state = lcr_util_strdup_s(st);
    if (run_flag) {
        info->init = c->init_pid(c);
    }

    nret = 0;
put_and_finish:
    lxc_container_put(c);
    if (nret != 0) {
        lcr_container_info_free(info);
        info = NULL;
    }
    return info;
}

/*
 * Get a complete list of all containers for a given lcrpath.
 * return Number of containers, or -1 on error.
 **/
int lcr_list_all_containers(const char *lcrpath, struct lcr_container_info **info_arr)
{
    char **container = NULL;
    int n = 0;
    int nret = -1;
    size_t info_size = 0;
    const char *path = lcrpath ? lcrpath : LCRPATH;

    clear_error_message(&g_lcr_error);
    n = list_all_containers(path, &container, NULL);
    if (n == -1) {
        n = 0;
    }

    nret = lcr_containers_info_get(path, info_arr, &info_size, container, n);
    if (info_arr == NULL && nret == 0) {
        return -1;
    } else if (info_arr == NULL || nret == -1) {
        lcr_containers_info_free(info_arr, info_size);
        return -1;
    }

    return (int)info_size;
}

static int create_partial(const struct lxc_container *c)
{
    size_t len = 0;
    int fd = 0;
    int ret = 0;
    struct flock lk;

    if (strlen(c->name) > ((SIZE_MAX - strlen(c->config_path)) - 10)) {
        return -1;
    }

    // $lxcpath + '/' + $name + '/partial' + \0
    len = strlen(c->config_path) + strlen(c->name) + 10;

    char *path = lcr_util_common_calloc_s(len);
    if (path == NULL) {
        ERROR("Out of memory in create_partial");
        return -1;
    }

    ret = snprintf(path, len, "%s/%s/partial", c->config_path, c->name);
    if (ret < 0 || (size_t)ret >= len) {
        ERROR("Error writing partial pathname");
        goto out_free;
    }

    fd = lcr_util_open(path, O_RDWR | O_CREAT | O_EXCL, DEFAULT_SECURE_FILE_MODE);
    if (fd < 0) {
        SYSERROR("Error creating partial file: %s", path);
        goto out_free;
    }
    lk.l_type = F_WRLCK;
    lk.l_whence = SEEK_SET;
    lk.l_start = 0;
    lk.l_len = 0;
    if (fcntl(fd, F_SETLKW, &lk) < 0) {
        SYSERROR("Error locking partial file %s", path);
        close(fd);
        goto out_free;
    }

    free(path);
    return fd;

out_free:
    free(path);
    return -1;
}

static void remove_partial(const struct lxc_container *c)
{
    size_t len = 0;
    int ret = 0;

    if (strlen(c->name) > ((SIZE_MAX - strlen(c->config_path)) - 10)) {
        return;
    }

    // $lxcpath + '/' + $name + '/partial' + \0
    len = strlen(c->config_path) + strlen(c->name) + 10;

    char *path = lcr_util_common_calloc_s(len);
    if (path == NULL) {
        ERROR("Out of memory in remove_partial");
        return;
    }

    ret = snprintf(path, len, "%s/%s/partial", c->config_path, c->name);
    if (ret < 0 || (size_t)ret >= len) {
        ERROR("Error writing partial pathname");
        goto out_free;
    }
    if (unlink(path) < 0) {
        SYSERROR("Error unlink partial file %s", path);
    }

out_free:
    free(path);
}

bool lcr_create_from_ocidata(const char *name, const char *lcrpath, const void *oci_json_data)
{
    oci_runtime_spec *oci_spec = NULL;
    bool ret = true;

    if (!container_parse(NULL, oci_json_data, &oci_spec)) {
        ret = false;
        goto out_free;
    }

    ret = lcr_create(name, lcrpath, oci_spec);
out_free:
    free_oci_runtime_spec(oci_spec);
    return ret;
}

static bool lcr_create_spec(struct lxc_container *c, oci_runtime_spec *oci_spec)
{
    // Translate oci config
    DEBUG("Translate oci config...\n");
    if (!translate_spec(c, oci_spec)) {
        return false;
    }
    DEBUG("Translate oci config... done\n");
    return true;
}

bool lcr_create(const char *name, const char *lcrpath, void *oci_config)
{
    struct lxc_container *c = NULL;
    int partial_fd = -1;
    bool bret = false;
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;
    oci_runtime_spec *oci_spec = (oci_runtime_spec *)oci_config;

    clear_error_message(&g_lcr_error);
    isula_libutils_set_log_prefix(name);

    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        isula_libutils_free_log_prefix();
        return false;
    }

    /* Mark that this container is being created */
    partial_fd = create_partial(c);
    if (partial_fd < 0) {
        lxc_container_put(c);
        isula_libutils_free_log_prefix();
        return false;
    }

    if (!lcr_create_spec(c, oci_spec)) {
        goto out_unlock;
    }

    bret = true;
out_unlock:
    if (partial_fd >= 0) {
        close(partial_fd);
        remove_partial(c);
    }
    if (!bret) {
        if (!c->destroy(c)) {
            WARN("Unable to clean lxc resources");
        }
    }
    lxc_container_put(c);
    isula_libutils_free_log_prefix();
    return bret;
}

static bool lcr_start_check_config(const char *lcrpath, const char *name)
{
    char config[PATH_MAX] = { 0 };
    int nret = 0;

    if (access(lcrpath, O_RDONLY) != 0) {
        ERROR("You lack permission to access %s", lcrpath);
        return false;
    }

    nret = snprintf(config, sizeof(config), "%s/%s/config", lcrpath, name);
    if (nret < 0 || (size_t)nret >= sizeof(config)) {
        SYSERROR("Failed to allocated memory");
        return false;
    }

    if (access(config, F_OK) != 0) {
        ERROR("File %s does not exist", config);
        return false;
    }
    return true;
}

static bool wait_start_pid(pid_t pid, int rfd, const char *name, const char *path)
{
    int ret;
    ssize_t size_read = 0;
    char buffer[BUFSIZ] = { 0 };

    ret = lcr_wait_for_pid(pid);
    if (ret == 0) {
        return true;
    }

    ERROR("Start container failed\n");
    // set default error
    lcr_set_error_message(LCR_ERR_RUNTIME, "runtime error");

    INFO("begin to stop container\n");
    if (!lcr_kill(name, path, SIGKILL)) {
        ERROR("Failed to stop container");
    }

    size_read = read(rfd, buffer, sizeof(buffer) - 1);
    if (size_read > 0) {
        ERROR("Runtime error: %s", buffer);
        lcr_set_error_message(LCR_ERR_RUNTIME, "runtime error: %s", buffer);
    }
    return false;
}

bool lcr_start(const struct lcr_start_request *request)
{
    int pipefd[2] = { -1, -1 };
    bool ret = false;
    pid_t pid = 0;
    const char *path = NULL;
    if (request == NULL) {
        return false;
    }
    path = request->lcrpath ? request->lcrpath : LCRPATH;

    clear_error_message(&g_lcr_error);
    if (request->name == NULL) {
        ERROR("Missing container name");
        return false;
    }
    isula_libutils_set_log_prefix(request->name);

    if (!lcr_start_check_config(path, request->name)) {
        goto out_free;
    }

    if (pipe(pipefd) != 0) {
        ERROR("Failed to create pipe\n");
        goto out_free;
    }

    pid = fork();
    if (pid == (pid_t) -1) {
        ERROR("Failed to fork()\n");
        close(pipefd[0]);
        close(pipefd[1]);
        goto out_free;
    }

    if (pid == (pid_t)0) {
        (void)unsetenv("NOTIFY_SOCKET");
        // child process, dup2 pipefd[1] to stderr
        close(pipefd[0]);
        dup2(pipefd[1], 2);

        execute_lxc_start(request->name, path, request);
    }

    close(pipefd[1]);
    ret = wait_start_pid(pid, pipefd[0], request->name, path);
    close(pipefd[0]);

out_free:
    isula_libutils_free_log_prefix();
    return ret;
}

static bool lcr_check_container_running(struct lxc_container *c, const char *name)
{
    if (!is_container_exists(c)) {
        ERROR("No such container");
        lcr_set_error_message(LCR_ERR_RUNTIME, "No such container:%s or the configuration files has been corrupted",
                              name);
        return false;
    }

    if (!is_container_can_control(c)) {
        ERROR("Insufficent privileges to control");
        return false;
    }

    if (!c->is_running(c)) {
        ERROR("Container is not running");
        lcr_set_error_message(LCR_ERR_RUNTIME, "Container is not running:%s", name);
        return false;
    }
    return true;
}

bool lcr_kill(const char *name, const char *lcrpath, uint32_t signal)
{
    struct lxc_container *c = NULL;
    const char *path = lcrpath ? lcrpath : LCRPATH;
    bool ret = false;
    int sret = 0;
    pid_t pid = 0;

    clear_error_message(&g_lcr_error);
    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }

    isula_libutils_set_log_prefix(name);
    if (signal >= NSIG) {
        ERROR("'%u' isn't a valid signal number", signal);
        isula_libutils_free_log_prefix();
        return false;
    }

    c = lxc_container_new(name, path);
    if (c == NULL) {
        ERROR("Failed to stop container.");
        isula_libutils_free_log_prefix();
        return false;
    }

    if (!lcr_check_container_running(c, name)) {
        goto out_put;
    }

    pid = c->init_pid(c);
    if (pid < 0) {
        ERROR("Failed to get init pid");
        goto out_put;
    }

    sret = kill(pid, (int)signal);
    if (sret < 0) {
        if (errno == ESRCH) {
            WARN("Can not kill process (pid=%d) with signal %d for container: no such process", pid, signal);
            ret = true;
            goto out_put;
        }
        ERROR("Can not kill process (pid=%d) with signal %d for container", pid, signal);
        goto out_put;
    }

    ret = true;

out_put:
    lxc_container_put(c);
    isula_libutils_free_log_prefix();
    return ret;
}

bool lcr_delete(const char *name, const char *lcrpath)
{
    struct lxc_container *c = NULL;
    const char *path = lcrpath ? lcrpath : LCRPATH;
    bool ret = true;

    clear_error_message(&g_lcr_error);
    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }
    isula_libutils_set_log_prefix(name);
    c = lxc_container_new(name, path);
    if (c == NULL) {
        ERROR("Failed to delete container.");
        isula_libutils_free_log_prefix();
        return false;
    }

    if (!is_container_can_control(c)) {
        ERROR("Insufficent privileges to control");
        ret = false;
        goto out_put;
    }

    if (!is_container_exists(c)) {
        ERROR("No such container");
        lcr_set_error_message(LCR_ERR_RUNTIME, "No such container:%s or the configuration files has been corrupted",
                              name);
        ret = false;
        (void)c->destroy(c);
        goto out_put;
    }

    if (c->is_running(c)) {
        ERROR("Container %s is running, Stop the container before remove", name);
        lcr_set_error_message(LCR_ERR_RUNTIME, "Container %s is running, Stop the container before remove", name);
        ret = false;
        goto out_put;
    }

    ret = c->destroy(c);
    if (!ret) {
        if (c->error_string != NULL) {
            lcr_set_error_message(LCR_ERR_RUNTIME, "%s", c->error_string);
        }
    }

out_put:
    lxc_container_put(c);
    isula_libutils_free_log_prefix();
    return ret;
}

bool lcr_exec(const struct lcr_exec_request *request, int *exit_code)
{
    const char *name = NULL;
    struct lxc_container *c = NULL;
    const char *tmp_path = NULL;
    bool bret = false;

    clear_error_message(&g_lcr_error);

    if (request == NULL || exit_code == NULL) {
        ERROR("Invalid input arguments");
        return bret;
    }

    name = request->name;
    tmp_path = request->lcrpath ? request->lcrpath : LCRPATH;

    if (name == NULL) {
        ERROR("Missing container name");
        return bret;
    }

    isula_libutils_set_log_prefix(name);

    if (geteuid()) {
        if (access(tmp_path, O_RDONLY) < 0) {
            ERROR("You lack access to %s", tmp_path);
            goto out;
        }
    }

    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to delete container.");
        goto out;
    }

    if (!lcr_check_container_running(c, name)) {
        goto out_put;
    }

    lxc_container_put(c);

    /* do attach to wait exit code */
    bret = do_attach(name, tmp_path, request, exit_code);
    goto out;

out_put:
    lxc_container_put(c);
out:
    isula_libutils_free_log_prefix();
    return bret;
}

bool lcr_clean(const char *name, const char *lcrpath, const char *logpath, const char *loglevel, pid_t pid)
{
    struct lxc_container *c = NULL;
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;
    bool bret = true;

    clear_error_message(&g_lcr_error);

    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }
    isula_libutils_set_log_prefix(name);

    if (geteuid()) {
        if (access(tmp_path, O_RDONLY) < 0) {
            ERROR("You lack access to %s", tmp_path);
            isula_libutils_free_log_prefix();
            return false;
        }
    }

    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to delete container.");
        isula_libutils_free_log_prefix();
        return false;
    }

    /* if container do not exist, just return true. */
    if (!is_container_exists(c)) {
        WARN("No such container: %s", name);
        bret = true;
        goto out_put;
    }

    if (!is_container_can_control(c)) {
        ERROR("Insufficent privileges to control");
        bret = false;
        goto out_put;
    }

    if (!c->clean_container_resource(c, pid)) {
        ERROR("Error: Failed to clean container %s resource\n", name);
        bret = false;
        goto out_put;
    }

out_put:
    lxc_container_put(c);

    isula_libutils_free_log_prefix();
    return bret;
}

bool lcr_state(const char *name, const char *lcrpath, struct lcr_container_state *lcs)
{
    struct lxc_container *c = NULL;
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;
    bool bret = true;

    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }
    isula_libutils_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failure to retrieve state infomation on %s", tmp_path);
        isula_libutils_free_log_prefix();
        return false;
    }

    if (!is_container_exists(c)) {
        ERROR("No such container: %s", name);
        bret = false;
        goto out_put;
    }

    if (!is_container_can_control(c)) {
        ERROR("Insufficent privileges to control");
        bret = false;
        goto out_put;
    }

    do_lcr_state(c, lcs);
out_put:
    lxc_container_put(c);
    isula_libutils_free_log_prefix();
    return bret;
}

bool lcr_get_container_pids(const char *name, const char *lcrpath, pid_t **pids, size_t *pids_len)
{
    struct lxc_container *c = NULL;
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;
    bool bret = true;

    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }
    isula_libutils_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failure to retrieve state infomation on %s", tmp_path);
        isula_libutils_free_log_prefix();
        return false;
    }

    if (!is_container_exists(c)) {
        ERROR("No such container");
        bret = false;
        goto out_put;
    }

    if (!c->get_container_pids(c, pids, pids_len)) {
        ERROR("Error: Failed to get container %s pids\n", name);
        bret = false;
        goto out_put;
    }

out_put:
    lxc_container_put(c);
    isula_libutils_free_log_prefix();
    return bret;
}

void lcr_container_state_free(struct lcr_container_state *lcs)
{
    free(lcs->name);
    lcs->name = NULL;
    free(lcs->state);
    lcs->state = NULL;
}

bool lcr_pause(const char *name, const char *lcrpath)
{
    struct lxc_container *c = NULL;
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;
    bool bret = true;

    clear_error_message(&g_lcr_error);

    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }

    isula_libutils_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to pause container");
        isula_libutils_free_log_prefix();
        return false;
    }

    if (!is_container_exists(c)) {
        ERROR("No such container");
        bret = false;
        goto out_put;
    }

    if (!is_container_can_control(c)) {
        ERROR("Insufficent privleges to contol");
        bret = false;
        goto out_put;
    }

    if (!c->freeze(c)) {
        ERROR("Failed to pause");
        bret = false;
        goto out_put;
    }

out_put:
    lxc_container_put(c);
    isula_libutils_free_log_prefix();
    return bret;
}

bool lcr_resume(const char *name, const char *lcrpath)
{
    struct lxc_container *c = NULL;
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;
    bool bret = false;

    clear_error_message(&g_lcr_error);
    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }
    isula_libutils_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to resume container");
        goto out;
    }

    if (!is_container_exists(c)) {
        ERROR("No such container");
        goto out_put;
    }

    if (!is_container_can_control(c)) {
        ERROR("Insufficent privleges to contol");
        goto out_put;
    }

    if (!c->unfreeze(c)) {
        ERROR("Failed to resume");
        goto out_put;
    }

    bret = true;

out_put:
    lxc_container_put(c);
out:
    isula_libutils_free_log_prefix();
    return bret;
}

bool lcr_resize(const char *name, const char *lcrpath, unsigned int height, unsigned int width)
{
    struct lxc_container *c = NULL;
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;
    bool bret = true;

    clear_error_message(&g_lcr_error);

    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }

    isula_libutils_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to pause container");
        isula_libutils_free_log_prefix();
        return false;
    }

    if (!is_container_exists(c)) {
        ERROR("No such container");
        bret = false;
        goto out_put;
    }

    if (!is_container_can_control(c)) {
        ERROR("Insufficent privleges to contol");
        bret = false;
        goto out_put;
    }

    if (!lcr_check_container_running(c, name)) {
        bret = false;
        goto out_put;
    }

    if (!c->set_terminal_winch(c, height, width)) {
        ERROR("Failed to pause");
        bret = false;
        goto out_put;
    }

out_put:
    lxc_container_put(c);
    isula_libutils_free_log_prefix();
    return bret;
}

bool lcr_exec_resize(const char *name, const char *lcrpath, const char *suffix, unsigned int height, unsigned int width)
{
    struct lxc_container *c = NULL;
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;
    bool bret = true;

    clear_error_message(&g_lcr_error);

    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }

    isula_libutils_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to pause container");
        isula_libutils_free_log_prefix();
        return false;
    }

    if (!is_container_exists(c)) {
        ERROR("No such container");
        bret = false;
        goto out_put;
    }

    if (!is_container_can_control(c)) {
        ERROR("Insufficent privleges to contol");
        bret = false;
        goto out_put;
    }

    if (!lcr_check_container_running(c, name)) {
        bret = false;
        goto out_put;
    }

    if (!c->set_exec_terminal_winch(c, suffix, height, width)) {
        ERROR("Failed to resize exec terminal");
        bret = false;
        goto out_put;
    }

out_put:
    lxc_container_put(c);
    isula_libutils_free_log_prefix();
    return bret;
}

bool lcr_console(const char *name, const char *lcrpath, const char *in_fifo, const char *out_fifo, const char *err_fifo)
{
    struct lxc_container *c = NULL;
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;
    bool bresult = true;

    clear_error_message(&g_lcr_error);
    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }
    isula_libutils_set_log_prefix(name);

    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to create container.");
        bresult = false;
        goto out;
    }

    if (!is_container_exists(c)) {
        ERROR("No such container");
        bresult = false;
        goto out_put;
    }

    if (!is_container_can_control(c)) {
        ERROR("Insufficent privleges to contol");
        lcr_set_error_message(LCR_ERR_RUNTIME, "Insufficent privleges to contol");
        bresult = false;
        goto out_put;
    }

    if (!c->is_running(c)) {
        ERROR("It's not running");
        lcr_set_error_message(LCR_ERR_RUNTIME, "You cannot attach to a stopped container, start it first");
        bresult = false;
        goto out_put;
    }

    bresult = c->add_terminal_fifos(c, in_fifo, out_fifo, err_fifo);

out_put:
    lxc_container_put(c);
out:
    isula_libutils_free_log_prefix();
    return bresult;
}

static char *lcr_get_config_item(struct lxc_container *c, const char *key, bool running)
{
    char *cret = NULL;
    size_t len = 0;
    int nret = 0;

    if (key == NULL) {
        ERROR("Key cannot be NULL");
        return cret;
    }

    if (running) {
        if (!c->is_running(c)) {
            ERROR("It's not running");
            goto out;
        }
        cret = c->get_running_config_item(c, key);
        goto out;
    }

    nret = c->get_config_item(c, key, NULL, 0);
    if (nret <= 0) {
        ERROR("get config item length failed");
        goto out;
    }

    len = (size_t)(nret);
    if (len > SIZE_MAX / sizeof(char) - 1) {
        ERROR("Config item length is too long!");
        goto out;
    }

    cret = lcr_util_common_calloc_s((len + 1) * sizeof(char));
    if (cret == NULL) {
        ERROR("Out of memory");
        goto out;
    }

    if ((size_t)c->get_config_item(c, key, cret, (int)len + 1) != len) {
        free(cret);
        cret = NULL;
    }

out:
    return cret;
}

void lcr_free_console_config(struct lcr_console_config *config)
{
    free(config->log_path);
    config->log_path = NULL;
    free(config->log_file_size);
    config->log_file_size = NULL;
    config->log_rotate = 0;
}

static bool lcr_get_console_config_items(struct lxc_container *c, struct lcr_console_config *config)
{
    bool ret = true;
    char *item = NULL;
    unsigned int trotate = 0;

    config->log_path = lcr_get_config_item(c, "lxc.console.logfile", false);
    if (config->log_path == NULL) {
        DEBUG("Log path is NULL");
    }
    config->log_file_size = lcr_get_config_item(c, "lxc.console.size", false);
    if (config->log_file_size == 0) {
        DEBUG("Log file size is 0");
    }

    item = lcr_get_config_item(c, "lxc.console.rotate", false);
    if (item == NULL) {
        DEBUG("Log rotate is NULL");
    } else {
        if (lcr_util_safe_uint(item, &trotate) == 0) {
            config->log_rotate = trotate;
        } else {
            ERROR("trans to uint failed");
            ret = false;
        }
        free(item);
    }
    return ret;
}

bool lcr_get_console_config(const char *name, const char *lcrpath, struct lcr_console_config *config)
{
    bool ret = true;
    struct lxc_container *c = NULL;
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;

    clear_error_message(&g_lcr_error);
    if (name == NULL || lcrpath == NULL || config == NULL) {
        ERROR("Parameter is NULL");
        return false;
    }
    isula_libutils_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to create container.");
        isula_libutils_free_log_prefix();
        return false;
    }

    if (!is_container_exists(c)) {
        ERROR("No such container");
        lcr_set_error_message(LCR_ERR_RUNTIME, "No such container:%s or the configuration files has been corrupted",
                              name);
        ret = false;
        goto out_put;
    }
    if (!is_container_can_control(c)) {
        ERROR("Insufficent privleges to contol");
        goto out_put;
    }

    ret = lcr_get_console_config_items(c, config);
    if (!ret) {
        lcr_free_console_config(config);
    }

out_put:
    lxc_container_put(c);
    isula_libutils_free_log_prefix();
    return ret;
}

bool lcr_update(const char *name, const char *lcrpath, const struct lcr_cgroup_resources *cr)
{
    struct lxc_container *c = NULL;
    bool bret = false;
    const char *tmp_path = NULL;

    clear_error_message(&g_lcr_error);
    if (name == NULL || cr == NULL) {
        ERROR("Invalid input");
        return false;
    }
    isula_libutils_set_log_prefix(name);

    tmp_path = lcrpath ? lcrpath : LCRPATH;
    if (access(tmp_path, O_RDONLY) < 0) {
        ERROR("You lack permission to access %s", tmp_path);
        isula_libutils_free_log_prefix();
        return false;
    }

    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to new container.");
        goto out_free;
    }

    if (!is_container_exists(c)) {
        ERROR("No such container");
        goto out_put;
    }

    if (!is_container_can_control(c)) {
        ERROR("Insufficent privileges to control");
        goto out_put;
    }

    if (c->is_running(c) && cr->kernel_memory_limit) {
        ERROR("Can not update kernel memory to a running container, please stop it first");
        goto out_put;
    }

    if (!do_update(c, name, tmp_path, (struct lcr_cgroup_resources *)cr)) {
        goto out_put;
    }

    bret = true;

out_put:
    lxc_container_put(c);

out_free:
    isula_libutils_free_log_prefix();
    if (!bret) {
        lcr_try_set_error_message(LCR_ERR_RUNTIME, "Runtime error when updating cgroup");
    }
    return bret;
}

const char *lcr_get_errmsg()
{
    if (g_lcr_error.errcode == LCR_SUCCESS) {
        return errno_to_error_message(LCR_SUCCESS);
    }
    if (g_lcr_error.errcode == LCR_ERR_MEMOUT) {
        return errno_to_error_message(LCR_ERR_MEMOUT);
    }
    if (g_lcr_error.errcode == LCR_ERR_FORMAT) {
        return errno_to_error_message(LCR_ERR_FORMAT);
    }
    return (const char *)g_lcr_error.errmsg;
}

void lcr_free_errmsg()
{
    clear_error_message(&g_lcr_error);
}

int lcr_log_init(const char *name, const char *file, const char *priority, const char *prefix, int quiet,
                 const char *lcrpath)
{
    char *full_path = NULL;
    char *pre_name = "fifo:";
    size_t pre_len = 0;
    struct isula_libutils_log_config lconf = { 0 };
    struct lxc_log lxc_log_conf = { 0 };

    pre_len = strlen(pre_name);
    lconf.name = "engine";
    if (file == NULL || strncmp(file, pre_name, pre_len) != 0) {
        lconf.file = NULL;
        lconf.driver = "stdout";
        lconf.priority = priority ? priority : "ERROR";
    } else {
        /* File has prefix "fifo:", */
        full_path = lcr_util_string_split_prefix(pre_len, file);
        lconf.file = full_path;
        lconf.driver = "fifo";
        lconf.priority = priority;
    }
    if (isula_libutils_log_enable(&lconf)) {
        fprintf(stderr, "Failed to init log");
        goto out;
    }
    if (full_path != NULL) {
        free(full_path);
    }

    lxc_log_conf.name = name;
    lxc_log_conf.lxcpath = lcrpath;
    lxc_log_conf.file = file;
    lxc_log_conf.level = priority;
    lxc_log_conf.prefix = prefix;
    lxc_log_conf.quiet = quiet > 0 ? true : false;
    return lxc_log_init(&lxc_log_conf);
out:
    free(full_path);
    return -1;
}

