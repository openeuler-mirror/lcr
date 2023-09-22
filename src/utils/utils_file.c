/******************************************************************************
 * isula: file utils
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
#include "utils_file.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include "utils_memory.h"
#include "utils_convert.h"
#include "constants.h"
#include "log.h"
#include "auto_cleanup.h"

#define ISSLASH(C) ((C) == '/')
#define IS_ABSOLUTE_FILE_NAME(F) (ISSLASH((F)[0]))
#define IS_RELATIVE_FILE_NAME(F) (!IS_ABSOLUTE_FILE_NAME(F))

static bool do_clean_path_continue(const char *endpos, const char *stpos, const char *respath, char **dst)
{
    if (endpos - stpos == 1 && stpos[0] == '.') {
        return true;
    } else if (endpos - stpos == 2 && stpos[0] == '.' && stpos[1] == '.') {
        char *dest = *dst;
        if (dest <= respath + 1) {
            return true;
        }
        for (--dest; dest > respath && !ISSLASH(dest[-1]); --dest) {
            continue;
        }
        *dst = dest;
        return true;
    }
    return false;
}

static int do_clean_path(const char *respath, const char *limit_respath, const char *stpos, char **dst)
{
    char *dest = *dst;
    const char *endpos = stpos;

    for (; *stpos; stpos = endpos) {
        while (ISSLASH(*stpos)) {
            ++stpos;
        }

        for (endpos = stpos; *endpos && !ISSLASH(*endpos); ++endpos) {
        }

        if (endpos - stpos == 0) {
            break;
        } else if (do_clean_path_continue(endpos, stpos, respath, &dest)) {
            continue;
        }

        if (!ISSLASH(dest[-1])) {
            *dest++ = '/';
        }

        if (dest + (endpos - stpos) >= limit_respath) {
            ERROR("Path is too long");
            if (dest > respath + 1) {
                dest--;
            }
            *dest = '\0';
            return -1;
        }

        (void)memcpy(dest, stpos, (size_t)(endpos - stpos));
        dest += endpos - stpos;
        *dest = '\0';
    }
    *dst = dest;
    return 0;
}

static char *cleanpath(const char *path, char *realpath, size_t realpath_len)
{
    char *respath = NULL;
    char *dest = NULL;
    const char *stpos = NULL;
    const char *limit_respath = NULL;

    if (path == NULL || path[0] == '\0' || realpath == NULL || (realpath_len < PATH_MAX)) {
        return NULL;
    }

    respath = realpath;

    (void)memset(respath, 0, realpath_len);
    limit_respath = respath + PATH_MAX;

    if (!IS_ABSOLUTE_FILE_NAME(path)) {
        if (!getcwd(respath, PATH_MAX)) {
            ERROR("Failed to getcwd");
            respath[0] = '\0';
            return NULL;
        }
        dest = strchr(respath, '\0');
        if (dest == NULL) {
            ERROR("Failed to get the end of respath");
            return NULL;
        }
        if (strlen(path) >= (PATH_MAX - 1) - strlen(respath)) {
            ERROR("%s path too long", path);
            return NULL;
        }
        (void)strcat(respath, path);
        stpos = path;
    } else {
        dest = respath;
        *dest++ = '/';
        stpos = path;
    }

    if (do_clean_path(respath, limit_respath, stpos, &dest)) {
        return NULL;
    }

    if (dest > respath + 1 && ISSLASH(dest[-1])) {
        --dest;
    }
    *dest = '\0';

    return respath;
}


/*
 * if path do not exist, this function will create it.
 */
int lcr_util_ensure_path(char **confpath, const char *path)
{
    __isula_auto_close int fd = -1;
    char real_path[PATH_MAX + 1] = { 0 };

    if (confpath == NULL || path == NULL) {
        return -1;
    }

    fd = lcr_util_open(path, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, DEFAULT_SECURE_FILE_MODE);
    if (fd < 0 && errno != EEXIST) {
        ERROR("failed to open '%s'", path);
        return -1;
    }

    if (strlen(path) > PATH_MAX || realpath(path, real_path) == NULL) {
        ERROR("Failed to get real path: %s", path);
        return -1;
    }

    *confpath = isula_strdup_s(real_path);
    return EXIT_SUCCESS;
}

/* util dir exists */
bool lcr_util_dir_exists(const char *path)
{
    struct stat s;
    int nret;

    if (path == NULL) {
        return false;
    }

    nret = stat(path, &s);
    if (nret < 0) {
        return false;
    }

    return S_ISDIR(s.st_mode);
}

