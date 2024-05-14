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
#include "utils_convert.h"

#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

#include "auto_cleanup.h"
#include "utils_memory.h"

static inline bool is_valid_str_bool_true(const char *str)
{
    return strcmp(str, "1") == 0 || strcmp(str, "t") == 0 || strcmp(str, "T") == 0 || strcmp(str, "true") == 0 ||
           strcmp(str, "TRUE") == 0 || strcmp(str, "True") == 0;
}

static inline bool is_valid_str_bool_false(const char *str)
{
    return strcmp(str, "0") == 0 || strcmp(str, "f") == 0 || strcmp(str, "F") == 0 || strcmp(str, "false") == 0 ||
           strcmp(str, "FALSE") == 0 || strcmp(str, "False") == 0;
}

int isula_safe_strto_bool(const char *boolstr, bool *converted)
{
    if (boolstr == NULL || converted == NULL) {
        return -EINVAL;
    }
    if (is_valid_str_bool_true(boolstr)) {
        *converted = true;
    } else if (is_valid_str_bool_false(boolstr)) {
        *converted = false;
    } else {
        return -EINVAL;
    }
    return 0;
}

static inline bool is_invalid_error_str(const char *err_str, const char *numstr)
{
    return err_str == NULL || err_str == numstr || *err_str != '\0';
}

int isula_safe_strto_uint16(const char *numstr, uint16_t *converted)
{
    char *err_str = NULL;
    unsigned long int ui;

    if (numstr == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    ui = strtoul(numstr, &err_str, 0);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, numstr)) {
        return -EINVAL;
    }

    if (ui > 0xFFFF) {
        return -ERANGE;
    }

    *converted = (uint16_t)ui;
    return 0;
}

int isula_safe_strto_uint64(const char *numstr, uint64_t *converted)
{
    char *err_str = NULL;
    uint64_t ull;

    if (numstr == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    ull = strtoull(numstr, &err_str, 0);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, numstr)) {
        return -EINVAL;
    }

    *converted = (uint64_t)ull;
    return 0;
}

int isula_safe_strto_double(const char *numstr, double *converted)
{
    char *err_str = NULL;
    double ld;

    if (numstr == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    ld = strtod(numstr, &err_str);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, numstr)) {
        return -EINVAL;
    }

    *converted = ld;
    return 0;
}

/* util safe uint */
int isula_safe_strto_uint(const char *numstr, unsigned int *converted)
{
    char *err_str = NULL;
    unsigned long int ui;

    if (numstr == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    ui = strtoul(numstr, &err_str, 0);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, numstr)) {
        return -EINVAL;
    }

    if (ui > UINT_MAX) {
        return -ERANGE;
    }

    *converted = (unsigned int)ui;
    return 0;
}

int isula_safe_strto_int(const char *num_str, int *converted)
{
    char *err_str = NULL;
    signed long int li;

    if (num_str == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    li = strtol(num_str, &err_str, 0);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, num_str)) {
        return -EINVAL;
    }

    if (li > INT_MAX || li < INT_MIN) {
        return -ERANGE;
    }

    *converted = (int)li;
    return 0;
}

/* util safe llong */
int isula_safe_strto_llong(const char *numstr, long long *converted)
{
    char *err_str = NULL;
    long long ll;

    if (numstr == NULL || converted == NULL) {
        return -EINVAL;
    }

    errno = 0;
    ll = strtoll(numstr, &err_str, 0);
    if (errno > 0) {
        return -errno;
    }

    if (is_invalid_error_str(err_str, numstr)) {
        return -EINVAL;
    }

    *converted = (long long)ll;
    return 0;
}

struct unit_map_def_lcr {
    int64_t mltpl;
    char *name;
};

