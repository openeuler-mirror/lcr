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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <read_file.h>
#include "defs.h"

#include "log.h"
#include "utils.h"

#define PARSE_ERR_BUFFER_SIZE 1024

define_cleaner_function(yajl_val, yajl_tree_free)
defs_process *defs_process_parse_data(const char *jsondata, const struct parser_context *ctx, parser_error *err)
{
    defs_process *ptr = NULL;
    __auto_cleanup(yajl_tree_free) yajl_val tree = NULL;
    char errbuf[PARSE_ERR_BUFFER_SIZE];
    struct parser_context tmp_ctx = { 0 };

    if (jsondata == NULL || err == NULL) {
        return NULL;
    }

    *err = NULL;
    if (ctx == NULL) {
        ctx = (const struct parser_context *)(&tmp_ctx);
    }

    tree = yajl_tree_parse(jsondata, errbuf, sizeof(errbuf));
    if (tree == NULL) {
        if (asprintf(err, "cannot parse the data: %s", errbuf) < 0) {
            *err = strdup("error allocating memory");
        }
        return NULL;
    }
    ptr = make_defs_process(tree, ctx, err);
    return ptr;
}

defs_process *
defs_process_parse_file(const char *filename, const struct parser_context *ctx, parser_error *err)
{
    defs_process *ptr = NULL;
    size_t filesize;
    __auto_free char *content = NULL;

    if (filename == NULL || err == NULL) {
        return NULL;
    }

    *err = NULL;
    content = read_file(filename, &filesize);
    if (content == NULL) {
        if (asprintf(err, "cannot read the file: %s", filename) < 0) {
            *err = strdup("error allocating memory");
        }
        return NULL;
    }

    ptr = defs_process_parse_data(content, ctx, err);
    return ptr;
}

static void
cleanup_yajl_gen (yajl_gen g)
{
    if (g) {
        yajl_gen_clear (g);
        yajl_gen_free (g);
    }
}

define_cleaner_function(yajl_gen, cleanup_yajl_gen)
char *defs_process_generate_json(const defs_process *ptr, const struct parser_context *ctx,
                                 parser_error *err)
{
    __auto_cleanup(cleanup_yajl_gen) yajl_gen g = NULL;
    struct parser_context tmp_ctx = { 0 };
    const unsigned char *gen_buf = NULL;
    char *json_buf = NULL;
    size_t gen_len = 0;

    if (ptr == NULL || err == NULL) {
        return NULL;
    }

    *err = NULL;
    if (ctx == NULL) {
        ctx = (const struct parser_context *)(&tmp_ctx);
    }

    if (!json_gen_init(&g, ctx)) {
        *err = strdup("Json_gen init failed");
        return json_buf;
    }

    if (yajl_gen_status_ok != gen_defs_process(g, ptr, ctx, err)) {
        if (*err == NULL) {
            *err = strdup("Failed to generate json");
        }
        return json_buf;
    }

    yajl_gen_get_buf(g, &gen_buf, &gen_len);
    if (gen_buf == NULL) {
        *err = strdup("Error to get generated json");
        return json_buf;
    }

    if (gen_len > SIZE_MAX - 1) {
        *err = strdup("Generated json too long");
        return json_buf;
    }

    json_buf = calloc(1, gen_len + 1);
    if (json_buf == NULL) {
        *err = strdup("Cannot allocate memory");
        return json_buf;
    }

    (void)memcpy((void *)json_buf, (void *)gen_buf, gen_len);
    json_buf[gen_len] = '\0';

    return json_buf;
}
