/******************************************************************************
 * Parser and generator for JSON definition of process
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 *
 * Authors:
 * Xu Xuepeng <xuxuepeng1@huawei.com>
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

#ifndef __ISULA_JSON_DEFS_PROCESS_H
#define __ISULA_JSON_DEFS_PROCESS_H

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

defs_process *defs_process_parse_data(const char *jsondata,
                                      const struct parser_context *ctx, parser_error *err);

defs_process *defs_process_parse_file(const char *filename,
                                      const struct parser_context *ctx, parser_error *err);

char *defs_process_generate_json(const defs_process *ptr, const struct parser_context *ctx,
                                 parser_error *err);

#ifdef __cplusplus
}
#endif

#endif

