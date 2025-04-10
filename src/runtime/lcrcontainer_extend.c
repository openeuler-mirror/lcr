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
#include <signal.h>

#include <lxc/lxccontainer.h>

#include "constants.h"
#include "error.h"
#include "lcrcontainer.h"
#include "lcrcontainer_extend.h"
#include "utils_file.h"
#include "utils_memory.h"
#include "log.h"
#include "conf.h"
#include "oci_runtime_hooks.h"
#include "utils_linked_list.h"

static struct lxc_container *lcr_new_container(const char *name, const char *path)
{
    struct lxc_container *c = lxc_container_new(name, path);
    if (c == NULL) {
        ERROR("Failed to delete container.");
        return NULL;
    }

    if (!c->is_defined(c)) {
        ERROR("No such container");
        lcr_set_error_message(LCR_ERR_RUNTIME, "No such container:%s or the configuration files has been corrupted",
                              name);
        goto out_put;
    }

    if (!c->may_control(c)) {
        ERROR("Insufficent privileges to control");
        goto out_put;
    }
    return c;
out_put:
    lxc_container_put(c);
    return NULL;
}

static int realloc_annotations(oci_runtime_spec *oci_spec, size_t new_lens)
{
    size_t new_size = 0;
    unsigned long old_size = 0;
    char **fkey = NULL;
    char **fval = NULL;
    int nret = 0;

    if (new_lens < 1) {
        return 0;
    }

    if (!oci_spec->annotations) {
        oci_spec->annotations = isula_common_calloc_s(sizeof(json_map_string_string));
        if (!oci_spec->annotations) {
            ERROR("Out of memory");
            nret = -1;
            goto out;
        }
    }

    if (new_lens > SIZE_MAX / sizeof(char *) - oci_spec->annotations->len) {
        ERROR("Too many annotations!");
        nret = -1;
        goto out;
    }
    new_size = (oci_spec->annotations->len + new_lens) * sizeof(char *);
    old_size = oci_spec->annotations->len * sizeof(char *);
    nret = isula_mem_realloc((void **)&fkey, new_size, (void **)&(oci_spec->annotations->keys), old_size);
    if (nret) {
        ERROR("Failed to realloc memory for files limit variables");
        nret = -1;
        goto out;
    }
    oci_spec->annotations->keys = fkey;

    nret = isula_mem_realloc((void **)&fval, new_size, (void **)&(oci_spec->annotations->values), old_size);
    if (nret) {
        ERROR("Failed to realloc memory for files limit variables");
        nret = -1;
        goto out;
    }
    oci_spec->annotations->values = fval;
    oci_spec->annotations->len += new_lens;

out:
    return nret;
}

static int make_annotations(oci_runtime_spec *container, const struct lxc_container *c, int fpos)
{
    int ret = -1;
    int nret;
    char default_path[PATH_MAX] = { 0 };
    char *realpath = NULL;
    json_map_string_string *anno = container->annotations;

    if (fpos == -1) {
        if (realloc_annotations(container, 1)) {
            ERROR("Realloc annotations failed");
            goto out;
        }
        fpos = (int)(anno->len - 1);
        anno->keys[fpos] = isula_strdup_s("log.console.file");
        anno->values[fpos] = NULL;
    }

    if (!anno->values[fpos]) {
        nret = snprintf(default_path, PATH_MAX, "%s/%s/%s", c->config_path, c->name, "console.log");
        if (nret < 0 || nret >= PATH_MAX) {
            ERROR("create default path: %s failed", default_path);
            goto out;
        }
        if (anno->values[fpos]) {
            free(anno->values[fpos]);
        }
        anno->values[fpos] = isula_strdup_s(default_path);
    }
    if (strcmp("none", anno->values[fpos]) == 0) {
        DEBUG("Disable console log.");
        ret = 0;
        goto out;
    }
    if (isula_file_ensure_path(&realpath, anno->values[fpos])) {
        SYSERROR("Invalid log path: %s.", anno->values[fpos]);
        goto out;
    }
    ret = 0;
out:
    free(realpath);
    return ret;
}

static inline bool is_annotations_key_console_file(const char *key)
{
    return strcmp(key, "log.console.file") == 0;
}

