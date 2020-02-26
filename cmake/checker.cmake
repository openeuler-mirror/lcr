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

#check python3
find_program(CMD_PYTHON python3)
_CHECK(CMD_PYTHON "CMD_PYTHON-NOTFOUND" "python3")

# check liblxc
pkg_check_modules(PC_LIBLXC REQUIRED "lxc>=3")
find_path(LIBLXC_INCLUDE_DIR lxc/lxccontainer.h
	HINTS ${PC_LIBLXC_INCLUDEDIR} ${PC_LIBLXC_INCLUDE_DIRS})
_CHECK(LIBLXC_INCLUDE_DIR "LIBLXC_INCLUDE_DIR-NOTFOUND" "lxc/lxccontainer.h")

find_library(LIBLXC_LIBRARY lxc
	HINTS ${PC_LIBLXC_LIBDIR} ${PC_LIBLXC_LIBRARY_DIRS})
_CHECK(LIBLXC_LIBRARY "LIBLXC_LIBRARY-NOTFOUND" "liblxc.so")

# check libyajl
pkg_check_modules(PC_LIBYAJL REQUIRED "yajl>=2")
find_path(LIBYAJL_INCLUDE_DIR yajl/yajl_tree.h
	HINTS ${PC_LIBYAJL_INCLUDEDIR} ${PC_LIBYAJL_INCLUDE_DIRS})
_CHECK(LIBYAJL_INCLUDE_DIR "LIBYAJL_INCLUDE_DIR-NOTFOUND" "yajl/yajl_tree.h")

find_library(LIBYAJL_LIBRARY yajl
	HINTS ${PC_LIBYAJL_LIBDIR} ${PC_LIBYAJL_LIBRARY_DIRS})
_CHECK(LIBYAJL_LIBRARY "LIBYAJL_LIBRARY-NOTFOUND" "libyajl.so")
