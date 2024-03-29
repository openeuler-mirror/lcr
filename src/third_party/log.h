/*
 * lxc: linux Container library
 *
 * (C) Copyright IBM Corp. 2007, 2008
 *
 * Authors:
 * Daniel Lezcano <daniel.lezcano at free.fr>
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
#ifndef __LXC_LOG_H
#define __LXC_LOG_H

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC 02000000
#endif

#ifndef F_DUPFD_CLOEXEC
#define F_DUPFD_CLOEXEC 1030
#endif

#define LXC_LOG_PREFIX_SIZE	32
#define LXC_LOG_BUFFER_SIZE	4096

/* This attribute is required to silence clang warnings */
#if defined(__GNUC__)
#define ATTR_UNUSED __attribute__ ((unused))
#else
#define ATTR_UNUSED
#endif

/* predefined lxc log priorities. */
enum lxc_loglevel {
	LXC_LOG_LEVEL_TRACE,
	LXC_LOG_LEVEL_DEBUG,
	LXC_LOG_LEVEL_INFO,
	LXC_LOG_LEVEL_NOTICE,
	LXC_LOG_LEVEL_WARN,
	LXC_LOG_LEVEL_ERROR,
	LXC_LOG_LEVEL_CRIT,
	LXC_LOG_LEVEL_ALERT,
	LXC_LOG_LEVEL_FATAL,
	LXC_LOG_LEVEL_NOTSET,
};

/* location information of the logging event */
struct lxc_log_locinfo {
	const char *file;
	const char *func;
	int line;
};

#define LXC_LOG_LOCINFO_INIT						\
	{ .file = __FILE__, .func = __func__, .line = __LINE__	}

/* brief logging event object */
struct lxc_log_event {
	const char *category;
	int priority;
	struct timespec timestamp;
	struct lxc_log_locinfo *locinfo;
	const char *fmt;
	va_list *vap;
};

/* log appender object */
struct lxc_log_appender {
	const char *name;
	int (*append)(const struct lxc_log_appender *, struct lxc_log_event *);

	/*
	 * appenders can be stacked
	 */
	struct lxc_log_appender *next;
};

/* log category object */
struct lxc_log_category {
	const char *name;
	int priority;
	struct lxc_log_appender *appender;
	const struct lxc_log_category *parent;
};

/*
 * Returns true if the chained priority is equal to or higher than
 * given priority.
 */
static inline bool lxc_log_priority_is_enabled(const struct lxc_log_category *category,
					      int priority)
{
	while (category->priority == LXC_LOG_LEVEL_NOTSET && category->parent)
		category = category->parent;

    if (priority >= category->priority) {
        return true;
    }

    return false;
}

/*
 * converts a priority to a literal string
 */
static inline const char *lxc_log_priority_to_string(int priority)
{
	switch (priority) {
	case LXC_LOG_LEVEL_TRACE:
		return "TRACE";
	case LXC_LOG_LEVEL_DEBUG:
		return "DEBUG";
	case LXC_LOG_LEVEL_INFO:
		return "INFO";
	case LXC_LOG_LEVEL_NOTICE:
		return "NOTICE";
	case LXC_LOG_LEVEL_WARN:
		return "WARN";
	case LXC_LOG_LEVEL_ERROR:
		return "ERROR";
	case LXC_LOG_LEVEL_CRIT:
		return "CRIT";
	case LXC_LOG_LEVEL_ALERT:
		return "ALERT";
	case LXC_LOG_LEVEL_FATAL:
		return "FATAL";
	}

	return "NOTSET";
}

/*
 * converts a literal priority to an int
 */
