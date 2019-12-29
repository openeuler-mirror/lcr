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
#include "oci_runtime_hooks.h"
#include "oci_runtime_spec.h"
#include "start_generate_config.h"

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
    info = util_common_calloc_s(sizeof(*info));
    if (info == NULL) {
        nret = -1;
        goto put_and_finish;
    }
    info->init = -1;
    info->running = run_flag;
    info->name = util_strdup_s(name);
    info->state = util_strdup_s(st);
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

static bool create_container_dir(const struct lxc_container *c)
{
    bool ret = false;
    int nret;
    char *s = NULL;
    mode_t mask = umask(S_IWOTH);
    size_t length = 0;

    if (strlen(c->name) > ((SIZE_MAX - strlen(c->config_path)) - 2)) {
        goto out;
    }

    length = strlen(c->config_path) + strlen(c->name) + 2;
    s = util_common_calloc_s(length);
    if (s == NULL) {
        goto out;
    }

    nret = snprintf(s, length, "%s/%s", c->config_path, c->name);
    if (nret < 0 || (size_t)nret >= length) {
        goto out;
    }
    // create container dir
    nret = util_mkdir_p(s, CONFIG_DIRECTORY_MODE);
    if (nret != 0 && errno != EEXIST) {
        SYSERROR("Failed to create container path %s", s);
        goto out;
    }
    ret = true;

out:
    free(s);
    umask(mask);
    return ret;
}

static bool remove_container_dir(const struct lxc_container *c)
{
    char *s = NULL;
    int ret = 0;
    size_t length = 0;

    if (strlen(c->name) > ((SIZE_MAX - strlen(c->config_path)) - 2)) {
        return false;
    }

    length = strlen(c->config_path) + strlen(c->name) + 2;
    s = util_common_calloc_s(length);
    if (s == NULL) {
        return false;
    }

    ret = snprintf(s, length, "%s/%s", c->config_path, c->name);
    if (ret < 0 || (size_t)ret >= length) {
        free(s);
        return false;
    }
    ret = util_recursive_rmdir(s, 0);
    free(s);
    if (ret != 0) {
        return false;
    }
    return true;
}

static int lcr_write_file(const char *path, const char *data, size_t len)
{
    char *real_path = NULL;
    int fd = -1;
    int ret = -1;

    if (path == NULL || strlen(path) == 0 || data == NULL || len == 0) {
        return -1;
    }

    if (util_ensure_path(&real_path, path) < 0) {
        ERROR("Failed to ensure path %s", path);
        goto out_free;
    }

    fd = util_open(real_path, O_CREAT | O_TRUNC | O_CLOEXEC | O_WRONLY, CONFIG_FILE_MODE);
    if (fd == -1) {
        ERROR("Create file %s failed", real_path);
        lcr_set_error_message(LCR_ERR_RUNTIME, "Create file %s failed", real_path);
        goto out_free;
    }

    if (write(fd, data, len) == -1) {
        ERROR("write data to %s failed: %s", real_path, strerror(errno));
        goto out_free;
    }

    ret = 0;

out_free:
    if (fd != -1) {
        close(fd);
    }
    free(real_path);
    return ret;
}

static bool lcr_write_ocihooks(const char *path, const oci_runtime_spec_hooks *hooks)
{
    bool ret = false;
    struct parser_context ctx = { OPT_PARSE_STRICT, stderr };
    parser_error err = NULL;

    char *json_hooks = oci_runtime_spec_hooks_generate_json(hooks, &ctx, &err);
    if (json_hooks == NULL) {
        ERROR("Failed to generate json: %s", err);
        goto out_free;
    }

    if (lcr_write_file(path, json_hooks, strlen(json_hooks)) == -1) {
        ERROR("write json hooks failed: %s", strerror(errno));
        goto out_free;
    }

    ret = true;

out_free:
    free(err);
    free(json_hooks);

    return ret;
}

