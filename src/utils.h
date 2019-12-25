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
 * Description: provide container utils functions
 ******************************************************************************/
#ifndef __LCR_UTILS_H
#define __LCR_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef O_CLOEXEC
#define O_CLOEXEC 02000000
#endif

#define ECOMMON 1
#define EINVALIDARGS 125
#define ECMDNOTFOUND 127

#define LCR_NUMSTRLEN64 21

#define SPACE_MAGIC_STR "[#)"

#define MAX_PATH_DEPTH 1024

#define SIZE_KB 1024LL
#define SIZE_MB (1024LL * SIZE_KB)
#define SIZE_GB (1024LL * SIZE_MB)
#define SIZE_TB (1024LL * SIZE_GB)
#define SIZE_PB (1024LL * SIZE_TB)

#define BUFSIZE 4096

#ifndef SIGTRAP
#define SIGTRAP 5
#endif

#ifndef SIGIOT
#define SIGIOT 6
#endif

#ifndef SIGEMT
#define SIGEMT 7
#endif

#ifndef SIGBUS
#define SIGBUS 7
#endif

#ifndef SIGSTKFLT
#define SIGSTKFLT 16
#endif

#ifndef SIGCLD
#define SIGCLD 17
#endif

#ifndef SIGURG
#define SIGURG 23
#endif

#ifndef SIGXCPU
#define SIGXCPU 24
#endif

#ifndef SIGXFSZ
#define SIGXFSZ 25
#endif

#ifndef SIGVTALRM
#define SIGVTALRM 26
#endif

#ifndef SIGPROF
#define SIGPROF 27
#endif

#ifndef SIGWINCH
#define SIGWINCH 28
#endif

#ifndef SIGIO
#define SIGIO 29
#endif

#ifndef SIGPOLL
#define SIGPOLL 29
#endif

#ifndef SIGINFO
#define SIGINFO 29
#endif

#ifndef SIGLOST
#define SIGLOST 37
#endif

#ifndef SIGPWR
#define SIGPWR 30
#endif

#ifndef SIGUNUSED
#define SIGUNUSED 31
#endif

#ifndef SIGSYS
#define SIGSYS 31
#endif

#ifndef SIGRTMIN1
#define SIGRTMIN1 34
#endif

#ifndef SIGRTMAX
#define SIGRTMAX 64
#endif

#define SIGNAL_MAP_DEFAULT                                                                                     \
    {                                                                                                          \
        { SIGHUP, "HUP" }, { SIGINT, "INT" }, { SIGQUIT, "QUIT" }, { SIGILL, "ILL" }, { SIGABRT, "ABRT" },     \
        { SIGFPE, "FPE" }, { SIGKILL, "KILL" }, { SIGSEGV, "SEGV" }, { SIGPIPE, "PIPE" }, { SIGALRM, "ALRM" }, \
        { SIGTERM, "TERM" }, { SIGUSR1, "USR1" }, { SIGUSR2, "USR2" }, { SIGCHLD, "CHLD" },                    \
        { SIGCONT, "CONT" }, { SIGSTOP, "STOP" }, { SIGTSTP, "TSTP" }, { SIGTTIN, "TTIN" },                    \
        { SIGTTOU, "TTOU" }, { SIGTRAP, "TRAP" }, { SIGIOT, "IOT" }, { SIGEMT, "EMT" }, { SIGBUS, "BUS" },     \
        { SIGSTKFLT, "STKFLT" }, { SIGCLD, "CLD" }, { SIGURG, "URG" }, { SIGXCPU, "XCPU" },                    \
        { SIGXFSZ, "XFSZ" }, { SIGVTALRM, "VTALRM" }, { SIGPROF, "PROF" }, { SIGWINCH, "WINCH" },              \
        { SIGIO, "IO" }, { SIGPOLL, "POLL" }, { SIGINFO, "INFO" }, { SIGLOST, "LOST" }, { SIGPWR, "PWR" },     \
        { SIGUNUSED, "UNUSED" }, { SIGSYS, "SYS" }, { SIGRTMIN, "RTMIN" }, { SIGRTMIN + 1, "RTMIN+1" },        \
        { SIGRTMIN + 2, "RTMIN+2" }, { SIGRTMIN + 3, "RTMIN+3" }, { SIGRTMIN + 4, "RTMIN+4" },                 \
        { SIGRTMIN + 5, "RTMIN+5" }, { SIGRTMIN + 6, "RTMIN+6" }, { SIGRTMIN + 7, "RTMIN+7" },                 \
        { SIGRTMIN + 8, "RTMIN+8" }, { SIGRTMIN + 9, "RTMIN+9" }, { SIGRTMIN + 10, "RTMIN+10" },               \
        { SIGRTMIN + 11, "RTMIN+11" }, { SIGRTMIN + 12, "RTMIN+12" }, { SIGRTMIN + 13, "RTMIN+13" },           \
        { SIGRTMIN + 14, "RTMIN+14" }, { SIGRTMIN + 15, "RTMIN+15" }, { SIGRTMAX - 14, "RTMAX-14" },           \
        { SIGRTMAX - 13, "RTMAX-13" }, { SIGRTMAX - 12, "RTMAX-12" }, { SIGRTMAX - 11, "RTMAX-11" },           \
        { SIGRTMAX - 10, "RTMAX-10" }, { SIGRTMAX - 9, "RTMAX-9" }, { SIGRTMAX - 8, "RTMAX-8" },               \
        { SIGRTMAX - 7, "RTMAX-7" }, { SIGRTMAX - 6, "RTMAX-6" }, { SIGRTMAX - 5, "RTMAX-5" },                 \
        { SIGRTMAX - 4, "RTMAX-4" }, { SIGRTMAX - 3, "RTMAX-3" }, { SIGRTMAX - 2, "RTMAX-2" },                 \
        { SIGRTMAX - 1, "RTMAX-1" }, { SIGRTMAX, "RTMAX" },                                                    \
    }

