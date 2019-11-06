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
 * Description: provide container utils functions
 ******************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>
#include "constants.h"
#include "utils.h"
#include "securec.h"
#include "log.h"

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
    errno_t ret;

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

        ret = memcpy_s(dest, (size_t)(endpos - stpos), stpos, (size_t)(endpos - stpos));
        if (ret != EOK) {
            ERROR("Failed at cleanpath memcpy");
            return -1;
        }
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
    errno_t ret;

    if (path == NULL || path[0] == '\0' || realpath == NULL || (realpath_len < PATH_MAX)) {
        return NULL;
    }

    respath = realpath;

    ret = memset_s(respath, realpath_len, 0, realpath_len);
    if (ret != EOK) {
        ERROR("Failed at cleanpath memset");
        goto error;
    }
    limit_respath = respath + PATH_MAX;

    if (!IS_ABSOLUTE_FILE_NAME(path)) {
        if (!getcwd(respath, PATH_MAX)) {
            ERROR("Failed to getcwd");
            respath[0] = '\0';
            goto error;
        }
        dest = strchr(respath, '\0');
        if (dest == NULL) {
            ERROR("Failed to get the end of respath");
            goto error;
        }
        ret = strcat_s(respath, PATH_MAX, path);
        if (ret != EOK) {
            ERROR("Failed at cleanpath strcat");
            goto error;
        }
        stpos = path;
    } else {
        dest = respath;
        *dest++ = '/';
        stpos = path;
    }

    if (do_clean_path(respath, limit_respath, stpos, &dest)) {
        goto error;
    }

    if (dest > respath + 1 && ISSLASH(dest[-1])) {
        --dest;
    }
    *dest = '\0';

    return respath;

error:
    return NULL;
}

bool file_exists(const char *path)
{
    struct stat s;

    if (path == NULL) {
        return false;
    }

    return stat(path, &s) == 0;
}

/* dir exists */
bool dir_exists(const char *path)
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

/* wait for pid */
int wait_for_pid(pid_t pid)
{
    int st;
    int nret = 0;

again:
    nret = waitpid(pid, &st, 0);
    if (nret == -1) {
        if (errno == EINTR) {
            goto again;
        }
        return -1;
    }
    if (nret != pid) {
        goto again;
    }
    if (!WIFEXITED(st) || WEXITSTATUS(st) != 0) {
        return -1;
    }
    return 0;
}

/* wait for pid status */
int wait_for_pid_status(pid_t pid)
{
    int st;
    int nret = 0;
rep:
    nret = waitpid(pid, &st, 0);
    if (nret == -1) {
        if (errno == EINTR) {
            goto rep;
        }
        return -1;
    }

    if (nret != pid) {
        goto rep;
    }
    return st;
}

static char *do_string_join(const char *sep, const char **parts, size_t parts_len, size_t result_len)
{
    size_t iter;

    char *res_string = calloc(result_len + 1, 1);
    if (res_string == NULL) {
        return NULL;
    }

    for (iter = 0; iter < parts_len - 1; iter++) {
        if (strcat_s(res_string, result_len + 1, parts[iter]) != EOK) {
            free(res_string);
            return NULL;
        }
        if (strcat_s(res_string, result_len + 1, sep) != EOK) {
            free(res_string);
            return NULL;
        }
    }
    if (strcat_s(res_string, result_len + 1, parts[parts_len - 1]) != EOK) {
        free(res_string);
        return NULL;
    }
    return res_string;
}

char *util_string_join(const char *sep, const char **parts, size_t len)
{
    size_t sep_len;
    size_t result_len;
    size_t iter;

    if (len == 0 || parts == NULL || sep == NULL) {
        return NULL;
    }

    sep_len = strlen(sep);

    if ((sep_len != 0) && (sep_len != 1) && (len > SIZE_MAX / sep_len + 1)) {
        return NULL;
    }
    result_len = (len - 1) * sep_len;
    for (iter = 0; iter < len; iter++) {
        if (parts[iter] == NULL || result_len >= SIZE_MAX - strlen(parts[iter])) {
            return NULL;
        }
        result_len += strlen(parts[iter]);
    }

    return do_string_join(sep, parts, len, result_len);
}

