/******************************************************************************
 * isula: convert utils
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
#ifndef _ISULA_UTILS_UTILS_CONVERT_H
#define _ISULA_UTILS_UTILS_CONVERT_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISULA_SIZE_KB 1024LL
#define ISULA_SIZE_MB (1024LL * ISULA_SIZE_KB)
#define ISULA_SIZE_GB (1024LL * ISULA_SIZE_MB)
#define ISULA_SIZE_TB (1024LL * ISULA_SIZE_GB)
#define ISULA_SIZE_PB (1024LL * ISULA_SIZE_TB)

int isula_safe_strto_bool(const char *boolstr, bool *converted);

int isula_safe_strto_uint16(const char *numstr, uint16_t *converted);

int isula_safe_strto_uint64(const char *numstr, uint64_t *converted);

int isula_safe_strto_int(const char *num_str, int *converted);

int isula_safe_strto_uint(const char *numstr, unsigned int *converted);

int isula_safe_strto_llong(const char *numstr, long long *converted);

int isula_safe_strto_double(const char *numstr, double *converted);

int isula_parse_byte_size_string(const char *s, int64_t *converted);

#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_UTILS_CONVERT_H */