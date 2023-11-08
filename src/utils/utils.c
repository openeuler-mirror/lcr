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
#include <regex.h>

#include "log.h"

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

/*
 * do not support greedy matching, like: '(:?xx)'
 * return value:
 * -1  failed
 *  0  match
 *  1  no match
 */
int isula_reg_match(const char *patten, const char *str)
{
#define EVENT_ARGS_MAX 255
    int nret = 0;
    char buffer[EVENT_ARGS_MAX] = { 0 };
    regex_t reg;

    if (patten == NULL || str == NULL) {
        ERROR("invalid NULL param");
        return -1;
    }

    nret = regcomp(&reg, patten, REG_EXTENDED | REG_NOSUB);
    if (nret != 0) {
        regerror(nret, &reg, buffer, EVENT_ARGS_MAX);
        ERROR("regcomp %s failed: %s", patten, buffer);
        return -1;
    }

    nret = regexec(&reg, str, 0, NULL, 0);
    if (nret == 0) {
        nret = 0;
        goto free_out;
    } else if (nret == REG_NOMATCH) {
        nret = 1;
        goto free_out;
    } else {
        nret = -1;
        ERROR("reg match failed");
        goto free_out;
    }

free_out:
    regfree(&reg);

    return nret;
}