static int check_annotations(oci_runtime_spec *container, const struct lxc_container *c)
{
    size_t i;
    int fpos = -1;
    bool ret = false;

    if (c == NULL) {
        return 0;
    }

    if (!container->annotations) {
        SAFE_MALLOC(container->annotations, sizeof(json_map_string_string), ret);
        if (!ret) {
            goto out;
        }
    } else {
        for (i = 0; i < container->annotations->len; i++) {
            if (is_annotations_key_console_file(container->annotations->keys[i])) {
                fpos = (int)i;
            }
        }
    }

    if (make_annotations(container, c, fpos)) {
        goto out;
    }
    ret = true;
out:
    return ret ? 0 : -1;
}

static int trans_rootfs_linux(struct isula_linked_list *lcr_conf, oci_runtime_spec *container,
                              char **seccomp)
{
    int ret = -1;
    struct isula_linked_list *node = NULL;

    /* lxc.rootfs
     * lxc.rootfs.options
     */
    if (container->root || container->linux) {
        node = trans_oci_root(container->root, container->linux);
        if (node == NULL) {
            ERROR("Failed to translate rootfs configure");
            goto out;
        }
        isula_linked_list_merge(lcr_conf, node);
    }

    /* lxc.idmap */
    if (container->linux) {
        node = trans_oci_linux(container->linux, seccomp);
        if (node == NULL) {
            ERROR("Failed to translate linux configure");
            goto out;
        }
        isula_linked_list_merge(lcr_conf, node);
    }

    ret = 0;
out:
    return ret;
}

static int trans_hostname_hooks_process_mounts(struct isula_linked_list *lcr_conf, const oci_runtime_spec *container)
{
    int ret = -1;

    /* lxc.uts.name */
    struct isula_linked_list *node = trans_oci_hostname(container->hostname);
    if (node == NULL) {
        ERROR("Failed to translate hostname");
        goto out;
    }
    isula_linked_list_add_tail(lcr_conf, node);

    /* lxc.init_{u|g}id
     * lxc.init_cmd
     * lxc.environment
     * lxc.cap.{drop/keep}
     * lxc.limit.*
     * lxc.aa_profile
     * lxc.selinux.context
     */
    node = trans_oci_process(container->process);
    if (node == NULL) {
        ERROR("Failed to translate hooks");
        goto out;
    }
    isula_linked_list_merge(lcr_conf, node);

    /* lxc.mount.entry */
    node = trans_oci_mounts(container);
    if (node == NULL) {
        ERROR("Failed to translate mount entry configure");
        goto out;
    }
    isula_linked_list_merge(lcr_conf, node);

    ret = 0;
out:
    return ret;
}

static bool do_stop_and_wait(struct lxc_container *c, long timeout, bool force)
{
    pid_t pid;
    int sret = 0;
    bool ret = true;

    pid = c->init_pid(c);
    if (pid < 1) {
        ERROR("Container is not running");
        return false;
    }

    if (!force) {
        sret = kill(pid, SIGTERM);
        if (sret < 0) {
            if (errno == ESRCH) {
                return true;
            }
        }
        ret = c->wait(c, "STOPPED", (int)timeout);
        if (ret) {
            return true;
        }
    }

    sret = kill(pid, SIGKILL);
    if (sret < 0) {
        if (errno == ESRCH) {
            return true;
        }
    }
    ret = c->wait(c, "STOPPED", -1);
    if (!ret) {
        ERROR("Failed to stop container %s", c->name);
    }
    return ret;
}

static bool do_stop(struct lxc_container *c, long timeout, bool force)
{
    bool ret = true;

    if (force && timeout) {
        ERROR("-k should not use with -T");
        return false;
    }

    if (!force && timeout < -1) {
        ERROR("Timeout should be >= -1");
        return false;
    }

    if (!c->is_defined(c)) {
        ERROR("No such container");
        ret = false;
        goto out;
    }

    if (!c->may_control(c)) {
        ERROR("Insufficent privleges to contol");
        ret = false;
        goto out;
    }

    if (!c->is_running(c)) {
        DEBUG("%s is already stopped", c->name);
        goto out;
    }

    ret = do_stop_and_wait(c, timeout, force);

out:
    return ret;
}

