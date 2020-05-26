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

set(cmdpath python3)
set(pysrcpath ${CMAKE_SOURCE_DIR}/third_party/libocispec/generate.py)
set(schemapath ${CMAKE_CURRENT_SOURCE_DIR}/schema)
set(outputpath ${CMAKE_BINARY_DIR}/json)

message("--  Generate .c and .h file into: " ${outputpath})

execute_process(COMMAND ${cmdpath} ${pysrcpath} --gen-common --gen-ref -r --root=${schemapath} --out=${outputpath} ${schemapath}
    ERROR_VARIABLE err
    )

if (err)
    message(FATAL_ERROR "ERROR: " ${err})
endif()
