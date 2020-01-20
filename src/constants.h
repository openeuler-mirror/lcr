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
 * Description: provide constant definition
 ******************************************************************************/

#ifndef _LCR_CONSTANTS_H
#define _LCR_CONSTANTS_H

/* mode of file and directory */

#define DEFAULT_SECURE_FILE_MODE 0640

#define DEFAULT_SECURE_DIRECTORY_MODE 0750

#define ROOTFS_MNT_DIRECTORY_MODE 0640

#define CONFIG_DIRECTORY_MODE 0750

#define CONFIG_FILE_MODE 0640

#define NETWORK_MOUNT_FILE_MODE 0644

#define ARCH_LOG_FILE_MODE 0440

#define WORKING_LOG_FILE_MODE 0640

#define LOG_DIRECTORY_MODE 0750

#define TEMP_DIRECTORY_MODE 0750

#define DEBUG_FILE_MODE 0640

#define DEBUG_DIRECTORY_MODE 0750

// Config file path
#define OCIHOOKSFILE "ocihooks.json"
#define OCICONFIGFILE "ociconfig.json"
#define LXCCONFIGFILE "config"

#define PARAM_NUM 50

#endif