static int lcr_spec_write_seccomp_line(int fd, const char *seccomp)
{
    size_t len;
    char *line = NULL;
    int ret = -1;
    int nret;
    ssize_t nwritten = -1;

    if (strlen(seccomp) > SIZE_MAX - strlen("lxc.seccomp.profile") - 3 - 1) {
        ERROR("the length of lxc.seccomp is too long!");
        goto cleanup;
    }

    len = strlen("lxc.seccomp.profile") + 3 + strlen(seccomp) + 1;

    line = isula_common_calloc_s(len * sizeof(char));
    if (line == NULL) {
        ERROR("Out of memory");
        goto cleanup;
    }

    nret = snprintf(line, len, "%s = %s", "lxc.seccomp.profile", seccomp);
    if (nret < 0 || (size_t)nret >= len) {
        ERROR("Sprintf failed");
        goto cleanup;
    }

    if ((size_t)nret > len - 1) {
        nret = (int)(len - 1);
    }

    line[nret] = '\n';
    nwritten = isula_file_total_write_nointr(fd, line, len);
    if (nwritten < 0 || (size_t)nwritten != len) {
        SYSERROR("Write file failed");
        goto cleanup;
    }

    ret = 0;
cleanup:
    free(line);
    return ret;
}

static char *lcr_save_seccomp_file(const char *bundle, const char *seccomp_conf)
{
    char seccomp[PATH_MAX] = { 0 };
    char *real_seccomp = NULL;
    int fd = -1;
    int nret;
    ssize_t nwritten = -1;

    nret = snprintf(seccomp, sizeof(seccomp), "%s/seccomp", bundle);
    if (nret < 0 || (size_t)nret >= sizeof(seccomp)) {
        goto cleanup;
    }

    nret = isula_file_ensure_path(&real_seccomp, seccomp);
    if (nret < 0) {
        ERROR("Failed to ensure path %s", seccomp);
        goto cleanup;
    }

    fd = isula_file_open(real_seccomp, O_CREAT | O_TRUNC | O_CLOEXEC | O_WRONLY, CONFIG_FILE_MODE);
    if (fd == -1) {
        SYSERROR("Create file %s failed", real_seccomp);
        goto cleanup;
    }

    nwritten = isula_file_total_write_nointr(fd, seccomp_conf, strlen(seccomp_conf));
    close(fd);
    if (nwritten < 0 || (size_t)nwritten != strlen(seccomp_conf)) {
        SYSERROR("write seccomp_conf failed");
        goto cleanup;
    }
    return real_seccomp;
cleanup:
    free(real_seccomp);
    return NULL;
}

bool lcr_delete_with_force(const char *name, const char *lcrpath, bool force)
{
    struct lxc_container *c = NULL;
    const char *path = lcrpath ? lcrpath : LCRPATH;
    bool ret = false;
    pid_t pid = 0;

    clear_error_message(&g_lcr_error);
    if (name == NULL) {
        ERROR("Missing container name");
        return false;
    }
    isula_libutils_set_log_prefix(name);
    c = lcr_new_container(name, path);
    if (c == NULL) {
        isula_libutils_free_log_prefix();
        return false;
    }

    if (c->is_running(c)) {
        if (!force) {
            ERROR("Container %s is running, Stop the container before remove", name);
            lcr_set_error_message(LCR_ERR_RUNTIME, "Container %s is running, Stop the container before remove", name);
            goto out_put;
        } else {
            pid = c->init_pid(c);
            if (pid < 1) {
                ERROR("Container is not running");
                goto clean_delete;
            }

            ret = do_stop(c, 0, true);
            if (!ret) {
                ERROR("Failed to stop container %s", name);
                goto out_put;
            }
        }
    }

clean_delete:
    ret = lcr_clean(name, path, NULL, NULL, pid);
    if (!ret) {
        ERROR("Failed to clean resource");
    }
    ret = lcr_delete(name, path);
    if (!ret) {
        ERROR("Failed to delete container");
    }

out_put:
    lxc_container_put(c);
    isula_libutils_free_log_prefix();
    return ret;
}

void lcr_free_config(struct isula_linked_list *lcr_conf)
{
    struct isula_linked_list *it = NULL;
    struct isula_linked_list *next = NULL;

    if (lcr_conf == NULL) {
        return;
    }

    isula_linked_list_for_each_safe(it, lcr_conf, next) {
        isula_linked_list_del(it);
        free_lcr_list_node(it);
    }
}

/*
 * Transform container JSON into oci_runtime_spec struct
 */
