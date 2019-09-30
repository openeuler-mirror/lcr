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
 * Description: provide container log functions
 ******************************************************************************/
#define _GNU_SOURCE
#define __STDC_FORMAT_MACROS /* Required for PRIu64 to work. */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>

#include "securec.h"
#include "log.h"
#include "utils.h"

const char * const g_engine_log_prio_name[] = {
    "FATAL",  "ALERT", "CRIT",  "ERROR", "WARN",
    "NOTICE", "INFO",  "DEBUG", "TRACE"
};

#define MAX_MSG_LENGTH 4096
#define MAX_LOG_PREFIX_LENGTH 15

static __thread char *g_engine_log_prefix = NULL;

static char *g_engine_log_vmname = NULL;
static bool g_engine_log_quiet = false;
static int g_engine_log_level = ENGINE_LOG_DEBUG;
static int g_engine_log_driver = LOG_DRIVER_STDOUT;
int g_engine_log_fd = -1;

/* engine set log prefix */
void engine_set_log_prefix(const char *prefix)
{
    if (prefix == NULL) {
        return;
    }

    free(g_engine_log_prefix);

    g_engine_log_prefix = util_strdup_s(prefix);
}

/* engine free log prefix */
void engine_free_log_prefix()
{
    free(g_engine_log_prefix);

    g_engine_log_prefix = NULL;
}

ssize_t write_nointr(int fd, const void *buf, size_t count);

void log_append_logfile(const struct engine_log_object_metadata *meta, const char *timestamp, const char *msg);
void log_append_stderr(const struct engine_log_object_metadata *meta, const char *timestamp, const char *msg);

/* engine change str logdriver to enum */
int engine_change_str_logdriver_to_enum(const char *driver)
{
    if (driver == NULL) {
        return LOG_DRIVER_NOSET;
    }
    if (!strcasecmp(driver, "stdout")) {
        return LOG_DRIVER_STDOUT;
    }
    if (!strcasecmp(driver, "fifo")) {
        return LOG_DRIVER_FIFO;
    }

    return -1;
}

#define LOG_FIFO_SIZE (1024 * 1024)

/* open fifo */
int open_fifo(const char *fifo_path)
{
    int nret;
    int fifo_fd = -1;

    nret = mknod(fifo_path, S_IFIFO | S_IRUSR | S_IWUSR, (dev_t)0);
    if (nret && errno != EEXIST) {
        printf("Mknod failed: %s\n", strerror(errno));
        return nret;
    }

    fifo_fd = util_open(fifo_path, O_RDWR | O_NONBLOCK, 0);
    if (fifo_fd == -1) {
        fprintf(stderr, "Open fifo %s failed: %s\n", fifo_path, strerror(errno));
        return -1;
    }

    if (fcntl(fifo_fd, F_SETPIPE_SZ, LOG_FIFO_SIZE) == -1) {
        fprintf(stderr, "Set fifo buffer size failed: %s", strerror(errno));
        close(fifo_fd);
        return -1;
    }

    return fifo_fd;
}

/* engine close log file */
void engine_close_log_file()
{
    if (g_engine_log_fd != -1) {
        close(g_engine_log_fd);
        g_engine_log_fd = -1;
    }
}

/* init log driver */
static int init_log_driver(const struct engine_log_config *log)
{
    int i, driver;

    for (i = ENGINE_LOG_FATAL; i < ENGINE_LOG_MAX; i++) {
        if (!strcasecmp(g_engine_log_prio_name[i], log->priority)) {
            g_engine_log_level = i;
            break;
        }
    }

    if (i == ENGINE_LOG_MAX) {
        fprintf(stderr, "Unable to parse logging level:%s\n", log->priority);
        return -1;
    }

    driver = engine_change_str_logdriver_to_enum(log->driver);
    if (driver < 0) {
        fprintf(stderr, "Invalid log driver: %s\n", log->driver);
        return -1;
    }
    g_engine_log_driver = driver;
    return 0;
}

/* log enable */
int log_enable(const struct engine_log_config *log)
{
    int nret = 0;
    char *full_path = NULL;

    if ((log->name == NULL) || (log->priority == NULL)) {
        return -1;
    }

    if (g_engine_log_fd != -1) {
        fprintf(stderr, "engine log already initialized\n");
        return 0;
    }
    if (init_log_driver(log)) {
        return -1;
    }

    free(g_engine_log_vmname);

    g_engine_log_vmname = util_strdup_s(log->name);

    g_engine_log_quiet = log->quiet;

    if ((log->file == NULL) || strcmp(log->file, "none") == 0) {
        if (g_engine_log_driver == LOG_DRIVER_FIFO) {
            fprintf(stderr, "Must set log file for driver %s\n", log->driver);
            nret = -1;
        }
        goto out;
    }
    full_path = util_strdup_s(log->file);
    if (full_path == NULL) {
        fprintf(stderr, "Out of memory\n");
        nret = -1;
        goto out;
    }

    if (util_build_dir(full_path)) {
        fprintf(stderr, "failed to create dir for log file\n");
        nret = -1;
        goto out;
    }
    g_engine_log_fd = open_fifo(full_path);

    if (g_engine_log_fd == -1) {
        nret = -1;
    }
out:
    if (nret) {
        if (g_engine_log_driver == LOG_DRIVER_FIFO) {
            g_engine_log_driver = LOG_DRIVER_NOSET;
        }
    }
    free(full_path);

    return nret;
}

