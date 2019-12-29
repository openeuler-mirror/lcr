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
 * Description: provide container error definition
 ******************************************************************************/
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
    g_lcr_error.errmsg = util_strdup_s(errbuf);
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
    g_lcr_error.errmsg = util_strdup_s(errbuf);
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
    result = util_string_append(g_lcr_error.errmsg, errbuf);
    if (result == NULL) {
        g_lcr_error.errcode = LCR_ERR_MEMOUT;
        return;
    }
    free(g_lcr_error.errmsg);
    g_lcr_error.errmsg = result;
}