static struct unit_map_def_lcr const g_lcr_unit_map[] = {
    { .mltpl = 1, .name = "I" },         { .mltpl = 1, .name = "B" },         { .mltpl = 1, .name = "IB" },
    { .mltpl = ISULA_SIZE_KB, .name = "K" },   { .mltpl = ISULA_SIZE_KB, .name = "KI" },  { .mltpl = ISULA_SIZE_KB, .name = "KB" },
    { .mltpl = ISULA_SIZE_KB, .name = "KIB" }, { .mltpl = ISULA_SIZE_MB, .name = "M" },   { .mltpl = ISULA_SIZE_MB, .name = "MI" },
    { .mltpl = ISULA_SIZE_MB, .name = "MB" },  { .mltpl = ISULA_SIZE_MB, .name = "MIB" }, { .mltpl = ISULA_SIZE_GB, .name = "G" },
    { .mltpl = ISULA_SIZE_GB, .name = "GI" },  { .mltpl = ISULA_SIZE_GB, .name = "GB" },  { .mltpl = ISULA_SIZE_GB, .name = "GIB" },
    { .mltpl = ISULA_SIZE_TB, .name = "T" },   { .mltpl = ISULA_SIZE_TB, .name = "TI" },  { .mltpl = ISULA_SIZE_TB, .name = "TB" },
    { .mltpl = ISULA_SIZE_TB, .name = "TIB" }, { .mltpl = ISULA_SIZE_PB, .name = "P" },   { .mltpl = ISULA_SIZE_PB, .name = "PI" },
    { .mltpl = ISULA_SIZE_PB, .name = "PB" },  { .mltpl = ISULA_SIZE_PB, .name = "PIB" }
};

static size_t const g_lcr_unit_map_len = sizeof(g_lcr_unit_map) / sizeof(g_lcr_unit_map[0]);

/* parse unit multiple */
static int parse_unit_multiple(const char *unit, int64_t *mltpl)
{
    size_t i;
    if (unit[0] == '\0') {
        *mltpl = 1;
        return 0;
    }

    for (i = 0; i < g_lcr_unit_map_len; i++) {
        if (strcasecmp(unit, g_lcr_unit_map[i].name) == 0) {
            *mltpl = g_lcr_unit_map[i].mltpl;
            return 0;
        }
    }
    return -EINVAL;
}

static int util_parse_size_int_and_float(const char *numstr, int64_t mlt, int64_t *converted)
{
    long long int_size = 0;
    double float_size = 0;
    long long int_real = 0;
    long long float_real = 0;
    int nret;

    char *dot = strchr(numstr, '.');
    if (dot != NULL) {
        char tmp;
        // interger.float
        if (dot == numstr || *(dot + 1) == '\0') {
            return -EINVAL;
        }
        // replace 123.456 to 120.456
        tmp = *(dot - 1);
        *(dot - 1) = '0';
        // parsing 0.456
        nret = isula_safe_strto_double(dot - 1, &float_size);
        // recover 120.456 to 123.456
        *(dot - 1) = tmp;
        if (nret < 0) {
            return nret;
        }
        float_real = (int64_t)float_size;
        if (mlt > 0) {
            if (INT64_MAX / mlt < (int64_t)float_size) {
                return -ERANGE;
            }
            float_real = (int64_t)(float_size * mlt);
        }
        *dot = '\0';
    }
    nret = isula_safe_strto_llong(numstr, &int_size);
    if (nret < 0) {
        return nret;
    }
    int_real = int_size;
    if (mlt > 0) {
        if (INT64_MAX / mlt < int_size) {
            return -ERANGE;
        }
        int_real = int_size * mlt;
    }
    if (INT64_MAX - int_real < float_real) {
        return -ERANGE;
    }

    *converted = int_real + float_real;
    return 0;
}

/* parse byte size string */
int isula_parse_byte_size_string(const char *s, int64_t *converted)
{
    int ret;
    int64_t mltpl = 0;
    __isula_auto_free char *dup = NULL;
    char *pmlt = NULL;

    if (converted == NULL || s == NULL || s[0] == '\0' || !isdigit(s[0])) {
        return -EINVAL;
    }

    // s != NULL, so dup cannot be NULL
    dup = isula_strdup_s(s);

    pmlt = dup;
    while (*pmlt != '\0' && (isdigit(*pmlt) || *pmlt == '.')) {
        pmlt++;
    }

    ret = parse_unit_multiple(pmlt, &mltpl);
    if (ret != 0) {
        return ret;
    }

    // replace the first multiple arg to '\0'
    *pmlt = '\0';
    ret = util_parse_size_int_and_float(dup, mltpl, converted);
    return ret;
}