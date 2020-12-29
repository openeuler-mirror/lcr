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

configure_file("data/ocihook.json" ${CMAKE_BINARY_DIR}/tests/ocihook.json COPYONLY)

macro(_DEFINE_NEW_TEST)
    add_executable(${ARGV0}
        ${TESTS_UTILS_SRCS}
        main.cpp
        ${ARGV0}.cpp
        )

    target_link_libraries(${ARGV0}
        isula_libutils
        ${GTEST_LIBRARY}
        ${LIBYAJL_LIBRARY}
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

# fuzz test
option(ENABLE_FUZZ "set lcr fuzz option" OFF)
if (ENABLE_FUZZ)
    add_subdirectory(fuzz)
endif()

#   api testcase
_DEFINE_NEW_TEST(log_ut log_testcase)
_DEFINE_NEW_TEST(libocispec_ut libocispec_testcase)
_DEFINE_NEW_TEST(go_crc64_ut go_crc64_testcase)


# mock test for run lcov to generate html
add_executable(mock_ut main.cpp)
target_include_directories(mock_ut PUBLIC
    ${GTEST_INCLUDE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_link_libraries(mock_ut
    ${GTEST_LIBRARY}
    pthread
    )
add_dependencies(mock_ut log_ut libocispec_ut go_crc64_ut)

# run ut and gcov
if (ENABLE_GCOV)
    set(result isula_libutils.info)
    set(result_dir lcr_gcovs)
    set(target_dir src/CMakeFiles/isula_libutils.dir)

    add_custom_command(TARGET mock_ut
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove -f ${result}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${result_dir}
        COMMAND ctest -VV
        COMMAND ${CMD_LCOV} --gcov-tool ${CMD_GCOV} --exclude "/usr/*" --exclude "*/src/utils.c" --exclude "*/json/*" -d ${CMAKE_BINARY_DIR}/${target_dir} -c -o ${result}
        COMMAND ${CMD_GENHTML} --ignore-errors source -o ${result_dir} ${result}
        COMMAND ${CMAKE_COMMAND} -E echo run gcov finish
        VERBATIM
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
        )
else()
    add_custom_command(TARGET mock_ut
        PRE_BUILD
        COMMAND ${CMAKE_CTEST_COMMAND} -VV
        COMMAND ${CMAKE_COMMAND} -E echo UT finish
        VERBATIM
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
        )
endif()

# --------------- testcase add finish -----------------

