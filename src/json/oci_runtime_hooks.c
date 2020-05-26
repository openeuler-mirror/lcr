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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <read_file.h>
#include "oci_runtime_hooks.h"

#include "log.h"
#include "utils.h"

#define PARSE_ERR_BUFFER_SIZE 1024

oci_runtime_spec_hooks *oci_runtime_spec_hooks_parse_file(const char *filename,
                                                          const struct parser_context *ctx,
                                                          parser_error *err)
{
    yajl_val tree;
    size_t filesize;
    char *content = NULL;
    char errbuf[PARSE_ERR_BUFFER_SIZE] = { 0 };
    struct parser_context tmp_ctx = { 0 };

    if (filename == NULL || err == NULL) {
        return NULL;
    }

    *err = NULL;
    if (ctx == NULL) {
        ctx = &tmp_ctx;
    }
    content = read_file(filename, &filesize);
    if (content == NULL) {
        if (asprintf(err, "cannot read the file: %s", filename) < 0) {
            *err = lcr_util_strdup_s("error allocating memory");
        }
        return NULL;
    }
    tree = yajl_tree_parse(content, errbuf, sizeof(errbuf));
    free(content);
    if (tree == NULL) {
        if (asprintf(err, "cannot parse the file: %s", errbuf) < 0) {
            *err = lcr_util_strdup_s("error allocating memory");
        }
        return NULL;
    }
    oci_runtime_spec_hooks *ptr = make_oci_runtime_spec_hooks(tree, ctx, err);
    yajl_tree_free(tree);
    return ptr;
}

char *oci_runtime_spec_hooks_generate_json(const oci_runtime_spec_hooks *ptr, const struct parser_context *ctx,
                                           parser_error *err)
{
    yajl_gen g = NULL;
    struct parser_context tmp_ctx = { 0 };
    const unsigned char *gen_buf = NULL;
    char *json_buf = NULL;
    size_t gen_len = 0;

    if (ptr == NULL || err == NULL) {
        return NULL;
    }

    *err = NULL;
    if (ctx == NULL) {
        ctx = &tmp_ctx;
    }

    if (!json_gen_init(&g, ctx)) {
        *err = strdup("Json_gen init failed");
        goto out;
    }
    if (yajl_gen_status_ok != gen_oci_runtime_spec_hooks(g, ptr, ctx, err)) {
        if (*err == NULL) {
            *err = strdup("Failed to generate json");
        }
        goto free_out;
    }
    yajl_gen_get_buf(g, &gen_buf, &gen_len);
    if (gen_buf == NULL) {
        *err = strdup("Error to get generated json");
        goto free_out;
    }

    if (gen_len > SIZE_MAX - 1) {
        *err = strdup("Generated json too long");
        goto free_out;
    }

    json_buf = malloc(gen_len + 1);
    if (json_buf == NULL) {
        *err = strdup("Out of memory");
        goto free_out;
    }
    (void)memcpy((void *)json_buf, (void *)gen_buf, gen_len);
    json_buf[gen_len] = '\0';

free_out:
    yajl_gen_clear(g);
    yajl_gen_free(g);
out:
    return json_buf;
}

