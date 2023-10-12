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

#ifndef PATH_MAX
// 4.4 BSD and Solaris don't have PATH_MAX
#define PATH_MAX 4096
#endif

#define ISULA_MAX_PATH_DEPTH 1024

bool isula_dir_exists(const char *path);

bool isula_file_exists(const char *f);

int isula_dir_build(const char *name);

int isula_dir_recursive_mk(const char *dir, mode_t mode);

int isula_file_ensure_path(char **confpath, const char *path);

int isula_dir_recursive_remove(const char *dirpath, int recursive_depth);

int isula_file_open(const char *filename, int flags, mode_t mode);

int isula_path_remove(const char *path);

ssize_t isula_file_read_nointr(int fd, void *buf, size_t count);

ssize_t isula_file_write_nointr(int fd, const void *buf, size_t count);

ssize_t isula_file_total_write_nointr(int fd, const char *buf, size_t count);

int isula_file_atomic_write(const char *filepath, const char *content);

int isula_close_inherited_fds(bool closeall, int fd_to_ignore);

#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_UTILS_FILE_H */