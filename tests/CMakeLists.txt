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

enable_testing()
find_package(GTest REQUIRED)

set(CMAKE_C_COMPILER "g++" CACHE PATH "c compiler")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

configure_file("data/ocihook.json" ${CMAKE_BINARY_DIR}/tests/ocihook.json COPYONLY)

macro(_DEFINE_NEW_TEST)
    add_executable(${ARGV0}
        ${TESTS_UTILS_SRCS}
        main.cpp
        ${ARGV0}.cpp
        )

    target_link_libraries(${ARGV0}
        isula_libutils
        gtest
        -lyajl
        pthread
        )

    target_include_directories(${ARGV0} PUBLIC
        ${GTEST_INCLUDE_DIR}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_SOURCE_DIR}/src
        PUBLIC ${CMAKE_SOURCE_DIR}/third_party
        PUBLIC ${CMAKE_SOURCE_DIR}/src/json
        PUBLIC ${CMAKE_SOURCE_DIR}/src/json/schema/src
        PUBLIC ${CMAKE_BINARY_DIR}/json
        PUBLIC ${CMAKE_BINARY_DIR}/conf
        )

    add_test(
        NAME ${ARGV1}
        COMMAND ${ARGV0}
    )
endmacro()

# --------------- testcase add here -----------------
#   api testcase
_DEFINE_NEW_TEST(log_llt log_testcase)
_DEFINE_NEW_TEST(json_llt json_testcase)

# --------------- testcase add finish -----------------

