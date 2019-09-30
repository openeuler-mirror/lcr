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
 * Author: maoweiyong
 * Create: 2018-11-08
 * Description: provide oci runtime hooks functions
 ******************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <securec.h>
#include "read_file.h"
#include "oci_runtime_hooks.h"

#define PARSE_ERR_BUFFER_SIZE 1024

char *oci_runtime_spec_hooks_generate_json(const oci_runtime_spec_hooks *ptr, const struct parser_context *ctx,
                                           parser_error *err)
{
    yajl_gen g = NULL;
    struct parser_context tmp_ctx = { 0 };
    const unsigned char *gen_buf = NULL;
    char *json_buf = NULL;
    size_t gen_len = 0;
    errno_t eret;

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
    eret = memcpy_s((void *)json_buf, gen_len + 1, (void *)gen_buf, gen_len);
    if (eret != EOK) {
        *err = strdup("Memcpy failed");
        goto free_out;
    }
    json_buf[gen_len] = '\0';

free_out:
    yajl_gen_clear(g);
    yajl_gen_free(g);
out:
    return json_buf;
}