int util_mkdir_p(const char *dir, mode_t mode)
{
    const char *tmp_pos = NULL;
    const char *base = NULL;
    char *cur_dir = NULL;
    ssize_t len = 0;

    if (dir == NULL || strlen(dir) > PATH_MAX) {
        goto err_out;
    }

    tmp_pos = dir;
    base = dir;

    do {
        dir = tmp_pos + strspn(tmp_pos, "/");
        tmp_pos = dir + strcspn(dir, "/");
        len = dir - base;
        if (len <= 0) {
            break;
        }
        cur_dir = strndup(base, (size_t)len);
        if (cur_dir == NULL) {
            ERROR("strndup failed");
            goto err_out;
        }
        if (*cur_dir) {
            if (mkdir(cur_dir, mode) && (errno != EEXIST || !util_dir_exists(cur_dir))) {
                ERROR("failed to create directory '%s': %s", cur_dir, strerror(errno));
                goto err_out;
            }
        }
        free(cur_dir);
    } while (tmp_pos != dir);

    return 0;
err_out:
    free(cur_dir);
    return -1;
}

/* lcr shrink array*/
static char **lcr_shrink_array(char **orig_array, size_t new_size)
{
    char **res_array = NULL;
    size_t i;

    if (new_size == 0) {
        return orig_array;
    }

    if (new_size > SIZE_MAX / sizeof(char *)) {
        return orig_array;
    }
    res_array = (char **)util_common_calloc_s(new_size * sizeof(char *));
    if (res_array == NULL) {
        return orig_array;
    }

    for (i = 0; i < new_size; i++) {
        res_array[i] = orig_array[i];
    }
    free(orig_array);
    return res_array;
}

/* lcr string split and trim*/
char **lcr_string_split_and_trim(const char *orig_str, char _sep)
{
    char *token = NULL;
    char *str = NULL;
    char *reserve_ptr = NULL;
    char deli[2] = { _sep, '\0' };
    char **res_array = NULL;
    size_t capacity = 0;
    size_t count = 0;
    int r, tmp_errno;

    if (orig_str == NULL) {
        return calloc(1, sizeof(char *));
    }

    str = util_strdup_s(orig_str);

    token = strtok_r(str, deli, &reserve_ptr);
    while (token != NULL) {
        while (token[0] == ' ' || token[0] == '\t') {
            token++;
        }
        size_t len = strlen(token);
        while (len > 0 && (token[len - 1] == ' ' || token[len - 1] == '\t')) {
            token[len - 1] = '\0';
            len--;
        }
        r = lcr_grow_array((void ***)&res_array, &capacity, count + 1, 16);
        if (r < 0) {
            goto error_out;
        }
        res_array[count] = util_strdup_s(token);
        count++;
        token = strtok_r(NULL, deli, &reserve_ptr);
    }
    free(str);

    return lcr_shrink_array(res_array, count + 1);

error_out:
    tmp_errno = errno;
    lcr_free_array((void **)res_array);
    free(str);
    errno = tmp_errno;
    return NULL;
}

/* lcr free array*/
void lcr_free_array(void **orig_array)
{
    void **p = NULL;
    for (p = orig_array; p && *p; p++) {
        free(*p);
        *p = NULL;
    }
    free((void *)orig_array);
}

/* lcr grow array*/
int lcr_grow_array(void ***orig_array, size_t *orig_capacity, size_t size, size_t increment)
{
    size_t add_capacity;
    void **add_array = NULL;

    if (orig_array == NULL || orig_capacity == NULL || size == SIZE_MAX) {
        return -1;
    }

    if ((*orig_array) == NULL || (*orig_capacity) == 0) {
        free(*orig_array);
        *orig_array = NULL;
        *orig_capacity = 0;
    }

    add_capacity = *orig_capacity;
    while (size + 1 > add_capacity) {
        if (add_capacity > SIZE_MAX - increment) {
            return -1;
        }
        add_capacity += increment;
    }
    if (add_capacity != *orig_capacity) {
        if (add_capacity > SIZE_MAX / sizeof(void *)) {
            return -1;
        }
        add_array = util_common_calloc_s(add_capacity * sizeof(void *));
        if (add_array == NULL) {
            return -1;
        }
        if (*orig_array) {
            if (memcpy_s(add_array, add_capacity * sizeof(void *), *orig_array, *orig_capacity * sizeof(void *)) !=
                EOK) {
                ERROR("Failed to memcpy memory");
                free(add_array);
                return -1;
            }
            free((void *)*orig_array);
        }

        *orig_array = add_array;
        *orig_capacity = add_capacity;
    }

    return 0;
}

