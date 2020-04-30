/******************************************************************************
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
 * engine licensed under the Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Author: wujing
 * Create: 2019-4-08
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
#include <time.h>

#include "utils.h"
#include "log.h"

const char * const g_engine_log_prio_name[] = {
    "FATAL",  "ALERT", "CRIT",  "ERROR", "WARN", "NOTICE", "INFO",  "DEBUG", "TRACE"
};

#define MAX_MSG_LENGTH 4096
#define MAX_LOG_PREFIX_LENGTH 15

static __thread char *g_engine_log_prefix = NULL;

static char *g_engine_log_module = NULL;
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
    g_engine_log_prefix = lcr_util_strdup_s(prefix);
}

/* engine free log prefix */
void engine_free_log_prefix(void)
{
    free(g_engine_log_prefix);
    g_engine_log_prefix = NULL;
}

static ssize_t write_nointr(int fd, const void *buf, size_t count);

static void do_fifo_log(const struct engine_log_object_metadata *metadata, const char *timestamp,
                        const char *msg);

static void do_stderr_log(const struct engine_log_object_metadata *metadata, const char *timestamp,
                          const char *msg);

/* engine change str logdriver to enum */
static int engine_change_str_logdriver_to_enum(const char *driver)
{
    if (driver == NULL) {
        return LOG_DRIVER_NOSET;
    }
    if (strcasecmp(driver, "stdout") == 0) {
        return LOG_DRIVER_STDOUT;
    }
    if (strcasecmp(driver, "fifo") == 0) {
        return LOG_DRIVER_FIFO;
    }

    return -1;
}

#define LOG_FIFO_SIZE (1024 * 1024)

/* open fifo */
static int open_fifo(const char *fifo_path)
{
    int nret;
    int fifo_fd = -1;

    nret = mknod(fifo_path, S_IFIFO | S_IRUSR | S_IWUSR, (dev_t)0);
    if (nret && errno != EEXIST) {
        COMMAND_ERROR("Mknod failed: %s", strerror(errno));
        return nret;
    }

    fifo_fd = lcr_util_open(fifo_path, O_RDWR | O_NONBLOCK, 0);
    if (fifo_fd == -1) {
        COMMAND_ERROR("Open fifo %s failed: %s", fifo_path, strerror(errno));
        return -1;
    }

    if (fcntl(fifo_fd, F_SETPIPE_SZ, LOG_FIFO_SIZE) == -1) {
        COMMAND_ERROR("Set fifo buffer size failed: %s", strerror(errno));
        close(fifo_fd);
        return -1;
    }

    return fifo_fd;
}

/* init log driver */
static int init_log_driver(const struct engine_log_config *log)
{
    int i, driver;

    for (i = ENGINE_LOG_FATAL; i < ENGINE_LOG_MAX; i++) {
        if (strcasecmp(g_engine_log_prio_name[i], log->priority) == 0) {
            g_engine_log_level = i;
            break;
        }
    }

    if (i == ENGINE_LOG_MAX) {
        COMMAND_ERROR("Unable to parse logging level:%s", log->priority);
        return -1;
    }

    driver = engine_change_str_logdriver_to_enum(log->driver);
    if (driver < 0) {
        COMMAND_ERROR("Invalid log driver: %s", log->driver);
        return -1;
    }
    g_engine_log_driver = driver;
    return 0;
}

static inline bool check_log_config_args(const struct engine_log_config *log)
{
    return (log == NULL || log->name == NULL || log->priority == NULL);
}

static int do_check_log_configs(const struct engine_log_config *log)
{
    bool invalid_arg = false;

    if (check_log_config_args(log)) {
        COMMAND_ERROR("Invalid arguments");
        return -1;
    }

    invalid_arg = ((log->file == NULL) && (g_engine_log_driver == LOG_DRIVER_FIFO));
    if (invalid_arg) {
        COMMAND_ERROR("Must set log file for driver %s", log->driver);
        return -1;
    }
    return 0;
}

/* log enable */
int engine_log_enable(const struct engine_log_config *log)
{
    int nret = 0;
    char *full_path = NULL;

    if (g_engine_log_fd != -1) {
        COMMAND_ERROR("engine log already initialized");
        return 0;
    }

    nret = do_check_log_configs(log);
    if (nret != 0) {
        return -1;
    }

    nret = init_log_driver(log);
    if (nret != 0) {
        return -1;
    }

    free(g_engine_log_module);
    g_engine_log_module = lcr_util_strdup_s(log->name);

    full_path = lcr_util_strdup_s(log->file);

    nret = lcr_util_build_dir(full_path);
    if (nret != 0) {
        COMMAND_ERROR("failed to create dir for log file");
        goto out;
    }

    g_engine_log_fd = open_fifo(full_path);
    if (g_engine_log_fd == -1) {
        nret = -1;
    }

out:
    if (nret != 0) {
        if (g_engine_log_driver == LOG_DRIVER_FIFO) {
            g_engine_log_driver = LOG_DRIVER_NOSET;
        }
    }
    free(full_path);

    return nret;
}

static int do_log_by_driver(const struct engine_log_object_metadata *metadata, const char *msg,
                            const char *date_time)
{
    switch (g_engine_log_driver) {
        case LOG_DRIVER_STDOUT:
            do_stderr_log(metadata, date_time, msg);
            break;
        case LOG_DRIVER_FIFO:
            if (g_engine_log_fd == -1) {
                COMMAND_ERROR("Do not set log file\n");
                return -1;
            }
            do_fifo_log(metadata, date_time, msg);
            break;
        case LOG_DRIVER_NOSET:
            break;
        default:
            COMMAND_ERROR("Invalid log driver\n");
            return -1;
    }
    return 0;
}