bool container_parse(const char *oci_filename, const char *oci_json_data, oci_runtime_spec **container)
{
    struct parser_context ctx = { OPT_PARSE_STRICT, stderr };
    parser_error err = NULL;
    bool ret = true;

    if (container == NULL) {
        ERROR("Invalid container arg");
        return false;
    }

    if (oci_json_data == NULL) {
        *container = oci_runtime_spec_parse_file(oci_filename, &ctx, &err);
    } else {
        *container = oci_runtime_spec_parse_data(oci_json_data, &ctx, &err);
    }

    if (*container == NULL) {
        ERROR("Failed to get OCI spec: %s", err);
        ret = false;
        goto out_free;
    }
out_free:
    free(err);
    return ret;
}

static int merge_annotations(const oci_runtime_spec *container, struct isula_linked_list *lcr_conf)
{
    int ret = 0;
    struct isula_linked_list *tmp = NULL;

    if (container->annotations != NULL) {
        tmp = trans_annotations(container->annotations);
        if (tmp == NULL) {
            ERROR("Failed to translate annotations configure");
            ret = -1;
            goto out;
        }
        isula_linked_list_merge(lcr_conf, tmp);
    }

out:
    return ret;
}

static int merge_needed_lxc_conf(struct isula_linked_list *lcr_conf)
{
    int ret = 0;

    struct isula_linked_list *tmp = get_needed_lxc_conf();
    if (tmp == NULL) {
        ERROR("Failed to append other lxc configure");
        ret = -1;
        goto out;
    }
    isula_linked_list_merge(lcr_conf, tmp);

out:
    return ret;
}

static int merge_auto_dev(const oci_runtime_spec *container, struct isula_linked_list *lcr_conf)
{
    int i = 0;
    struct isula_linked_list *tmp = NULL;

    for (i = 0; i < container->mounts_len; i++) {
        if (strcmp("/dev", container->mounts[i]->destination) == 0) {
            tmp = get_zero_auto_dev_conf();
            if (tmp == NULL) {
                ERROR("Failed to translate auto dev configure");
                return -1;
            }
            isula_linked_list_merge(lcr_conf, tmp);
            break;
        }
    }

    return 0;
}

struct isula_linked_list *lcr_oci2lcr(const struct lxc_container *c, oci_runtime_spec *container,
                             char **seccomp)
{
    struct isula_linked_list *lcr_conf = NULL;

    if (container == NULL) {
        ERROR("Invalid arguments");
        return NULL;
    }

    lcr_conf = isula_common_calloc_s(sizeof(*lcr_conf));
    if (lcr_conf == NULL) {
        goto out_free;
    }
    isula_linked_list_init(lcr_conf);

    if (check_annotations(container, c)) {
        ERROR("Check annotations failed");
        goto out_free;
    }

    if (trans_rootfs_linux(lcr_conf, container, seccomp)) {
        goto out_free;
    }

    if (trans_hostname_hooks_process_mounts(lcr_conf, container)) {
        goto out_free;
    }

    /* annotations.files.limit */
    if (merge_annotations(container, lcr_conf) != 0) {
        goto out_free;
    }

    // set lxc.autodev to 0 when there is a dev mount
    if (merge_auto_dev(container, lcr_conf) != 0) {
        goto out_free;
    }

    /* Append other lxc configurations */
    if (merge_needed_lxc_conf(lcr_conf) != 0) {
        goto out_free;
    }

    return lcr_conf;

out_free:
    lcr_free_config(lcr_conf);
    free(lcr_conf);

    return NULL;
}

static int lcr_open_config_file(const char *bundle)
{
    char config[PATH_MAX] = { 0 };
    char *real_config = NULL;
    int fd = -1;
    int nret;

    nret = snprintf(config, sizeof(config), "%s/config", bundle);
    if (nret < 0 || (size_t)nret >= sizeof(config)) {
        goto out;
    }

    nret = isula_file_ensure_path(&real_config, config);
    if (nret < 0) {
        ERROR("Failed to ensure path %s", config);
        goto out;
    }

    fd = isula_file_open(real_config, O_CREAT | O_TRUNC | O_CLOEXEC | O_WRONLY, CONFIG_FILE_MODE);
    if (fd == -1) {
        SYSERROR("Create file %s failed", real_config);
        lcr_set_error_message(LCR_ERR_RUNTIME, "Create file %s failed", real_config);
        goto out;
    }

out:
    free(real_config);
    return fd;
}