static inline int lxc_log_priority_to_int(const char *name)
{
	if (strcasecmp("TRACE", name) == 0)
		return LXC_LOG_LEVEL_TRACE;
	if (strcasecmp("DEBUG", name) == 0)
		return LXC_LOG_LEVEL_DEBUG;
	if (strcasecmp("INFO", name) == 0)
		return LXC_LOG_LEVEL_INFO;
	if (strcasecmp("NOTICE", name) == 0)
		return LXC_LOG_LEVEL_NOTICE;
	if (strcasecmp("WARN", name) == 0)
		return LXC_LOG_LEVEL_WARN;
	if (strcasecmp("ERROR", name) == 0)
		return LXC_LOG_LEVEL_ERROR;
	if (strcasecmp("CRIT", name) == 0)
		return LXC_LOG_LEVEL_CRIT;
	if (strcasecmp("ALERT", name) == 0)
		return LXC_LOG_LEVEL_ALERT;
	if (strcasecmp("FATAL", name) == 0)
		return LXC_LOG_LEVEL_FATAL;

	return LXC_LOG_LEVEL_NOTSET;
}

static inline void __lxc_log_append(const struct lxc_log_appender *appender,
				    struct lxc_log_event *event)
{
	va_list va;
	va_list *va_keep = event->vap;

	while (appender) {
		va_copy(va, *va_keep);
		event->vap = &va;
		appender->append(appender, event);
		appender = appender->next;
		va_end(va);
	}
}

static inline void __lxc_log(const struct lxc_log_category *category,
			     struct lxc_log_event *event)
{
	while (category) {
		__lxc_log_append(category->appender, event);
		category = category->parent;
	}
}

/*
 * Helper macro to define log functions.
 */
#define lxc_log_priority_define(acategory, LEVEL)				\
										\
ATTR_UNUSED __attribute__ ((format (printf, 2, 3)))				\
static inline void LXC_##LEVEL(struct lxc_log_locinfo *, const char *, ...);	\
										\
