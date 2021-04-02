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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/utsname.h>
#include <linux/oom.h>

#include "conf.h"
#include "lcrcontainer.h"
#include "lcrcontainer_extend.h"
#include "error.h"
#include "utils.h"
#include "log.h"
#include "buffer.h"

#define SUB_UID_PATH "/etc/subuid"
#define SUB_GID_PATH "/etc/subgid"
#define ID_MAP_LEN 100

/* files limit checker for cgroup v1 */
static int files_limit_checker_v1(const char *value)
{
    long long limit = 0;
    int ret = 0;
    int cgroup_version = 0;

    cgroup_version = lcr_util_get_cgroup_version();
    if (cgroup_version < 0) {
        return -1;
    }

    // If cgroup version not match, skip the item
    if (cgroup_version != CGROUP_VERSION_1) {
        return 1;
    }

    ret = lcr_util_safe_llong(value, &limit);
    if (ret) {
        ret = -1;
    }

    return ret;
}

/* files limit checker for cgroup v2 */
static int files_limit_checker_v2(const char *value)
{
    long long limit = 0;
    int ret = 0;
    int cgroup_version = 0;

    cgroup_version = lcr_util_get_cgroup_version();
    if (cgroup_version < 0) {
        return -1;
    }

    // If cgroup version not match, skip the item
    if (cgroup_version != CGROUP_VERSION_2) {
        return 1;
    }

    ret = lcr_util_safe_llong(value, &limit);
    if (ret) {
        ret = -1;
    }

    return ret;
}

/* check console log file */
static int check_console_log_file(const char *value)
{
    int ret = 0;

    if (value == NULL) {
        return -1;
    }

    if (strcmp(value, "none") == 0) {
        ret = 1;
    }

    return ret;
}

/* check console log filesize */
static int check_console_log_filesize(const char *value)
{
    int ret = -1;
    int64_t tmp = 0;
    int64_t min = 4 * SIZE_KB;

    if (value == NULL) {
        return ret;
    }

    if (lcr_parse_byte_size_string(value, &tmp) == 0 && tmp >= min) {
        ret = 0;
    }

    return ret;
}

/* check oom score adj */
static int check_oom_score_adj(const char *value)
{
    int ret = -1;
    int tmp = 0;
    int min = OOM_SCORE_ADJ_MIN;
    int max = OOM_SCORE_ADJ_MAX;

    if (value == NULL) {
        return ret;
    }

    if (lcr_util_safe_int(value, &tmp) == 0 && tmp >= min && tmp <= max) {
        ret = 0;
    }
    lcr_set_error_message(LCR_ERR_RUNTIME, "Invalid value %s, range for oom score adj is [%d, %d]", value, min, max);
    return ret;
}
/* check console log filerotate */
static int check_console_log_filerotate(const char *value)
{
    int ret = -1;
    unsigned int tmp = 0;

    if (value == NULL) {
        return ret;
    }

    if (lcr_util_safe_uint(value, &tmp) == 0) {
        ret = 0;
    }

    return ret;
}

/* check rootfs mount */
static int check_rootfs_mount(const char *value)
{
    if (value == NULL) {
        return -1;
    }

    if (!lcr_util_dir_exists(value)) {
        lcr_set_error_message(LCR_ERR_RUNTIME, "Container rootfs mount path '%s' is not exist", value);
        return -1;
    }

    return 0;
}

static inline bool is_native_umask_normal(const char *value)
{
    return strcmp(value, "normal") == 0;
}

static inline bool is_native_umask_secure(const char *value)
{
    return strcmp(value, "secure") == 0;
}


/* check umask */
static int check_native_umask(const char *value)
{
    if (value == NULL) {
        return -1;
    }

    if (!is_native_umask_normal(value) && !is_native_umask_secure(value)) {
        ERROR("Invalid native umask: %s", value);
        return -1;
    }

    return 0;
}

/* check system container */
static int check_system_container(const char *value)
{
    if (value == NULL) {
        return -1;
    }

    if (strcmp(value, "true") != 0) {
        ERROR("Invalid system container: %s", value);
        return -1;
    }

    return 0;
}

/* check cgroup dir */
static int check_cgroup_dir(const char *value)
{
    if (value == NULL) {
        return -1;
    }
    return 0;
}

static int check_empty_value(const char *value)
{
    if (value == NULL || strlen(value) == 0) {
        return -1;
    }
    return 0;
}

static int check_console_log_driver(const char *value)
{
    if (value == NULL || strlen(value) == 0) {
        return -1;
    }
    if (strcmp(value, "syslog") == 0) {
        return 0;
    }
    if (strcmp(value, "json-file") == 0) {
        return 0;
    }
    return -1;
}

static const lcr_annotation_item_t g_require_annotations[] = {
    {
        "files.limit",
        "lxc.cgroup.files.limit",
        files_limit_checker_v1,
    },
    {
        "files.limit",
        "lxc.cgroup2.files.limit",
        files_limit_checker_v2,
    },
    {
        "log.console.file",
        "lxc.console.logfile",
        check_console_log_file,
    },
    {
        "log.console.filesize",
        "lxc.console.size",
        check_console_log_filesize,
    },
    {
        "log.console.filerotate",
        "lxc.console.rotate",
        check_console_log_filerotate,
    },
    {
        "log.console.driver",
        "lxc.console.logdriver",
        check_console_log_driver,
    },
    {
        "log.console.tag",
        "lxc.console.syslog_tag",
        check_empty_value,
    },
    {
        "log.console.facility",
        "lxc.console.syslog_facility",
        check_empty_value,
    },
    {
        "rootfs.mount",
        "lxc.rootfs.mount",
        check_rootfs_mount,
    },
    {
        "cgroup.dir",
        "lxc.cgroup.dir",
        check_cgroup_dir,
    },
    {
        "native.umask",
        "lxc.isulad.umask",
        check_native_umask,
    },
    {
        "system.container",
        "lxc.isulad.systemd",
        check_system_container,
    },
    {
        "proc.oom_score_adj",
        "lxc.proc.oom_score_adj",
        check_oom_score_adj,
    },
};

/* create lcr list node */
struct lcr_list *create_lcr_list_node(const char *key, const char *value)
{
    struct lcr_list *node = NULL;
    lcr_config_item_t *entry = NULL;

    node = lcr_util_common_calloc_s(sizeof(*node));
    if (node == NULL) {
        return NULL;
    }
    entry = lcr_util_common_calloc_s(sizeof(*entry));
    if (entry == NULL) {
        free(node);
        return NULL;
    }
    entry->name = lcr_util_strdup_s(key);

    entry->value = lcr_util_strdup_s(value);

    node->elem = entry;
    return node;
}

/* free lcr list node */
void free_lcr_list_node(struct lcr_list *node)
{
    lcr_config_item_t *entry = NULL;

    if (node == NULL) {
        return;
    }

    entry = node->elem;
    if (entry != NULL) {
        free(entry->name);
        free(entry->value);
    }
    free(node->elem);
    node->elem = NULL;
    free(node);
}

/* trans oci hostname */
struct lcr_list *trans_oci_hostname(const char *hostname)
{
    if (hostname == NULL) {
        return NULL;
    }

    return create_lcr_list_node("lxc.uts.name", hostname);
}

static bool valid_sep_len(size_t sep_len, size_t len)
{
    if (sep_len && (sep_len != 1) && (len > SIZE_MAX / sep_len + 1)) {
        return false;
    }

    return true;
}

/* capabilities join */
static char *capabilities_join(const char *sep, const char **parts, size_t len)
{
    char *result = NULL;
    size_t sep_len;
    size_t result_len;
    size_t iter;

    sep_len = strlen(sep);
    if (valid_sep_len(sep_len, len) == false) {
        return NULL;
    }
    result_len = (len - 1) * sep_len;
    /* calculate new string length
     * dont calculate `CAP_`
     */
    for (iter = 0; iter < len; iter++) {
        if (result_len > 4 && (result_len - 4 >= SIZE_MAX - strlen(parts[iter]))) {
            return NULL;
        }
        result_len += strlen(parts[iter]) - 4;
    }

    result = calloc(result_len + 1, 1);
    if (result == NULL) {
        return NULL;
    }

    for (iter = 0; iter < len - 1; iter++) {
        (void)strcat(result, &(parts[iter][4]));
        (void)strcat(result, sep);
    }
    (void)strcat(result, &(parts[len - 1][4]));

    // Lower case
    for (iter = 0; iter < result_len; iter++) {
        if (result[iter] >= 'A' && result[iter] <= 'Z') {
            result[iter] = (char)(result[iter] + 32);
        }
    }

    return result;
}

