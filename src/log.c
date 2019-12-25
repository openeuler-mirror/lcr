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

/* predefined priorities. */
enum engine_log_priority {
    LOG_PRIORITY_FATAL = LOG_EMERG,
    LOG_PRIORITY_ALERT = LOG_ALERT,
    LOG_PRIORITY_CRIT = LOG_CRIT,
    LOG_PRIORITY_ERROR = LOG_ERR,
    LOG_PRIORITY_WARN = LOG_WARNING,
    LOG_PRIORITY_NOTICE = LOG_NOTICE,
    LOG_PRIORITY_INFO = LOG_INFO,
    LOG_PRIORITY_DEBUG = LOG_DEBUG,
    LOG_PRIORITY_TRACE,
    LOG_PRIORITY_MAX
};

#define MAX_MSG_LENGTH 4096
#define MAX_LOG_PREFIX_LENGTH 15

static __thread char *g_engine_log_prefix = NULL;

static char *g_engine_log_vmname = NULL;
static bool g_engine_log_quiet = false;
static int g_engine_log_level = LOG_PRIORITY_DEBUG;
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

void log_append_logfile(const struct engine_log_event *event, const char *timestamp, const char *msg);
void log_append_stderr(const struct engine_log_event *event, const char *timestamp, const char *msg);
int engine_unix_trans_to_utc(char *buf, size_t bufsize, const struct timespec *time);

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

    for (i = LOG_PRIORITY_FATAL; i < LOG_PRIORITY_MAX; i++) {
        if (!strcasecmp(g_engine_log_prio_name[i], log->priority)) {
            g_engine_log_level = i;
            break;
        }
    }

    if (i == LOG_PRIORITY_MAX) {
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

/* engine log append */
int engine_log_append(const struct engine_log_event *event, const char *format, ...)
{
    int rc;
    va_list args;
    char msg[MAX_MSG_LENGTH] = { 0 };
    char date_time[ENGINE_LOG_TIME_SIZE] = { 0 };
    struct timespec timestamp;

    va_start(args, format);
    rc = vsprintf_s(msg, MAX_MSG_LENGTH, format, args);
    va_end(args);
    if (rc < 0 || rc >= MAX_MSG_LENGTH) {
        rc = sprintf_s(msg, MAX_MSG_LENGTH, "%s", "!!LONG LONG A LOG!!");
        if (rc < 0) {
            return 0;
        }
    }

    if (clock_gettime(CLOCK_REALTIME, &timestamp) == -1) {
        fprintf(stderr, "Failed to get real time\n");
        return 0;
    }
    if (engine_unix_trans_to_utc(date_time, ENGINE_LOG_TIME_SIZE, &timestamp) < 0) {
        return 0;
    }

    switch (g_engine_log_driver) {
        case LOG_DRIVER_STDOUT:
            if (g_engine_log_quiet) {
                break;
            }
            log_append_stderr(event, date_time, msg);
            break;
        case LOG_DRIVER_FIFO:
            if (g_engine_log_fd == -1) {
                fprintf(stderr, "Do not set log file\n");
                return -1;
            }
            log_append_logfile(event, date_time, msg);
            break;
        case LOG_DRIVER_NOSET:
            break;
        default:
            fprintf(stderr, "Invalid log driver\n");
            return -1;
    }

    return 0;
}

/* log append logfile */
void log_append_logfile(const struct engine_log_event *event, const char *timestamp, const char *msg)
{
    char log_buffer[ENGINE_LOG_BUFFER_SIZE] = { 0 };
    int log_fd = -1;
    int nret;
    size_t size;
    char *tmp_prefix = NULL;

    if (event->priority > g_engine_log_level) {
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
                     timestamp, g_engine_log_prio_name[event->priority],
                     g_engine_log_vmname ? g_engine_log_vmname : "engine", event->locinfo->file, event->locinfo->func,
                     event->locinfo->line, msg);

    if (nret < 0) {
        nret = sprintf_s(log_buffer, sizeof(log_buffer), "%15s %s %-8s %s - %s:%s:%d - %s",
                         tmp_prefix ? tmp_prefix : "", timestamp, g_engine_log_prio_name[event->priority],
                         g_engine_log_vmname ? g_engine_log_vmname : "engine", event->locinfo->file,
                         event->locinfo->func, event->locinfo->line, "Large log message");
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
void log_append_stderr(const struct engine_log_event *event, const char *timestamp, const char *msg)
{
    char *tmp_prefix = NULL;
    if (event->priority > g_engine_log_level) {
        return;
    }

    tmp_prefix = g_engine_log_prefix ? g_engine_log_prefix : g_engine_log_vmname;
    if (tmp_prefix != NULL && strlen(tmp_prefix) > MAX_LOG_PREFIX_LENGTH) {
        tmp_prefix = tmp_prefix + (strlen(tmp_prefix) - MAX_LOG_PREFIX_LENGTH);
    }
    fprintf(stderr, "%15s %s %-8s ", tmp_prefix ? tmp_prefix : "", timestamp, g_engine_log_prio_name[event->priority]);
    fprintf(stderr, "%s - ", g_engine_log_vmname ? g_engine_log_vmname : "engine");
    fprintf(stderr, "%s:%s:%d - ", event->locinfo->file, event->locinfo->func, event->locinfo->line);
    fprintf(stderr, "%s", msg);
    fprintf(stderr, "\n");
}

/* engine unix trans to utc */
int engine_unix_trans_to_utc(char *buf, size_t bufsize, const struct timespec *time)
{
    int64_t trans_to_days, all_days, age, doa, yoa, doy, nom, hours_to_sec, trans_to_sec;
    int64_t real_year, real_day, real_month, real_hours, real_minutes, real_seconds;
    char ns[LCR_NUMSTRLEN64] = { 0 };
    int ret;

    /* Transtate seconds to number of days. */
    trans_to_days = time->tv_sec / 86400;

    /* Calculate days from 0000-03-01 to 1970-01-01.Days base it */
    all_days = trans_to_days + 719468;

    /* compute the age.One age means 400 years(146097 days) */
    age = (all_days >= 0 ? all_days : all_days - 146096) / 146097;

    /* The day-of-age (doa) can then be found by subtracting the  genumber */
    doa = (all_days - age * 146097);

    /* Calculate year-of-age (yoa, range [0, 399]) */
    yoa = ((doa - doa / 1460) + (doa / 36524 - doa / 146096)) / 365;

    /* Compute the year this moment */
    real_year = yoa + age * 400;

    /* Calculate the day-of-year */
    doy = doa - (365 * yoa + yoa / 4 - yoa / 100);

    /* Compute the month number. */
    nom = (5 * doy + 2) / 153;

    /* Compute the real_day. */
    real_day = (doy - (153 * nom + 2) / 5) + 1;

    /* Compute the correct month. */
    real_month = nom + (nom < 10 ? 3 : -9);

    /* Add one year before March */
    if (real_month < 3) {
        real_year++;
    }

    /* Translate days in the age to seconds. */
    trans_to_sec = trans_to_days * 86400;

    /* Compute the real_hours */
    real_hours = (time->tv_sec - trans_to_sec) / 3600;

    /* Translate the real hours to seconds. */
    hours_to_sec = real_hours * 3600;

    /* Calculate the real minutes */
    real_minutes = ((time->tv_sec - trans_to_sec) - hours_to_sec) / 60;

    /* Calculate the real seconds */
    real_seconds = (((time->tv_sec - trans_to_sec) - hours_to_sec) - (real_minutes * 60));

    ret = sprintf_s(ns, LCR_NUMSTRLEN64, "%ld", time->tv_nsec);
    if (ret < 0 || ret >= LCR_NUMSTRLEN64) {
        return -1;
    }

    /* Create the final timestamp */
    ret = sprintf_s(buf, bufsize, "%" PRId64 "%02" PRId64 "%02" PRId64 "%02" PRId64 "%02" PRId64 "%02" PRId64 ".%.3s",
                    real_year, real_month, real_day, real_hours, real_minutes, real_seconds, ns);
    if (ret < 0 || (size_t)ret >= bufsize) {
        return -1;
    }

    return 0;
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
