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

#include <stdbool.h>
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

/*errmsg that defined in lcrc & lcrd*/
#define DAEMON_ERROR_GRPC_INIT_STR "Init failed"
#define DAEMON_ERROR_GRPC_CONNENCT_STR "Can not connect with server.Is the docker dameon running on the host?"
#define DAEMON_ERROR_GRPC_SERVER_STR "Server internal error"

enum engine_log_level {
    ENGINE_LOG_FATAL = 0,
    ENGINE_LOG_ALERT,
    ENGINE_LOG_CRIT,
    ENGINE_LOG_ERROR,
    ENGINE_LOG_WARN,
    ENGINE_LOG_NOTICE,
    ENGINE_LOG_INFO,
    ENGINE_LOG_DEBUG,
    ENGINE_LOG_TRACE,
    ENGINE_LOG_MAX
};

enum g_engine_log_driver {
    LOG_DRIVER_STDOUT,
    LOG_DRIVER_FIFO,
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

#define ENGINE_LOG_LOCINFO_INIT                               \
    {                                                         \
        .file = __FILE__, .func = __func__, .line = __LINE__, \
    }

/* brief logging object metadata */
struct engine_log_object_metadata {
    int level;

    /* location information of the logging item */
    const char *file;
    const char *func;
    int line;
};

extern void engine_close_log_file();
int log_enable(const struct engine_log_config *log);

void engine_set_log_prefix(const char *prefix);

void engine_free_log_prefix();

int engine_change_str_logdriver_to_enum(const char *driver);

int engine_log_append(const struct engine_log_object_metadata *event, const char *format, ...);

#define COMMON_LOG(loglevel, format, ...)                                      \
    do {                                                                    \
        struct engine_log_object_metadata meta = ENGINE_LOG_LOCINFO_INIT;   \
        meta.level = loglevel;                                          \
        (void)engine_log_append(&meta, format, ##__VA_ARGS__);              \
    } while (0)

#define DEBUG(format, ...)                                                  \
    COMMON_LOG(ENGINE_LOG_FATAL, format, ##__VA_ARGS__);

#define INFO(format, ...)                                                   \
    COMMON_LOG(ENGINE_LOG_INFO, format, ##__VA_ARGS__);

#define NOTICE(format, ...)                                                 \
    COMMON_LOG(ENGINE_LOG_NOTICE, format, ##__VA_ARGS__);

#define WARN(format, ...)                                                   \
    COMMON_LOG(ENGINE_LOG_WARN, format, ##__VA_ARGS__);

#define ERROR(format, ...)                                                  \
    COMMON_LOG(ENGINE_LOG_ERROR, format, ##__VA_ARGS__);

#define CRIT(format, ...)                                                   \
    COMMON_LOG(ENGINE_LOG_CRIT, format, ##__VA_ARGS__);

#define ALERT(format, ...)                                                  \
    COMMON_LOG(ENGINE_LOG_ALERT, format, ##__VA_ARGS__);

#define FATAL(format, ...)                                                  \
    COMMON_LOG(ENGINE_LOG_FATAL, format, ##__VA_ARGS__);

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