#define UID_MAX_SIZE 21
/* UID to use within a private user namespace for init */
static int trans_oci_process_init_uid(const defs_process *proc, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    char buf[UID_MAX_SIZE] = { 0 };
    int nret;
    int ret = -1;
    if (proc->user != NULL && proc->user->uid != INVALID_INT) {
        nret = snprintf(buf, sizeof(buf), "%u", (unsigned int)proc->user->uid);
        if (nret < 0 || (size_t)nret >= sizeof(buf)) {
            goto out;
        }

        node = create_lcr_list_node("lxc.init.uid", buf);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    ret = 0;
out:
    return ret;
}

/* GID to use within a private user namespace for init */
static int trans_oci_process_init_gid(const defs_process *proc, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    char buf[UID_MAX_SIZE] = { 0 };
    int nret;
    int ret = -1;
    if (proc->user != NULL && proc->user->gid != INVALID_INT) {
        nret = snprintf(buf, sizeof(buf), "%u", (unsigned int)proc->user->gid);
        if (nret < 0 || (size_t)nret >= sizeof(buf)) {
            goto out;
        }

        node = create_lcr_list_node("lxc.init.gid", buf);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    ret = 0;
out:
    return ret;
}

/* additional groups for init command */
static int trans_oci_process_init_groups(const defs_process *proc, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int nret;
    size_t i = 0;
    int ret = -1;
    if (proc->user != NULL && proc->user->additional_gids != NULL && proc->user->additional_gids_len > 0) {
        if (proc->user->additional_gids_len > (SIZE_MAX / (LCR_NUMSTRLEN64 + 1))) {
            goto out;
        }

        size_t total_len = (LCR_NUMSTRLEN64 + 1) * proc->user->additional_gids_len;
        char *gids = lcr_util_common_calloc_s(total_len);
        if (gids == NULL) {
            goto out;
        }

        nret = snprintf(gids, total_len, "%u", (unsigned int)(proc->user->additional_gids[0]));
        if (nret < 0 || (size_t)nret >= total_len) {
            free(gids);
            goto out;
        }
        for (i = 1; i < proc->user->additional_gids_len; i++) {
            size_t old_len = strlen(gids);
            nret = snprintf(gids + old_len, total_len - old_len, " %u",
                            (unsigned int)(proc->user->additional_gids[i]));
            if (nret < 0 || (size_t)nret >= (total_len - old_len)) {
                free(gids);
                goto out;
            }
        }

        node = create_lcr_list_node("lxc.isulad.init.groups", gids);
        free(gids);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    ret = 0;
out:
    return ret;
}

/* Sets the command to use as the init system for the containers */
static int trans_oci_process_init_args(const defs_process *proc, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    size_t i = 0;
    int ret = -1;
    for (i = 0; i < proc->args_len; i++) {
        node = create_lcr_list_node("lxc.isulad.init.args", proc->args[i]);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    ret = 0;
out:
    return ret;
}

/* working directory to use within container */
static int trans_oci_process_init_cwd(const defs_process *proc, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int ret = -1;
    if (proc->cwd != NULL) {
        node = create_lcr_list_node("lxc.init.cwd", proc->cwd);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    ret = 0;
out:
    return ret;
}

/* trans oci process init */
static int trans_oci_process_init(const defs_process *proc, struct lcr_list *conf)
{
    int ret = -1;
    if (trans_oci_process_init_uid(proc, conf)) {
        goto out;
    }

    if (trans_oci_process_init_gid(proc, conf)) {
        goto out;
    }

    if (trans_oci_process_init_groups(proc, conf)) {
        goto out;
    }

    if (trans_oci_process_init_args(proc, conf)) {
        goto out;
    }

    if (trans_oci_process_init_cwd(proc, conf)) {
        goto out;
    }

    ret = 0;
out:
    return ret;
}

/* trans oci process env and cap */
static int trans_oci_process_env_and_cap(const defs_process *proc, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    char *boundings = NULL;
    int ret = -1;
    size_t i;

    for (i = 0; i < proc->env_len; i++) {
        char *replaced = lcr_util_string_replace(" ", SPACE_MAGIC_STR, proc->env[i]);
        if (replaced == NULL) {
            ERROR("memory allocation error");
            goto out;
        }
        node = create_lcr_list_node("lxc.environment", replaced);
        free(replaced);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }

    if (proc->capabilities != NULL && proc->capabilities->bounding_len > 0) {
        boundings =
            capabilities_join(" ", (const char **)(proc->capabilities->bounding), proc->capabilities->bounding_len);
        if (boundings == NULL) {
            ERROR("Failed to join bounding capabilities");
            goto out;
        }
        node = create_lcr_list_node("lxc.cap.keep", boundings);
        free(boundings);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    } else {
        node = create_lcr_list_node("lxc.cap.keep", "ISULAD_KEEP_NONE");
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    ret = 0;
out:
    return ret;
}

/* trans oci process prlimit */
static int trans_oci_process_prlimit(const defs_process *proc, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int ret = -1;
    int nret;
    size_t i;

    for (i = 0; i < proc->rlimits_len; i++) {
        defs_process_rlimits_element *lr = proc->rlimits[i];
        char buf_key[30] = { 0 };
        char buf_value[60] = { 0 };
        size_t j;
        char *type = lcr_util_strdup_s(lr->type);

        // Lower case type,eg. RLIMIT_NOFILE -> RLIMIT_nofile
        for (j = strlen("RLIMIT_"); j < strlen(type); j++) {
            type[j] = (char)tolower(type[j]);
        }

        // Skip `RLIMIT_`
        nret = snprintf(buf_key, sizeof(buf_key), "lxc.prlimit.%s", &(type[7]));
        free(type);
        if (nret < 0 || (size_t)nret >= sizeof(buf_key)) {
            goto out;
        }

        // We always use format `soft_limit:hard_limit`
        nret = snprintf(buf_value, sizeof(buf_value), "%llu:%llu", (unsigned long long)lr->soft,
                        (unsigned long long)lr->hard);
        if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
            goto out;
        }

        node = create_lcr_list_node(buf_key, buf_value);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    ret = 0;
out:
    return ret;
}

/* trans oci process no new privs */
static int trans_oci_process_no_new_privs(const defs_process *proc, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int ret = -1;

    if (proc->no_new_privileges) {
        node = create_lcr_list_node("lxc.no_new_privs", "1");
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    ret = 0;
out:
    return ret;
}

static int trans_oci_process_apparmor(const defs_process *proc, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int ret = -1;

    if (proc->apparmor_profile != NULL) {
        node = create_lcr_list_node("lxc.aa_profile", proc->apparmor_profile);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }

    ret = 0;
out:
    return ret;
}

static int trans_oci_process_selinux(const defs_process *proc, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int ret = -1;

    if (proc->selinux_label != NULL) {
        node = create_lcr_list_node("lxc.selinux.context", proc->selinux_label);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }

    ret = 0;
out:
    return ret;
}

/* trans oci process apparmor and selinux */
static int trans_oci_process_apparmor_and_selinux(const defs_process *proc, struct lcr_list *conf)
{
    int ret = -1;

    if (trans_oci_process_apparmor(proc, conf) != 0) {
        goto out;
    }

    if (trans_oci_process_selinux(proc, conf) != 0) {
        goto out;
    }

    ret = 0;
out:
    return ret;
}

/* trans oci process */
struct lcr_list *trans_oci_process(const defs_process *proc)
{
    struct lcr_list *conf = NULL;

    conf = lcr_util_common_calloc_s(sizeof(struct lcr_list));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    if (trans_oci_process_init(proc, conf)) {
        goto out_free;
    }

    if (trans_oci_process_env_and_cap(proc, conf)) {
        goto out_free;
    }

    if (trans_oci_process_prlimit(proc, conf)) {
        goto out_free;
    }

    if (trans_oci_process_no_new_privs(proc, conf)) {
        goto out_free;
    }

    if (trans_oci_process_apparmor_and_selinux(proc, conf)) {
        goto out_free;
    }

    return conf;

out_free:
    lcr_free_config(conf);
    free(conf);
    return NULL;
}

#define APPEND_COMMA_END_SIZE 2
/* trans oci root rootfs */
static int trans_oci_root_rootfs(const oci_runtime_spec_root *root, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int ret = -1;

    if ((root != NULL) && root->path != NULL) {
        if (strcmp(root->path, "/") != 0) {
            node = create_lcr_list_node("lxc.rootfs.path", root->path);
            if (node == NULL) {
                goto out;
            }
            lcr_list_add_tail(conf, node);
        }
    }
    ret = 0;
out:
    return ret;
}

static inline bool is_root_readonly(const oci_runtime_spec_root *root)
{
    return root != NULL && root->readonly;
}

/* trans oci root rootfsoptions */
static int trans_oci_root_rootfs_options(const oci_runtime_spec_root *root, struct lcr_list *conf,
                                         const oci_runtime_config_linux *linux)
{
    struct lcr_list *node = NULL;
    char *value = NULL;
    char *tmpvalue = NULL;
    int ret = -1;
    int nret;

    if (is_root_readonly(root)) {
        value = lcr_util_strdup_s("ro");
    }

    if ((linux != NULL) && linux->rootfs_propagation != NULL) {
        if (value != NULL) {
            size_t newsize;
            if (strlen(value) > (SIZE_MAX - strlen(linux->rootfs_propagation)) - APPEND_COMMA_END_SIZE) {
                ERROR("Out of range!");
                goto out;
            }
            newsize = strlen(linux->rootfs_propagation) + strlen(value) + APPEND_COMMA_END_SIZE;
            nret = lcr_mem_realloc((void **)&tmpvalue, newsize, value, strlen(value));
            if (nret < 0) {
                ERROR("Out of memory");
                goto out;
            }
            value = tmpvalue;
            size_t tmp = newsize - strlen(value);
            nret = snprintf(value + strlen(value), tmp, ",%s", linux->rootfs_propagation);
            if (nret < 0 || (size_t)nret >= tmp) {
                ERROR("Failed to print string");
                goto out;
            }
        } else {
            value = lcr_util_strdup_s(linux->rootfs_propagation);
        }
    }

    if (value != NULL) {
        node = create_lcr_list_node("lxc.rootfs.options", value);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    ret = 0;
out:
    free(value);
    return ret;
}

/* trans oci root */
struct lcr_list *trans_oci_root(const oci_runtime_spec_root *root, const oci_runtime_config_linux *linux)
{
    struct lcr_list *conf = NULL;

    conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    if (trans_oci_root_rootfs(root, conf)) {
        goto out_free;
    }

    if (trans_oci_root_rootfs_options(root, conf, linux)) {
        goto out_free;
    }

    return conf;
out_free:
    lcr_free_config(conf);
    free(conf);

    return NULL;
}

static inline bool is_mount_options_invalid(const defs_mount *mount)
{
    return mount == NULL || mount->type == NULL;
}

static inline bool is_mount_type_bind(const char *type)
{
    return strcmp(type, "bind") == 0;
}

static inline bool is_mount_type_cgroup(const char *type)
{
    return strcmp(type, "cgroup") == 0;
}

static inline bool is_mount_type_sysfs(const char *type)
{
    return strcmp(type, "sysfs") == 0;
}

static inline bool is_mount_type_proc(const char *type)
{
    return strcmp(type, "proc") == 0;
}

/* trans mount to lxc options */
static char *trans_mount_to_lxc_options(const defs_mount *mount)
{
    char *result = NULL;
    char *prefix = NULL;
    char *lxc_options = NULL;
    int rc;
    bool isdir = true;
    struct stat st;

    if (is_mount_options_invalid(mount)) {
        return NULL;
    }

    if (is_mount_type_bind(mount->type)) {
        rc = stat(mount->source, &st);
        if (rc != 0) {
            ERROR("Failed to get stat of %s", mount->source);
            goto free_out;
        }
        isdir = S_ISDIR(st.st_mode);
    }

    lxc_options = isdir ? ",create=dir" : ",create=file";

    prefix = lcr_util_string_join(",", (const char **)mount->options, mount->options_len);
    if (prefix == NULL) {
        prefix = lcr_util_strdup_s("defaults");
    }

    result = lcr_util_string_append(lxc_options, prefix);
    free(prefix);
    return result;

free_out:
    free(prefix);
    free(result);
    return NULL;
}

static bool get_mount_option_ro(const defs_mount *mount)
{
    bool ro = false;

    if ((mount->options != NULL) && mount->options_len) {
        size_t i = 0;
        for (i = 0; i < mount->options_len; i++) {
            if ((mount->options[i] != NULL) && !strcmp(mount->options[i], "ro")) {
                ro = true;
                break;
            }
        }
    }

    return ro;
}

static char *get_mount_readmode_options(const defs_mount *mount, const char *type)
{
    char *options = NULL;
    bool readonly = get_mount_option_ro(mount);

    if (is_mount_type_cgroup(type)) {
        if (readonly) {
            options = lcr_util_strdup_s("ro:force");
        } else {
            options = lcr_util_strdup_s("rw:force");
        }
    } else {
        if (readonly) {
            options = lcr_util_strdup_s("ro");
        } else {
            options = lcr_util_strdup_s("rw");
        }
    }

    return options;
}

/* trans mount auto to lxc */
static struct lcr_list *trans_mount_auto_to_lxc(const defs_mount *mount)
{
    struct lcr_list *node = NULL;
    size_t buf_len = 0;
    char *buf = NULL;
    char *options = NULL;
    int ret;
    char *type = NULL;

    if (is_mount_options_invalid(mount)) {
        ERROR("oci container mounts element(type) is empty");
        return NULL;
    }
    type = mount->type;
    if (is_mount_type_sysfs(type)) {
        type = "sys";
    }

    options = get_mount_readmode_options(mount, type);
    if (options == NULL) {
        ERROR("Failed to trans to lxc options");
        goto out_free;
    }

    buf_len = strlen(type) + strlen(options) + 2;
    buf = calloc(buf_len, 1);
    if (buf == NULL) {
        DEBUG("Out of memory");
        goto out_free;
    }

    ret = snprintf(buf, buf_len, "%s:%s", type, options);
    if (ret < 0 || (size_t)ret >= buf_len) {
        DEBUG("Failed to print string");
        goto out_free;
    }
    node = create_lcr_list_node("lxc.mount.auto", buf);

out_free:
    free(options);
    free(buf);
    return node;
}

/* trans mount entry to lxc */
static struct lcr_list *trans_mount_entry_to_lxc(const defs_mount *mount)
{
    struct lcr_list *node = NULL;
    size_t buf_len = 0;
    char *buf = NULL;
    char *options = NULL;
    char *replaced_dest = NULL;
    int ret;

    char *replaced_source = lcr_util_string_replace(" ", SPACE_MAGIC_STR, mount->source);
    if (replaced_source == NULL) {
        ERROR("memory allocation error");
        goto err_out;
    }
    replaced_dest = lcr_util_string_replace(" ", SPACE_MAGIC_STR, mount->destination);
    if (replaced_dest == NULL) {
        ERROR("memory allocation error");
        free(replaced_source);
        goto err_out;
    }

    options = trans_mount_to_lxc_options(mount);
    if (options == NULL) {
        ERROR("Failed to trans to lxc options");
        goto out_free;
    }

    buf_len = strlen(replaced_dest) + strlen(mount->type) + strlen(replaced_source) + strlen(options) + 8;
    buf = calloc(buf_len, 1);
    if (buf == NULL) {
        ERROR("Out of memory");
        goto out_free;
    }

    ret = snprintf(buf, buf_len, "%s %s %s %s 0 0", replaced_source, replaced_dest + 1, mount->type, options);
    if (ret < 0 || (size_t)ret >= buf_len) {
        ERROR("Failed to print string");
        goto out_free;
    }
    node = create_lcr_list_node("lxc.mount.entry", buf);

out_free:
    free(options);
    free(buf);
    free(replaced_source);
    free(replaced_dest);
err_out:
    return node;
}

bool is_system_container(const oci_runtime_spec *container)
{
    size_t i = 0;
    for (i = 0; container->annotations != NULL && i < container->annotations->len; i++) {
        if (strcmp(container->annotations->keys[i], "system.container") == 0) {
            return true;
        }
    }
    return false;
}

static bool is_external_rootfs(const oci_runtime_spec *container)
{
    size_t i = 0;
    for (i = 0; container->annotations != NULL && i < container->annotations->len; i++) {
        if (strcmp(container->annotations->keys[i], "external.rootfs") == 0) {
            return true;
        }
    }
    return false;
}

static struct lcr_list *trans_oci_mounts_normal(const defs_mount *tmp)
{
    struct lcr_list *node = NULL;
    if (is_mount_type_cgroup(tmp->type) || is_mount_type_proc(tmp->type) || is_mount_type_sysfs(tmp->type)) {
        node = trans_mount_auto_to_lxc(tmp);
    } else {
        node = trans_mount_entry_to_lxc(tmp);
    }

    return node;
}

static struct lcr_list *trans_oci_mounts_system_container(const defs_mount *tmp)
{
    struct lcr_list *node = NULL;
    if (is_mount_type_cgroup(tmp->type) || (is_mount_type_proc(tmp->source) && is_mount_type_proc(tmp->type)) ||
        is_mount_type_sysfs(tmp->type)) {
        node = trans_mount_auto_to_lxc(tmp);
    } else {
        node = trans_mount_entry_to_lxc(tmp);
    }

    return node;
}

static struct lcr_list *trans_oci_mounts_node(const oci_runtime_spec *c, const defs_mount *tmp)
{
    struct lcr_list *node = NULL;
    // system container
    if (is_system_container(c)) {
        node = trans_oci_mounts_system_container(tmp);
    } else {
        node = trans_oci_mounts_normal(tmp);
    }

    return node;
}

static inline bool is_mount_destination_dev(const char *destination)
{
    return destination != NULL && strcmp(destination, "/dev") == 0;
}

static inline bool should_ignore_dev_mount(const defs_mount *tmp, bool system_container, bool external_rootfs)
{
    return system_container && external_rootfs && is_mount_destination_dev(tmp->destination);
}

/* trans oci mounts */
struct lcr_list *trans_oci_mounts(const oci_runtime_spec *c)
{
    struct lcr_list *conf = NULL;
    struct lcr_list *node = NULL;
    defs_mount *tmp = NULL;
    size_t i;
    bool system_container = is_system_container(c);
    bool external_rootfs = is_external_rootfs(c);

    conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    for (i = 0; i < c->mounts_len; i++) {
        tmp = c->mounts[i];
        if (tmp->type == NULL) {
            goto out_free;
        }

        if (should_ignore_dev_mount(tmp, system_container, external_rootfs)) {
            continue;
        }
        node = trans_oci_mounts_node(c, tmp);
        if (node == NULL) {
            goto out_free;
        }
        lcr_list_add_tail(conf, node);
    }

    return conf;
out_free:
    lcr_free_config(conf);
    free(conf);

    return NULL;
}

static int trans_one_oci_id_mapping(struct lcr_list *conf, const char *typ, const defs_id_mapping *id, const char *path)
{
    int nret;
    struct lcr_list *node = NULL;
    char buf_value[300] = { 0 };
    char subid[ID_MAP_LEN] = { 0 };

    nret = snprintf(buf_value, sizeof(buf_value), "%s %u %u %u", typ, id->container_id, id->host_id, id->size);
    if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
        return -1;
    }

    node = create_lcr_list_node("lxc.idmap", buf_value);
    if (node == NULL) {
        return -1;
    }
    lcr_list_add_tail(conf, node);

    nret = snprintf(subid, sizeof(subid), "%u:%u:%u", id->container_id, id->host_id, id->size);
    if (nret < 0 || (size_t)nret >= sizeof(subid)) {
        return -1;
    }
    nret = lcr_util_atomic_write_file(path, subid);
    if (nret < 0) {
        return -1;
    }
    return 0;
}

static int trans_oci_uid_mapping(struct lcr_list *conf, defs_id_mapping **uid_mappings, size_t uid_mappings_len)
{
    size_t i;

    for (i = 0; uid_mappings != NULL && i < uid_mappings_len; i++) {
        int nret = trans_one_oci_id_mapping(conf, "u", uid_mappings[i], SUB_UID_PATH);
        if (nret < 0) {
            return nret;
        }
    }
    return 0;
}

static int trans_oci_gid_mapping(struct lcr_list *conf, defs_id_mapping **gid_mappings, size_t gid_mappings_len)
{
    size_t i;

    for (i = 0; gid_mappings != NULL && i < gid_mappings_len; i++) {
        int nret = trans_one_oci_id_mapping(conf, "g", gid_mappings[i], SUB_GID_PATH);
        if (nret < 0) {
            return nret;
        }
    }
    return 0;
}

/* trans oci id mapping */
static struct lcr_list *trans_oci_id_mapping(const oci_runtime_config_linux *l)
{
    struct lcr_list *conf = NULL;
    int nret = 0;

    conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    nret = trans_oci_uid_mapping(conf, l->uid_mappings, l->uid_mappings_len);
    if (nret < 0) {
        goto out_free;
    }

    nret = trans_oci_gid_mapping(conf, l->gid_mappings, l->gid_mappings_len);
    if (nret < 0) {
        goto out_free;
    }

    return conf;

out_free:
    lcr_free_config(conf);
    free(conf);

    return NULL;
}

#define WILDCARD (-1LL)

static int trans_conf_int(struct lcr_list *conf, const char *lxc_key, int val)
{
    struct lcr_list *node = NULL;
    char buf_value[300] = { 0 };
    int nret;

    nret = snprintf(buf_value, sizeof(buf_value), "%d", val);
    if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
        return -1;
    }
    node = create_lcr_list_node(lxc_key, buf_value);
    if (node == NULL) {
        return -1;
    }
    lcr_list_add_tail(conf, node);
    return 0;
}

static int trans_conf_uint32(struct lcr_list *conf, const char *lxc_key, uint32_t val)
{
    struct lcr_list *node = NULL;
    char buf_value[300] = { 0 };
    int nret;

    nret = snprintf(buf_value, sizeof(buf_value), "%u", (unsigned int)val);
    if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
        return -1;
    }
    node = create_lcr_list_node(lxc_key, buf_value);
    if (node == NULL) {
        return -1;
    }
    lcr_list_add_tail(conf, node);
    return 0;
}

static int trans_conf_int64(struct lcr_list *conf, const char *lxc_key, int64_t val)
{
    struct lcr_list *node = NULL;
    char buf_value[300] = { 0 };
    int nret;

    nret = snprintf(buf_value, sizeof(buf_value), "%lld", (long long)val);
    if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
        return -1;
    }
    node = create_lcr_list_node(lxc_key, buf_value);
    if (node == NULL) {
        return -1;
    }
    lcr_list_add_tail(conf, node);
    return 0;
}

static int trans_conf_uint64(struct lcr_list *conf, const char *lxc_key, uint64_t val)
{
    struct lcr_list *node = NULL;
    char buf_value[300] = { 0 };
    int nret;

    nret = snprintf(buf_value, sizeof(buf_value), "%llu", (unsigned long long)val);
    if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
        return -1;
    }
    node = create_lcr_list_node(lxc_key, buf_value);
    if (node == NULL) {
        return -1;
    }
    lcr_list_add_tail(conf, node);
    return 0;
}

static int trans_conf_string(struct lcr_list *conf, const char *lxc_key, const char *val)
{
    struct lcr_list *node = NULL;

    node = create_lcr_list_node(lxc_key, val);
    if (node == NULL) {
        return -1;
    }
    lcr_list_add_tail(conf, node);
    return 0;
}

/* trans resources mem swap of cgroup v1 */
static int trans_resources_mem_swap_v1(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;
    int nret;

    if (res->memory->reservation != INVALID_INT) {
        /* set soft limit of memory usage */
        nret = trans_conf_int64(conf, "lxc.cgroup.memory.soft_limit_in_bytes", res->memory->reservation);
        if (nret < 0) {
            goto out;
        }
    }
    if (res->memory->swap != INVALID_INT) {
        /* set limit of memory+swap usage */
        nret = trans_conf_int64(conf, "lxc.cgroup.memory.memsw.limit_in_bytes", res->memory->swap);
        if (nret < 0) {
            goto out;
        }
    }

    if (res->memory->swappiness != -1) {
        /* set swappiness parameter of vmscan */
        nret = trans_conf_uint64(conf, "lxc.cgroup.memory.swappiness", res->memory->swappiness);
        if (nret < 0) {
            goto out;
        }
    }
    ret = 0;
out:
    return ret;
}

static int trans_resources_mem_limit_v1(const defs_resources *res, struct lcr_list *conf)
{
    if (res->memory->limit != INVALID_INT) {
        /* set limit of memory usage */
        int nret = trans_conf_int64(conf, "lxc.cgroup.memory.limit_in_bytes", res->memory->limit);
        if (nret < 0) {
            return -1;
        }
    }
    return 0;
}

/* trans resources mem kernel of cgroup v1 */
static int trans_resources_mem_kernel_v1(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;
    int nret;

    if (res->memory->kernel != INVALID_INT) {
        /* set hard limit for kernel memory */
        nret = trans_conf_int64(conf, "lxc.cgroup.memory.kmem.limit_in_bytes", res->memory->kernel);
        if (nret < 0) {
            goto out;
        }
    }
    if (res->memory->kernel_tcp != INVALID_INT) {
        /* set hard limit for tcp buf memory */
        nret = trans_conf_int64(conf, "lxc.cgroup.memory.kmem.tcp.limit_in_bytes", res->memory->kernel_tcp);
        if (nret < 0) {
            goto out;
        }
    }
    ret = 0;
out:
    return ret;
}

static int trans_resources_mem_disable_oom_v1(const defs_resources *res, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    if (res->memory->disable_oom_killer) {
        node = create_lcr_list_node("lxc.cgroup.memory.oom_control", "1");
        if (node == NULL) {
            return -1;
        }
        lcr_list_add_tail(conf, node);
    }
    return 0;
}

/* trans resources memory of cgroup v1 */
static int trans_resources_memory_v1(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;

    if (res->memory == NULL) {
        return 0;
    }

    if (trans_resources_mem_limit_v1(res, conf) != 0) {
        goto out;
    }

    if (trans_resources_mem_swap_v1(res, conf) != 0) {
        goto out;
    }

    if (trans_resources_mem_kernel_v1(res, conf) != 0) {
        goto out;
    }

    if (trans_resources_mem_disable_oom_v1(res, conf) != 0) {
        goto out;
    }
    ret = 0;
out:
    return ret;
}

static int trans_conf_int64_with_max(struct lcr_list *conf, const char *lxc_key, int64_t val)
{
    int ret = 0;

    if (val == -1) {
        ret = trans_conf_string(conf, lxc_key, "max");
    } else {
        ret = trans_conf_int64(conf, lxc_key, val);
    }
    if (ret < 0) {
        return -1;
    }

    return ret;
}

static int trans_resources_devices_node_v1(const defs_device_cgroup *lrd, struct lcr_list *conf,
                                           const char *buf_value)
{
    struct lcr_list *node = NULL;
    int ret = -1;

    if (lrd->allow == true) {
        node = create_lcr_list_node("lxc.cgroup.devices.allow", buf_value);
    } else {
        node = create_lcr_list_node("lxc.cgroup.devices.deny", buf_value);
    }
    if (node == NULL) {
        goto out;
    }
    lcr_list_add_tail(conf, node);

    ret = 0;
out:
    return ret;
}

static int trans_resources_devices_no_match(const defs_device_cgroup *lrd, char *buf_value,
                                            size_t size)
{
    int ret = 0;
    if (lrd->minor != WILDCARD) {
        ret = snprintf(buf_value, size, "%s %lld:%lld %s", lrd->type ? lrd->type : "a", (long long)(lrd->major),
                       (long long)lrd->minor, lrd->access ? lrd->access : "rwm");
    } else {
        ret = snprintf(buf_value, size, "%s %lld:* %s", lrd->type ? lrd->type : "a", (long long)(lrd->major),
                       lrd->access ? lrd->access : "rwm");
    }

    return ret;
}

static int trans_resources_devices_match(const defs_device_cgroup *lrd, char *buf_value, size_t size)
{
    int ret = 0;
    if (lrd->minor != WILDCARD) {
        ret = snprintf(buf_value, size, "%s *:%lld %s", lrd->type ? lrd->type : "a", (long long)(lrd->minor),
                       lrd->access ? lrd->access : "rwm");
    } else {
        ret = snprintf(buf_value, size, "%s *:* %s", lrd->type ? lrd->type : "a", lrd->access ? lrd->access : "rwm");
    }

    return ret;
}

static int trans_resources_devices_ret(const defs_device_cgroup *lrd, char *buf_value, size_t size)
{
    int ret = 0;
    if (lrd->major != WILDCARD) {
        ret = trans_resources_devices_no_match(lrd, buf_value, size);
    } else {
        ret = trans_resources_devices_match(lrd, buf_value, size);
    }

    return ret;
}

/* trans resources devices for cgroup v1 */
static int trans_resources_devices_v1(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;
    size_t i = 0;
    char buf_value[300] = { 0 };

    for (i = 0; i < res->devices_len; i++) {
        defs_device_cgroup *lrd = res->devices[i];
        if (trans_resources_devices_ret(lrd, buf_value, sizeof(buf_value)) < 0) {
            goto out;
        }

        if (trans_resources_devices_node_v1(lrd, conf, buf_value) < 0) {
            goto out;
        }
    }
    ret = 0;
out:
    return ret;
}

/* trans resources cpu cfs */
static int trans_resources_cpu_cfs(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;

    if (res->cpu->quota != INVALID_INT) {
        if (trans_conf_int64(conf, "lxc.cgroup.cpu.cfs_quota_us", res->cpu->quota) < 0) {
            goto out;
        }
    }
    if (res->cpu->period != INVALID_INT) {
        if (trans_conf_uint64(conf, "lxc.cgroup.cpu.cfs_period_us", res->cpu->period) < 0) {
            goto out;
        }
    }
    ret = 0;
out:
    return ret;
}

/* trans resources cpu rt */
static int trans_resources_cpu_rt(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;

    if (res->cpu->realtime_runtime != INVALID_INT) {
        if (trans_conf_int64(conf, "lxc.cgroup.cpu.rt_runtime_us", res->cpu->realtime_runtime) < 0) {
            goto out;
        }
    }
    if (res->cpu->realtime_period != INVALID_INT) {
        if (trans_conf_uint64(conf, "lxc.cgroup.cpu.rt_period_us", res->cpu->realtime_period) < 0) {
            goto out;
        }
    }
    ret = 0;
out:
    return ret;
}

/* trans resources cpu set */
static int trans_resources_cpu_set(const defs_resources *res, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int ret = -1;

    if (res->cpu->cpus != NULL) {
        node = create_lcr_list_node("lxc.cgroup.cpuset.cpus", res->cpu->cpus);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    if (res->cpu->mems != NULL) {
        node = create_lcr_list_node("lxc.cgroup.cpuset.mems", res->cpu->mems);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }
    ret = 0;
out:
    return ret;
}

/* trans resources cpu shares */
static int trans_resources_cpu_shares(const defs_resources *res, struct lcr_list *conf)
{
    if (res->cpu->shares != INVALID_INT) {
        int nret = trans_conf_int64(conf, "lxc.cgroup.cpu.shares", (int64_t)(res->cpu->shares));
        if (nret < 0) {
            return -1;
        }
    }
    return 0;
}

/* trans resources cpu of cgroup v1 */
static int trans_resources_cpu_v1(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;

    if (res->cpu == NULL) {
        return 0;
    }

    if (trans_resources_cpu_cfs(res, conf)) {
        goto out;
    }

    if (trans_resources_cpu_rt(res, conf)) {
        goto out;
    }

    if (trans_resources_cpu_set(res, conf)) {
        goto out;
    }

    if (trans_resources_cpu_shares(res, conf)) {
        goto out;
    }

    ret = 0;

out:
    return ret;
}

/* trans resources blkio weight of cgroup v1 */
static int trans_blkio_weight_v1(const defs_resources_block_io *block_io, struct lcr_list *conf)
{
    int ret = -1;

    if (block_io->weight != INVALID_INT) {
        if (trans_conf_int(conf, "lxc.cgroup.blkio.weight", block_io->weight) < 0) {
            goto out;
        }
    }
    if (block_io->leaf_weight != INVALID_INT) {
        if (trans_conf_int(conf, "lxc.cgroup.blkio.leaf_weight", block_io->leaf_weight) < 0) {
            goto out;
        }
    }
    ret = 0;

out:
    return ret;
}

/* trans resources blkio wdevice of cgroup v1 */
static int trans_blkio_wdevice_v1(const defs_resources_block_io *block_io, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int ret = -1;
    size_t i = 0;
    char buf_value[300] = { 0 };

    for (i = 0; i < block_io->weight_device_len; i++) {
        int nret;
        defs_block_io_device_weight *wd = block_io->weight_device[i];
        if ((wd != NULL) && wd->weight != INVALID_INT) {
            nret = snprintf(buf_value, sizeof(buf_value), "%lld:%lld %d", (long long)(wd->major), (long long)wd->minor,
                            wd->weight);
            if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
                goto out;
            }

            node = create_lcr_list_node("lxc.cgroup.blkio.weight_device", buf_value);
            if (node == NULL) {
                goto out;
            }
            lcr_list_add_tail(conf, node);
        }
        if ((wd != NULL) && wd->leaf_weight != INVALID_INT) {
            nret = snprintf(buf_value, sizeof(buf_value), "%lld:%lld %d", (long long)(wd->major),
                            (long long)(wd->minor), wd->leaf_weight);
            if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
                goto out;
            }

            node = create_lcr_list_node("lxc.cgroup.blkio.leaf_weight_device", buf_value);
            if (node == NULL) {
                goto out;
            }
            lcr_list_add_tail(conf, node);
        }
    }
    ret = 0;
out:
    return ret;
}

/* trans resources blkio throttle of cgroup v1 */
static int trans_blkio_throttle_v1(defs_block_io_device_throttle **throttle, size_t len,
                                   const char *lxc_key, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int ret = -1;
    size_t i;

    if ((throttle == NULL) || len == 0) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        if (throttle[i] && throttle[i]->rate != INVALID_INT) {
            int nret;
            char buf_value[300] = { 0x00 };
            nret = snprintf(buf_value, sizeof(buf_value), "%lld:%lld %llu", (long long)throttle[i]->major,
                            (long long)(throttle[i]->minor), (unsigned long long)(throttle[i]->rate));
            if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
                goto out;
            }

            node = create_lcr_list_node(lxc_key, buf_value);
            if (node == NULL) {
                goto out;
            }
            lcr_list_add_tail(conf, node);
        }
    }
    ret = 0;
out:
    return ret;
}

