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
#ifndef __ENGINE_LOG_H
#define __ENGINE_LOG_H

#include <stdbool.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC 02000000
#endif

#define ENGINE_LOG_BUFFER_SIZE 4096

#define ENGINE_LOG_TIME_STR_MAX_LEN 21

enum engine_log_drivers {
    LOG_DRIVER_STDOUT,
    LOG_DRIVER_FIFO,
    LOG_DRIVER_NOSET,
};

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

struct engine_log_config {
    const char *name;
    const char *file;
    const char *priority;
    const char *prefix;
    const char *driver;
};

/* brief logging event object */
struct engine_log_object_metadata {
    /* location information of the logging item */
    const char *file;
    const char *func;
    int line;

    int level;
};

int engine_log_enable(const struct engine_log_config *log);

void engine_set_log_prefix(const char *prefix);

void engine_free_log_prefix(void);

int engine_log(const struct engine_log_object_metadata *metadata, const char *format, ...);

#define COMMON_LOG(loglevel, format, ...)                                               \
    do {                                                                                \
        struct engine_log_object_metadata meta = {                                     \
            .file = __FILENAME__, .func = __func__, .line = __LINE__, .level = loglevel,    \
        };                                                                              \
        (void)engine_log(&meta, format, ##__VA_ARGS__);                         \
    } while (0)

#define DEBUG(format, ...)                                            \
    COMMON_LOG(ENGINE_LOG_DEBUG, format, ##__VA_ARGS__)

#define INFO(format, ...)                                             \
    COMMON_LOG(ENGINE_LOG_INFO, format, ##__VA_ARGS__)

#define NOTICE(format, ...)                                           \
    COMMON_LOG(ENGINE_LOG_NOTICE, format, ##__VA_ARGS__)

#define WARN(format, ...)                                             \
    COMMON_LOG(ENGINE_LOG_WARN, format, ##__VA_ARGS__)

#define ERROR(format, ...)                                            \
    COMMON_LOG(ENGINE_LOG_ERROR, format, ##__VA_ARGS__)

#define CRIT(format, ...)                                             \
    COMMON_LOG(ENGINE_LOG_CRIT, format, ##__VA_ARGS__)

#define ALERT(format, ...)                                            \
    COMMON_LOG(ENGINE_LOG_ALERT, format, ##__VA_ARGS__)

#define FATAL(format, ...)                                            \
    COMMON_LOG(ENGINE_LOG_FATAL, format, ##__VA_ARGS__)

#define SYSERROR(format, ...)                                  \
    do {                                                       \
        ERROR("%s - " format, strerror(errno), ##__VA_ARGS__); \
    } while (0)

#define COMMAND_ERROR(fmt, args...)              \
    do {                                         \
        (void)fprintf(stderr, fmt "\n", ##args); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __ENGINE_LOG_H */
