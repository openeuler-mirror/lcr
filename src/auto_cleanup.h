/******************************************************************************
 * lcr: utils library for iSula
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

#ifndef __ISULA_AUTO_CLEANUP_H
#define __ISULA_AUTO_CLEANUP_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define isula_transfer_fd(fd)  \
    ({                         \
        int __tmp_fd = (fd);   \
        (fd) = -EBADF;         \
        __tmp_fd;              \
    })

#define isula_transfer_ptr(ptr)        \
    ({                                 \
        __typeof__(ptr) __tmp_ptr = (ptr); \
        (ptr) = NULL;                  \
        __tmp_ptr;                     \
    })

#define auto_cleanup_tag(name) __attribute__((__cleanup__(name##_cb)))

// define all used auto tags
#define __isula_auto_free auto_cleanup_tag(free_pointer)
#define __isula_auto_file auto_cleanup_tag(close_file)
#define __isula_auto_dir auto_cleanup_tag(close_dir)
#define __isula_auto_close auto_cleanup_tag(auto_close)
#define __isula_auto_pm_unlock auto_cleanup_tag(auto_pm_unlock)
#define __isula_auto_prw_unlock auto_cleanup_tag(auto_prw_unlock)

static inline void free_pointer_cb(void *ptr)
{
    void *real = *(void **)ptr;
    if (real != NULL) {
        free(real);
    }
}

static inline void close_file_cb(FILE **p)
{
    FILE *fp = *p;
    if (fp == NULL) {
        return;
    }

    while (fclose(fp) != 0 && errno == EINTR) {
        // if interupt by signal, just retry it
    }
}

static inline void close_dir_cb(DIR **p)
{
    DIR *dp = *p;
    if (dp == NULL) {
        return;
    }

    while (closedir(dp) < 0 && errno == EINTR) {
        // if interupt by signal, just retry it
    }
}

static inline void auto_close_cb(int *p)
{
    int fd = *p;
    int _save_err = errno;

    if (fd < 0) {
        return;
    }

    while (close(fd) < 0 && errno == EINTR) {
        // if interupt by signal, just retry it
    }

    errno = _save_err;
}

static inline void auto_pm_unlock_cb(pthread_mutex_t **p)
{
    if (*p != NULL) {
        (void)pthread_mutex_unlock(*p);
    }
}

static inline void auto_prw_unlock_cb(pthread_rwlock_t **p)
{
    if (*p != NULL) {
        (void)pthread_rwlock_unlock(*p);
    }
}

#ifdef __cplusplus
}
#endif

#endif