/* lcr array len*/
size_t lcr_array_len(void **orig_array)
{
    void **p = NULL;
    size_t length = 0;

    for (p = orig_array; p && *p; p++) {
        length++;
    }

    return length;
}

/* util common malloc s*/
void *util_common_calloc_s(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    return calloc(1, size);
}

int mem_realloc(void **newptr, size_t newsize, void *oldptr, size_t oldsize)
{
    int nret = 0;
    void *addr = NULL;

    if (newptr == NULL) {
        return -1;
    }

    if (oldsize >= newsize || newsize == 0) {
        goto err_out;
    }

    addr = util_common_calloc_s(newsize);
    if (addr == NULL) {
        goto err_out;
    }

    if (oldptr != NULL) {
        nret = memcpy_s(addr, newsize, oldptr, oldsize);
        if (nret != EOK) {
            free(addr);
            goto err_out;
        }
        free(oldptr);
    }

    *newptr = addr;
    return 0;

err_out:
    return -1;
}

bool util_valid_cmd_arg(const char *arg)
{
    return arg && strchr(arg, '|') == NULL && strchr(arg, '`') == NULL && strchr(arg, '&') == NULL &&
           strchr(arg, ';') == NULL;
}

static inline bool is_invalid_error_str(const char *err_str, const char *numstr)
{
    return err_str == NULL || err_str == numstr || *err_str != '\0';
}

int util_safe_strtod(const char *numstr, double *converted)
{
    char *err_str = NULL;
    double ld;

    if (numstr == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    ld = strtod(numstr, &err_str);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, numstr)) {
        return -EINVAL;
    }

    *converted = ld;
    return 0;
}

/* util safe ullong*/
int util_safe_ullong(const char *numstr, unsigned long long *converted)
{
    char *err_str = NULL;
    unsigned long long sli;

    if (numstr == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    sli = strtoull(numstr, &err_str, 0);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, numstr)) {
        return -EINVAL;
    }

    *converted = (unsigned long long)sli;
    return 0;
}

/* util safe uint*/
int util_safe_uint(const char *numstr, unsigned int *converted)
{
    char *err_str = NULL;
    unsigned long int ui;

    if (numstr == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    ui = strtoul(numstr, &err_str, 0);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, numstr)) {
        return -EINVAL;
    }

    if (ui > UINT_MAX) {
        return -ERANGE;
    }

    *converted = (unsigned int)ui;
    return 0;
}

struct unit_map_def_lcr {
    int64_t mltpl;
    char *name;
};

static struct unit_map_def_lcr const g_lcr_unit_map[] = {
    { .mltpl = 1, .name = "I" },         { .mltpl = 1, .name = "B" },         { .mltpl = 1, .name = "IB" },
    { .mltpl = SIZE_KB, .name = "K" },   { .mltpl = SIZE_KB, .name = "KI" },  { .mltpl = SIZE_KB, .name = "KB" },
    { .mltpl = SIZE_KB, .name = "KIB" }, { .mltpl = SIZE_MB, .name = "M" },   { .mltpl = SIZE_MB, .name = "MI" },
    { .mltpl = SIZE_MB, .name = "MB" },  { .mltpl = SIZE_MB, .name = "MIB" }, { .mltpl = SIZE_GB, .name = "G" },
    { .mltpl = SIZE_GB, .name = "GI" },  { .mltpl = SIZE_GB, .name = "GB" },  { .mltpl = SIZE_GB, .name = "GIB" },
    { .mltpl = SIZE_TB, .name = "T" },   { .mltpl = SIZE_TB, .name = "TI" },  { .mltpl = SIZE_TB, .name = "TB" },
    { .mltpl = SIZE_TB, .name = "TIB" }, { .mltpl = SIZE_PB, .name = "P" },   { .mltpl = SIZE_PB, .name = "PI" },
    { .mltpl = SIZE_PB, .name = "PB" },  { .mltpl = SIZE_PB, .name = "PIB" }
};

