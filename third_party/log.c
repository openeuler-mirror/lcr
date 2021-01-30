/*
 * lxc: linux Container library
 *
 * (C) Copyright IBM Corp. 2007, 2008
 *
 * Authors:
 * Cedric Le Goater <legoater@free.fr>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#define __STDC_FORMAT_MACROS /* Required for PRIu64 to work. */
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "utils.h"

#define STRARRAYLEN(x) (sizeof(x) - 1)

/* Calculate the number of chars needed to represent a given integer as a C
 * string. Include room for '-' to indicate negative numbers and the \0 byte.
 * This is based on systemd.
 */
#define INTTYPE_TO_STRLEN(type)                   \
	(2 + (sizeof(type) <= 1                   \
		  ? 3                             \
		  : sizeof(type) <= 2             \
			? 5                       \
			: sizeof(type) <= 4       \
			      ? 10                \
			      : sizeof(type) <= 8 \
				    ? 20          \
				    : sizeof(int[-2 * (sizeof(type) > 8)])))

/* We're logging in seconds and nanoseconds. Assuming that the underlying
 * datatype is currently at maximum a 64bit integer, we have a date string that
 * is of maximum length (2^64 - 1) * 2 = (21 + 21) = 42.
 */
#define LXC_LOG_TIME_SIZE ((INTTYPE_TO_STRLEN(uint64_t)) * 2)

#define MAX_LOG_PREFIX_LENGTH 16
static __thread char g_log_prefix[MAX_LOG_PREFIX_LENGTH] = "iSula";
static int g_lxc_log_fd = -1;
static char *log_fname = NULL;

int isula_libutils_get_log_fd(void)
{
    return g_lxc_log_fd;
}

void isula_libutils_default_log_config(const char *name, struct isula_libutils_log_config *log)
{
    log->name = name;
    log->file = NULL;
    log->priority = "DEBUG";
    log->quiet = true;
    log->driver = ISULA_LOG_DRIVER_STDOUT;
}

void isula_libutils_set_log_prefix(const char *prefix)
{
    if (prefix == NULL || strlen(prefix) == 0) {
        return;
    }

	(void)strncpy(g_log_prefix, prefix, MAX_LOG_PREFIX_LENGTH - 1);
}

void isula_libutils_free_log_prefix(void)
{
	(void)strncpy(g_log_prefix, "iSula", MAX_LOG_PREFIX_LENGTH - 1);
}

/*---------------------------------------------------------------------------*/
static int log_append_stderr(const struct lxc_log_appender *appender, struct lxc_log_event *event)
{
    if (event->locinfo->file == NULL) {
        return 0;
    }

	fprintf(stderr, "%15s %s: %s: %d ", g_log_prefix, event->locinfo->file,
	        event->locinfo->func, event->locinfo->line);
	vfprintf(stderr, event->fmt, *event->vap);
	fprintf(stderr, "\n");

	return 0;
}

