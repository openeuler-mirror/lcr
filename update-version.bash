#######################################################################
# lcr: utils library for iSula
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
#######################################################################
#!/bin/bash
#
# usage
# ./update-version.bash
topDir=$(git rev-parse --show-toplevel)
specfile="${topDir}/lcr.spec"
CMakefile="${topDir}/CMakeLists.txt"
old_version=$(cat ${specfile} | grep "%global" | grep "_version" | awk  {'print $3'})
first_old_version=$(cat ${specfile} | grep "%global" | grep "_version" | awk  {'print $3'} | awk -F "." {'print $1'})
second_old_version=$(cat ${specfile} | grep "%global" | grep "_version" | awk  {'print $3'} | awk -F "." {'print $2'})
third_old_version=$(cat ${specfile} | grep "%global" | grep "_version" | awk  {'print $3'} | awk -F "." {'print $3'})
read -p "Which level version do you want to upgrade?[1/2/3/d/N](default:N)  select:" choice
if [[ ! -n "${choice}" || ${choice} == "N" ]]; then
  echo "The version number has not been modified, it is still ${old_version}"
  exit 0
fi

if [[ ${choice} -eq "1" ]]; then
  first_old_version=$(($first_old_version+1))
  second_old_version="0"
  third_old_version="0"
elif [[ ${choice} -eq "2" ]]; then
  second_old_version=$(($second_old_version+1))
  third_old_version="0"
elif [[ ${choice} -eq "3" ]]; then
  third_old_version=$(($third_old_version+1))
fi

new_version=${first_old_version}.${second_old_version}.${third_old_version}

echo "The version number has been modified: ${old_version} => ${new_version}"

old_release=$(cat ${specfile} | grep "%global" | grep "_release" | awk  {'print $3'})
commit_id_long=`git log  --pretty=oneline  -1 | awk {'print $1'}`
commit_id=${commit_id_long:0:8}
new_release=`date "+%Y%m%d"`.`date "+%H%M%S"`.git$commit_id
echo "The relase version  has been modified, it is ${new_release}"
sed -i "s/set(LCR_VERSION \"${old_version}\")/set(LCR_VERSION \"${new_version}\")/g" ${CMakefile}
sed -i "s/^\%global _version ${old_version}$/\%global _version ${new_version}/g" ${specfile}
sed -i "s/^\%global _release ${old_release}$/\%global _release ${new_release}/g" ${specfile}

echo "The release number has been modified: ${old_release} => ${new_release}"