static size_t const g_lcr_unit_map_len = sizeof(g_lcr_unit_map) / sizeof(g_lcr_unit_map[0]);

/* parse unit multiple*/
static int parse_unit_multiple(const char *unit, int64_t *mltpl)
{
    size_t i;
    if (unit[0] == '\0') {
        *mltpl = 1;
        return 0;
    }

    for (i = 0; i < g_lcr_unit_map_len; i++) {
        if (strcasecmp(unit, g_lcr_unit_map[i].name) == 0) {
            *mltpl = g_lcr_unit_map[i].mltpl;
            return 0;
        }
    }
    return -EINVAL;
}

static int util_parse_size_int_and_float(const char *numstr, int64_t mlt, int64_t *converted)
{
    long long int_size = 0;
    double float_size = 0;
    long long int_real = 0;
    long long float_real = 0;
    int nret;

    char *dot = strchr(numstr, '.');
    if (dot != NULL) {
        char tmp;
        // interger.float
        if (dot == numstr || *(dot + 1) == '\0') {
            return -EINVAL;
        }
        // replace 123.456 to 120.456
        tmp = *(dot - 1);
        *(dot - 1) = '0';
        // parsing 0.456
        nret = util_safe_strtod(dot - 1, &float_size);
        // recover 120.456 to 123.456
        *(dot - 1) = tmp;
        if (nret < 0) {
            return nret;
        }
        float_real = (int64_t)float_size;
        if (mlt > 0) {
            if (INT64_MAX / mlt < (int64_t)float_size) {
                return -ERANGE;
            }
            float_real = (int64_t)(float_size * mlt);
        }
        *dot = '\0';
    }
    nret = util_safe_llong(numstr, &int_size);
    if (nret < 0) {
        return nret;
    }
    int_real = int_size;
    if (mlt > 0) {
        if (INT64_MAX / mlt < int_size) {
            return -ERANGE;
        }
        int_real = int_size * mlt;
    }
    if (INT64_MAX - int_real < float_real) {
        return -ERANGE;
    }

    *converted = int_real + float_real;
    return 0;
}

/* parse byte size string*/
int parse_byte_size_string(const char *s, int64_t *converted)
{
    int ret;
    int64_t mltpl = 0;
    char *dup = NULL;
    char *pmlt = NULL;

    if (converted == NULL || s == NULL || s[0] == '\0' || !isdigit(s[0])) {
        return -EINVAL;
    }

    dup = util_strdup_s(s);

    pmlt = dup;
    while (*pmlt != '\0' && (isdigit(*pmlt) || *pmlt == '.')) {
        pmlt++;
    }

    ret = parse_unit_multiple(pmlt, &mltpl);
    if (ret) {
        free(dup);
        return ret;
    }

    // replace the first multiple arg to '\0'
    *pmlt = '\0';
    ret = util_parse_size_int_and_float(dup, mltpl, converted);
    free(dup);
    return ret;
}

/*
 * if path do not exist, this function will create it.
 */
int util_ensure_path(char **confpath, const char *path)
{
    int err = -1;
    int fd;
    char real_path[PATH_MAX + 1] = { 0 };

    if (confpath == NULL || path == NULL) {
        return -1;
    }

    fd = util_open(path, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, DEFAULT_SECURE_FILE_MODE);
    if (fd < 0 && errno != EEXIST) {
        ERROR("failed to open '%s'", path);
        goto err;
    }
    if (fd >= 0) {
        close(fd);
    }

    if (strlen(path) > PATH_MAX || realpath(path, real_path) == NULL) {
        ERROR("Failed to get real path: %s", path);
        goto err;
    }

    *confpath = util_strdup_s(real_path);

    err = EXIT_SUCCESS;

err:
    return err;
}

/* util dir exists*/
bool util_dir_exists(const char *path)
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


