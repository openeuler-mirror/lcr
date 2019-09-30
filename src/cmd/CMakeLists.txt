# get lcr source files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} cmdsrcs)

# set lcr binary
add_executable(lcr ${cmdsrcs})
target_link_libraries(lcr liblcr)

# set lcr include headers
target_include_directories(lcr
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_SOURCE_DIR}/src/json
        PUBLIC ${CMAKE_SOURCE_DIR}/src/json/schema/src
		PUBLIC ${CMAKE_BINARY_DIR}/json
		PUBLIC ${CMAKE_BINARY_DIR}/conf
		)
if (LCR_GCOV)
    target_link_libraries(lcr -lgcov)
endif()

# install all files
install(TARGETS lcr
	RUNTIME DESTINATION bin PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)