/*---------------------------------------------------------------------------*/
static int lxc_unix_epoch_to_utc(char *buf, size_t bufsize, const struct timespec *time)
{
	int64_t epoch_to_days, z, era, doe, yoe, year, doy, mp, day, month,
	    d_in_s, hours, h_in_s, minutes, seconds;
	char nanosec[INTTYPE_TO_STRLEN(int64_t)];
	int ret;

	/* See https://howardhinnant.github.io/date_algorithms.html for an
	 * explanation of the algorithm used here.
	 */

	/* Convert Epoch in seconds to number of days. */
	epoch_to_days = time->tv_sec / 86400;

	/* Shift the Epoch from 1970-01-01 to 0000-03-01. */
	z = epoch_to_days + 719468;

	/* compute the era from the serial date by simply dividing by the number
	 * of days in an era (146097).
	 */
	era = (z >= 0 ? z : z - 146096) / 146097;

	/* The day-of-era (doe) can then be found by subtracting the era number
	 * times the number of days per era, from the serial date.
	 */
	doe = (z - era * 146097);

	/* From the day-of-era (doe), the year-of-era (yoe, range [0, 399]) can
	 * be computed.
	 */
	yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;

	/* Given year-of-era, and era, one can now compute the year. */
	year = yoe + era * 400;

	/* Also the day-of-year, again with the year beginning on Mar. 1, can be
	 * computed from the day-of-era and year-of-era.
	 */
	doy = doe - (365 * yoe + yoe / 4 - yoe / 100);

	/* Given day-of-year, find the month number. */
	mp = (5 * doy + 2) / 153;

	/* From day-of-year and month-of-year we can now easily compute
	 * day-of-month.
	 */
	day = doy - (153 * mp + 2) / 5 + 1;

	/* Transform the month number from the [0, 11] / [Mar, Feb] system to
	 * the civil system: [1, 12] to find the correct month.
	 */
	month = mp + (mp < 10 ? 3 : -9);

	/* The algorithm assumes that a year begins on 1 March, so add 1 before
	 * that. */
	if (month < 3)
		year++;

	/* Transform days in the epoch to seconds. */
	d_in_s = epoch_to_days * 86400;

	/* To find the current hour simply substract the Epoch_to_days from the
	 * total Epoch and divide by the number of seconds in an hour.
	 */
	hours = (time->tv_sec - d_in_s) / 3600;

	/* Transform hours to seconds. */
	h_in_s = hours * 3600;

	/* Calculate minutes by subtracting the seconds for all days in the
	 * epoch and for all hours in the epoch and divide by the number of
	 * minutes in an hour.
	 */
	minutes = (time->tv_sec - d_in_s - h_in_s) / 60;

	/* Calculate the seconds by subtracting the seconds for all days in the
	 * epoch, hours in the epoch and minutes in the epoch.
	 */
	seconds = (time->tv_sec - d_in_s - h_in_s - (minutes * 60));

	/* Make string from nanoseconds. */
	ret = snprintf(nanosec, sizeof(nanosec), "%"PRId64, (int64_t)time->tv_nsec);
	if (ret < 0 || ret >= sizeof(nanosec))
		return -1;

	/* Create final timestamp for the log and shorten nanoseconds to 3
	 * digit precision.
	 */
	ret = snprintf(buf, bufsize,
		       "%" PRId64 "%02" PRId64 "%02" PRId64 "%02" PRId64
		       "%02" PRId64 "%02" PRId64 ".%.3s",
		       year, month, day, hours, minutes, seconds, nanosec);
	if (ret < 0 || (size_t)ret >= bufsize)
		return -1;

	return 0;
}

/* This function needs to make extra sure that it is thread-safe. We had some
 * problems with that before. This especially involves time-conversion
 * functions. I don't want to find any localtime() or gmtime() functions or
 * relatives in here. Not even localtime_r() or gmtime_r() or relatives. They
 * all fiddle with global variables and locking in various libcs. They cause
 * deadlocks when liblxc is used multi-threaded and no matter how smart you
 * think you are, you __will__ cause trouble using them.
 * (As a short example how this can cause trouble: LXD uses forkstart to fork
 * off a new process that runs the container. At the same time the go runtime
 * LXD relies on does its own multi-threading thing which we can't control. The
 * fork()ing + threading then seems to mess with the locking states in these
 * time functions causing deadlocks.)
 * The current solution is to be good old unix people and use the Epoch as our
 * reference point and simply use the seconds and nanoseconds that have past
 * since then. This relies on clock_gettime() which is explicitly marked MT-Safe
 * with no restrictions! This way, anyone who is really strongly invested in
 * getting the actual time the log entry was created, can just convert it for
 * themselves. Our logging is mostly done for debugging purposes so don't try
 * to make it pretty. Pretty might cost you thread-safety.
 */
static int log_append_logfile(const struct lxc_log_appender *appender,
			      struct lxc_log_event *event)
{
	char buffer[LXC_LOG_BUFFER_SIZE];
	char date_time[LXC_LOG_TIME_SIZE];
	int n;
	ssize_t ret;
	int fd_to_use = g_lxc_log_fd;

	if (fd_to_use == -1)
		return 0;

	if (lxc_unix_epoch_to_utc(date_time, LXC_LOG_TIME_SIZE, &event->timestamp) < 0)
		return -1;

    if (event->locinfo->file != NULL) {
        n = snprintf(buffer, sizeof(buffer),
                 "%15s %s %-8s %s:%s:%d - ",
                 g_log_prefix,
                 date_time,
                 lxc_log_priority_to_string(event->priority),
                 event->locinfo->file, event->locinfo->func,
                 event->locinfo->line);
    } else {
        n = snprintf(buffer, sizeof(buffer), "%15s %s - ", g_log_prefix, date_time);
    }

