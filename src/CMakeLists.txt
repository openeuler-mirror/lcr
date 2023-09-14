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

set(check_incs
    ${LIBLXC_INCLUDE_DIR}
    )

set(check_libs
    ${LIBLXC_LIBRARY}
    )

list(REMOVE_DUPLICATES check_incs)
list(REMOVE_DUPLICATES check_libs)

set(check_isula_utils_incs
    ${LIBYAJL_INCLUDE_DIR}
    )
set(check_isula_utils_libs
    ${LIBYAJL_LIBRARY}
    )
list(REMOVE_DUPLICATES check_isula_utils_libs)
list(REMOVE_DUPLICATES check_isula_utils_incs)


# generate .c and .h files for json
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/json)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils)

# all lcr runtime code files in runtime dir
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/runtime)

# get json source files
aux_source_directory(${CMAKE_BINARY_DIR}/json generatesrcs)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/json topjsonsrcs)

# get top source files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} topsrcs)
message("--  Get top srcs: " ${topsrcs})

set(isula_libutils_srcs
    ${topsrcs}
    ${THIRD_PARTY_SRCS}
    ${generatesrcs}
    ${commonjsonsrcs}
    ${topjsonsrcs}
    ${UTILS_SRCS}
    )

set(isula_libutils_incs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/json
    ${CMAKE_CURRENT_SOURCE_DIR}/json/schema/src
    ${CMAKE_BINARY_DIR}/json
    ${THIRD_PARTY_INCS}
    ${check_isula_utils_incs}
    ${UTILS_INCS}
    )

# set libisula_libutils library
add_library(isula_libutils
    SHARED
    ${isula_libutils_srcs}
    )

# set include dirs
#PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/utils
target_include_directories(isula_libutils
    PUBLIC ${isula_libutils_incs}
    )

target_link_libraries(isula_libutils ${check_isula_utils_libs})

add_library(isulad_shim_libutils
    STATIC
    ${topsrcs}
    ${THIRD_PARTY_SRCS}
    ${commonjsonsrcs}
    ${topjsonsrcs}
    ${UTILS_SRCS}
    ${CMAKE_BINARY_DIR}/json/logger_json_file.c
    ${CMAKE_BINARY_DIR}/json/shim_client_process_state.c
    ${CMAKE_BINARY_DIR}/json/shim_client_runtime_stats.c
    ${CMAKE_BINARY_DIR}/json/shim_client_cgroup_resources.c
    ${CMAKE_BINARY_DIR}/json/json_common.c
    ${CMAKE_BINARY_DIR}/json/defs.c
    ${CMAKE_BINARY_DIR}/json/oci_runtime_config_linux.c
    ${CMAKE_BINARY_DIR}/json/oci_runtime_pspec.c
    ${CMAKE_BINARY_DIR}/json/oci_runtime_spec.c
    ${CMAKE_BINARY_DIR}/json/oci_runtime_state.c
    )

target_include_directories(isulad_shim_libutils
    PUBLIC ${isula_libutils_incs}
    )
target_link_libraries(isulad_shim_libutils ${LIBYAJL_LIBRARY})

if (ENABLE_LIBLCR)
# set liblcr library
add_library(liblcr SHARED ${RUNTIME_SRCS})

set(liblcr_incs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/conf
    ${CMAKE_CURRENT_SOURCE_DIR}/json/schema/src
    ${CMAKE_CURRENT_SOURCE_DIR}/json
    ${RUNTIME_INCS}
    ${check_incs}
)

# set liblcr include dirs
target_include_directories(liblcr
    PUBLIC ${liblcr_incs}
    )

# set liblcr compile flags
set_target_properties(liblcr PROPERTIES PREFIX "")
target_link_libraries(liblcr ${check_libs} isula_libutils)
target_compile_options(liblcr PRIVATE -fvisibility=hidden)

if (CMAKE_TOOLCHAIN_FILE)
    target_link_directories(liblcr PUBLIC ${CMAKE_LIBRARY_PATH})
    target_link_libraries(liblcr ${EXTRAL_LINK_LIBS})
endif()


add_library(liblcr_s STATIC ${RUNTIME_SRCS})
target_include_directories(liblcr_s
    PUBLIC ${liblcr_incs}
    )
target_link_libraries(liblcr_s ${check_libs} isula_libutils)
set_target_properties(liblcr_s PROPERTIES PREFIX "")
set_target_properties(liblcr_s PROPERTIES OUTPUT_NAME liblcr)

# install all files
install(TARGETS liblcr
    LIBRARY DESTINATION ${LIB_INSTALL_DIR_DEFAULT} PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(TARGETS liblcr_s
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR_DEFAULT} PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

install(TARGETS isula_libutils
    LIBRARY DESTINATION ${LIB_INSTALL_DIR_DEFAULT} PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(TARGETS isulad_shim_libutils
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR_DEFAULT} PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
