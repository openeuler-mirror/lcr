# iSula-libutils: utils library for iSula
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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libocispec)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/http_parser)

# get current directory sources files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} local_utils_srcs)

set(THIRD_PARTY_SRCS
    ${local_utils_srcs}
    ${LIBOCISPEC_SRCS}
    ${HTTP_PARSER_SRCS}
    PARENT_SCOPE
    )

set(THIRD_PARTY_INCS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libocispec/
    ${HTTP_PARSER_INCS}
    PARENT_SCOPE
    )

