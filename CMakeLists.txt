cmake_minimum_required (VERSION 3.12.1)
project (lcr)

option(VERSION "set lcr version" ON)
if (VERSION STREQUAL "ON")
	set(LCR_VERSION "1.0.15")
endif()

option(DEBUG "set lcr gcc option" ON)
if (DEBUG STREQUAL "ON")
    add_definitions("-g -o2")
endif()

option(GCOV "set lcr gcov option" OFF)
if (GCOV STREQUAL "ON")
    set(LCR_GCOV "ON")
endif()

if (LIB_INSTALL_DIR)
  set(LIB_INSTALL_DIR_DEFAULT ${LIB_INSTALL_DIR})
else()
  set(LIB_INSTALL_DIR_DEFAULT "lib")
endif()

# check depends libs and headers
include(cmake/checker.cmake)
if (CHECKER_RESULT)
	return()
endif()

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("--  commit id: " ${GIT_COMMIT_HASH})

set(CMAKE_C_FLAGS "-fPIC -fstack-protector-all -D_FORTIFY_SOURCE=2 -O2 -Wall -Werror")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-E -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wtrampolines -fPIE -pie -shared -pthread")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-E -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wtrampolines -fPIE -pie")

add_definitions(-DLCRPATH="${CMAKE_INSTALL_PREFIX}/var/lib/lcr")
add_definitions(-DLOGPATH="${CMAKE_INSTALL_PREFIX}/var/log/lcr")
add_definitions(-DLCR_GIT_COMMIT="${GIT_COMMIT_HASH}")

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
	"${CMAKE_BINARY_DIR}/conf/config.h"
)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/lcr.pc.in"
	"${CMAKE_BINARY_DIR}/conf/lcr.pc"
)

# build which type of lcr library
option(USESHARED "set type of liblcr, default is shared" ON)
if (USESHARED STREQUAL "ON")
	set(LIBTYPE "SHARED")
	message("--  Build shared library")
else ()
	set(LIBTYPE "STATIC")
	message("--  Build static library")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

# install all files
install(FILES ${CMAKE_BINARY_DIR}/conf/lcr.pc
	DESTINATION ${LIB_INSTALL_DIR_DEFAULT}/pkgconfig PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE)
install(FILES src/lcrcontainer.h
	DESTINATION include/lcr)