/* trans resources blkio of cgroup v1 */
static int trans_resources_blkio_v1(const defs_resources_block_io *block_io, struct lcr_list *conf)
{
    int ret = -1;

    if (block_io == NULL) {
        return 0;
    }

    if (trans_blkio_weight_v1(block_io, conf)) {
        goto out;
    }

    if (trans_blkio_wdevice_v1(block_io, conf)) {
        goto out;
    }

    if (trans_blkio_throttle_v1(block_io->throttle_read_bps_device, block_io->throttle_read_bps_device_len,
                                "lxc.cgroup.blkio.throttle.read_bps_device", conf)) {
        goto out;
    }

    if (trans_blkio_throttle_v1(block_io->throttle_write_bps_device, block_io->throttle_write_bps_device_len,
                                "lxc.cgroup.blkio.throttle.write_bps_device", conf)) {
        goto out;
    }

    if (trans_blkio_throttle_v1(block_io->throttle_read_iops_device, block_io->throttle_read_iops_device_len,
                                "lxc.cgroup.blkio.throttle.read_iops_device", conf)) {
        goto out;
    }

    if (trans_blkio_throttle_v1(block_io->throttle_write_iops_device, block_io->throttle_write_iops_device_len,
                                "lxc.cgroup.blkio.throttle.write_iops_device", conf)) {
        goto out;
    }

    ret = 0;
out:
    return ret;
}

