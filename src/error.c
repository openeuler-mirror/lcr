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
#include "error.h"
#include <stdarg.h>
#include <stdlib.h>

// record the lcr error
__thread engine_error_t g_lcr_error = {
    .errcode = LCR_SUCCESS,
    .errmsg = NULL
};

#define LCR_ERRMSG_GEN(n, s) { LCR_##n, s },
struct lcr_strerror_tab_t {
    lcr_errno_t lcr_errno;
    const char *errmsg;
};
static const struct lcr_strerror_tab_t g_lcr_strerror_tab[] = {
    LCR_ERRNO_MAP(LCR_ERRMSG_GEN)
};
#undef LCR_ERRMSG_GEN

const char *errno_to_error_message(lcr_errno_t err)
{
    if ((size_t)err >= sizeof(g_lcr_strerror_tab) / sizeof(g_lcr_strerror_tab[0])) {
        return g_lcr_strerror_tab[LCR_ERR_UNKNOWN].errmsg;
    }
    return g_lcr_strerror_tab[err].errmsg;
}

void clear_error_message(engine_error_t *error)
{
    if (error == NULL) {
        return;
    }
    error->errcode = LCR_SUCCESS;
    free(error->errmsg);
    error->errmsg = NULL;
}

void lcr_set_error_message(lcr_errno_t errcode, const char *format, ...)
{
    int ret = 0;
    char errbuf[BUFSIZ + 1] = { 0 };

    va_list argp;
    va_start(argp, format);

    ret = vsprintf(errbuf, format, argp);
    va_end(argp);
    clear_error_message(&g_lcr_error);
    if (ret < 0) {
        g_lcr_error.errcode = LCR_ERR_FORMAT;
        return;
    }
    g_lcr_error.errcode = errcode;
    g_lcr_error.errmsg = lcr_util_strdup_s(errbuf);
}

void lcr_try_set_error_message(lcr_errno_t errcode, const char *format, ...)
{
    int ret = 0;
    char errbuf[BUFSIZ + 1] = { 0 };
    va_list argp;

    if (g_lcr_error.errmsg != NULL || g_lcr_error.errcode != LCR_SUCCESS) {
        return;
    }
    va_start(argp, format);
    ret = vsprintf(errbuf, format, argp);
    va_end(argp);
    clear_error_message(&g_lcr_error);
    if (ret < 0) {
        g_lcr_error.errcode = LCR_ERR_FORMAT;
        return;
    }
    g_lcr_error.errcode = errcode;
    g_lcr_error.errmsg = lcr_util_strdup_s(errbuf);
}

void lcr_append_error_message(lcr_errno_t errcode, const char *format, ...)
{
    int ret = 0;
    char errbuf[BUFSIZ + 1] = { 0 };
    char *result = NULL;

    va_list argp;
    va_start(argp, format);

    ret = vsprintf(errbuf, format, argp);
    va_end(argp);
    if (ret < 0) {
        g_lcr_error.errcode = LCR_ERR_FORMAT;
        return;
    }
    g_lcr_error.errcode = errcode;
    result = lcr_util_string_append(g_lcr_error.errmsg, errbuf);
    if (result == NULL) {
        g_lcr_error.errcode = LCR_ERR_MEMOUT;
        return;
    }
    free(g_lcr_error.errmsg);
    g_lcr_error.errmsg = result;
}