static bool lcr_save_ocihooks(const char *name, const char *lcrpath, const oci_runtime_spec_hooks *hooks)
{
    const char *path = lcrpath ? lcrpath : LCRPATH;
    char ocihook[PATH_MAX] = { 0 };
    char *bundle = NULL;
    bool bret = false;
    int nret = 0;

    if (name == NULL) {
        ERROR("Missing name");
        return false;
    }

    bundle = lcr_get_bundle(path, name);
    if (bundle == NULL) {
        return false;
    }

    nret = snprintf(ocihook, sizeof(ocihook), "%s/%s", bundle, OCIHOOKSFILE);
    if (nret < 0 || (size_t)nret >= sizeof(ocihook)) {
        ERROR("Failed to print string");
        goto out_free;
    }

    bret = lcr_write_ocihooks(ocihook, hooks);

out_free:
    free(bundle);
    return bret;
}

static bool lcr_write_container(const char *path, const oci_runtime_spec *container)
{
    bool ret = false;
    struct parser_context ctx = { OPT_PARSE_STRICT, stderr };
    parser_error err = NULL;

    char *json_container = oci_runtime_spec_generate_json(container, &ctx, &err);
    if (json_container == NULL) {
        ERROR("Failed to generate json: %s", err);
        goto out_free;
    }

    if (lcr_write_file(path, json_container, strlen(json_container)) == -1) {
        ERROR("write json container failed: %s", strerror(errno));
        goto out_free;
    }

    ret = true;

out_free:
    free(err);
    free(json_container);
    return ret;
}

static bool lcr_save_container(const char *name, const char *lcrpath, const oci_runtime_spec *container)
{
    bool bret = false;
    const char *path = lcrpath ? lcrpath : LCRPATH;
    char ociconfig[PATH_MAX] = { 0 };
    char *bundle = NULL;
    int nret = 0;

    if (name == NULL) {
        ERROR("Missing name");
        return bret;
    }

    bundle = lcr_get_bundle(path, name);
    if (bundle == NULL) {
        goto out_free;
    }

    nret = snprintf(ociconfig, sizeof(ociconfig), "%s/%s", bundle, OCICONFIGFILE);
    if (nret < 0 || (size_t)nret >= sizeof(ociconfig)) {
        ERROR("Failed to print string");
        goto out_free;
    }

    bret = lcr_write_container(ociconfig, container);

out_free:
    free(bundle);
    return bret;
}
/*
 * Expand array for container->mounts
 */
static bool mounts_expand(oci_runtime_spec *container, size_t add_len)
{
    defs_mount **tmp_mount = NULL;
    int ret = 0;
    size_t old_len = container->mounts_len;
    if (add_len >= SIZE_MAX / sizeof(defs_mount *) - old_len) {
        ERROR("Too many mount elements!");
        return false;
    }
    ret = mem_realloc((void **)&tmp_mount, (old_len + add_len) * sizeof(defs_mount *), container->mounts,
                      old_len * sizeof(defs_mount *));
    if (ret == -1) {
        ERROR("memory realloc failed for mount array expand");
        return false;
    }
    container->mounts = tmp_mount;
    container->mounts_len = old_len + add_len;

    return true;
}
/*
 * Get the file path that needs to be mount
 */
static bool mount_get_bundle_file(char **bundle, const char *container_name, const char *lcrpath, const char *filename)
{
    const char *path = lcrpath ? lcrpath : LCRPATH;
    int nret = 0;
    size_t len = 0;

    if (strlen(container_name) > (((SIZE_MAX - strlen(path)) - strlen(filename)) - 3)) {
        return false;
    }

    /* bundle = lcrpath + '/' + container_name + '/' + filename + '\0' */
    len = strlen(path) + strlen(container_name) + strlen(filename) + 3;
    *bundle = util_common_calloc_s(len);
    if (*bundle == NULL) {
        return false;
    }
    nret = snprintf(*bundle, len, "%s/%s/%s", path, container_name, filename);
    if (nret < 0 || (size_t)nret >= len) {
        return false;
    }
    return true;
}
/*
 * Mount file
 */