static inline bool is_dot(const char *value)
{
    return strcmp(value, ".") == 0;
}

static inline bool is_double_dot(const char *value)
{
    return strcmp(value, "..") == 0;
}


/* util rmdir one */
static void util_rmdir_one(const char *dirpath, const struct dirent *pdirent, int recursive_depth, int *failure)
{
    struct stat fstat;
    int nret;
    char fname[PATH_MAX] = { 0 };

    if (is_dot(pdirent->d_name) || is_double_dot(pdirent->d_name)) {
        return;
    }

    nret = snprintf(fname, PATH_MAX, "%s/%s", dirpath, pdirent->d_name);
    if (nret < 0 || nret >= PATH_MAX) {
        ERROR("Pathname too long");
        *failure = -1;
        return;
    }

    nret = lstat(fname, &fstat);
    if (nret) {
        ERROR("Failed to stat %s", fname);
        *failure = -1;
        return;
    }

    if (S_ISDIR(fstat.st_mode)) {
        if (lcr_util_recursive_rmdir(fname, (recursive_depth + 1)) < 0) {
            *failure = -1;
        }
    } else {
        if (unlink(fname) < 0) {
            ERROR("Failed to delete %s", fname);
            *failure = -1;
        }
    }
}

/* util recursive rmdir */
int lcr_util_recursive_rmdir(const char *dirpath, int recursive_depth)
{
    struct dirent *pdirent = NULL;
    __isula_auto_dir DIR *directory = NULL;
    int ret = 0;

    if ((recursive_depth + 1) > MAX_PATH_DEPTH) {
        ERROR("Reach max path depth: %s", dirpath);
        return -1;
    }

    if (!lcr_util_dir_exists(dirpath)) { /* dir not exists, just ignore */
        return 0;
    }

    directory = opendir(dirpath);
    if (directory == NULL) {
        ERROR("Failed to open %s", dirpath);
        return -1;
    }

    pdirent = readdir(directory);
    while (pdirent != NULL) {
        util_rmdir_one(dirpath, pdirent, recursive_depth, &ret);
        pdirent = readdir(directory);
    }

    if (rmdir(dirpath) < 0) {
        ERROR("Failed to delete %s", dirpath);
        ret = -1;
    }

    return ret;
}

int lcr_util_open(const char *filename, int flags, mode_t mode)
{
    char rpath[PATH_MAX] = { 0x00 };

    if (cleanpath(filename, rpath, sizeof(rpath)) == NULL) {
        return -1;
    }
    if (mode) {
        return open(rpath, flags | O_CLOEXEC, mode);
    } else {
        return open(rpath, flags | O_CLOEXEC);
    }
}

/* util check inherited */
static bool util_dir_skip_current(const struct dirent *pdirent)
{
    if (is_dot(pdirent->d_name)) {
        return true;
    }

    if (is_double_dot(pdirent->d_name)) {
        return true;
    }
    return false;
}

static bool util_is_std_fileno(int fd)
{
    return fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO;
}

int lcr_util_check_inherited(bool closeall, int fd_to_ignore)
{
    struct dirent *pdirent = NULL;
    int fd = -1;
    int fddir = -1;
    DIR *directory = NULL;

restart:
    if (directory != NULL) {
        closedir(directory);
    }
    directory = opendir("/proc/self/fd");
    if (directory == NULL) {
        WARN("Failed to open directory: %m.");
        return -1;
    }

    fddir = dirfd(directory);
    pdirent = readdir(directory);
    for (; pdirent != NULL; pdirent = readdir(directory)) {
        if (util_dir_skip_current(pdirent)) {
            continue;
        }

        if (lcr_util_safe_int(pdirent->d_name, &fd) < 0) {
            continue;
        }

        if (util_is_std_fileno(fd) || fd == fddir || fd == fd_to_ignore) {
            continue;
        }

        if (closeall) {
            if (fd >= 0) {
                close(fd);
                fd = -1;
            }
            goto restart;
        }
    }

    closedir(directory);
    return 0;
}

static void set_char_to_terminator(char *p)
{
    *p = '\0';
}
/*
 * @name is absolute path of this file.
 * make all directory in this absolute path.
 * */
