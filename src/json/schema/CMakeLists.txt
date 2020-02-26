project (JSONGenerator)

set(cmdpath python3)
set(pysrcpath ${CMAKE_CURRENT_SOURCE_DIR}/src/generate.py)
set(schemapath ${CMAKE_CURRENT_SOURCE_DIR}/schema)
set(outputpath ${CMAKE_BINARY_DIR}/json)
add_subdirectory(src)

message("--  Generate .c and .h file into: " ${outputpath})

execute_process(COMMAND ${cmdpath} ${pysrcpath} --gen-common --gen-ref -r --root=${schemapath} --out=${outputpath} ${schemapath}
	ERROR_VARIABLE err
	)
if (err)
    message("error: " ${err})
endif()
