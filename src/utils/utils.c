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
#include "utils.h"

#include <sys/wait.h>
#include <errno.h>
#include <time.h>

int isula_wait_pid_ret_status(pid_t pid)
{
    int st = 0;
    int nret = 0;

    while (1) {
        nret = waitpid(pid, &st, 0);
        if (nret == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (nret != pid) {
            continue;
        }
        break;
    }

    return st;
}

int isula_wait_pid(pid_t pid)
{
    int st;
 
    st = isula_wait_pid_ret_status(pid);
    if (st == -1) {
        return -1;
    }

    if (!WIFEXITED(st) || WEXITSTATUS(st) != 0) {
        return -1;
    }

    return 0;
}

void isula_usleep_nointerupt(unsigned long usec)
{
#define SECOND_TO_USECOND_MUTIPLE 1000000
    int ret = 0;
    struct timespec request = { 0 };
    struct timespec remain = { 0 };
    if (usec == 0) {
        return;
    }

    request.tv_sec = (time_t)(usec / SECOND_TO_USECOND_MUTIPLE);
    request.tv_nsec = (long)((usec % SECOND_TO_USECOND_MUTIPLE) * 1000);

    do {
        ret = nanosleep(&request, &remain);
        request = remain;
    } while (ret == -1 && errno == EINTR);
}