bool file_exists(const char *path);
bool dir_exists(const char *path);
int wait_for_pid(pid_t pid);
int wait_for_pid_status(pid_t pid);
char *util_string_join(const char *sep, const char **parts, size_t len);
int util_mkdir_p(const char *dir, mode_t mode);
char **lcr_string_split_and_trim(const char *str, char _sep);
void lcr_free_array(void **array);
int lcr_grow_array(void ***array, size_t *capacity, size_t new_size, size_t capacity_increment);
size_t lcr_array_len(void **array);
int mem_realloc(void **newptr, size_t newsize, void *oldptr, size_t oldsize);
bool util_valid_cmd_arg(const char *arg);
int util_safe_ullong(const char *numstr, unsigned long long *converted);
int util_safe_strtod(const char *numstr, double *converted);
int util_safe_uint(const char *numstr, unsigned int *converted);
int parse_byte_size_string(const char *s, int64_t *converted);
bool util_dir_exists(const char *path);
int util_ensure_path(char **confpath, const char *path);
int util_recursive_rmdir(const char *dirpath, int recursive_depth);
char *util_string_replace(const char *needle, const char *replacement, const char *haystack);
int util_open(const char *filename, int flags, mode_t mode);

FILE *util_fopen(const char *filename, const char *mode);

void *util_common_calloc_s(size_t size);
int util_safe_int(const char *numstr, int *converted);
int util_check_inherited(bool closeall, int fd_to_ignore);
char *util_string_append(const char *post, const char *pre);
char *util_string_split_prefix(size_t prefix_len, const char *file);

int util_build_dir(const char *name);
ssize_t util_write_nointr(int fd, const void *buf, size_t count);
ssize_t util_read_nointr(int fd, void *buf, size_t count);
void util_free_array(char **array);

int util_sig_parse(const char *signame);
bool util_valid_signal(int sig);
size_t util_array_len(char **array);
const char *str_skip_str(const char *str, const char *skip);
int util_array_append(char ***array, const char *element);
int util_safe_llong(const char *numstr, long long *converted);
char *util_strdup_s(const char *src);
int util_null_stdfds(void);

bool util_copy_file(const char *src_file, const char *dst_file, mode_t mode);

bool util_write_file(const char *filepath, const char *content, size_t len, bool add_newline, mode_t mode);

int util_atomic_write_file(const char *filepath, const char *content);

#ifdef __cplusplus
}
#endif

#endif /* __LCR_UTILS_H */