/* trans resources hugetlb of cgroup v1 */
static int trans_resources_hugetlb_v1(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;
    size_t i = 0;
    char buf_key[300] = { 0 };

    for (i = 0; i < res->hugepage_limits_len; i++) {
        defs_resources_hugepage_limits_element *lrhl = res->hugepage_limits[i];
        if (lrhl->page_size != NULL) {
            int nret = snprintf(buf_key, sizeof(buf_key), "lxc.cgroup.hugetlb.%s.limit_in_bytes", lrhl->page_size);
            if (nret < 0 || (size_t)nret >= sizeof(buf_key)) {
                goto out;
            }

            if (trans_conf_uint64(conf, buf_key, lrhl->limit) < 0) {
                return -1;
            }
        }
    }

    ret = 0;
out:
    return ret;
}

/* trans resources network of cgroup v1 */
static int trans_resources_network_v1(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;
    size_t i = 0;
    char buf_value[300] = { 0 };

    if (!res->network) {
        return 0;
    }

    if (res->network->class_id != INVALID_INT) {
        if (trans_conf_uint32(conf, "lxc.cgroup.net_cls.classid", res->network->class_id) < 0) {
            return -1;
        }
    }

    for (i = 0; i < res->network->priorities_len; i++) {
        defs_network_interface_priority *lrnp = res->network->priorities[i];
        if ((lrnp != NULL) && lrnp->name != NULL && lrnp->priority != INVALID_INT) {
            int nret = snprintf(buf_value, sizeof(buf_value), "%s %u", lrnp->name, lrnp->priority);
            if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
                goto out;
            }

            struct lcr_list *node = create_lcr_list_node("lxc.cgroup.net_prio.ifpriomap", buf_value);
            if (node == NULL) {
                goto out;
            }
            lcr_list_add_tail(conf, node);
        }
    }

    ret = 0;
