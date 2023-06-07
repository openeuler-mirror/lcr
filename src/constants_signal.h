/******************************************************************************
 * isula: signal utils
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
#ifndef _ISULA_UTILS_CONSTANTS_SIGNAL_H
#define _ISULA_UTILS_CONSTANTS_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

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


#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_CONSTANTS_SIGNAL_H */