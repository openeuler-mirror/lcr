project(lcr_LLT)

SET(EXE utils_llt)

add_executable(${EXE}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/log.c
    ${CMAKE_BINARY_DIR}/json/json_common.c
    utils_convert_llt.cc
    utils_llt.cc)

target_include_directories(${EXE} PUBLIC
    ${GTEST_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_BINARY_DIR}/json
    )
set_target_properties(${EXE} PROPERTIES LINK_FLAGS "-Wl,--wrap,util_strdup_s -Wl,--wrap,calloc -Wl,--wrap,strcat_s")
target_link_libraries(${EXE} ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} -lyajl -lsecurec -lz)