/* util rmdir one*/
static void util_rmdir_one(const char *dirpath, const struct dirent *pdirent, int recursive_depth, int *failure)
{
    struct stat fstat;
    int nret;
    char fname[PATH_MAX] = { 0 };

    if (is_dot(pdirent->d_name) || is_double_dot(pdirent->d_name)) {
        return;
    }

    nret = sprintf_s(fname, PATH_MAX, "%s/%s", dirpath, pdirent->d_name);
    if (nret < 0) {
        ERROR("Pathname too long");
        *failure = 1;
        return;
    }

    nret = lstat(fname, &fstat);
    if (nret) {
        ERROR("Failed to stat %s", fname);
        *failure = 1;
        return;
    }

    if (S_ISDIR(fstat.st_mode)) {
        if (util_recursive_rmdir(fname, (recursive_depth + 1)) < 0) {
            *failure = 1;
        }
    } else {
        if (unlink(fname) < 0) {
            ERROR("Failed to delete %s", fname);
            *failure = 1;
        }
    }
}

/* util recursive rmdir*/
int util_recursive_rmdir(const char *dirpath, int recursive_depth)
{
    struct dirent *pdirent = NULL;
    DIR *directory = NULL;
    int nret;
    int failure = 0;

    if ((recursive_depth + 1) > MAX_PATH_DEPTH) {
        ERROR("Reach max path depth: %s", dirpath);
        failure = 1;
        goto err_out;
    }

    if (!util_dir_exists(dirpath)) { /*dir not exists*/
        goto err_out;
    }

    directory = opendir(dirpath);
    if (directory == NULL) {
        ERROR("Failed to open %s", dirpath);
        failure = 1;
        goto err_out;
    }

    pdirent = readdir(directory);
    while (pdirent != NULL) {
        util_rmdir_one(dirpath, pdirent, recursive_depth, &failure);
        pdirent = readdir(directory);
    }

    if (rmdir(dirpath) < 0) {
        ERROR("Failed to delete %s", dirpath);
        failure = 1;
    }

    nret = closedir(directory);
    if (nret) {
        ERROR("Failed to close directory %s", dirpath);
        failure = 1;
    }

err_out:
    return failure ? -1 : 0;
}

/* util string replace one*/
static ssize_t util_string_replace_one(const char *needle, const char *replace, const char *haystack, char **result)
{
    char *res_string = *result;
    char *p = NULL;
    char *next_p = NULL;
    ssize_t length = 0;
    size_t replace_len, nl_len, part_len;

    replace_len = strlen(replace);
    nl_len = strlen(needle);

    for (next_p = (char *)haystack, p = strstr(next_p, needle); p != NULL; next_p = p, p = strstr(next_p, needle)) {
        part_len = (size_t)(p - next_p);
        if ((res_string != NULL) && part_len > 0) {
            if (memcpy_s(&res_string[length], part_len, next_p, part_len) != EOK) {
                ERROR("Failed to copy memory!");
                return -1;
            }
        }
        length += (ssize_t)part_len;
        if ((res_string != NULL) && replace_len > 0) {
            if (memcpy_s(&res_string[length], replace_len, replace, replace_len) != EOK) {
                ERROR("Failed to copy memory!");
                return -1;
            }
        }
        length += (ssize_t)replace_len;
        p += nl_len;
    }
    part_len = strlen(next_p);
    if ((res_string != NULL) && part_len > 0) {
        if (memcpy_s(&res_string[length], part_len, next_p, part_len) != EOK) {
            ERROR("Failed to copy memory!");
            return -1;
        }
    }
    length += (ssize_t)part_len;
    return length;
}

/* util string replace*/
char *util_string_replace(const char *needle, const char *replace, const char *haystack)
{
    ssize_t length = -1;
    ssize_t reserve_len = -1;
    char *res_string = NULL;

    if ((needle == NULL) || (replace == NULL) || (haystack == NULL)) {
        ERROR("Invalid NULL pointer");
        return NULL;
    }

    while (length == -1 || res_string == NULL) {
        if (length != -1) {
            res_string = calloc(1, (size_t)length + 1);
            if (res_string == NULL) {
                return NULL;
            }
            reserve_len = length;
        }
        length = util_string_replace_one(needle, replace, haystack, &res_string);
        if (length < 0) {
            free(res_string);
            return NULL;
        }
    }

    if (reserve_len != length) {
        free(res_string);
        return NULL;
    }
    if (res_string[length] != '\0') {
        free(res_string);
        return NULL;
    }

    return res_string;
}

int util_open(const char *filename, int flags, mode_t mode)
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

