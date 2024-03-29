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
configure_file("data/process.json" ${CMAKE_BINARY_DIR}/tests/process.json COPYONLY)

function(gmock_find_library _name)
  find_library(${_name}
    NAMES ${ARGN}
    HINTS
      $ENV{GMOCK_ROOT}
      ${GMOCK_ROOT}
  )
  mark_as_advanced(${_name})
endfunction()

find_path(GMOCK_INCLUDE_DIR gmock/gmock.h
  HINTS
    $ENV{GMOCK_ROOT}/include
    ${GMOCK_ROOT}/include
)
mark_as_advanced(GMOCK_INCLUDE_DIR)

gmock_find_library(GMOCK_LIBRARY            gmock)
gmock_find_library(GMOCK_LIBRARY_DEBUG      gmockd)
gmock_find_library(GMOCK_MAIN_LIBRARY       gmock_main)
gmock_find_library(GMOCK_MAIN_LIBRARY_DEBUG gmock_maind)

file(GLOB_RECURSE test_util_srcs
    "${CMAKE_SOURCE_DIR}/src/third_party/*.c"
    "${CMAKE_SOURCE_DIR}/src/third_party/**/*.c"
    "${CMAKE_SOURCE_DIR}/src/json/*.c"
    "${CMAKE_SOURCE_DIR}/src/json/**/*.c"
    "${CMAKE_SOURCE_DIR}/src/utils/*.c"
    "${CMAKE_SOURCE_DIR}/src/utils/**/*.c"
    "${CMAKE_BINARY_DIR}/json/*.c"
)

add_library(test_libisula_utils STATIC ${test_util_srcs})
target_include_directories(test_libisula_utils
    PUBLIC ${CMAKE_SOURCE_DIR}/src
    PUBLIC ${CMAKE_SOURCE_DIR}/src/utils
    PUBLIC ${CMAKE_SOURCE_DIR}/src/third_party
    PUBLIC ${CMAKE_SOURCE_DIR}/src/third_party/libocispec
    PUBLIC ${CMAKE_SOURCE_DIR}/src/json
    PUBLIC ${CMAKE_SOURCE_DIR}/src/json/schema/src
    PUBLIC ${CMAKE_BINARY_DIR}/json
    PUBLIC ${CMAKE_BINARY_DIR}/conf
    )
target_link_libraries(test_libisula_utils ${LIBYAJL_LIBRARY})

macro(_DEFINE_NEW_TEST)
    add_executable(${ARGV0}
        main.cpp
        ${ARGV0}.cpp
        )
    target_link_libraries(${ARGV0}
        test_libisula_utils
        ${GTEST_LIBRARY}
        ${GMOCK_LIBRARY}
        ${GMOCK_MAIN_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        -lz
        )

    target_include_directories(${ARGV0} PUBLIC
        ${GTEST_INCLUDE_DIR}
        PUBLIC ${CMAKE_SOURCE_DIR}/src
        PUBLIC ${CMAKE_SOURCE_DIR}/src/utils
        PUBLIC ${CMAKE_SOURCE_DIR}/src/third_party
        PUBLIC ${CMAKE_SOURCE_DIR}/src/third_party/libocispec
        PUBLIC ${CMAKE_SOURCE_DIR}/src/json
        PUBLIC ${CMAKE_SOURCE_DIR}/src/json/schema/src
        PUBLIC ${CMAKE_BINARY_DIR}/json
        PUBLIC ${CMAKE_BINARY_DIR}/conf
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
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
_DEFINE_NEW_TEST(defs_process_ut defs_process_testcase)
_DEFINE_NEW_TEST(go_crc64_ut go_crc64_testcase)
_DEFINE_NEW_TEST(auto_cleanup_ut autocleanup_testcase)
_DEFINE_NEW_TEST(utils_buffer_ut isula_buffer_testcase)
_DEFINE_NEW_TEST(utils_memory_ut isula_memory_testcase)
_DEFINE_NEW_TEST(utils_array_ut utils_array_testcase)
_DEFINE_NEW_TEST(utils_string_ut utils_string_testcase)
_DEFINE_NEW_TEST(utils_convert_ut utils_convert_testcase)
_DEFINE_NEW_TEST(utils_file_ut utils_file_testcase)
_DEFINE_NEW_TEST(utils_utils_ut utils_utils_testcase)
_DEFINE_NEW_TEST(utils_linked_list_ut utils_linked_list_testcase)
_DEFINE_NEW_TEST(utils_mainloop_ut utils_mainloop_testcase)

set_target_properties(utils_array_ut PROPERTIES LINK_FLAGS "-Wl,--wrap,calloc")
set_target_properties(utils_string_ut PROPERTIES LINK_FLAGS "-Wl,--wrap,calloc")

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
add_dependencies(mock_ut log_ut libocispec_ut defs_process_ut go_crc64_ut
    auto_cleanup_ut utils_memory_ut utils_array_ut utils_string_ut
    utils_convert_ut utils_file_ut utils_utils_ut utils_linked_list_ut
    utils_mainloop_ut
    )

IF(ENABLE_GCOV)
    add_custom_target(coverage
        COMMAND lcov --directory . --zerocounters --rc lcov_branch_coverage=1
        COMMAND lcov -c -i -d . -o coverage.base --rc lcov_branch_coverage=1

        COMMAND ctest

        COMMAND lcov --directory . --capture --output-file coverage.info --rc lcov_branch_coverage=1
        COMMAND lcov -a coverage.base -a coverage.info --output-file coverage.total --rc lcov_branch_coverage=1
        COMMAND lcov --remove coverage.total '/usr/*' '*/json/*' --output-file ${PROJECT_BINARY_DIR}/coverage.info.cleaned --rc lcov_branch_coverage=1
        COMMAND genhtml -o coverage ${PROJECT_BINARY_DIR}/coverage.info.cleaned --branch-coverage --rc lcov_branch_coverage=1
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

