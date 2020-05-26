/******************************************************************************
 * iSula-libutils: utils library for iSula
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

#ifndef _CONTAINER_HOOKS_H
# define _CONTAINER_HOOKS_H

# include "oci_runtime_spec.h"

#ifdef __cplusplus
extern "C" {
#endif

oci_runtime_spec_hooks *oci_runtime_spec_hooks_parse_file(const char *filename,
                                                          const struct parser_context *ctx, parser_error *err);

char *oci_runtime_spec_hooks_generate_json(const oci_runtime_spec_hooks *ptr, const struct parser_context *ctx,
                                           parser_error *err);

#ifdef __cplusplus
}
#endif

#endif

