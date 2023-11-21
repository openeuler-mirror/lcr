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
        --gtest_output=xml:${ARGV1}-Results.xml
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
_DEFINE_NEW_TEST(utils_ut utils_testcase)


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
add_dependencies(mock_ut log_ut libocispec_ut go_crc64_ut utils_ut)

IF(ENABLE_GCOV)
    add_custom_target(coverage
        COMMAND lcov --directory . --zerocounters
        COMMAND lcov -c -i -d . -o coverage.base

        COMMAND ctest

        COMMAND lcov --directory . --capture --output-file coverage.info
        COMMAND lcov -a coverage.base -a coverage.info --output-file coverage.total
        COMMAND lcov --remove coverage.total '/usr/*' '*/json/*' --output-file ${PROJECT_BINARY_DIR}/coverage.info.cleaned
        COMMAND genhtml -o coverage ${PROJECT_BINARY_DIR}/coverage.info.cleaned
        COMMAND ${CMAKE_COMMAND} -E remove coverage.base coverage.total ${PROJECT_BINARY_DIR}/coverage.info.cleaned

        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMENT "generating report..."
    )

    add_custom_command(TARGET coverage POST_BUILD
        COMMAND ;
        COMMENT "open .test/coverage/index.html in browser to view the coverage analysis report."
    )
ENDIF(ENABLE_GCOV)
# --------------- testcase add finish -----------------

