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

# check liblxc
pkg_check_modules(PC_LIBLXC REQUIRED "lxc>=3")
find_path(LIBLXC_INCLUDE_DIR lxc/lxccontainer.h
	HINTS ${PC_LIBLXC_INCLUDEDIR} ${PC_LIBLXC_INCLUDE_DIRS})
_CHECK(LIBLXC_INCLUDE_DIR "LIBLXC_INCLUDE_DIR-NOTFOUND" "lxc/lxccontainer.h")

find_library(LIBLXC_LIBRARY lxc
	HINTS ${PC_LIBLXC_LIBDIR} ${PC_LIBLXC_LIBRARY_DIRS})
_CHECK(LIBLXC_LIBRARY "LIBLXC_LIBRARY-NOTFOUND" "liblxc.so")

# check iSula libutils
pkg_check_modules(PC_ISULA_LIBUTILS REQUIRED "isula_libutils")
find_path(ISULA_LIBUTILS_INCLUDE_DIR isula_libutils/log.h
	HINTS ${PC_ISULA_LIBUTILS_INCLUDEDIR} ${PC_ISULA_LIBUTILS_INCLUDE_DIRS})
_CHECK(ISULA_LIBUTILS_INCLUDE_DIR "ISULA_LIBUTILS_INCLUDE_DIR-NOTFOUND" "isula_libutils/log.h")

find_library(ISULA_LIBUTILS_LIBRARY isula_libutils
	HINTS ${PC_ISULA_LIBUTILS_LIBDIR} ${PC_ISULA_LIBUTILS_LIBRARY_DIRS})
_CHECK(ISULA_LIBUTILS_LIBRARY "ISULA_LIBUTILS_LIBRARY-NOTFOUND" "libisula_libutils.so")
