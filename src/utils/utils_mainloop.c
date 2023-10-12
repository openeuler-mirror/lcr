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
#include "utils_mainloop.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/epoll.h>

#include "utils_memory.h"

struct epoll_loop_handler {
    isula_epoll_loop_cb_t cb;
    int cbfd;
    void *cbdata;
};

#define MAX_EVENTS 100

/* epoll loop */
int isula_epoll_loop(isula_epoll_descr_t *descr, int t)
{
    int i;
    int ret = 0;
    struct epoll_loop_handler *epoll_handler = NULL;
    struct epoll_event evs[MAX_EVENTS];

    if (descr == NULL) {
        return -1;
    }

    while (1) {
        int ep_fds = epoll_wait(descr->fd, evs, MAX_EVENTS, t);
        if (ep_fds < 0) {
            if (errno == EINTR) {
                continue;
            }
            ret = -1;
            goto out;
        }

        for (i = 0; i < ep_fds; i++) {
            epoll_handler = (struct epoll_loop_handler *)(evs[i].data.ptr);
            if (epoll_handler->cb(epoll_handler->cbfd, evs[i].events, epoll_handler->cbdata, descr) !=
                EPOLL_LOOP_HANDLE_CONTINUE) {
                goto out;
            }
        }

        if (ep_fds == 0 && t != 0) {
            if (descr->timeout_cb != NULL) {
                descr->timeout_cb(descr->timeout_cbdata);
            }
            goto out;
        }

        if (isula_linked_list_empty(&descr->handler_list)) {
            goto out;
        }
    }
out:
    return ret;
}

/* epoll loop add handler */
int isula_epoll_add_handler(isula_epoll_descr_t *descr, int fd, isula_epoll_loop_cb_t callback, void *data)
{
    struct epoll_event ev = { 0 };
    struct epoll_loop_handler *epoll_handler = NULL;
    struct isula_linked_list *node = NULL;

    if (descr == NULL) {
        return -1;
    }

    epoll_handler = isula_common_calloc_s(sizeof(*epoll_handler));
    if (epoll_handler == NULL) {
        goto fail_out;
    }

    epoll_handler->cbfd = fd;
    epoll_handler->cb = callback;
    epoll_handler->cbdata = data;

    ev.events = EPOLLIN;
    ev.data.ptr = epoll_handler;

    if (epoll_ctl(descr->fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        goto fail_out;
    }

    node = isula_common_calloc_s(sizeof(struct isula_linked_list));
    if (node == NULL) {
        goto fail_out;
    }

    node->elem = epoll_handler;
    isula_linked_list_add(&descr->handler_list, node);
    return 0;

fail_out:
    (void)epoll_ctl(descr->fd, EPOLL_CTL_DEL, fd, &ev);
    free(epoll_handler);
    return -1;
}

/* epoll loop del handler */
int isula_epoll_remove_handler(isula_epoll_descr_t *descr, int fd)
{
    struct epoll_loop_handler *epoll_handler = NULL;
    struct isula_linked_list *index = NULL;

    if (descr == NULL) {
        return -1;
    }

    isula_linked_list_for_each(index, &descr->handler_list) {
        epoll_handler = index->elem;

        if (fd == epoll_handler->cbfd) {
            if (epoll_ctl(descr->fd, EPOLL_CTL_DEL, fd, NULL)) {
                goto fail_out;
            }

            isula_linked_list_del(index);
            free(index->elem);
            free(index);
            return 0;
        }
    }

fail_out:
    return -1;
}

/* epoll loop open */
int isula_epoll_open(isula_epoll_descr_t *descr)
{
    if (descr == NULL) {
        return -1;
    }

    descr->fd = epoll_create1(EPOLL_CLOEXEC);
    if (descr->fd < 0) {
        return -1;
    }

    isula_linked_list_init(&(descr->handler_list));
    descr->timeout_cb = NULL;
    descr->timeout_cbdata = NULL;
    return 0;
}

/* epoll loop close */
int isula_epoll_close(isula_epoll_descr_t *descr)
{
    struct isula_linked_list *index = NULL;
    struct isula_linked_list *next = NULL;
    int ret = 0;

    if (descr == NULL) {
        return ret;
    }

    isula_linked_list_for_each_safe(index, &(descr->handler_list), next) {
        isula_linked_list_del(index);
        free(index->elem);
        free(index);
    }

    ret = close(descr->fd);
    descr->fd = -1;

    return ret;
}

