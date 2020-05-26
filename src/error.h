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

#ifndef __LCR_ERROR_H
#define __LCR_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct engine_error {
    uint32_t errcode;
    char *errmsg;
} engine_error_t;

/* record the lcr error */
extern __thread engine_error_t g_lcr_error;

#define DEF_SUCCESS_STR "Success"
#define DEF_ERR_RUNTIME_STR "Runtime error"

#define LCR_ERRNO_MAP(XX)                                           \
    XX(SUCCESS, DEF_SUCCESS_STR)                                    \
    \
    /* err in posix api call */                                     \
    XX(ERR_MEMOUT, "Out of memory")                                 \
    XX(ERR_MEMSET, "Memory set error")                              \
    \
    /* err in other case or call function int thirdparty library */ \
    XX(ERR_FORMAT, "Error message is too long")                     \
    XX(ERR_INPUT, "Invalid input parameter")                        \
    XX(ERR_INTERNAL, "Server internal error")                       \
    \
    /* err in runtime module */                                     \
    XX(ERR_RUNTIME, DEF_ERR_RUNTIME_STR)                            \
    \
    /* err max */                                                   \
    XX(ERR_UNKNOWN, "Unknown error")

#define LCR_ERRNO_GEN(n, s) LCR_##n,
typedef enum { LCR_ERRNO_MAP(LCR_ERRNO_GEN) } lcr_errno_t;
#undef LCR_ERRNO_GEN

const char *errno_to_error_message(lcr_errno_t err);

void clear_error_message(engine_error_t *error);

void lcr_set_error_message(lcr_errno_t errcode, const char *format, ...);

void lcr_try_set_error_message(lcr_errno_t errcode, const char *format, ...);

void lcr_append_error_message(lcr_errno_t errcode, const char *format, ...);

#ifdef __cplusplus
}
#endif
#endif
