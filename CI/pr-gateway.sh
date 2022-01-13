#!/bin/bash
#######################################################################
##- @Copyright (C) Huawei Technologies., Ltd. 2022. All rights reserved.
# - iSulad licensed under the Mulan PSL v2.
# - You can use this software according to the terms and conditions of the Mulan PSL v2.
# - You may obtain a copy of Mulan PSL v2 at:
# -     http://license.coscl.org.cn/MulanPSL2
# - THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
# - IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
# - PURPOSE.
# - See the Mulan PSL v2 for more details.
##- @Description:provide gateway checker for pull request of lcr
##- @Author: haozi007
##- @Create: 2022-02-22
#######################################################################

dnf install -y gtest-devel gmock-devel gtest gmock diffutils cmake gcc-c++ yajl-devel patch make libtool git chrpath zlib-devel

cd ~

rm -rf lxc
git clone https://gitee.com/src-openeuler/lxc.git
pushd lxc
rm -rf lxc-4.0.3
./apply-patches || exit 1
pushd lxc-4.0.3
./autogen.sh && ./configure || exit 1
make -j $(nproc) || exit 1
make install
popd
popd

ldconfig
pushd lcr
rm -rf build
mkdir build
pushd build
cmake -DDEBUG=ON -DCMAKE_SKIP_RPATH=TRUE -DENABLE_UT=ON ../ || exit 1
make -j $(nproc) || exit 1
make install || exit 1
ctest -V || exit 1
popd
popd