static bool mount_file(oci_runtime_spec *container, const char *bundle, const char *filename, const char *targetdir)
{
    char dest[PATH_MAX] = { 0 };
    char **options = NULL;
    size_t options_len = 2;
    bool ret = false;
    int nret = 0;
    defs_mount *tmp_mounts = NULL;

    nret = snprintf(dest, sizeof(dest), "%s/%s", targetdir, filename);
    if (nret < 0 || (size_t)nret >= sizeof(dest)) {
        ERROR("Failed to print string");
        goto out_free;
    }

    if (options_len > (SIZE_MAX / sizeof(char *))) {
        goto out_free;
    }

    options = util_common_calloc_s(options_len * sizeof(char *));
    if (options == NULL) {
        ERROR("Out of memory");
        goto out_free;
    }
    options[0] = util_strdup_s("rbind");
    options[1] = util_strdup_s("rprivate");
    /* generate mount node */
    tmp_mounts = util_common_calloc_s(sizeof(defs_mount));
    if (tmp_mounts == NULL) {
        ERROR("Malloc tmp_mounts memory failed");
        goto out_free;
    }

    tmp_mounts->destination = util_strdup_s(dest);
    tmp_mounts->source = util_strdup_s(bundle);
    tmp_mounts->type = util_strdup_s("bind");
    tmp_mounts->options = options;
    tmp_mounts->options_len = options_len;
    options = NULL;

    /* expand mount array */
    if (!mounts_expand(container, 1)) {
        goto out_free;
    }
    /* add a new mount node */
    container->mounts[container->mounts_len - 1] = tmp_mounts;

    ret = true;
out_free:

    if (!ret) {
        util_free_array(options);
        free_defs_mount(tmp_mounts);
    }
    return ret;
}

/*
 * Mount hostname file to  /etc/hostname
 */
static bool mount_hostname(oci_runtime_spec *container, const struct lxc_container *c)
{
    bool ret = true;
    char *bundle = NULL;
    char *filename = "hostname";
    char *targetdir = "/etc";

    if (container == NULL || container->hostname == NULL) {
        return true;
    }
    /* 1.get file path for hostname */
    ret = mount_get_bundle_file(&bundle, c->name, c->config_path, filename);
    if (!ret) {
        goto out_free;
    }
    /* 2.generate hostname file that need to mount */
    ret = util_write_file(bundle, container->hostname, strlen(container->hostname),
                          true, NETWORK_MOUNT_FILE_MODE);
    if (!ret) {
        goto out_free;
    }
    /* 3.Add one mount nodes to container->mounts */
    ret = mount_file(container, bundle, filename, targetdir);
    if (!ret) {
        ERROR("mount hostname file failed");
        goto out_free;
    }
out_free:
    free(bundle);
    return ret;
}

/*
 * Mount network file, such as hosts, resolv.conf
 */
static bool mount_network_file(oci_runtime_spec *container, const struct lxc_container *c, const char *full_path,
                               const char *default_str)
{
    bool ret = false;
    char *bundle = NULL;
    char *filename = NULL;
    char *targetdir = NULL;

    if (full_path == NULL) {
        return false;
    }
    targetdir = util_strdup_s(full_path);
    filename = strrchr(targetdir, '/');
    if (filename == NULL) {
        ERROR("Invalid path: %s", targetdir);
        goto out_free;
    }
    *filename = '\0';
    filename += 1;
    // 1. get file path for hosts
    ret = mount_get_bundle_file(&bundle, c->name, c->config_path, filename);
    if (!ret) {
        goto out_free;
    }
    // 2. copy /etc/hosts into container hosts file that need to mount
    if (file_exists(full_path)) {
        ret = util_copy_file(full_path, bundle, NETWORK_MOUNT_FILE_MODE);
    } else {
        // write default value into bundle
        if (default_str != NULL && strlen(default_str) > 0) {
            ret = util_write_file(bundle, default_str, strlen(default_str), false, NETWORK_MOUNT_FILE_MODE);
        } else {
            ret = false;
            ERROR("Default value is NULL");
        }
    }
    if (!ret) {
        goto out_free;
    }
    // 3. Add one mount nodes to container->mounts
    ret = mount_file(container, bundle, filename, targetdir);
    if (!ret) {
        ERROR("mount hostname file failed");
        goto out_free;
    }
out_free:
    free(targetdir);
    free(bundle);
    return ret;
}