// escape_string_encode unzip some escape characters
static char *escape_string_encode(const char *src)
{
    size_t src_end = 0;
    size_t dst_end = 0;
    size_t len = 0;
    char *dst = NULL;

    if (src == NULL) {
        return NULL;
    }

    len = strlen(src);
    if (len == 0) {
        return NULL;
    }

    if (len > (SIZE_MAX - 1) / 2) {
        return NULL;
    }

    dst = isula_common_calloc_s(2 * len + 1);
    if (dst == NULL) {
        ERROR("Out of memory");
        return NULL;
    }

    while (src_end < len) {
        switch (src[src_end++]) {
            case '\r':
                dst[dst_end++] = '\\';
                dst[dst_end++] = 'r';
                break;
            case '\n':
                dst[dst_end++] = '\\';
                dst[dst_end++] = 'n';
                break;
            case '\f':
                dst[dst_end++] = '\\';
                dst[dst_end++] = 'f';
                break;
            case '\b':
                dst[dst_end++] = '\\';
                dst[dst_end++] = 'b';
                break;
            case '\t':
                dst[dst_end++] = '\\';
                dst[dst_end++] = 't';
                break;
            case '\\':
                dst[dst_end++] = '\\';
                dst[dst_end++] = '\\';
                break;
            // default do not encode
            default:
                dst[dst_end++] = src[src_end - 1];
                break;
        }
    }

    return dst;
}

static int lcr_spec_write_config(int fd, const struct isula_linked_list *lcr_conf)
{
    size_t len;
    char *line = NULL;
    struct isula_linked_list *it = NULL;
    char *line_encode = NULL;
    int ret = -1;

    isula_linked_list_for_each(it, lcr_conf) {
        lcr_config_item_t *item = it->elem;
        int nret;
        size_t encode_len;
        ssize_t nwritten = -1;
        if (item != NULL) {
            if (strlen(item->value) > ((SIZE_MAX - strlen(item->name)) - 4)) {
                goto cleanup;
            }
            len = strlen(item->name) + 3 + strlen(item->value) + 1;
            line = isula_common_calloc_s(len);
            if (line == NULL) {
                ERROR("Out of memory");
                goto cleanup;
            }

            nret = snprintf(line, len, "%s = %s", item->name, item->value);

            if (nret < 0 || (size_t)nret >= len) {
                ERROR("Sprintf failed");
                goto cleanup;
            }

            line_encode = escape_string_encode(line);
            if (line_encode == NULL) {
                ERROR("String encode failed");
                goto cleanup;
            }

            encode_len = strlen(line_encode);

            line_encode[encode_len] = '\n';
            nwritten = isula_file_total_write_nointr(fd, line_encode, encode_len + 1);
            if (nwritten < 0 || (size_t)nwritten != encode_len + 1) {
                SYSERROR("Write file failed");
                goto cleanup;
            }

            free(line);
            line = NULL;
            free(line_encode);
            line_encode = NULL;
        }
    }

    ret = 0;
cleanup:
    free(line);
    free(line_encode);
    return ret;
}

static char *lcr_get_bundle(const char *lcrpath, const char *name)
{
    size_t len = 0;
    int nret = 0;
    char *bundle = NULL;
    struct stat s;

    if (strlen(name) > ((SIZE_MAX - strlen(lcrpath)) - 2)) {
        goto cleanup;
    }

    /* bundle = lcrpath + '/' + name + '\0' */
    len = strlen(lcrpath) + strlen(name) + 2;
    bundle = isula_common_calloc_s(len);
    if (bundle == NULL) {
        ERROR("Out of memory");
        goto cleanup;
    }

    nret = snprintf(bundle, len, "%s/%s", lcrpath, name);
    if (nret < 0 || (size_t)nret >= len) {
        ERROR("Print bundle string failed");
        goto cleanup;
    }

    if (stat(bundle, &s) != 0) {
        switch (errno) {
            case EACCES:
                ERROR("You lack permission to access %s", bundle);
                break;
            case ENOENT:
                ERROR("Bundle %s does not exist", bundle);
                break;
            default:
                SYSERROR("Access %s failed", bundle);
        }
        goto cleanup;
    }
    return bundle;
cleanup:
    free(bundle);
    return NULL;
}