out:
    return ret;
}

/* trans resources pids of cgroup v1 */
static int trans_resources_pids_v1(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;
    char buf_value[300] = { 0 };

    if (res->pids == NULL) {
        return 0;
    }

    if (res->pids->limit != INVALID_INT) {
        int nret;
        if (res->pids->limit == -1) {
            nret = snprintf(buf_value, sizeof(buf_value), "max");
        } else {
            nret = snprintf(buf_value, sizeof(buf_value), "%lld", (long long)(res->pids->limit));
        }
        if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
            goto out;
        }

        struct lcr_list *node = create_lcr_list_node("lxc.cgroup.pids.max", buf_value);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }

    ret = 0;
out:
    return ret;
}

/* trans oci resources to lxc cgroup config v1 */
static struct lcr_list *trans_oci_resources_v1(const defs_resources *res)
{
    struct lcr_list *conf = NULL;

    conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    if (trans_resources_devices_v1(res, conf)) {
        goto out_free;
    }

    if (trans_resources_memory_v1(res, conf)) {
        goto out_free;
    }

    if (trans_resources_cpu_v1(res, conf)) {
        goto out_free;
    }

    if (trans_resources_blkio_v1(res->block_io, conf)) {
        goto out_free;
    }

    if (trans_resources_hugetlb_v1(res, conf)) {
        goto out_free;
    }

    if (trans_resources_network_v1(res, conf)) {
        goto out_free;
    }

    if (trans_resources_pids_v1(res, conf)) {
        goto out_free;
    }

    return conf;

out_free:
    lcr_free_config(conf);
    free(conf);

    return NULL;
}

static int trans_resources_devices_node_v2(const defs_device_cgroup *lrd, struct lcr_list *conf,
                                           const char *buf_value)
{
    struct lcr_list *node = NULL;
    int ret = -1;

    if (lrd->allow == true) {
        node = create_lcr_list_node("lxc.cgroup2.devices.allow", buf_value);
    } else {
        node = create_lcr_list_node("lxc.cgroup2.devices.deny", buf_value);
    }
    if (node == NULL) {
        goto out;
    }
    lcr_list_add_tail(conf, node);

    ret = 0;
out:
    return ret;
}

/* trans resources devices for cgroup v2 */
static int trans_resources_devices_v2(const defs_resources *res, struct lcr_list *conf)
{
    int ret = -1;
    size_t i = 0;
    char buf_value[300] = { 0 };

    for (i = 0; i < res->devices_len; i++) {
        defs_device_cgroup *lrd = res->devices[i];
        if (trans_resources_devices_ret(lrd, buf_value, sizeof(buf_value)) < 0) {
            goto out;
        }

        if (trans_resources_devices_node_v2(lrd, conf, buf_value) < 0) {
            goto out;
        }
    }
    ret = 0;
out:
    return ret;
}

/* set limit of memory usage of cgroup v2 */
static int trans_resources_mem_limit_v2(const defs_resources *res, struct lcr_list *conf)
{
    if (res->memory->limit != INVALID_INT) {
        if (trans_conf_int64_with_max(conf, "lxc.cgroup2.memory.max", res->memory->limit) != 0) {
            return -1;
        }
    }

    if (res->memory->reservation != INVALID_INT) {
        if (trans_conf_int64_with_max(conf, "lxc.cgroup2.memory.low", res->memory->reservation) != 0) {
            return -1;
        }
    }

    return 0;
}

/* trans resources mem swap of cgroup v2 */
static int trans_resources_mem_swap_v2(const defs_resources *res, struct lcr_list *conf)
{
    int64_t swap = 0;

    if (res->memory->swap == INVALID_INT) {
        return 0;
    }

    if (lcr_util_get_real_swap(res->memory->limit, res->memory->swap, &swap) != 0) {
        return -1;
    }

    if (trans_conf_int64_with_max(conf, "lxc.cgroup2.memory.swap.max", swap) != 0) {
        return -1;
    }

    return 0;
}

/* trans resources memory of cgroup v2 */
static int trans_resources_memory_v2(const defs_resources *res, struct lcr_list *conf)
{
    if (res->memory == NULL) {
        return 0;
    }

    if (trans_resources_mem_limit_v2(res, conf) != 0) {
        return -1;
    }

    if (trans_resources_mem_swap_v2(res, conf) != 0) {
        return -1;
    }

    return 0;
}

/* trans resources cpu weight of cgroup v2, it's called cpu shares in cgroup v1 */
static int trans_resources_cpu_weight_v2(const defs_resources *res, struct lcr_list *conf)
{
    if (res->cpu->shares == INVALID_INT) {
        return 0;
    }

    if (res->cpu->shares < 2 || res->cpu->shares > 262144) {
        ERROR("invalid cpu shares %lld out of range [2-262144]", (long long)res->cpu->shares);
        return -1;
    }

    if (trans_conf_int64(conf, "lxc.cgroup2.cpu.weight", lcr_util_trans_cpushare_to_cpuweight(res->cpu->shares)) != 0) {
        return -1;
    }

    return 0;
}

/* trans resources cpu max of cgroup v2, it's called quota/period in cgroup v1 */
static int trans_resources_cpu_max_v2(const defs_resources *res, struct lcr_list *conf)
{
    char buf_value[300] = {0};
    uint64_t period = res->cpu->period;
    int nret = 0;

    if (res->cpu->quota == 0 && period == 0) {
        return 0;
    }

    if (period == 0) {
        period = DEFAULT_CPU_PERIOD;
    }

    // format:
    // $MAX $PERIOD
    if (res->cpu->quota > 0) {
        nret = snprintf(buf_value, sizeof(buf_value), "%lld %llu", (long long) res->cpu->quota,
                        (unsigned long long)period);
    } else {
        nret = snprintf(buf_value, sizeof(buf_value), "max %llu", (unsigned long long)period);
    }
    if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
        ERROR("failed to printf cpu max");
        return -1;
    }

    if (trans_conf_string(conf, "lxc.cgroup2.cpu.max", buf_value) != 0) {
        return -1;
    }

    return 0;
}