static char *parse_timespec_to_human()
{
    struct timespec timestamp;
    struct tm ptm = { 0 };
    char date_time[ENGINE_LOG_TIME_SIZE] = { 0 };
    int nret;

    if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1) {
        fprintf(stderr, "Failed to get real time\n");
        return 0;
    }

    if (localtime_r(&(timestamp.tv_sec), &ptm) == NULL) {
        SYSERROR("Transfer timespec failed");
        return NULL;
    }

    nret = sprintf_s(date_time, ENGINE_LOG_TIME_SIZE, "%04d%02d%02d%02d%02d%02d.%03ld",
                     ptm.tm_year + 1900, ptm.tm_mon + 1, ptm.tm_mday, ptm.tm_hour, ptm.tm_min, ptm.tm_sec,
                     timestamp.tv_nsec / 1000000);

    if (nret < 0) {
        COMMAND_ERROR("Sprintf failed");
        return NULL;
    }

    return util_strdup_s(date_time);
}

/* engine log append */
int engine_log_append(const struct engine_log_object_metadata *meta, const char *format, ...)
{
    int rc;
    va_list args;
    char msg[MAX_MSG_LENGTH] = { 0 };
    char *date_time = NULL;
    int ret = 0;

    va_start(args, format);
    rc = vsprintf_s(msg, MAX_MSG_LENGTH, format, args);
    va_end(args);
    if (rc < 0 || rc >= MAX_MSG_LENGTH) {
        rc = sprintf_s(msg, MAX_MSG_LENGTH, "%s", "!!LONG LONG A LOG!!");
        if (rc < 0) {
            return 0;
        }
    }

    date_time = parse_timespec_to_human();
    if (date_time == NULL) {
        goto out;
    }

    switch (g_engine_log_driver) {
        case LOG_DRIVER_STDOUT:
            if (g_engine_log_quiet) {
                break;
            }
            log_append_stderr(meta, date_time, msg);
            break;
        case LOG_DRIVER_FIFO:
            if (g_engine_log_fd == -1) {
                fprintf(stderr, "Do not set log file\n");
                ret = -1;
                goto out;
            }
            log_append_logfile(meta, date_time, msg);
            break;
        case LOG_DRIVER_NOSET:
            break;
        default:
            fprintf(stderr, "Invalid log driver\n");
            ret = -1;
            goto out;
    }

out:
    free(date_time);
    return ret;
}

/* log append logfile */
void log_append_logfile(const struct engine_log_object_metadata *meta, const char *timestamp, const char *msg)
{
    char log_buffer[ENGINE_LOG_BUFFER_SIZE] = { 0 };
    int log_fd = -1;
    int nret;
    size_t size;
    char *tmp_prefix = NULL;

    if (meta->level > g_engine_log_level) {
        return;
    }
    log_fd = g_engine_log_fd;
    if (log_fd == -1) {
        return;
    }

    tmp_prefix = g_engine_log_prefix ? g_engine_log_prefix : g_engine_log_vmname;
    if (tmp_prefix != NULL && strlen(tmp_prefix) > MAX_LOG_PREFIX_LENGTH) {
        tmp_prefix = tmp_prefix + (strlen(tmp_prefix) - MAX_LOG_PREFIX_LENGTH);
    }
    nret = sprintf_s(log_buffer, sizeof(log_buffer), "%15s %s %-8s %s - %s:%s:%d - %s", tmp_prefix ? tmp_prefix : "",
                     timestamp, g_engine_log_prio_name[meta->level],
                     g_engine_log_vmname ? g_engine_log_vmname : "engine", meta->file, meta->func,
                     meta->line, msg);

    if (nret < 0) {
        nret = sprintf_s(log_buffer, sizeof(log_buffer), "%15s %s %-8s %s - %s:%s:%d - %s",
                         tmp_prefix ? tmp_prefix : "", timestamp, g_engine_log_prio_name[meta->level],
                         g_engine_log_vmname ? g_engine_log_vmname : "engine", meta->file,
                         meta->func, meta->line, "Large log message");
        if (nret < 0) {
            return;
        }
    }
    size = (size_t)nret;

    if (size > (sizeof(log_buffer) - 1)) {
        size = sizeof(log_buffer) - 1;
    }

    log_buffer[size] = '\n';

    if (write_nointr(log_fd, log_buffer, (size + 1)) == -1) {
        fprintf(stderr, "write log into logfile failed");
    }
}

/* log append stderr */
void log_append_stderr(const struct engine_log_object_metadata *meta, const char *timestamp, const char *msg)
{
    char *tmp_prefix = NULL;
    if (meta->level > g_engine_log_level) {
        return;
    }

    tmp_prefix = g_engine_log_prefix ? g_engine_log_prefix : g_engine_log_vmname;
    if (tmp_prefix != NULL && strlen(tmp_prefix) > MAX_LOG_PREFIX_LENGTH) {
        tmp_prefix = tmp_prefix + (strlen(tmp_prefix) - MAX_LOG_PREFIX_LENGTH);
    }
    fprintf(stderr, "%15s %s %-8s ", tmp_prefix ? tmp_prefix : "", timestamp, g_engine_log_prio_name[meta->level]);
    fprintf(stderr, "%s - ", g_engine_log_vmname ? g_engine_log_vmname : "engine");
    fprintf(stderr, "%s:%s:%d - ", meta->file, meta->func, meta->line);
    fprintf(stderr, "%s", msg);
    fprintf(stderr, "\n");
}

/* write nointr */
ssize_t write_nointr(int fd, const void *buf, size_t count)
{
    ssize_t nret;
    for (;;) {
        nret = write(fd, buf, count);
        if (nret < 0 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }
    return nret;
}
