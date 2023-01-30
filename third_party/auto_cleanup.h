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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define auto_cleanup_tag(name) __attribute__((__cleanup__(name##_cb)))

// define all used auto tags
#define __isula_auto_free auto_cleanup_tag(free_pointer)
#define __isula_auto_file auto_cleanup_tag(close_file)
#define __isula_auto_dir auto_cleanup_tag(close_dir)
#define __isula_auto_close auto_cleanup_tag(auto_close)

static inline void free_pointer_cb(void **ptr)
{
    void *real = *ptr;
    if (real == NULL) {
        return;
    }
    free(real);
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

    if (fd <= 0) {
        return;
    }

    while (close(fd) < 0 && errno == EINTR) {
        // if interupt by signal, just retry it
    }
}

#ifdef __cplusplus
}
#endif

#endif