FILE *util_fopen(const char *filename, const char *mode)
{
    unsigned int fdmode = 0;
    int f_fd = -1;
    int tmperrno;
    FILE *fp = NULL;
    char rpath[PATH_MAX] = { 0x00 };

    if (mode == NULL) {
        return NULL;
    }

    if (cleanpath(filename, rpath, sizeof(rpath)) == NULL) {
        ERROR("cleanpath failed");
        return NULL;
    }
    if (!strncmp(mode, "a+", 2)) {
        fdmode = O_RDWR | O_CREAT | O_APPEND;
    } else if (!strncmp(mode, "a", 1)) {
        fdmode = O_WRONLY | O_CREAT | O_APPEND;
    } else if (!strncmp(mode, "w+", 2)) {
        fdmode = O_RDWR | O_TRUNC | O_CREAT;
    } else if (!strncmp(mode, "w", 1)) {
        fdmode = O_WRONLY | O_TRUNC | O_CREAT;
    } else if (!strncmp(mode, "r+", 2)) {
        fdmode = O_RDWR;
    } else if (!strncmp(mode, "r", 1)) {
        fdmode = O_RDONLY;
    }

    fdmode |= O_CLOEXEC;

    f_fd = open(rpath, (int)fdmode, 0666);
    if (f_fd < 0) {
        return NULL;
    }

    fp = fdopen(f_fd, mode);
    tmperrno = errno;
    if (fp == NULL) {
        close(f_fd);
    }
    errno = tmperrno;
    return fp;
}

/* util safe int*/
int util_safe_int(const char *numstr, int *converted)
{
    char *err_str = NULL;
    signed long int li;

    if (numstr == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    li = strtol(numstr, &err_str, 0);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, numstr)) {
        return -EINVAL;
    }

    if (li > INT_MAX) {
        return -ERANGE;
    }

    *converted = (int)li;
    return 0;
}

/* util check inherited*/
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