static char *parse_timespec_to_human()
{
    struct timespec timestamp;
    struct tm ptm = {0};
    char date_time[ENGINE_LOG_TIME_STR_MAX_LEN] = { 0 };
    int nret;
#define SEC_TO_NSEC 1000000
#define FIRST_YEAR_OF_GMT 1900

    if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1) {
        COMMAND_ERROR("Failed to get real time");
        return 0;
    }

    if (localtime_r(&(timestamp.tv_sec), &ptm) == NULL) {
        SYSERROR("Transfer timespec failed");
        return NULL;
    }

    nret = snprintf(date_time, ENGINE_LOG_TIME_STR_MAX_LEN, "%04d%02d%02d%02d%02d%02d.%03ld",
                    ptm.tm_year + FIRST_YEAR_OF_GMT, ptm.tm_mon + 1, ptm.tm_mday, ptm.tm_hour, ptm.tm_min, ptm.tm_sec,
                    timestamp.tv_nsec / SEC_TO_NSEC);

    if (nret < 0 || nret >= ENGINE_LOG_TIME_STR_MAX_LEN) {
        COMMAND_ERROR("Sprintf failed");
        return NULL;
    }

    return lcr_util_strdup_s(date_time);
}

/* use to add log to driver */
int engine_log(const struct engine_log_object_metadata *metadata, const char *format, ...)
{
    int rc;
    char msg[MAX_MSG_LENGTH] = { 0 };
    va_list args;
    char *date_time = NULL;
    int ret = 0;

    va_start(args, format);
    rc = vsnprintf(msg, MAX_MSG_LENGTH, format, args);
    va_end(args);
    if (rc < 0) {
        rc = snprintf(msg, MAX_MSG_LENGTH, "%s", "!!LONG LONG A LOG!!");
        if (rc < 0) {
            return 0;
        }
    }

    date_time = parse_timespec_to_human();
    if (date_time == NULL) {
        goto out;
    }

    ret = do_log_by_driver(metadata, msg, date_time);

out:
    free(date_time);
    return ret;
}

static void do_write_log_into_file(int log_fd, char *log_msg, size_t max_len, size_t write_size)
{
    size_t size = 0;

    size = write_size;
    if (size > (max_len - 1)) {
        size = max_len - 1;
    }

    log_msg[size] = '\n';

    if (write_nointr(log_fd, log_msg, (size + 1)) == -1) {
        COMMAND_ERROR("write log into logfile failed");
    }
}

/* log append logfile */
static void do_fifo_log(const struct engine_log_object_metadata *metadata, const char *timestamp,
                        const char *msg)
{
    char log_buffer[ENGINE_LOG_BUFFER_SIZE] = { 0 };
    int log_fd = -1;
    int nret;
    size_t size;
    char *tmp_prefix = NULL;

    if (metadata->level > g_engine_log_level) {
        return;
    }
    log_fd = g_engine_log_fd;
    if (log_fd == -1) {
        return;
    }

    tmp_prefix = g_engine_log_prefix ? g_engine_log_prefix : g_engine_log_module;
    if (tmp_prefix != NULL && strlen(tmp_prefix) > MAX_LOG_PREFIX_LENGTH) {
        tmp_prefix = tmp_prefix + (strlen(tmp_prefix) - MAX_LOG_PREFIX_LENGTH);
    }
    nret = snprintf(log_buffer, sizeof(log_buffer), "%15s %s %-8s %s:%s:%d - %s", tmp_prefix ? tmp_prefix : "",
                    timestamp, g_engine_log_prio_name[metadata->level], metadata->file,
                    metadata->func, metadata->line, msg);

    if (nret < 0) {
        nret = snprintf(log_buffer, sizeof(log_buffer), "%15s %s %-8s %s:%s:%d - %s",
                        tmp_prefix ? tmp_prefix : "", timestamp, g_engine_log_prio_name[metadata->level],
                        metadata->file, metadata->func, metadata->line, "Large log message");
        if (nret < 0) {
            return;
        }
    }
    size = (size_t)nret;

    do_write_log_into_file(log_fd, log_buffer, sizeof(log_buffer), size);
}

/* log append stderr */
static void do_stderr_log(const struct engine_log_object_metadata *metadata, const char *timestamp,
                          const char *msg)
{
    char *tmp_prefix = NULL;
    if (metadata->level > g_engine_log_level) {
        return;
    }

    tmp_prefix = g_engine_log_prefix ? g_engine_log_prefix : g_engine_log_module;
    if (tmp_prefix != NULL && strlen(tmp_prefix) > MAX_LOG_PREFIX_LENGTH) {
        tmp_prefix = tmp_prefix + (strlen(tmp_prefix) - MAX_LOG_PREFIX_LENGTH);
    }
    COMMAND_ERROR("%15s %s %-8s ", tmp_prefix ? tmp_prefix : "", timestamp, g_engine_log_prio_name[metadata->level]);
    COMMAND_ERROR("%s:%s:%d - ", metadata->file, metadata->func, metadata->line);
    COMMAND_ERROR("%s\n", msg);
}

/* write nointr */
static ssize_t write_nointr(int fd, const void *buf, size_t count)
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