bool lcr_save_spec(const char *name, const char *lcrpath, const struct isula_linked_list *lcr_conf, const char *seccomp_conf)
{
    bool bret = false;
    const char *path = lcrpath ? lcrpath : LCRPATH;
    char *bundle = NULL;
    char *seccomp = NULL;
    int fd = -1;
    int nret = 0;

    if (name == NULL) {
        ERROR("Missing container name");
        return bret;
    }

    if (lcr_conf == NULL) {
        ERROR("Empty lcr conf");
        return bret;
    }

    bundle = lcr_get_bundle(path, name);
    if (bundle == NULL) {
        goto out_free;
    }

    if (seccomp_conf != NULL) {
        seccomp = lcr_save_seccomp_file(bundle, seccomp_conf);
        if (seccomp == NULL) {
            goto out_free;
        }
    }

    fd = lcr_open_config_file(bundle);
    if (fd == -1) {
        goto out_free;
    }

    if (lcr_spec_write_config(fd, lcr_conf)) {
        goto out_free;
    }

    if (seccomp_conf != NULL) {
        nret = lcr_spec_write_seccomp_line(fd, seccomp);
        if (nret) {
            goto out_free;
        }
    }

    bret = true;

out_free:
    free(bundle);
    free(seccomp);
    if (fd != -1) {
        close(fd);
    }

    return bret;
}

static int lcr_write_file(const char *path, const char *data, size_t len)
{
    char *real_path = NULL;
    int fd = -1;
    int ret = -1;
    ssize_t nwritten = -1;

    if (path == NULL || strlen(path) == 0 || data == NULL || len == 0) {
        return -1;
    }

    if (isula_file_ensure_path(&real_path, path) < 0) {
        ERROR("Failed to ensure path %s", path);
        goto out_free;
    }

    fd = isula_file_open(real_path, O_CREAT | O_TRUNC | O_CLOEXEC | O_WRONLY, CONFIG_FILE_MODE);
    if (fd == -1) {
        ERROR("Create file %s failed", real_path);
        lcr_set_error_message(LCR_ERR_RUNTIME, "Create file %s failed", real_path);
        goto out_free;
    }

    nwritten = isula_file_total_write_nointr(fd, data, len);
    if (nwritten < 0 || (size_t)nwritten != len) {
        SYSERROR("write data to %s failed", real_path);
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
        SYSERROR("write json hooks failed");
        goto out_free;
    }

    ret = true;

out_free:
    free(err);
    free(json_hooks);

    return ret;
}

#define OCIHOOKSFILE "ocihooks.json"
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

bool translate_spec(const struct lxc_container *c, oci_runtime_spec *container)
{
    bool ret = false;
    struct isula_linked_list *lcr_conf = NULL;
    char *seccomp_conf = NULL;

    INFO("Translate new specification file");

    lcr_conf = lcr_oci2lcr(c, container, &seccomp_conf);
    if (lcr_conf == NULL) {
        ERROR("Translate configuration failed");
        goto out_free_conf;
    }

    if (container->hooks != NULL && !lcr_save_ocihooks(c->name, c->config_path, container->hooks)) {
        ERROR("Failed to save %s", OCIHOOKSFILE);
        goto out_free_conf;
    }

    if (!lcr_save_spec(c->name, c->config_path, lcr_conf, seccomp_conf)) {
        ERROR("Failed to save configuration");
        goto out_free_conf;
    }

    ret = true;

out_free_conf:
    lcr_free_config(lcr_conf);
    free(lcr_conf);

    free(seccomp_conf);
    return ret;
}

static void delete_specific_spec(const char *bundle, const char *name)
{
    char filepath[PATH_MAX] = { 0 };
    int nret = snprintf(filepath, sizeof(filepath), "%s/%s", bundle, name);
    if (nret < 0 || (size_t)nret >= sizeof(filepath)) {
        ERROR("Failed to print string");
        return;
    }

    if (unlink(filepath) != 0) {
        SYSERROR("Failed to delete %s", filepath);
        return;
    }
}

void lcr_delete_spec(const struct lxc_container *c, oci_runtime_spec *container)
{
    const char *path = NULL;
    const char *name = NULL;
    char *bundle = NULL;

    if (c == NULL || c->name == NULL || container == NULL) {
        ERROR("Invalid arguments");
        return;
    }

    path = c->config_path ? c->config_path : LCRPATH;
    name = c->name;
    bundle = lcr_get_bundle(path, name);
    if (bundle == NULL) {
        return;
    }

    if (container->hooks != NULL) {
        delete_specific_spec(bundle, OCIHOOKSFILE);
    }

    delete_specific_spec(bundle, "config");

    // There might not exist seccomp file, try to delete anyway
    delete_specific_spec(bundle, "seccomp");

    free(bundle);
}