	if (n < 0)
		return n;

	if ((size_t)n < STRARRAYLEN(buffer)) {
		ret = vsnprintf(buffer + n, sizeof(buffer) - n, event->fmt, *event->vap);
		if (ret < 0)
			return 0;

		n += ret;
	}

	if ((size_t)n >= sizeof(buffer))
		n = STRARRAYLEN(buffer);

	buffer[n] = '\n';

	return lcr_util_write_nointr(fd_to_use, buffer, n + 1);
}

static struct lxc_log_appender log_appender_stderr = {
	.name		= "stderr",
	.append		= log_append_stderr,
	.next		= NULL,
};

static struct lxc_log_appender log_appender_logfile = {
	.name		= "logfile",
	.append		= log_append_logfile,
	.next		= NULL,
};

static struct lxc_log_category log_root = {
	.name		= "root",
	.priority	= LXC_LOG_LEVEL_ERROR,
	.appender	= NULL,
	.parent		= NULL,
};

struct lxc_log_category g_lxc_log_category_lxc = {
	.name		= "lxc",
	.priority	= LXC_LOG_LEVEL_ERROR,
	.appender	= &log_appender_logfile,
	.parent		= &log_root
};

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

static void clean_pre_init()
{
    g_lxc_log_category_lxc.appender = &log_appender_stderr;
    g_lxc_log_category_lxc.priority = LXC_LOG_LEVEL_ERROR;
}

static bool init_log_file(const char *fname)
{
    if (fname == NULL) {
        return false;
    }
    if (strcmp(fname, "none") == 0) {
        return true;
    }
    if (lcr_util_build_dir(fname) != 0) {
        CMD_SYSERROR("build log path \"%s\" failed", fname);
        goto clean_out;
    }
    g_lxc_log_fd = open_fifo(fname);
    if (g_lxc_log_fd == -1) {
        CMD_SYSERROR("Open log fifo \"%s\" failed", fname);
        goto clean_out;
    }

    free(log_fname);
    log_fname = lcr_util_strdup_s(fname);
    return true;
clean_out:
    clean_pre_init();
    return false;
}

static bool choice_log_driver(const struct isula_libutils_log_config *log)
{
    bool is_fifo = false;

    // if driver is null, mean disable log
    if (log->driver == NULL) {
        g_lxc_log_category_lxc.priority = LXC_LOG_LEVEL_FATAL;
        return true;
    }
	g_lxc_log_category_lxc.appender = &log_appender_logfile;

    is_fifo = strcasecmp(log->driver, ISULA_LOG_DRIVER_FIFO) == 0;

    // if set file, only use log_append_logfile
    // we only support fifo driver with file
    if (is_fifo) {
        return init_log_file(log->file);
    }
    if (log->file != NULL) {
        clean_pre_init();
        return false;
    }

    if (strcasecmp(log->driver, ISULA_LOG_DRIVER_STDOUT) == 0) {
		g_lxc_log_category_lxc.appender = &log_appender_stderr;
    }
    return true;
}

int isula_libutils_log_enable(const struct isula_libutils_log_config *log)
{
	int lxc_priority = LXC_LOG_LEVEL_ERROR;

	if (log == NULL)
		return -1;

	if (g_lxc_log_fd != -1) {
		WARN("Log already initialized");
		return 0;
	}

    if (log->quiet) {
        g_lxc_log_category_lxc.priority = LXC_LOG_LEVEL_FATAL;
        return 0;
    }

    if (!choice_log_driver(log)) {
        COMMAND_ERROR("Invalid log config of driver");
        return -1;
    }

	if (log->priority != NULL) {
		lxc_priority = lxc_log_priority_to_int(log->priority);
    }
	g_lxc_log_category_lxc.priority = lxc_priority;

    isula_libutils_set_log_prefix(log->prefix != NULL ? log->prefix : log->name);

	return 0;
}

static inline void lxc_log_close(void)
{
	if (g_lxc_log_fd == -1)
		return;

	close(g_lxc_log_fd);
	g_lxc_log_fd = -1;

	free(log_fname);
	log_fname = NULL;
}

void isula_libutils_log_disable()
{
    lxc_log_close();
}