/* trans resources cpu set of cgroup v2 */
static int trans_resources_cpuset_v2(const defs_resources *res, struct lcr_list *conf)
{
    if (res->cpu->cpus != NULL) {
        if (trans_conf_string(conf, "lxc.cgroup2.cpuset.cpus", res->cpu->cpus) != 0) {
            return -1;
        }
    }

    if (res->cpu->mems != NULL) {
        if (trans_conf_string(conf, "lxc.cgroup2.cpuset.mems", res->cpu->mems) != 0) {
            return -1;
        }
    }

    return 0;
}

/* trans resources cpu of cgroup v2 */
static int trans_resources_cpu_v2(const defs_resources *res, struct lcr_list *conf)
{
    if (res->cpu == NULL) {
        return 0;
    }

    if (trans_resources_cpu_weight_v2(res, conf) != 0) {
        return -1;
    }

    if (trans_resources_cpu_max_v2(res, conf) != 0) {
        return -1;
    }

    if (trans_resources_cpuset_v2(res, conf) != 0) {
        return -1;
    }

    return 0;
}

/* trans resources io.weight/io.weight_device of cgroup v2 */
static int trans_io_weight_v2(const defs_resources_block_io *block_io, struct lcr_list *conf)
{
    size_t i = 0;
    uint64_t weight = 0;
    defs_block_io_device_weight **weight_device = block_io->weight_device;
    size_t len = block_io->weight_device_len;

    if (block_io->weight != INVALID_INT) {
        weight = lcr_util_trans_blkio_weight_to_io_weight(block_io->weight);
        if (weight < CGROUP2_WEIGHT_MIN || weight > CGROUP2_WEIGHT_MAX) {
            ERROR("invalid io weight cased by invalid blockio weight %d", block_io->weight);
            return -1;
        }

        if (trans_conf_int(conf, "lxc.cgroup2.io.weight", (int)weight) != 0) {
            return -1;
        }
    }

    if ((weight_device == NULL) || len == 0) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        if (weight_device[i] && weight_device[i]->weight != INVALID_INT) {
            int nret = 0;
            char buf_value[300] = { 0x00 };

            weight = lcr_util_trans_blkio_weight_to_io_weight(weight_device[i]->weight);
            if (weight < CGROUP2_WEIGHT_MIN || weight > CGROUP2_WEIGHT_MAX) {
                ERROR("invalid io weight cased by invalid blockio weight %d", weight_device[i]->weight);
                return -1;
            }

            nret = snprintf(buf_value, sizeof(buf_value), "%lld:%lld %d", (long long)weight_device[i]->major,
                            (long long)(weight_device[i]->minor), (int)weight);
            if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
                ERROR("print device weight failed");
                return -1;
            }

            if (trans_conf_string(conf, "lxc.cgroup2.io.weight_device", buf_value) != 0) {
                return -1;
            }
        }
    }

    return 0;
}

/* trans resources io.bfq.weight/io.bfq.weight_device of cgroup v2 */
static int trans_io_bfq_weight_v2(const defs_resources_block_io *block_io, struct lcr_list *conf)
{
    size_t i = 0;
    uint64_t weight = 0;
    defs_block_io_device_weight **weight_device = block_io->weight_device;
    size_t len = block_io->weight_device_len;

    if (block_io->weight != INVALID_INT) {
        weight = lcr_util_trans_blkio_weight_to_io_bfq_weight(block_io->weight);
        if (weight < CGROUP2_BFQ_WEIGHT_MIN || weight > CGROUP2_BFQ_WEIGHT_MAX) {
            ERROR("invalid io weight cased by invalid blockio weight %d", block_io->weight);
            return -1;
        }

        if (trans_conf_int(conf, "lxc.cgroup2.io.bfq.weight", weight) != 0) {
            return -1;
        }
    }

    if ((weight_device == NULL) || len == 0) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        if (weight_device[i] && weight_device[i]->weight != INVALID_INT) {
            int nret = 0;
            char buf_value[300] = { 0x00 };

            weight = lcr_util_trans_blkio_weight_to_io_weight(weight_device[i]->weight);
            if (weight < CGROUP2_BFQ_WEIGHT_MIN || weight > CGROUP2_BFQ_WEIGHT_MAX) {
                ERROR("invalid io weight cased by invalid blockio weight %d", weight_device[i]->weight);
                return -1;
            }

            nret = snprintf(buf_value, sizeof(buf_value), "%lld:%lld %d", (long long)weight_device[i]->major,
                            (long long)(weight_device[i]->minor), (int)weight);
            if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
                ERROR("print device weight failed");
                return -1;
            }

            if (trans_conf_string(conf, "lxc.cgroup2.io.bfq.weight_device", buf_value) != 0) {
                return -1;
            }
        }
    }

    return 0;
}

/* trans resources io throttle of cgroup v2 */
static int trans_io_throttle_v2(defs_block_io_device_throttle **throttle, size_t len,
                                const char *lxc_key, const char *rate_key, struct lcr_list *conf)
{
    int ret = -1;
    size_t i;

    if ((throttle == NULL) || len == 0) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        if (throttle[i] && throttle[i]->rate != INVALID_INT) {
            int nret = 0;
            char buf_value[300] = { 0x00 };
            nret = snprintf(buf_value, sizeof(buf_value), "%lld:%lld %s=%llu", (long long)throttle[i]->major,
                            (long long)(throttle[i]->minor), rate_key, (unsigned long long)(throttle[i]->rate));
            if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
                goto out;
            }

            if (trans_conf_string(conf, lxc_key, buf_value) != 0) {
                goto out;
            }
        }
    }
    ret = 0;
out:
    return ret;
}


/* trans resources blkio of cgroup v2 */
static int trans_resources_blkio_v2(const defs_resources_block_io *block_io, struct lcr_list *conf)
{
    if (block_io == NULL) {
        return 0;
    }

    if (trans_io_weight_v2(block_io, conf)) {
        return -1;
    }

    if (trans_io_bfq_weight_v2(block_io, conf)) {
        return -1;
    }

    if (trans_io_throttle_v2(block_io->throttle_read_bps_device, block_io->throttle_read_bps_device_len,
                             "lxc.cgroup2.io.max", "rbps", conf) != 0) {
        return -1;
    }

    if (trans_io_throttle_v2(block_io->throttle_write_bps_device, block_io->throttle_write_bps_device_len,
                             "lxc.cgroup2.io.max", "wbps", conf) != 0) {
        return -1;
    }

    if (trans_io_throttle_v2(block_io->throttle_read_iops_device, block_io->throttle_read_iops_device_len,
                             "lxc.cgroup2.io.max", "riops", conf) != 0) {
        return -1;
    }

    if (trans_io_throttle_v2(block_io->throttle_write_iops_device, block_io->throttle_write_iops_device_len,
                             "lxc.cgroup2.io.max", "wiops", conf) != 0) {
        return -1;
    }

    return 0;
}

/* trans resources hugetlb of cgroup v2 */
static int trans_resources_hugetlb_v2(const defs_resources *res, struct lcr_list *conf)
{
    size_t i = 0;
    char buf_key[300] = { 0 };

    for (i = 0; i < res->hugepage_limits_len; i++) {
        defs_resources_hugepage_limits_element *lrhl = res->hugepage_limits[i];
        if (lrhl->page_size == NULL) {
            continue;
        }
        int nret = snprintf(buf_key, sizeof(buf_key), "lxc.cgroup2.hugetlb.%s.max", lrhl->page_size);
        if (nret < 0 || (size_t)nret >= sizeof(buf_key)) {
            return -1;
        }

        if (trans_conf_uint64(conf, buf_key, lrhl->limit) < 0) {
            return -1;
        }
    }

    return 0;
}

/* trans resources pids of cgroup v2 */
static int trans_resources_pids_v2(const defs_resources *res, struct lcr_list *conf)
{
    if (res->pids == NULL) {
        return 0;
    }

    if (res->pids->limit != INVALID_INT) {
        if (trans_conf_int64_with_max(conf, "lxc.cgroup2.pids.max", res->pids->limit) != 0) {
            return -1;
        }
    }

    return 0;
}

/* trans oci resources to lxc cgroup config v2 */
static struct lcr_list *trans_oci_resources_v2(const defs_resources *res)
{
    struct lcr_list *conf = NULL;

    conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    if (trans_resources_devices_v2(res, conf)) {
        goto out_free;
    }

    if (trans_resources_memory_v2(res, conf)) {
        goto out_free;
    }

    if (trans_resources_cpu_v2(res, conf)) {
        goto out_free;
    }

    if (trans_resources_blkio_v2(res->block_io, conf)) {
        goto out_free;
    }

    if (trans_resources_hugetlb_v2(res, conf)) {
        goto out_free;
    }

    if (trans_resources_pids_v2(res, conf)) {
        goto out_free;
    }

    return conf;

out_free:
    lcr_free_config(conf);
    free(conf);

    return NULL;
}

/* trans oci resources to lxc cgroup config */
/* note: we write both cgroup v1 and cgroup v2 config to lxc config file, let lxc choose the right one */
/* references: */
/* oci config: https://github.com/opencontainers/runtime-spec/blob/master/schema/config-linux.json */
/* cgroup v1 config: https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v1/index.html */
/* cgroup v2 config: https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html */
static struct lcr_list *trans_oci_resources(const defs_resources *res)
{
    int cgroup_version = 0;

    cgroup_version = lcr_util_get_cgroup_version();
    if (cgroup_version < 0) {
        return NULL;
    }

    if (cgroup_version == CGROUP_VERSION_2) {
        return trans_oci_resources_v2(res);
    } else {
        return trans_oci_resources_v1(res);
    }
}

struct namespace_map_def {
    char *ns_name;
    char *lxc_name;
};

static char *trans_oci_namespace_to_lxc(const char *typ)
{
    struct namespace_map_def namespaces_map[] = {
        { "pid", "lxc.namespace.share.pid" },       { "network", "lxc.namespace.share.net" },
        { "ipc", "lxc.namespace.share.ipc" },       { "uts", "lxc.namespace.share.uts" },
        { "mount", "lxc.namespace.share.mnt" },     { "user", "lxc.namespace.share.user" },
        { "cgroup", "lxc.namespace.share.cgroup" }, { NULL, NULL }
    };
    const struct namespace_map_def *p = NULL;

    for (p = namespaces_map; p != NULL && p->ns_name != NULL; p++) {
        if (strcmp(typ, p->ns_name) == 0) {
            return lcr_util_strdup_s(p->lxc_name);
        }
    }
    return NULL;
}

