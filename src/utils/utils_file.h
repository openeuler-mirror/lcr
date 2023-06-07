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
#ifndef _ISULA_UTILS_UTILS_FILE_H
#define _ISULA_UTILS_UTILS_FILE_H

#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC 02000000
#endif

#define MAX_PATH_DEPTH 1024

int lcr_util_build_dir(const char *name);
bool lcr_util_dir_exists(const char *path);
int lcr_util_ensure_path(char **confpath, const char *path);
int lcr_util_recursive_rmdir(const char *dirpath, int recursive_depth);

int lcr_util_open(const char *filename, int flags, mode_t mode);

ssize_t lcr_util_write_nointr(int fd, const void *buf, size_t count);
ssize_t lcr_util_read_nointr(int fd, void *buf, size_t count);
int lcr_util_atomic_write_file(const char *filepath, const char *content);

int lcr_util_check_inherited(bool closeall, int fd_to_ignore);
int lcr_util_null_stdfds(void);

#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_UTILS_FILE_H */