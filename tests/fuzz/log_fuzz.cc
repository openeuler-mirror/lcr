/******************************************************************************
 * log_fuzz: testcase for log
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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "log.h"


extern "C" void testLog(struct isula_libutils_log_config *conf)
{
    (void)isula_libutils_log_enable(conf);
    INFO("info log");
    isula_libutils_set_log_prefix(conf->prefix);
    INFO("test prefix info");
    isula_libutils_log_disable();
}


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    const char *default_name = "iSula";
    struct isula_libutils_log_config tconf = {0};
    std::string testData(reinterpret_cast<const char *>(data), size);
    std::vector<std::string> ret_vec;
    std::string tmpstr;
    std::istringstream istr(testData);
    while (std::getline(istr, tmpstr, ',')) {
        ret_vec.push_back(tmpstr);
    }

    if (ret_vec.size() == 5) {
        if (ret_vec[0] != "") {
            tconf.name = ret_vec[0].c_str();
        }
        if (ret_vec[1] != "") {
            tconf.file = ret_vec[1].c_str();
        }
        if (ret_vec[2] != "") {
            tconf.priority = ret_vec[2].c_str();
        }
        if (ret_vec[3] != "") {
            tconf.prefix = ret_vec[3].c_str();
        }
        if (ret_vec[4] != "") {
            tconf.driver = ret_vec[4].c_str();
        }
    } else {
        isula_libutils_default_log_config(default_name, &tconf);
    }

    testLog(&tconf);
    return 0;
}