/* trans oci namespaces */
static struct lcr_list *trans_oci_namespaces(const oci_runtime_config_linux *l)
{
    struct lcr_list *conf = NULL;
    struct lcr_list *node = NULL;
    size_t i;
    defs_namespace_reference *ns = NULL;

    conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    for (i = 0; i < l->namespaces_len; i++) {
        char *ns_name = NULL;
        ns = l->namespaces[i];

        if (ns->type == NULL || ns->path == NULL) {
            continue;
        }

        ns_name = trans_oci_namespace_to_lxc(ns->type);
        if (ns_name == NULL) {
            continue;
        }

        node = create_lcr_list_node(ns_name, ns->path);
        free(ns_name);
        if (node == NULL) {
            goto out_free;
        }
        lcr_list_add_tail(conf, node);
    }

    return conf;

out_free:
    lcr_free_config(conf);
    free(conf);

    return NULL;
}

/* trans oci mask ro paths */
static struct lcr_list *trans_oci_mask_ro_paths(const oci_runtime_config_linux *l)
{
    struct lcr_list *conf = NULL;
    struct lcr_list *node = NULL;
    size_t i;
    char *path = NULL;

    conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    for (i = 0; i < l->masked_paths_len; i++) {
        path = l->masked_paths[i];
        if (path == NULL) {
            continue;
        }
        node = create_lcr_list_node("lxc.isulad.rootfs.maskedpaths", path);
        if (node == NULL) {
            goto out_free;
        }
        lcr_list_add_tail(conf, node);
    }

    for (i = 0; i < l->readonly_paths_len; i++) {
        path = l->readonly_paths[i];
        if (path == NULL) {
            continue;
        }
        node = create_lcr_list_node("lxc.isulad.rootfs.ropaths", path);
        if (node == NULL) {
            goto out_free;
        }
        lcr_list_add_tail(conf, node);
    }

    return conf;

out_free:
    lcr_free_config(conf);
    free(conf);

    return NULL;
}

#define POPULATE_DEVICE_SIZE (300 + PATH_MAX)
/* trans oci linux devices */
static struct lcr_list *trans_oci_linux_devices(const oci_runtime_config_linux *l)
{
    struct lcr_list *conf = NULL;
    struct lcr_list *node = NULL;
    size_t i = 0;
    int nret = 0;
    defs_device *device = NULL;
    char buf_value[POPULATE_DEVICE_SIZE] = { 0 };

    conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    for (i = 0; i < l->devices_len; i++) {
        device = l->devices[i];

        if (device->type == NULL || device->path == NULL) {
            continue;
        }

        /* lxc.populate_device = PATH_IN_CONTAINER:DEVICETYPE:MAJOR:MINOR:MODE:UID:GID
         * For e.g. lxc.populate_device = /dev/sda:b:8:0:0666:0:0
         */
        nret = snprintf(buf_value, sizeof(buf_value), "%s:%s:%lld:%lld:%d:%u:%u", device->path, device->type,
                        (long long int)(device->major), (long long int)(device->minor), device->file_mode, device->uid,
                        device->gid);
        if (nret < 0 || (size_t)nret >= sizeof(buf_value)) {
            ERROR("Failed to get populate device string");
            goto out_free;
        }

        node = create_lcr_list_node("lxc.isulad.populate.device", buf_value);
        if (node == NULL) {
            goto out_free;
        }
        lcr_list_add_tail(conf, node);
    }

    return conf;

out_free:
    lcr_free_config(conf);
    free(conf);

    return NULL;
}

static inline bool is_seccomp_action_kill(const char *value)
{
    return strcmp(value, "SCMP_ACT_KILL") == 0;
}

static inline bool is_seccomp_action_trap(const char *value)
{
    return strcmp(value, "SCMP_ACT_TRAP") == 0;
}

static inline bool is_seccomp_action_allow(const char *value)
{
    return strcmp(value, "SCMP_ACT_ALLOW") == 0;
}

static inline bool is_seccomp_action_trace(const char *value)
{
    return strcmp(value, "SCMP_ACT_TRACE") == 0;
}

static inline bool is_seccomp_action_errno(const char *value)
{
    return strcmp(value, "SCMP_ACT_ERRNO") == 0;
}

/* seccomp trans action */
static char *seccomp_trans_action(const char *action)
{
    if (is_seccomp_action_kill(action)) {
        return lcr_util_strdup_s("kill");
    } else if (is_seccomp_action_trap(action)) {
        return lcr_util_strdup_s("trap");
    } else if (is_seccomp_action_allow(action)) {
        return lcr_util_strdup_s("allow");
    } else if (is_seccomp_action_trace(action)) {
        return lcr_util_strdup_s("trace 1");
    } else if (is_seccomp_action_errno(action)) {
        return lcr_util_strdup_s("errno 1");
    }

    return NULL;
}

static bool is_action_allow(const char *value)
{
    return strcmp(value, "allow") == 0;
}

#define DEFAULT_ACTION_OFFSET 12
/* seccomp append head info */
static int seccomp_append_head_info(const char *action, Buffer *buffer)
{
    int ret = 0;
    char *default_action = NULL;

    if (action == NULL) {
        return -1;
    }

    default_action = seccomp_trans_action(action);
    if (default_action == NULL) {
        ERROR("Failed to translate seccomp action");
        return -1;
    }

    if (is_action_allow(default_action)) {
        ret = buffer_nappendf(buffer, strlen(default_action) + DEFAULT_ACTION_OFFSET, "blacklist %s\n", default_action);
    } else {
        ret = buffer_nappendf(buffer, strlen(default_action) + DEFAULT_ACTION_OFFSET, "whitelist %s\n", default_action);
    }
    if (ret != 0) {
        ERROR("Failed to append seccomp config head info\n");
        ret = -1;
        goto out;
    }

out:
    free(default_action);
    return ret;
}

/* get hostarch */
static char *get_hostarch(void)
{
    struct utsname uts;
    size_t len;
    size_t i;
    /* no x32 kernels */
    lcr_host_arch arch_type[] = {
        { "i686", "[x86]", 4 },      { "x32", "[x32]", 3 },           { "x86_64", "[x86_64]", 6 },
        { "armv7", "[arm]", 5 },     { "aarch64", "[arm64]", 7 },     { "ppc64le", "[ppc64le]", 7 },
        { "ppc64", "[ppc64]", 5 },   { "ppc", "[ppc]", 3 },           { "mips64n32", "[mips64n32]", 9 },
        { "mips64", "[mips64]", 6 }, { "mips", "[mips]", 4 },         { "s390x", "[s390x]", 5 },
        { "s390", "[s390]", 4 },     { "parisc64", "[parisc64]", 8 }, { "parisc", "[parisc]", 6 },
    };

    if (uname(&uts) < 0) {
        SYSERROR("Failed to read host arch");
        return NULL;
    }
    len = sizeof(arch_type) / sizeof(lcr_host_arch);
    for (i = 0; i < len; i++) {
        if (i == 0 || i == 1 || i == 2) {
            if (strcmp(uts.machine, arch_type[i].arch) == 0) {
                return lcr_util_strdup_s(arch_type[i].value);
            }
        } else {
            if (strncmp(uts.machine, arch_type[i].arch, (size_t)(arch_type[i].num)) == 0) {
                return lcr_util_strdup_s(arch_type[i].value);
            }
        }
    }

    ERROR("Failed to get machine type");
    return NULL;
}

/* seccomp trans arch */
static char *seccomp_trans_arch(const char *arch)
{
    lcr_arch_value arch_type[] = {
        { "SCMP_ARCH_X86", "[x86]" },
        { "SCMP_ARCH_X86_64", "[x86_64]" },
        { "SCMP_ARCH_X32", "[x32]" },
        { "SCMP_ARCH_ARM", "[arm]" },
        { "SCMP_ARCH_AARCH64", "[arm64]" },
        { "SCMP_ARCH_MIPS", "[mips]" },
        { "SCMP_ARCH_MIPS64", "[mips64]" },
        { "SCMP_ARCH_MIPS64N32", "[mips64n32]" },
        { "SCMP_ARCH_MIPSEL", "[mipsel]" },
        { "SCMP_ARCH_MIPSEL64", "[mipsel64]" },
        { "SCMP_ARCH_MIPSEL64N32", "[mipsel64n32]" },
        { "SCMP_ARCH_PPC", "[ppc]" },
        { "SCMP_ARCH_PPC64", "[ppc64]" },
        { "SCMP_ARCH_PPC64LE", "[ppc64le]" },
        { "SCMP_ARCH_S390", "[s390]" },
        { "SCMP_ARCH_S390X", "[s390x]" },
        { "SCMP_ARCH_PARISC", "[parisc]" },
        { "SCMP_ARCH_PARISC64", "[parisc64]" },
        { "SCMP_ARCH_ALL", "[all]" },
        { "SCMP_ARCH_AUTO", "" },
    };
    size_t len;
    size_t i = 0;
    len = sizeof(arch_type) / sizeof(lcr_arch_value);
    for (i = 0; i < len; i++) {
        if (strcmp(arch_type[i].arch, "SCMP_ARCH_AUTO") == 0) {
            return get_hostarch();
        } else if (strcmp(arch, arch_type[i].arch) == 0) {
            return lcr_util_strdup_s(arch_type[i].value);
        }
    }
    return NULL;
}

/* seccomp append arch */
static int seccomp_append_arch(char *arch, Buffer *buffer)
{
    int ret = 0;
    char *trans_arch = NULL;

    if (arch == NULL) {
        return -1;
    }

    trans_arch = seccomp_trans_arch(arch);
    if (trans_arch == NULL) {
        ERROR("Failed to translate seccomp arch: %s", arch);
        return -1;
    }

    if (buffer_nappendf(buffer, strlen(trans_arch) + 2, "%s\n", trans_arch)) {
        ERROR("Failed to append seccomp config head info\n");
        ret = -1;
    }

    free(trans_arch);
    return ret;
}