int util_check_inherited(bool closeall, int fd_to_ignore)
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

        if (util_safe_int(pdirent->d_name, &fd) < 0) {
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

/* util string append */
char *util_string_append(const char *post, const char *pre)
{
    char *res_string = NULL;
    size_t length = 0;

    if (post == NULL && pre == NULL) {
        return NULL;
    }
    if (pre == NULL) {
        return util_strdup_s(post);
    }
    if (post == NULL) {
        return util_strdup_s(pre);
    }
    if (strlen(post) > ((SIZE_MAX - strlen(pre)) - 1)) {
        ERROR("String is too long to be appended");
        return NULL;
    }
    length = strlen(post) + strlen(pre) + 1;
    res_string = util_common_calloc_s(length);
    if (res_string == NULL) {
        return NULL;
    }
    if (strcat_s(res_string, length, pre) != EOK) {
        free(res_string);
        return NULL;
    }
    if (strcat_s(res_string, length, post) != EOK) {
        free(res_string);
        return NULL;
    }

    return res_string;
}

/* util string split prefix */
char *util_string_split_prefix(size_t prefix_len, const char *file)
{
    size_t file_len = 0;
    size_t len = 0;
    char *path = NULL;

    if (file == NULL) {
        return NULL;
    }

    file_len = strlen(file);
    if (file_len < prefix_len) {
        return NULL;
    }
    len = strlen(file) - prefix_len;
    if (len > SIZE_MAX / sizeof(char) - 1) {
        return NULL;
    }
    path = util_common_calloc_s((len + 1) * sizeof(char));
    if (path == NULL) {
        return NULL;
    }
    if (strncpy_s(path, len + 1, file + prefix_len, len) != EOK) {
        ERROR("Failed to copy string!");
        free(path);
        return NULL;
    }
    path[len] = '\0';

    return path;
}

static void set_char_to_terminator(char *p)
{
    *p = '\0';
}
/*
 * @name is absolute path of this file.
 * make all directory in this absolute path.
 * */
int util_build_dir(const char *name)
{
    char *n = NULL; // because we'll be modifying it
    char *p = NULL;
    char *e = NULL;
    int nret;

    if (name == NULL) {
        return -1;
    }

    n = util_strdup_s(name);
    e = &(n[strlen(n)]);
    for (p = n + 1; p < e; p++) {
        if (*p != '/') {
            continue;
        }
        set_char_to_terminator(p);
        if (access(n, F_OK)) {
            nret = mkdir(n, DEFAULT_SECURE_DIRECTORY_MODE);
            if (nret && (errno != EEXIST || !dir_exists(n))) {
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
ssize_t util_write_nointr(int fd, const void *buf, size_t count)
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
ssize_t util_read_nointr(int fd, void *buf, size_t count)
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

/* util free array */
void util_free_array(char **array)
{
    char **p = NULL;

    /* free a null pointer may cause codex error */
    if (array == NULL) {
        return;
    }

    for (p = array; p && *p; p++) {
        free(*p);
        *p = NULL;
    }
    free((void *)array);
}

/* sig num */
static int sig_num(const char *sig)
{
    int n;

    if (util_safe_int(sig, &n) < 0) {
        return -1;
    }

    return n;
}

struct signame {
    int num;
    const char *name;
};

/* util sig parse */
int util_sig_parse(const char *signame)
{
    size_t n;
    const struct signame signames[] = SIGNAL_MAP_DEFAULT;

    if (signame == NULL) {
        return -1;
    }

    if (isdigit(*signame)) {
        return sig_num(signame);
    } else if (strncasecmp(signame, "sig", 3) == 0) {
        signame += 3;
        for (n = 0; n < sizeof(signames) / sizeof(signames[0]); n++) {
            if (strcasecmp(signames[n].name, signame) == 0) {
                return signames[n].num;
            }
        }
    } else {
        for (n = 0; n < sizeof(signames) / sizeof(signames[0]); n++) {
            if (strcasecmp(signames[n].name, signame) == 0) {
                return signames[n].num;
            }
        }
    }

    return -1;
}

/* util valid signal */
bool util_valid_signal(int sig)
{
    size_t n = 0;
    const struct signame signames[] = SIGNAL_MAP_DEFAULT;

    for (n = 0; n < sizeof(signames) / sizeof(signames[0]); n++) {
        if (signames[n].num == sig) {
            return true;
        }
    }

    return false;
}

/* str skip str */
const char *str_skip_str(const char *str, const char *skip)
{
    if (str == NULL || skip == NULL) {
        return NULL;
    }

    for (;; str++, skip++) {
        if (!*skip) {
            return str;
        } else if (*str != *skip) {
            return NULL;
        }
    }
}

/* util array len */
size_t util_array_len(char **array)
{
    char **pos = NULL;
    size_t len = 0;

    for (pos = array; pos && *pos; pos++) {
        len++;
    }

    return len;
}

/* util array append */
int util_array_append(char ***array, const char *element)
{
    size_t len;
    char **new_array = NULL;

    if (array == NULL || element == NULL) {
        return -1;
    }

    // let newlen to len + 2 for element and null
    len = util_array_len(*array);
    if (len > SIZE_MAX / sizeof(char *) - 2) {
        ERROR("Array size is too big!");
        return -1;
    }
    new_array = util_common_calloc_s((len + 2) * sizeof(char *));
    if (new_array == NULL) {
        ERROR("Out of memory");
        return -1;
    }
    if (*array) {
        if (memcpy_s(new_array, (len + 2) * sizeof(char *), *array, len * sizeof(char *)) != EOK) {
            ERROR("Failed to memcpy memory");
            free(new_array);
            return -1;
        }
        free((void *)*array);
    }
    *array = new_array;

    new_array[len] = util_strdup_s(element);

    return 0;
}

/* util safe llong */
int util_safe_llong(const char *numstr, long long *converted)
{
    char *err_str = NULL;
    long long ll;

    if (numstr == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    ll = strtoll(numstr, &err_str, 0);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, numstr)) {
        return -EINVAL;
    }

    *converted = (long long)ll;
    return 0;
}

char *util_strdup_s(const char *src)
{
    char *dst = NULL;

    if (src == NULL) {
        return NULL;
    }

    dst = strdup(src);
    if (dst == NULL) {
        abort();
    }

    return dst;
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

int util_null_stdfds(void)
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

bool util_copy_file(const char *src_file, const char *dst_file)
{
    bool ret = false;
    char *nret = NULL;
    char real_src_file[PATH_MAX + 1] = { 0 };
    int src_fd = -1;
    int dst_fd = -1;
    char buf[BUFSIZE + 1] = { 0 };

    if (src_file == NULL || dst_file == NULL) {
        return ret;
    }
    nret = realpath(src_file, real_src_file);
    if (nret == NULL) {
        ERROR("real path: %s, return: %s", src_file, strerror(errno));
        return ret;
    }
    src_fd = util_open(real_src_file, O_RDONLY, CONFIG_FILE_MODE);
    if (src_fd < 0) {
        ERROR("Open src file: %s, failed: %s", real_src_file, strerror(errno));
        goto free_out;
    }
    dst_fd = util_open(dst_file, O_WRONLY | O_CREAT | O_TRUNC, DEFAULT_SECURE_FILE_MODE);
    if (dst_fd < 0) {
        ERROR("Creat file: %s, failed: %s", dst_file, strerror(errno));
        goto free_out;
    }
    while (true) {
        ssize_t len = util_read_nointr(src_fd, buf, BUFSIZE);
        if (len < 0) {
            ERROR("Read src file failed: %s", strerror(errno));
            goto free_out;
        } else if (len == 0) {
            break;
        }
        if (util_write_nointr(dst_fd, buf, (size_t)len) != len) {
            ERROR("Write file failed: %s", strerror(errno));
            goto free_out;
        }
    }
    ret = true;
free_out:
    if (src_fd >= 0) {
        close(src_fd);
    }
    if (dst_fd >= 0) {
        close(dst_fd);
    }
    return ret;
}

bool util_write_file(const char *filepath, const char *content, size_t len, bool add_newline)
{
    bool ret = true;
    int rfd = -1;

    if (filepath == NULL || content == NULL) {
        return false;
    }

    rfd = util_open(filepath, O_CREAT | O_TRUNC | O_WRONLY, CONFIG_FILE_MODE);
    if (rfd == -1) {
        ERROR("Create file %s failed: %s", filepath, strerror(errno));
        return false;
    }
    if (write(rfd, content, len) == -1) {
        ERROR("Write hostname failed: %s", strerror(errno));
        ret = false;
        goto out_free;
    }

    if (add_newline && write(rfd, "\n", 1) == -1) {
        ERROR("Write new line failed: %s", strerror(errno));
        ret = false;
        goto out_free;
    }

out_free:
    close(rfd);
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
    int ret = 0;
    size_t length = 0;
    size_t content_len = 0;
    char *line = NULL;
    char *tmp_str = NULL;
    bool need_append = true;
    while (getline(&line, &length, fp) != -1) {
        if (line == NULL) {
            ERROR("Failed to read content from file ptr");
            ret = -1;
            goto out;
        }
        util_trim_newline(line);
        if (!strcmp(content, line)) {
            need_append = false;
            break;
        }
    }
    if (need_append) {
        if (strlen(content) > ((SIZE_MAX - strlen("\n")) - 1)) {
            ret = -1;
            goto out;
        }
        content_len = strlen(content) + strlen("\n") + 1;
        tmp_str = util_common_calloc_s(content_len);
        if (tmp_str == NULL) {
            ERROR("Out of memory");
            ret = -1;
            goto out;
        }
        if (sprintf_s(tmp_str, content_len, "%s\n", content) < 0) {
            ERROR("Failed to print string");
            ret = -1;
            goto out;
        }
        if (fwrite(tmp_str, 1, strlen(tmp_str), fp) == 0) {
            ERROR("Failed to write content: '%s'", content);
            ret = -1;
            goto out;
        }
    }

out:
    free(tmp_str);
    free(line);
    return ret;
}

int util_atomic_write_file(const char *filepath, const char *content)
{
    int fd;
    int ret = 0;
    FILE *fp = NULL;
    struct flock lk;

    if (filepath == NULL || content == NULL) {
        return -1;
    }
    fd = util_open(filepath, O_RDWR | O_CREAT | O_APPEND, DEFAULT_SECURE_FILE_MODE);
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
            ret = -1;
            goto out;
        }
        ret = append_new_content_to_file(fp, content);
    }
out:
    if (fp != NULL) {
        fclose(fp);
    }
    close(fd);
    return ret;
}
