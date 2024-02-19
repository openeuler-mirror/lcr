# iSula-libutils: import http_parser for iSula utils library
#
# Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
#
# Authors:
# xuxuepeng <xuxuepeng1@huawei.com>
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

# get current directory sources files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} local_http_parser_srcs)

set(HTTP_PARSER_SRCS
    ${local_http_parser_srcs}
    PARENT_SCOPE
    )

set(HTTP_PARSER_INCS
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
    )

