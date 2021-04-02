/******************************************************************************
 * lcr: utils library for iSula
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
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

#ifndef __LCR_UTILS_H
#define __LCR_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CGROUP2_WEIGHT_MIN 1
#define CGROUP2_WEIGHT_MAX 10000
#define CGROUP2_BFQ_WEIGHT_MIN 1
#define CGROUP2_BFQ_WEIGHT_MAX 1000

#define DEFAULT_CPU_PERIOD 100000
#define CGROUP_MOUNTPOINT "/sys/fs/cgroup"

#ifndef CGROUP2_SUPER_MAGIC
#define CGROUP2_SUPER_MAGIC 0x63677270
#endif

#ifndef CGROUP_SUPER_MAGIC
#define CGROUP_SUPER_MAGIC 0x27e0eb
#endif

#define CGROUP_VERSION_1 1
#define CGROUP_VERSION_2 2

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

int lcr_wait_for_pid(pid_t pid);
int lcr_wait_for_pid_status(pid_t pid);
char *lcr_util_string_join(const char *sep, const char **parts, size_t len);
char **lcr_string_split_and_trim(const char *str, char _sep);
void lcr_free_array(void **array);
int lcr_grow_array(void ***array, size_t *capacity, size_t new_size, size_t capacity_increment);
size_t lcr_array_len(void **array);
int lcr_mem_realloc(void **newptr, size_t newsize, void *oldptr, size_t oldsize);
int lcr_util_safe_strtod(const char *numstr, double *converted);
int lcr_util_safe_uint(const char *numstr, unsigned int *converted);
int lcr_parse_byte_size_string(const char *s, int64_t *converted);
bool lcr_util_dir_exists(const char *path);
int lcr_util_ensure_path(char **confpath, const char *path);
int lcr_util_recursive_rmdir(const char *dirpath, int recursive_depth);
char *lcr_util_string_replace(const char *needle, const char *replacement, const char *haystack);
int lcr_util_open(const char *filename, int flags, mode_t mode);

void *lcr_util_common_calloc_s(size_t size);
int lcr_util_safe_int(const char *numstr, int *converted);
int lcr_util_check_inherited(bool closeall, int fd_to_ignore);
char *lcr_util_string_append(const char *post, const char *pre);
char *lcr_util_string_split_prefix(size_t prefix_len, const char *file);

int lcr_util_build_dir(const char *name);
ssize_t lcr_util_write_nointr(int fd, const void *buf, size_t count);
ssize_t lcr_util_read_nointr(int fd, void *buf, size_t count);

size_t lcr_util_array_len(char **array);

int lcr_util_safe_llong(const char *numstr, long long *converted);
char *lcr_util_strdup_s(const char *src);
int lcr_util_null_stdfds(void);

int lcr_util_atomic_write_file(const char *filepath, const char *content);

int lcr_util_get_real_swap(int64_t memory, int64_t memory_swap, int64_t *swap);
int lcr_util_trans_cpushare_to_cpuweight(int64_t cpu_share);
uint64_t lcr_util_trans_blkio_weight_to_io_weight(int weight);
uint64_t lcr_util_trans_blkio_weight_to_io_bfq_weight(int weight);
int lcr_util_get_cgroup_version();

#ifdef __cplusplus
}
#endif

#endif /* __LCR_UTILS_H */
