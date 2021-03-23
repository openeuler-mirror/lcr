# lcr: fuzz tests
#
# Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
#
# Authors:
# Haozi007 <liuhao27@huawei.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#

#!/bin/bash

LIB_FUZZING_ENGINE="/lib64/libFuzzer.a"
FUZZ_OPTION="-dict=./dict/log_fuzz.dict -runs=10000000 -max_total_time=3600"

if [ ! -f "$LIB_FUZZING_ENGINE" ];then
    echo "$LIB_FUZZING_ENGINE not exist, pls check"
    exit 1
fi

# compile fuzz testcase
make -j

# run fuzz testcases
./log_fuzz ${FUZZ_OPTION} -artifact_prefix=log_fuzz-

echo "########### Fuzz Result ##############"
crash=`find -name "*-crash-*"`
if [ x"${crash}" != x"" ];then
    echo "find bugs while fuzzing, pls check <*-crash-*> file"
    find -name "*-crash-*"
    exit 1
else
    echo "all fuzz success."
fi