/*
 * Mount hosts file to  /etc/hosts
 */
static bool mount_hosts(oci_runtime_spec *container, const struct lxc_container *c)
{
    bool ret = false;
    char *bundle = NULL;
    char *content = NULL;
    char *filename = "hosts";
    char *targetdir = "/etc";
    size_t content_len = 0;
    int nret = 0;
    const char *default_config = "127.0.0.1       localhost\n"
                                 "::1     localhost ip6-localhost ip6-loopback\n"
                                 "fe00::0 ip6-localnet\n"
                                 "ff00::0 ip6-mcastprefix\n"
                                 "ff02::1 ip6-allnodes\n"
                                 "ff02::2 ip6-allrouters\n";
    const char *loop_ip = "127.0.0.1    ";

    /* 3.generate content for hosts that need to mount */

    if (strlen(container->hostname) > (((SIZE_MAX - strlen(default_config)) - strlen(loop_ip)) - 2)) {
        goto out_free;
    }

    content_len = strlen(default_config) + strlen(loop_ip) + strlen(container->hostname) + 1 + 1;
    content = util_common_calloc_s(content_len);
    if (content == NULL) {
        ERROR("Memory out");
        goto out_free;
    }

    nret = snprintf(content, content_len, "%s%s%s\n", default_config, loop_ip, container->hostname);
    if (nret < 0 || (size_t)nret >= content_len) {
        ERROR("Snprintf failed");
        goto out_free;
    }
    /* 4.get file path for hosts */
    ret = mount_get_bundle_file(&bundle, c->name, c->config_path, filename);
    if (!ret) {
        goto out_free;
    }
    /* 5.generate hosts file that need to mount */
    ret = util_write_file(bundle, content, strlen(content), false, NETWORK_MOUNT_FILE_MODE);
    if (!ret) {
        goto out_free;
    }
    /* 6.Add one mount nodes to container->mounts */
    ret = mount_file(container, bundle, filename, targetdir);
    if (!ret) {
        ERROR("mount hostname file failed");
        goto out_free;
    }

out_free:
    free(bundle);
    free(content);
    return ret;
}

static bool is_system_container(const oci_runtime_spec *container)
{
    size_t i = 0;
    for (i = 0; container->annotations != NULL && i < container->annotations->len; i++) {
        if (strcmp(container->annotations->keys[i], "system.container") == 0) {
            return true;
        }
    }
    return false;
}

static bool copy_host_file_to_bundle(const struct lxc_container *c, const char *rootfs, const char *filename)
{
    char *bundle = NULL;
    char full_path[PATH_MAX] = { 0 };
    bool ret = true;
    int nret;

    nret = snprintf(full_path, sizeof(full_path), "%s%s%s", rootfs, "/etc/", filename);
    if (nret < 0 || (size_t)nret >= sizeof(full_path)) {
        goto out_free;
    }

    ret = mount_get_bundle_file(&bundle, c->name, c->config_path, filename);
    if (!ret) {
        goto out_free;
    }
    ret = util_copy_file(full_path, bundle, NETWORK_MOUNT_FILE_MODE);
    if (!ret) {
        goto out_free;
    }

out_free:
    free(bundle);
    return ret;
}