ATTR_UNUSED static inline void LXC_##LEVEL(struct lxc_log_locinfo* locinfo,	\
					   const char* format, ...)		\
{										\
	if (lxc_log_priority_is_enabled(acategory, LXC_LOG_LEVEL_##LEVEL)) {	\
		va_list va_ref;							\
		int saved_errno;						\
		struct lxc_log_event evt = {					\
			.category	= (acategory)->name,			\
			.priority	= LXC_LOG_LEVEL_##LEVEL		\
		};								\
		evt.locinfo	= locinfo;		\
		evt.fmt		= format;				\
										\
		/* clock_gettime() is explicitly marked as MT-Safe		\
		 * without restrictions. So let's use it for our		\
		 * logging stamps.						\
		 */								\
		saved_errno = errno;						\
		(void)clock_gettime(CLOCK_REALTIME, &evt.timestamp);		\
										\
		va_start(va_ref, format);					\
		evt.vap = &va_ref;						\
		__lxc_log(acategory, &evt);					\
		va_end(va_ref);							\
		errno = saved_errno;						\
	}									\
}

/*
 * Helper macro to define and use static categories.
 */
/*
#define lxc_log_category_define(name, parent)				\
	extern struct lxc_log_category lxc_log_category_##parent;	\
	struct lxc_log_category lxc_log_category_##name = {		\
		#name,							\
		LXC_LOG_LEVEL_NOTSET,					\
		NULL,							\
		&lxc_log_category_##parent				\
	};

#define lxc_log_define(name, parent)					\
	lxc_log_category_define(name, parent)				\
									\
	lxc_log_priority_define(&lxc_log_category_##name, TRACE)	\
	lxc_log_priority_define(&lxc_log_category_##name, DEBUG)	\
	lxc_log_priority_define(&lxc_log_category_##name, INFO)		\
	lxc_log_priority_define(&lxc_log_category_##name, NOTICE)	\
	lxc_log_priority_define(&lxc_log_category_##name, WARN)		\
	lxc_log_priority_define(&lxc_log_category_##name, ERROR)	\
	lxc_log_priority_define(&lxc_log_category_##name, CRIT)		\
	lxc_log_priority_define(&lxc_log_category_##name, ALERT)	\
	lxc_log_priority_define(&lxc_log_category_##name, FATAL)
*/
extern struct lxc_log_category g_lxc_log_category_lxc;
lxc_log_priority_define(&g_lxc_log_category_lxc, TRACE);
lxc_log_priority_define(&g_lxc_log_category_lxc, DEBUG);
lxc_log_priority_define(&g_lxc_log_category_lxc, INFO);
lxc_log_priority_define(&g_lxc_log_category_lxc, NOTICE);
lxc_log_priority_define(&g_lxc_log_category_lxc, WARN);
lxc_log_priority_define(&g_lxc_log_category_lxc, ERROR);
lxc_log_priority_define(&g_lxc_log_category_lxc, CRIT);
lxc_log_priority_define(&g_lxc_log_category_lxc, ALERT);
lxc_log_priority_define(&g_lxc_log_category_lxc, FATAL);

#define lxc_log_category_priority(name) 				\
	(lxc_log_priority_to_string(lxc_log_category_##name.priority))

/*
 * Helper macro to define errno string.
 */
#if HAVE_STRERROR_R
	#ifndef HAVE_DECL_STRERROR_R
		#ifdef STRERROR_R_CHAR_P
			char *strerror_r(int errnum, char *buf, size_t buflen);
		#else
			int strerror_r(int errnum, char *buf, size_t buflen);
		#endif
	#endif

	#ifdef STRERROR_R_CHAR_P
		#define lxc_log_strerror_r                                               \
			char errno_buf[PATH_MAX / 2] = {"Failed to get errno string"};   \
			char *ptr = NULL;                                                \
			{                                                                \
				int saved_errno = errno;				 \
				ptr = strerror_r(errno, errno_buf, sizeof(errno_buf));   \
				errno = saved_errno;					 \
				if (!ptr)                                                \
					ptr = errno_buf;                                 \
			}
	#else
		#define lxc_log_strerror_r                                               \
			char errno_buf[PATH_MAX / 2] = {"Failed to get errno string"};   \
			char *ptr = errno_buf;                                           \
			{                                                                \
				int saved_errno = errno;				 \
				(void)strerror_r(errno, errno_buf, sizeof(errno_buf));   \
				errno = saved_errno;					 \
			}
	#endif
#elif ENFORCE_THREAD_SAFETY
	#error ENFORCE_THREAD_SAFETY was set but cannot be guaranteed
#else
	#define lxc_log_strerror_r							 \
		char *ptr = NULL;              						 \
		{                              						 \
			ptr = strerror(errno); 						 \
		}
#endif

/*
 * top categories
 */
#define TRACE(format, ...) do {						\
	struct lxc_log_locinfo locinfo = LXC_LOG_LOCINFO_INIT;		\
	LXC_TRACE(&locinfo, format, ##__VA_ARGS__);			\
} while (0)

#define DEBUG(format, ...) do {						\
	struct lxc_log_locinfo locinfo = LXC_LOG_LOCINFO_INIT;		\
	LXC_DEBUG(&locinfo, format, ##__VA_ARGS__);			\
} while (0)

#define INFO(format, ...) do {						\
	struct lxc_log_locinfo locinfo = LXC_LOG_LOCINFO_INIT;		\
	LXC_INFO(&locinfo, format, ##__VA_ARGS__);			\
} while (0)

#define NOTICE(format, ...) do {					\
	struct lxc_log_locinfo locinfo = LXC_LOG_LOCINFO_INIT;		\
	LXC_NOTICE(&locinfo, format, ##__VA_ARGS__);			\
} while (0)

#define WARN(format, ...) do {						\
	struct lxc_log_locinfo locinfo = LXC_LOG_LOCINFO_INIT;		\
	LXC_WARN(&locinfo, format, ##__VA_ARGS__);			\
} while (0)

#define ERROR(format, ...) do {						\
	struct lxc_log_locinfo locinfo = LXC_LOG_LOCINFO_INIT;		\
	LXC_ERROR(&locinfo, format, ##__VA_ARGS__);			\
} while (0)

#define EVENT(format, ...) do {                       \
    struct lxc_log_locinfo locinfo = {                \
        .file = NULL, .func = NULL, .line = 0,        \
    };                                                \
	LXC_ERROR(&locinfo, format, ##__VA_ARGS__);		  \
} while (0)

#define CRIT(format, ...) do {						\
	struct lxc_log_locinfo locinfo = LXC_LOG_LOCINFO_INIT;		\
	LXC_CRIT(&locinfo, format, ##__VA_ARGS__);			\
} while (0)

#define ALERT(format, ...) do {						\
	struct lxc_log_locinfo locinfo = LXC_LOG_LOCINFO_INIT;		\
	LXC_ALERT(&locinfo, format, ##__VA_ARGS__);			\
} while (0)

#define FATAL(format, ...) do {						\
	struct lxc_log_locinfo locinfo = LXC_LOG_LOCINFO_INIT;		\
	LXC_FATAL(&locinfo, format, ##__VA_ARGS__);			\
} while (0)

#define SYSTRACE(format, ...)                              \
	do {                                               \
		lxc_log_strerror_r;                        \
		TRACE("%s - " format, ptr, ##__VA_ARGS__); \
	} while (0)

#define SYSDEBUG(format, ...)                              \
	do {                                               \
		lxc_log_strerror_r;                        \
		DEBUG("%s - " format, ptr, ##__VA_ARGS__); \
	} while (0)

#define SYSINFO(format, ...)                              \
	do {                                              \
		lxc_log_strerror_r;                       \
		INFO("%s - " format, ptr, ##__VA_ARGS__); \
	} while (0)

#define SYSNOTICE(format, ...)                              \
	do {                                                \
		lxc_log_strerror_r;                         \
		NOTICE("%s - " format, ptr, ##__VA_ARGS__); \
	} while (0)

#define SYSWARN(format, ...)                              \
	do {                                              \
		lxc_log_strerror_r;                       \
		WARN("%s - " format, ptr, ##__VA_ARGS__); \
	} while (0)

#define SYSERROR(format, ...)                              \
	do {                                               \
		lxc_log_strerror_r;                        \
		ERROR("%s - " format, ptr, ##__VA_ARGS__); \
	} while (0)

#define CMD_SYSERROR(format, ...)                                    \
	do {                                                         \
		lxc_log_strerror_r;                                  \
		fprintf(stderr, "%s - " format "\n", ptr, ##__VA_ARGS__); \
	} while (0)

#define CMD_SYSINFO(format, ...)                            \
	do {                                                \
		lxc_log_strerror_r;                         \
		printf("%s - " format "\n", ptr, ##__VA_ARGS__); \
	} while (0)

#define COMMAND_ERROR(fmt, args...)              \
    do {                                         \
        (void)fprintf(stderr, fmt "\n", ##args); \
    } while (0)


#define ISULA_LOG_DRIVER_STDOUT "stdout"
#define ISULA_LOG_DRIVER_FIFO "fifo"

struct isula_libutils_log_config {
    const char *name;
    const char *file;
    const char *priority;
    const char *prefix;
    const char *driver;
    bool quiet;
};

void isula_libutils_default_log_config(const char *name, struct isula_libutils_log_config *log);
int isula_libutils_log_enable(const struct isula_libutils_log_config *log);
void isula_libutils_set_log_prefix(const char *prefix);
void isula_libutils_free_log_prefix(void);
void isula_libutils_log_disable();

int isula_libutils_get_log_fd(void);

#define CALL_CHECK_TIMEOUT(timeout, caller)                            \
    do {                                                               \
        struct timespec tbeg, tend;                                    \
        (void)clock_gettime(CLOCK_REALTIME, &tbeg);                    \
        caller;                                                        \
        (void)clock_gettime(CLOCK_REALTIME, &tend);                    \
        if (tend.tv_sec - tbeg.tv_sec >= (timeout)) {                  \
            ERROR("Call: "#caller" use %d sec, timeout!!", timeout);   \
        }                                                              \
    } while (0)

#ifdef __cplusplus
}
#endif
#endif
