/******************************************************************************
 * isula: mainloop utils
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
#ifndef _ISULA_UTILS_UTILS_MAINLOOP_H
#define _ISULA_UTILS_UTILS_MAINLOOP_H

#include <stdint.h>

#include "utils_linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif

enum isula_mainloop_code {
    EPOLL_LOOP_HANDLE_CONTINUE = 0,
    EPOLL_LOOP_HANDLE_CLOSE = 1,
};

typedef void (*isula_epoll_timeout_cb_t)(void *data);

struct __isula_epoll_descr {
    int fd;
    struct isula_linked_list handler_list;
    void *timeout_cbdata;

    isula_epoll_timeout_cb_t timeout_cb;
};

typedef struct __isula_epoll_descr isula_epoll_descr_t;

typedef int (*isula_epoll_loop_cb_t)(int fd, uint32_t event, void *data, isula_epoll_descr_t *descr);

extern int isula_epoll_open(isula_epoll_descr_t *descr);

extern int isula_epoll_loop(isula_epoll_descr_t *descr, int t);

extern int isula_epoll_add_handler(isula_epoll_descr_t *descr, int fd, isula_epoll_loop_cb_t callback, void *data);

extern int isula_epoll_remove_handler(isula_epoll_descr_t *descr, int fd);

extern int isula_epoll_close(isula_epoll_descr_t *descr);

#ifdef __cplusplus
}
#endif

#endif
