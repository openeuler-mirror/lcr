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
#ifndef __LCR_LOG_H
#define __LCR_LOG_H

#include <syslog.h>
#include <stdbool.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC 02000000
#endif

#ifndef F_DUPFD_CLOEXEC
#define F_DUPFD_CLOEXEC 1030
#endif

#define ENGINE_LOG_BUFFER_SIZE 4096

/* We're logging in seconds and nanoseconds. Assuming that the underlying
 * datatype is currently at maximum a 64bit integer, we have a date string that
 * is of maximum length (2^64 - 1) * 2 = (21 + 21) = 42.
 * */
#define ENGINE_LOG_TIME_SIZE 42

/* errmsg that defined in iSulad */
#define DAEMON_ERROR_GRPC_INIT_STR "Init failed"
#define DAEMON_ERROR_GRPC_CONNENCT_STR "Can not connect with server.Is the docker dameon running on the host?"
#define DAEMON_ERROR_GRPC_SERVER_STR "Server internal error"

enum g_engine_log_driver {
    LOG_DRIVER_STDOUT,
    LOG_DRIVER_FIFO,
    LOG_DRIVER_SYSLOG,
    LOG_DRIVER_NOSET,
};

struct engine_log_config {
    const char *name;
    const char *file;
    const char *priority;
    const char *prefix;
    const char *driver;
    bool quiet;
};

/* location information of the logging event */
struct engine_log_locinfo {
    const char *file;
    const char *func;
    int line;
};

#define ENGINE_LOG_LOCINFO_INIT                               \
    {                                                         \
        .file = __FILE__, .func = __func__, .line = __LINE__, \
    }

/* brief logging event object */
struct engine_log_event {
    int priority;
    struct engine_log_locinfo *locinfo;
};
extern void engine_close_log_file();
int log_enable(const struct engine_log_config *log);

void engine_set_log_prefix(const char *prefix);

void engine_free_log_prefix();

int engine_change_str_logdriver_to_enum(const char *driver);

int engine_log_append(const struct engine_log_event *event, const char *format, ...);

#define DEBUG(format, ...)                                           \
    do {                                                             \
        struct engine_log_locinfo locinfo = ENGINE_LOG_LOCINFO_INIT; \
        struct engine_log_event event;                               \
        event.locinfo = &locinfo;                                    \
        event.priority = LOG_DEBUG;                                  \
        (void)engine_log_append(&event, format, ##__VA_ARGS__);      \
    } while (0)

#define INFO(format, ...)                                            \
    do {                                                             \
        struct engine_log_locinfo locinfo = ENGINE_LOG_LOCINFO_INIT; \
        struct engine_log_event event;                               \
        event.locinfo = &locinfo;                                    \
        event.priority = LOG_INFO;                                   \
        (void)engine_log_append(&event, format, ##__VA_ARGS__);      \
    } while (0)

#define NOTICE(format, ...)                                          \
    do {                                                             \
        struct engine_log_locinfo locinfo = ENGINE_LOG_LOCINFO_INIT; \
        struct engine_log_event event;                               \
        event.locinfo = &locinfo;                                    \
        event.priority = LOG_NOTICE;                                 \
        (void)engine_log_append(&event, format, ##__VA_ARGS__);      \
    } while (0)

#define WARN(format, ...)                                            \
    do {                                                             \
        struct engine_log_locinfo locinfo = ENGINE_LOG_LOCINFO_INIT; \
        struct engine_log_event event;                               \
        event.locinfo = &locinfo;                                    \
        event.priority = LOG_WARNING;                                \
        (void)engine_log_append(&event, format, ##__VA_ARGS__);      \
    } while (0)

#define ERROR(format, ...)                                           \
    do {                                                             \
        struct engine_log_locinfo locinfo = ENGINE_LOG_LOCINFO_INIT; \
        struct engine_log_event event;                               \
        event.locinfo = &locinfo;                                    \
        event.priority = LOG_ERR;                                    \
        (void)engine_log_append(&event, format, ##__VA_ARGS__);      \
    } while (0)

#define CRIT(format, ...)                                            \
    do {                                                             \
        struct engine_log_locinfo locinfo = ENGINE_LOG_LOCINFO_INIT; \
        struct engine_log_event event;                               \
        event.locinfo = &locinfo;                                    \
        event.priority = LOG_CRIT;                                   \
        (void)engine_log_append(&event, format, ##__VA_ARGS__);      \
    } while (0)

#define ALERT(format, ...)                                           \
    do {                                                             \
        struct engine_log_locinfo locinfo = ENGINE_LOG_LOCINFO_INIT; \
        struct engine_log_event event;                               \
        event.locinfo = &locinfo;                                    \
        event.priority = LOG_ALERT;                                  \
        (void)engine_log_append(&event, format, ##__VA_ARGS__);      \
    } while (0)

#define FATAL(format, ...)                                           \
    do {                                                             \
        struct engine_log_locinfo locinfo = ENGINE_LOG_LOCINFO_INIT; \
        struct engine_log_event event;                               \
        event.locinfo = &locinfo;                                    \
        event.priority = LOG_EMERG;                                  \
        (void)engine_log_append(&event, format, ##__VA_ARGS__);      \
    } while (0)

#define SYSERROR(format, ...)                                  \
    do {                                                       \
        ERROR("%s - " format, strerror(errno), ##__VA_ARGS__); \
    } while (0)

#define COMMAND_ERROR(fmt, args...)                \
    do {                                           \
        (void)fprintf(stderr, fmt "\n", ##args);   \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __LCR_LOG_H */
