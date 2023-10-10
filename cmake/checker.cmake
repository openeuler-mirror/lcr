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

include(CheckFunctionExists)
include(CheckCSourceCompiles)

# check depends library and headers
find_package(PkgConfig REQUIRED)

macro(_CHECK)
if (${ARGV0} STREQUAL "${ARGV1}")
	message("error: can not find " ${ARGV2} " program")
	set(CHECKER_RESULT 1)
else()
	message("--  found " ${ARGV2} " --- works")
endif()
endmacro()

if (ENABLE_LIBLCR)
# check liblxc
pkg_check_modules(PC_LIBLXC REQUIRED "lxc>=3")
find_path(LIBLXC_INCLUDE_DIR lxc/lxccontainer.h
	HINTS ${PC_LIBLXC_INCLUDEDIR} ${PC_LIBLXC_INCLUDE_DIRS})
_CHECK(LIBLXC_INCLUDE_DIR "LIBLXC_INCLUDE_DIR-NOTFOUND" "lxc/lxccontainer.h")

find_library(LIBLXC_LIBRARY lxc
	HINTS ${PC_LIBLXC_LIBDIR} ${PC_LIBLXC_LIBRARY_DIRS})
_CHECK(LIBLXC_LIBRARY "LIBLXC_LIBRARY-NOTFOUND" "liblxc.so")
endif()

#check python3
find_program(CMD_PYTHON python3)
_CHECK(CMD_PYTHON "CMD_PYTHON-NOTFOUND" "python3")

# check libyajl
pkg_check_modules(PC_LIBYAJL REQUIRED "yajl>=2")
if (NOT PC_LIBYAJL_FOUND)
	message("error: can not find yajl>=2")
	set(CHECKER_RESULT 1)
endif()
find_path(LIBYAJL_INCLUDE_DIR yajl/yajl_tree.h
	HINTS ${PC_LIBYAJL_INCLUDEDIR} ${PC_LIBYAJL_INCLUDE_DIRS})
_CHECK(LIBYAJL_INCLUDE_DIR "LIBYAJL_INCLUDE_DIR-NOTFOUND" "yajl/yajl_tree.h")

find_library(LIBYAJL_LIBRARY yajl
	HINTS ${PC_LIBYAJL_LIBDIR} ${PC_LIBYAJL_LIBRARY_DIRS})
_CHECK(LIBYAJL_LIBRARY "LIBYAJL_LIBRARY-NOTFOUND" "libyajl.so")

if (ENABLE_UT)
    pkg_check_modules(PC_GTEST "gtest")
    find_path(GTEST_INCLUDE_DIR gtest/gtest.h
        HINTS ${PC_GTEST_INCLUDEDIR} ${PC_GTEST_INCLUDE_DIRS})
    _CHECK(GTEST_INCLUDE_DIR "GTEST_INCLUDE_DIR-NOTFOUND" "gtest.h")
    find_library(GTEST_LIBRARY gtest
        HINTS ${PC_GTEST_LIBDIR} ${PC_GTEST_LIBRARY_DIRS})
    _CHECK(GTEST_LIBRARY "GTEST_LIBRARY-NOTFOUND" "libgtest.so")
endif()

if (ENABLE_GCOV)
    find_program(CMD_GCOV gcov)
    _CHECK(CMD_GCOV "CMD_GCOV-NOTFOUND" "gcov")

    find_program(CMD_LCOV lcov)
    _CHECK(CMD_LCOV "CMD_LCOV-NOTFOUND" "lcov")

    find_program(CMD_GENHTML genhtml)
    _CHECK(CMD_GENHTML "CMD_GENHTML-NOTFOUND" "genhtml")
endif()

check_function_exists(strerror_r HAVE_STRERROR_R)

check_c_source_compiles(
    "#define _GNU_SOURCE\n#include <string.h>\nint main() { char err_str[128]; char *ptr = strerror_r(-2, err_str, 128); return ptr != (void *)0L; }"
    STRERROR_R_CHAR_P
)