static bool init_system_container_network(const struct lxc_container *c, const char *rootfs)
{
    if (!copy_host_file_to_bundle(c, rootfs, "hostname")) {
        ERROR("Failed to copy hostname from rootfs to container bundle");
        return false;
    }

    if (!copy_host_file_to_bundle(c, rootfs, "hosts")) {
        ERROR("Failed to copy hosts from rootfs to container bundle");
        return false;
    }

    if (!copy_host_file_to_bundle(c, rootfs, "resolv.conf")) {
        ERROR("Failed to copy resolv.conf from rootfs to container bundle");
        return false;
    }

    return true;
}

static inline bool is_mount_destination_hosts(const char *destination)
{
    return strcmp(destination, "/etc/hosts") == 0;
}

static inline bool is_mount_destination_resolv(const char *destination)
{
    return strcmp(destination, "/etc/resolv.conf") == 0;
}

static bool init_network_files_mount(oci_runtime_spec *container, const struct lxc_container *c, bool share_host)
{
    bool ret = false;
    bool has_hosts_mount = false;
    bool has_resolv_mount = false;
    size_t i = 0;

    for (i = 0; i < container->mounts_len; i++) {
        if (is_mount_destination_hosts(container->mounts[i]->destination)) {
            has_hosts_mount = true;
        }
        if (is_mount_destination_resolv(container->mounts[i]->destination)) {
            has_resolv_mount = true;
        }
    }
    ret = true;
    if (!has_resolv_mount) {
        const char *default_ipv4_dns = "\nnameserver 8.8.8.8\nnameserver 8.8.4.4\n";
        // 2. create resolv.conf, hosts files
        if (!mount_network_file(container, c, "/etc/resolv.conf", default_ipv4_dns)) {
            ERROR("Mount /etc/resolv.conf failed");
            return false;
        }
    }
    if (!has_hosts_mount) {
        if (share_host && file_exists("/etc/hosts")) {
            ret = mount_network_file(container, c, "/etc/hosts", NULL);
        } else {
            ret = mount_hosts(container, c);
        }
    }
    if (!ret) {
        ERROR("Mount /etc/hosts failed");
    }
    return ret;
}

static inline bool is_invalid_container(const struct lxc_container *c)
{
    return c == NULL || c->name == NULL || c->config_path == NULL;
}

static inline bool is_annotation_key_net_mode(const char *key)
{
    return strcmp(key, "host.network.mode") == 0;
}

static inline bool is_annotation_value_host(const char *value)
{
    return strcmp(value, "host") == 0;
}

static bool init_network_files(oci_runtime_spec *container, const struct lxc_container *c)
{
    bool share_container = false;
    bool share_host = false;
    size_t i = 0;

    if (is_invalid_container(c)) {
        ERROR("Invalid lxc container");
        return false;
    }
    if (container == NULL || container->hostname == NULL) {
        return true;
    }
    if (is_system_container(container)) {
        return init_system_container_network(c, container->root->path);
    }
    // 1. get network mode
    for (i = 0; container->annotations != NULL && i < container->annotations->len; i++) {
        if (is_annotation_key_net_mode(container->annotations->keys[i])) {
            share_container = strncmp(container->annotations->values[i], "container:", strlen("container:")) == 0;
            share_host = is_annotation_value_host(container->annotations->values[i]);
            break;
        }
    }
    if (share_container) {
        return true;
    }

    return init_network_files_mount(container, c, share_host);
}

