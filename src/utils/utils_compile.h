/******************************************************************************
 * isula: compile utils
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
#ifndef _ISULA_UTILS_UTILS_COMPILE_H
#define _ISULA_UTILS_UTILS_COMPILE_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) && (__GNUC__ >= 4)
#ifndef __HIDDEN__
#define __HIDDEN__ __attribute__((visibility("hidden")))
#endif

#ifndef __EXPORT__
#define __EXPORT__ __attribute__((visibility("default")))
#endif

#else
#define __HIDDEN__
#ifndef __EXPORT__
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* _ISULA_UTILS_UTILS_COMPILE_H */