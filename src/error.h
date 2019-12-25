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