static inline bool is_root(const char *path)
{
    return strcmp(path, "/") == 0;
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

    char *path = util_common_calloc_s(len);
    if (path == NULL) {
        ERROR("Out of memory in create_partial");
        return -1;
    }

    ret = snprintf(path, len, "%s/%s/partial", c->config_path, c->name);
    if (ret < 0 || (size_t)ret >= len) {
        ERROR("Error writing partial pathname");
        goto out_free;
    }

    fd = util_open(path, O_RDWR | O_CREAT | O_EXCL, DEFAULT_SECURE_FILE_MODE);
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

    char *path = util_common_calloc_s(len);
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

static int prepare_rootfs(struct lxc_container *c, const char *rootfs, char **container_rootfs)
{
    int ret = 0;
    struct stat st;

    if (is_root(rootfs)) {
        DEBUG("Rootfs type: \"/\"");
        *container_rootfs = util_strdup_s("/");
    } else if (strncmp(rootfs, "overlayfs:", 10) == 0) {
        DEBUG("Rootfs type: OverlayFS");
        *container_rootfs = util_strdup_s(rootfs);
    } else {
        ret = stat(rootfs, &st);
        if (ret == 0 && S_ISBLK(st.st_mode)) {
            DEBUG("Rootfs type: block device");
            *container_rootfs = util_strdup_s(rootfs);
        } else if (ret == 0 && S_ISDIR(st.st_mode)) {
            *container_rootfs = util_strdup_s(rootfs);
        } else {
            ERROR("Not supported rootfs type:%s", rootfs);
            ret = 1;
            goto err_out;
        }
    }
err_out:
    return ret;
}

static struct lxc_container *lcr_create_new_container(const char *name, const char *lcrpath)
{
    struct lxc_container *c = NULL;

    c = lxc_container_new(name, lcrpath);
    if (c == NULL) {
        ERROR("Failed to new container.");
        return NULL;
    }

    if (is_container_exists(c)) {
        lxc_container_put(c);
        ERROR("Container already exists.");
        lcr_set_error_message(LCR_ERR_RUNTIME, "Runtime error:Container already exists:%s", name);
        return NULL;
    }

    if (!create_container_dir(c)) {
        lxc_container_put(c);
        return NULL;
    }
    return c;
}


static bool lcr_create_spec(const struct lxc_container *c, const char *real_rootfs, const char *oci_config_data)
{
    // Translate oci config
    DEBUG("Translate oci config...\n");
    if (!translate_spec(c, oci_config_data, real_rootfs)) {
        return false;
    }
    DEBUG("Translate oci config... done\n");
    return true;
}

bool lcr_create(const char *name, const char *lcrpath, const char *rootfs, const void *oci_config_data)
{
    struct lxc_container *c = NULL;
    int partial_fd = -1;
    bool bret = false;
    char *real_rootfs = NULL; /* the real rootfs used by the container */
    const char *tmp_path = lcrpath ? lcrpath : LCRPATH;

    clear_error_message(&g_lcr_error);
    engine_set_log_prefix(name);

    c = lcr_create_new_container(name, tmp_path);
    if (c == NULL) {
        engine_free_log_prefix();
        return false;
    }

    /* Mark that this container is being created */
    partial_fd = create_partial(c);
    if (partial_fd < 0) {
        lxc_container_put(c);
        engine_free_log_prefix();
        return false;
    }

    if (prepare_rootfs(c, rootfs, &real_rootfs)) {
        ERROR("Failed to prepare rootfs");
        goto out_unlock;
    }

    if (!lcr_create_spec(c, real_rootfs, oci_config_data)) {
        goto out_unlock;
    }

    bret = true;
out_unlock:
    free(real_rootfs);
    if (partial_fd >= 0) {
        close(partial_fd);
        remove_partial(c);
    }
    if (!bret) {
        if (!remove_container_dir(c)) {
            WARN("Unable to clean container directory");
        }
        if (!c->destroy(c)) {
            WARN("Unable to clean lxc resources");
        }
    }
    lxc_container_put(c);
    engine_free_log_prefix();
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

    ret = wait_for_pid(pid);
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

static int save_container_config_file(const char *rootpath, const char *id, const char *json_data, const char *fname)
{
    int ret = 0;
    int nret = 0;
    char filename[PATH_MAX] = { 0 };
    int fd = -1;
    ssize_t len = 0;

    if (json_data == NULL || strlen(json_data) == 0) {
        goto out;
    }
    nret = snprintf(filename, sizeof(filename), "%s/%s/%s", rootpath, id, fname);
    if (nret < 0 || (size_t)nret >= sizeof(filename)) {
        ERROR("Failed to print string");
        ret = -1;
        goto out;
    }
    if (file_exists(filename)) {
        goto out;
    }

    fd = util_open(filename, O_CREAT | O_TRUNC | O_CLOEXEC | O_WRONLY, CONFIG_FILE_MODE);
    if (fd == -1) {
        ERROR("Create file %s failed: %s", filename, strerror(errno));
        ret = -1;
        goto out;
    }

    len = util_write_nointr(fd, json_data, strlen(json_data));
    if (len < 0 || ((size_t)len) != strlen(json_data)) {
        ERROR("Write file %s failed: %s", filename, strerror(errno));
        ret = -1;
    }
    close(fd);
out:
    return ret;
}

#define START_GENERATE_CONFIG "start_generate_config.json"
static int save_start_generate_config_json(const char *rootpath, const struct lcr_start_request *request)
{
    start_generate_config sconf = { 0 };
    char *jsonstr = NULL;
    parser_error jerr = NULL;
    int ret = 0;

    if (request->uid == 0 && request->gid == 0 &&
        (request->additional_gids == NULL || request->additional_gids_len == 0)) {
        return 0;
    }
    sconf.uid = request->uid;
    sconf.gid = request->gid;
    sconf.additional_gids = request->additional_gids;
    sconf.additional_gids_len = request->additional_gids_len;

    jsonstr = start_generate_config_generate_json(&sconf, NULL, &jerr);
    ret = save_container_config_file(rootpath, request->name, jsonstr, START_GENERATE_CONFIG);

    free(jerr);
    free(jsonstr);
    return ret;
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
    engine_set_log_prefix(request->name);

    if (!lcr_start_check_config(path, request->name)) {
        goto out_free;
    }

    if (pipe(pipefd) != 0) {
        ERROR("Failed to create pipe\n");
        goto out_free;
    }
    /* generate start config */
    if (save_start_generate_config_json(path, request) != 0) {
        ERROR("Failed to generate start config json file");
        goto out_free;
    }

    pid = fork();
    if (pid == (pid_t) - 1) {
        ERROR("Failed to fork()\n");
        close(pipefd[0]);
        close(pipefd[1]);
        goto out_free;
    }

    if (pid == (pid_t)0) {
        // child process, dup2 pipefd[1] to stderr
        close(pipefd[0]);
        dup2(pipefd[1], 2);

        execute_lxc_start(request->name, path, request);
    }

    close(pipefd[1]);
    ret = wait_start_pid(pid, pipefd[0], request->name, path);
    close(pipefd[0]);

out_free:
    engine_free_log_prefix();
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

    engine_set_log_prefix(name);
    if (signal >= NSIG) {
        ERROR("'%u' isn't a valid signal number", signal);
        engine_free_log_prefix();
        return false;
    }

    c = lxc_container_new(name, path);
    if (c == NULL) {
        ERROR("Failed to stop container.");
        engine_free_log_prefix();
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
    engine_free_log_prefix();
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
    engine_set_log_prefix(name);
    c = lxc_container_new(name, path);
    if (c == NULL) {
        ERROR("Failed to delete container.");
        engine_free_log_prefix();
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
    engine_free_log_prefix();
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

    engine_set_log_prefix(name);

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
    engine_free_log_prefix();
    return bret;
}

static bool lcr_check_container_stopped(struct lxc_container *c, const char *name)
{
    if (!is_container_can_control(c)) {
        ERROR("Insufficent privileges to control");
        return false;
    }

    if (c->is_running(c)) {
        ERROR("Container is still running");
        lcr_set_error_message(LCR_ERR_RUNTIME, "Container is still running:%s", name);
        return false;
    }
    return true;
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
    engine_set_log_prefix(name);

    if (geteuid()) {
        if (access(tmp_path, O_RDONLY) < 0) {
            ERROR("You lack access to %s", tmp_path);
            engine_free_log_prefix();
            return false;
        }
    }

    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to delete container.");
        engine_free_log_prefix();
        return false;
    }

    /* if container do not exist, just return true. */
    if (!is_container_exists(c)) {
        WARN("No such container: %s", name);
        bret = true;
        goto out_put;
    }

    if (!lcr_check_container_stopped(c, name)) {
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

    engine_free_log_prefix();
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
    engine_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failure to retrieve state infomation on %s", tmp_path);
        engine_free_log_prefix();
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
    engine_free_log_prefix();
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
    engine_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failure to retrieve state infomation on %s", tmp_path);
        engine_free_log_prefix();
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
    engine_free_log_prefix();
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

    engine_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to pause container");
        engine_free_log_prefix();
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
    engine_free_log_prefix();
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
    engine_set_log_prefix(name);
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
    engine_free_log_prefix();
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
    engine_set_log_prefix(name);

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
    engine_free_log_prefix();
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

    cret = util_common_calloc_s((len + 1) * sizeof(char));
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
        if (util_safe_uint(item, &trotate) == 0) {
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
    engine_set_log_prefix(name);
    c = lxc_container_new(name, tmp_path);
    if (c == NULL) {
        ERROR("Failed to create container.");
        engine_free_log_prefix();
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
    engine_free_log_prefix();
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
    engine_set_log_prefix(name);

    tmp_path = lcrpath ? lcrpath : LCRPATH;
    if (access(tmp_path, O_RDONLY) < 0) {
        ERROR("You lack permission to access %s", tmp_path);
        engine_free_log_prefix();
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

    if (!do_update(c, name, tmp_path, cr)) {
        goto out_put;
    }

    bret = true;

out_put:
    lxc_container_put(c);

out_free:
    engine_free_log_prefix();
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
    struct engine_log_config lconf = { 0 };
    struct lxc_log lxc_log_conf = { 0 };

    pre_len = strlen(pre_name);
    lconf.name = "engine";
    if (file == NULL || strncmp(file, pre_name, pre_len) != 0) {
        lconf.file = NULL;
        lconf.driver = "stdout";
        lconf.priority = priority ? priority : "ERROR";
    } else {
        /* File has prefix "fifo:", */
        full_path = util_string_split_prefix(pre_len, file);
        lconf.file = full_path;
        lconf.driver = "fifo";
        lconf.priority = priority;
    }
    if (log_enable(&lconf)) {
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

bool translate_spec(const struct lxc_container *c, const char *oci_json_data, const char *container_rootfs)
{
    bool ret = false;
    struct lcr_list *lcr_conf = NULL;
    oci_runtime_spec *container = NULL;
    char *seccomp_conf = NULL;

    INFO("Translate new specification file");
    if (!container_parse(NULL, oci_json_data, &container)) {
        goto out_free_conf;
    }

    if (!is_system_container(container)) {
        if (!mount_hostname(container, c)) {
            goto out_free_conf;
        }
    }

    if (!init_network_files(container, c)) {
        goto out_free_conf;
    }

    lcr_conf = lcr_oci2lcr(c, container_rootfs, container, &seccomp_conf);
    if (lcr_conf == NULL) {
        ERROR("Create specific configuration failed");
        goto out_free_conf;
    }

    if (container->hooks != NULL && !lcr_save_ocihooks(c->name, c->config_path, container->hooks)) {
        ERROR("Failed to save %s", OCIHOOKSFILE);
        goto out_free_conf;
    }

    if (!lcr_save_container(c->name, c->config_path, container)) {
        ERROR("Failed to save %s", OCICONFIGFILE);
        goto out_free_conf;
    }

    if (!lcr_save_spec(c->name, c->config_path, lcr_conf, seccomp_conf)) {
        ERROR("Failed to save configuration");
        goto out_free_conf;
    }

    ret = true;

out_free_conf:
    free_oci_runtime_spec(container);

    lcr_free_config(lcr_conf);
    free(lcr_conf);

    free(seccomp_conf);
    return ret;
}
