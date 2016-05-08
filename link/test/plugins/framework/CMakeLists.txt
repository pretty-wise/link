include(${CMAKE_SOURCE_DIR}/link/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/base/config.cmake)

set( SOURCES main.cpp)

plugin_framework(Framework ${SOURCES})
