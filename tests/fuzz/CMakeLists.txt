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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize-coverage=trace-pc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-coverage=trace-pc")
set(EXE log_fuzz)

configure_file("dict/log_fuzz.dict" ${CMAKE_BINARY_DIR}/tests/fuzz/dict/log_fuzz.dict COPYONLY)
configure_file("fuzz.sh" ${CMAKE_BINARY_DIR}/tests/fuzz/fuzz.sh COPYONLY)

add_executable(${EXE} log_fuzz.cc)

target_include_directories(${EXE} PUBLIC
    ${GTEST_INCLUDE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/third_party
    PUBLIC ${CMAKE_SOURCE_DIR}/third_party/libocispec
    )

set_target_properties(${EXE} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${EXE} PROPERTIES LINK_FLAGS "-fsanitize-coverage=trace-pc")
target_link_libraries(${EXE} ${CMAKE_THREAD_LIBS_INIT} isula_libutils Fuzzer pthread)
