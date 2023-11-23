#!/bin/bash
#######################################################################
##- @Copyright (C) Huawei Technologies., Ltd. 2021. All rights reserved.
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
##- @Create: 2021-12-20
#######################################################################

sed -i "s#http://repo.openeuler.org#https://repo.huaweicloud.com/openeuler#g" /etc/yum.repos.d/openEuler.repo

dnf update -y

dnf install -y python3-pip docbook2X doxygen gtest-devel gmock-devel diffutils cmake gcc-c++ yajl-devel patch make libtool libevent-devel libevhtp-devel grpc grpc-plugins grpc-devel protobuf-devel libcurl libcurl-devel sqlite-devel libarchive-devel device-mapper-devel http-parser-devel libseccomp-devel libcap-devel libselinux-devel libwebsockets libwebsockets-devel systemd-devel git chrpath ncurses-devel
if [ $? -ne 0 ]; then
    echo "install dependences failed"
    exit 1
fi

pip3 install meson ninja

cd ~

rm -rf lxc
git clone https://gitee.com/src-openeuler/lxc.git
pushd lxc
rm -rf lxc-5.0.2
./apply-patches || exit 1
pushd lxc-5.0.2
mkdir -p build
meson setup -Dtests=true -Dprefix=/usr build || exit 1
meson install -C build || exit 1
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
popd
popd

# build iSulad with grpc
ldconfig
rm -rf iSulad
git clone https://gitee.com/openeuler/iSulad.git
pushd iSulad
mkdir -p build
pushd build
cmake -DDEBUG=ON -DCMAKE_INSTALL_PREFIX=/usr -DENABLE_UT=ON -DENABLE_SHIM_V2=OFF ../ || exit 1
make -j $(nproc) || exit 1
popd
popd

# build iSulad with restful
ldconfig
pushd iSulad
rm -rf build
mkdir build
pushd build
cmake -DDEBUG=ON -DCMAKE_INSTALL_PREFIX=/usr -DEANBLE_IMAGE_LIBARAY=OFF -DENABLE_SHIM_V2=OFF -DENABLE_GRPC=OFF  ../ || exit 1
make -j $(nproc) || exit 1
popd
popd

pushd lcr
pushd build
ctest -V || exit 1
popd
popd
