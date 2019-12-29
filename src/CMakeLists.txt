# generate .c and .h files for json
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/json)

# get liblcr source files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} topsrcs)
message("--  Get top srcs: " ${topsrcs})
aux_source_directory(${CMAKE_BINARY_DIR}/json generatesrcs)
message("--  Get generate srcs: " ${generatesrcs})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/json/schema/src commonjsonsrcs)
message("--  Get common json srcs: " ${commonjsonsrcs})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/json topjsonsrcs)
message("--  Get top json srcs: " ${topjsonsrcs})

# set liblcr library
add_library(liblcr ${LIBTYPE} ${topsrcs} ${topjsonsrcs} ${commonjsonsrcs} ${generatesrcs})

set(check_incs
	${LIBLXC_INCLUDE_DIR}
	${LIBYAJL_INCLUDE_DIR}
	)
set(check_libs
	${LIBLXC_LIBRARY}
	${LIBYAJL_LIBRARY}
	)
list(REMOVE_DUPLICATES check_incs)
list(REMOVE_DUPLICATES check_libs)

# set liblcr include dirs
target_include_directories(liblcr
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/json
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/json/schema/src
		PUBLIC ${CMAKE_BINARY_DIR}/json
		PUBLIC ${CMAKE_BINARY_DIR}/conf
		PUBLIC ${check_incs}
		)

# set liblcr compile flags
set_target_properties(liblcr PROPERTIES PREFIX "")

if (LCR_GCOV)
    set(CMAKE_C_FLAGS_DEBUG "-Wall -fprofile-arcs -ftest-coverage")
    message("------compile with gcov-------------")
    message("-----CFLAGS: " ${CMAKE_C_FLAGS_DEBUG})
    message("------------------------------------")
endif()

target_link_libraries(liblcr ${check_libs})

if (CMAKE_TOOLCHAIN_FILE)
    target_link_directories(liblcr PUBLIC ${CMAKE_LIBRARY_PATH})
    target_link_libraries(liblcr ${EXTRAL_LINK_LIBS})
endif()

if (LCR_GCOV)
    target_link_libraries(liblcr -lgcov)
endif()

# install all files
install(TARGETS liblcr
  LIBRARY DESTINATION ${LIB_INSTALL_DIR_DEFAULT} PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)