/* seccomp append rule */
static int seccomp_append_rule(const defs_syscall *syscall, size_t i, Buffer *buffer, char *action)
{
    int ret = 0;
    size_t j = 0;

    if (syscall->names[i] == NULL) {
        ERROR("Failed to get syscall name");
        ret = -1;
        goto out;
    }
    if (buffer_nappendf(buffer, strlen(syscall->names[i]) + strlen(action) + 2, "%s %s", syscall->names[i], action)) {
        ERROR("Failed to append syscall name and action\n");
        ret = -1;
        goto out;
    }

    for (j = 0; j < syscall->args_len; j++) {
        if ((syscall->args[j] == NULL) || (syscall->args[j]->op == NULL)) {
            ERROR("Failed to get syscall args");
            ret = -1;
            goto out;
        }
        if (buffer_nappendf(buffer, 20 * 3 + strlen(syscall->args[j]->op), " [%u,%llu,%s,%llu]",
                            syscall->args[j]->index, syscall->args[j]->value, syscall->args[j]->op,
                            syscall->args[j]->value_two)) {
            ERROR("Failed to append syscall rules\n");
            ret = -1;
            goto out;
        }
    }

    if (buffer_nappendf(buffer, 2, "\n")) {
        ERROR("Failed to append newline\n");
        ret = -1;
        goto out;
    }
out:
    return ret;
}

/* seccomp append rules */
static int seccomp_append_rules(const defs_syscall *syscall, Buffer *buffer)
{
    int ret = 0;
    size_t i = 0;
    char *action = NULL;

    if (syscall == NULL) {
        return -1;
    }

    if ((syscall->action == NULL) || syscall->names_len == 0) {
        return -1;
    }

    action = seccomp_trans_action(syscall->action);
    if (action == NULL) {
        ERROR("Failed to translate action");
        ret = -1;
        goto out;
    }

    for (i = 0; i < syscall->names_len; i++) {
        if (seccomp_append_rule(syscall, i, buffer, action)) {
            ret = -1;
            goto out;
        }
    }
out:
    free(action);
    return ret;
}

static struct lcr_list *trans_oci_linux_sysctl(const json_map_string_string *sysctl)
{
    struct lcr_list *conf = NULL;
    struct lcr_list *node = NULL;
    size_t i;

    conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    for (i = 0; i < sysctl->len; i++) {
        char sysk[BUFSIZ] = { 0 };
        int nret = snprintf(sysk, sizeof(sysk), "lxc.sysctl.%s", sysctl->keys[i]);
        if (nret < 0 || (size_t)nret >= sizeof(sysk)) {
            ERROR("Failed to print string");
            goto out_free;
        }
        node = create_lcr_list_node(sysk, sysctl->values[i]);
        if (node == NULL) {
            goto out_free;
        }
        lcr_list_add_tail(conf, node);
    }

    return conf;

out_free:
    lcr_free_config(conf);
    free(conf);
    return NULL;
}

static int append_seccomp_with_archs(const oci_runtime_config_linux_seccomp *seccomp, Buffer *buffer)
{
    int ret = 0;
    size_t i = 0;
    size_t j = 0;

    for (i = 0; i < seccomp->architectures_len; i++) {
        if (seccomp_append_arch(seccomp->architectures[i], buffer)) {
            ret = -1;
            goto out;
        }
        /* append rules */
        for (j = 0; j < seccomp->syscalls_len; j++) {
            if (seccomp_append_rules(seccomp->syscalls[j], buffer)) {
                ret = -1;
                goto out;
            }
        }
    }
out:
    return ret;
}

/* lxc seccomp conf format:
	To support limit syscall arguments, extend the version 2 file to the following format:

	syscall_name action [index,value,op,valueTwo] [index,value,op]...
	for one arguments, [index,value,valueTwo,op]

	index: the index for syscall arguments (type uint)

	value: the value for syscall arguments (type uint64)

	op: the operator for syscall arguments(string), a valid list of constants as of libseccomp v2.3.2 is
	SCMP_CMP_NE,SCMP_CMP_LE,SCMP_CMP_LE, SCMP_CMP_EQ, SCMP_CMP_GE,
	SCMP_CMP_GT, SCMP_CMP_MASKED_EQ, or !=,<=,==,>=,>,&=

	valueTwo: the value for syscall arguments only used for mask eq (type uint64, optional)

For example:

	2
	blacklist allow
	reject_force_umount  # comment this to allow umount -f;  not recommended
	[all]
	kexec_load errno 1 [0,1,SCMP_CMP_LE][3,1,==][5,1,SCMP_CMP_MASKED_EQ,1]
	open_by_handle_at errno 1
	init_module errno 1
	finit_module errno 1
	delete_module errno 1

*/
static int trans_oci_seccomp(const oci_runtime_config_linux_seccomp *seccomp, char **seccomp_conf)
{
    int ret = 0;
    size_t j = 0;
    size_t init_size = 4 * SIZE_KB;

    Buffer *buffer = buffer_alloc(init_size);
    if (buffer == NULL) {
        ERROR("Failed to malloc output_buffer\n");
        return -1;
    }

    /* config version */
    if (buffer_nappendf(buffer, 3, "2\n")) {
        ERROR("Failed to append seccomp config version\n");
        ret = -1;
        goto out_free;
    }

    /* append head info */
    if (seccomp_append_head_info(seccomp->default_action, buffer)) {
        ret = -1;
        goto out_free;
    }

    /* append architectures */
    if (seccomp->architectures_len != 0) {
        ret = append_seccomp_with_archs(seccomp, buffer);
        if (ret != 0) {
            goto out_free;
        }
    } else {
        // add rules directly(eg: blacklist)
        for (j = 0; j < seccomp->syscalls_len; j++) {
            if (seccomp_append_rules(seccomp->syscalls[j], buffer)) {
                ret = -1;
                goto out_free;
            }
        }
    }
    *seccomp_conf = buffer_to_s(buffer);
    if (*seccomp_conf == NULL) {
        ret = -1;
        goto out_free;
    }

out_free:
    buffer_free(buffer);
    return ret;
}

static int trans_oci_file_selinux(const oci_runtime_config_linux *l, struct lcr_list *conf)
{
    struct lcr_list *node = NULL;
    int ret = -1;

    if (l->mount_label != NULL) {
        node = create_lcr_list_node("lxc.selinux.mount_context", l->mount_label);
        if (node == NULL) {
            goto out;
        }
        lcr_list_add_tail(conf, node);
    }

    ret = 0;

out:
    return ret;
}

/* trans oci linux */
struct lcr_list *trans_oci_linux(const oci_runtime_config_linux *l, char **seccomp_conf)
{
    int ret = 0;
    struct lcr_list *tmp = NULL;

    struct lcr_list *conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    // UID/GID Mapping
    tmp = trans_oci_id_mapping(l);
    if (tmp == NULL) {
        goto out_free;
    }
    lcr_list_merge(conf, tmp);

    // Resources
    if (l->resources != NULL) {
        tmp = trans_oci_resources(l->resources);
        if (tmp == NULL) {
            goto out_free;
        }
        lcr_list_merge(conf, tmp);
    }

    // linux devices
    tmp = trans_oci_linux_devices(l);
    if (tmp == NULL) {
        goto out_free;
    }
    lcr_list_merge(conf, tmp);

    // Namespaces
    tmp = trans_oci_namespaces(l);
    if (tmp == NULL) {
        goto out_free;
    }
    lcr_list_merge(conf, tmp);

    // MaskedPaths and ReadonlyPaths
    tmp = trans_oci_mask_ro_paths(l);
    if (tmp == NULL) {
        goto out_free;
    }
    lcr_list_merge(conf, tmp);

    // sysctl
    if (l->sysctl != NULL && l->uid_mappings == NULL && l->gid_mappings == NULL) {
        tmp = trans_oci_linux_sysctl(l->sysctl);
        if (tmp == NULL) {
            goto out_free;
        }
        lcr_list_merge(conf, tmp);
    }

    // seccomp
    if (l->seccomp != NULL && seccomp_conf != NULL) {
        ret = trans_oci_seccomp(l->seccomp, seccomp_conf);
        if (ret != 0) {
            goto out_free;
        }
    }

    // selinux mount label
    ret = trans_oci_file_selinux(l, conf);
    if (ret != 0) {
        goto out_free;
    }

    return conf;

out_free:
    lcr_free_config(conf);
    free(conf);

    return NULL;
}

/* trans annotations */
struct lcr_list *trans_annotations(const json_map_string_string *anno)
{
    size_t i, j;
    size_t len;
    int ret = 0;

    struct lcr_list *conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    len = sizeof(g_require_annotations) / sizeof(lcr_annotation_item_t);

    // trans annotations
    for (i = 0; i < anno->len; i++) {
        if (anno->keys[i] == NULL) {
            continue;
        }
        for (j = 0; j < len; j++) {
            if (strcmp(anno->keys[i], g_require_annotations[j].name) != 0) {
                continue;
            }

            ret = g_require_annotations[j].checker(anno->values[i]);
            if (ret == -1) {
                ERROR("item: %s, value: %s, checker failed", anno->keys[i], anno->values[i]);
                goto out_free;
            } else if (ret == 1) {
                DEBUG("Skip this config item: %s", anno->keys[i]);
                continue;
            }

            struct lcr_list *node = create_lcr_list_node(g_require_annotations[j].lxc_item_name, anno->values[i]);
            if (node == NULL) {
                goto out_free;
            }
            lcr_list_add_tail(conf, node);
            break;
        }
    }

    return conf;
out_free:
    lcr_free_config(conf);
    free(conf);

    return NULL;
}

static int add_needed_pty_conf(struct lcr_list *conf)
{
    struct lcr_list *node = create_lcr_list_node("lxc.pty.max", "1024");
    if (node == NULL) {
        return -1;
    }
    lcr_list_add_tail(conf, node);

    return 0;
}

static int add_needed_net_conf(struct lcr_list *conf)
{
    struct lcr_list *node = create_lcr_list_node("lxc.net.0.type", "empty");
    if (node == NULL) {
        return -1;
    }
    lcr_list_add_tail(conf, node);

    node = create_lcr_list_node("lxc.net.0.flags", "up");
    if (node == NULL) {
        return -1;
    }
    lcr_list_add_tail(conf, node);
    return 0;
}

/* get needed lxc conf */
struct lcr_list *get_needed_lxc_conf()
{
    struct lcr_list *conf = lcr_util_common_calloc_s(sizeof(*conf));
    if (conf == NULL) {
        return NULL;
    }
    lcr_list_init(conf);

    if (add_needed_pty_conf(conf) < 0) {
        goto out_free;
    }
    if (add_needed_net_conf(conf) < 0) {
        goto out_free;
    }

    return conf;
out_free:
    lcr_free_config(conf);
    free(conf);
    return NULL;
}