int lcr_util_build_dir(const char *name)
{
    char *n = NULL; // because we'll be modifying it
    char *p = NULL;
    char *e = NULL;
    int nret;

    if (name == NULL) {
        return -1;
    }

    n = isula_strdup_s(name);
    e = &(n[strlen(n)]);
    for (p = n + 1; p < e; p++) {
        if (*p != '/') {
            continue;
        }
        set_char_to_terminator(p);
        if (access(n, F_OK)) {
            nret = mkdir(n, DEFAULT_SECURE_DIRECTORY_MODE);
            if (nret && (errno != EEXIST || !lcr_util_dir_exists(n))) {
                ERROR("failed to create directory '%s'.", n);
                free(n);
                return -1;
            }
        }
        *p = '/';
    }
    free(n);
    return 0;
}

/* util write nointr */
ssize_t lcr_util_write_nointr(int fd, const void *buf, size_t count)
{
    ssize_t nret;

    if (buf == NULL) {
        return -1;
    }

    for (;;) {
        nret = write(fd, buf, count);
        if (nret < 0 && errno == EINTR) {
            continue;
        } else if (nret < 0 && errno == EAGAIN) {
            continue;
        } else {
            break;
        }
    }
    return nret;
}

/* util read nointr */
ssize_t lcr_util_read_nointr(int fd, void *buf, size_t count)
{
    ssize_t nret;

    if (buf == NULL) {
        return -1;
    }

    for (;;) {
        nret = read(fd, buf, count);
        if (nret < 0 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }

    return nret;
}

static int open_devnull(void)
{
    int fd = open("/dev/null", O_RDWR);
    if (fd < 0) {
        SYSERROR("Can't open /dev/null");
    }

    return fd;
}

static int set_stdfds(int fd)
{
    int ret = 0;

    if (fd < 0) {
        return -1;
    }

    ret = dup2(fd, STDIN_FILENO);
    if (ret < 0) {
        return -1;
    }

    ret = dup2(fd, STDOUT_FILENO);
    if (ret < 0) {
        return -1;
    }

    ret = dup2(fd, STDERR_FILENO);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int lcr_util_null_stdfds(void)
{
    int ret = -1;
    int fd;

    fd = open_devnull();
    if (fd >= 0) {
        ret = set_stdfds(fd);
        close(fd);
    }

    return ret;
}

static void util_trim_newline(char *s)
{
    if (s == NULL) {
        return;
    }
    size_t len = strlen(s);
    while ((len > 1) && (s[len - 1] == '\n')) {
        s[--len] = '\0';
    }
}

static int append_new_content_to_file(FILE *fp, const char *content)
{
    size_t length = 0;
    size_t content_len = 0;
    __isula_auto_free char *line = NULL;
    __isula_auto_free char *tmp_str = NULL;
    bool need_append = true;

    while (getline(&line, &length, fp) != -1) {
        if (line == NULL) {
            ERROR("Failed to read content from file ptr");
            return -1;
        }
        util_trim_newline(line);
        if (!strcmp(content, line)) {
            need_append = false;
            break;
        }
    }

    if (!need_append) {
        return 0;
    }

    if (strlen(content) > ((SIZE_MAX - strlen("\n")) - 1)) {
        return -1;
    }
    content_len = strlen(content) + strlen("\n") + 1;
    tmp_str = isula_common_calloc_s(content_len);
    if (tmp_str == NULL) {
        ERROR("Out of memory");
        return -1;
    }
    int num = snprintf(tmp_str, content_len, "%s\n", content);
    if (num < 0 || (size_t)num >= content_len) {
        ERROR("Failed to print string");
        return -1;
    }
    if (fwrite(tmp_str, 1, strlen(tmp_str), fp) == 0) {
        ERROR("Failed to write content: '%s'", content);
        return -1;
    }

    return 0;
}

int lcr_util_atomic_write_file(const char *filepath, const char *content)
{
    __isula_auto_close int fd = -1;
    int ret = 0;
    FILE *fp = NULL;
    struct flock lk;

    if (filepath == NULL || content == NULL) {
        return -1;
    }
    fd = lcr_util_open(filepath, O_RDWR | O_CREAT | O_APPEND, DEFAULT_SECURE_FILE_MODE);
    if (fd < 0) {
        ERROR("Failed to open: %s", filepath);
        return -1;
    }
    lk.l_type = F_WRLCK;
    lk.l_whence = SEEK_SET;
    lk.l_start = 0;
    lk.l_len = 0;
    if (fcntl(fd, F_SETLKW, &lk) == 0) {
        fp = fdopen(fd, "a+");
        if (fp == NULL) {
            ERROR("Failed to open fd: %d", fd);
            return -1;
        }
        ret = append_new_content_to_file(fp, content);
        fclose(fp);
    }

    return ret;
}
