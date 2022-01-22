message("${BoldGreen}---- Selected options begin ----${ColourReset}")

option(ENABLE_GCOV "set lcr gcov option" OFF)
if (ENABLE_GCOV STREQUAL "ON")
    message("${Green}--  Enable gcov${ColourReset}")
endif()

option(ENABLE_LIBLCR "enable lcr liblcr option" ON)
if (ENABLE_LIBLCR STREQUAL "ON")
    add_definitions(-DENABLE_LIBLCR=1)
    set(ENABLE_LIBLCR 1)
    message("${Green}--  Enable liblcr${ColourReset}")
endif()

message("${BoldGreen}---- Selected options end ----${ColourReset}")
