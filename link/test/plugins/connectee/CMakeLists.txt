include (${CMAKE_SOURCE_DIR}/link/utils/utils.cmake)
include (${CMAKE_SOURCE_DIR}/link/link/config.cmake)
include (${CMAKE_SOURCE_DIR}/base/config.cmake)

set( SOURCES
  main.cpp
  connectee.h
  connectee.cpp
)

plugin_framework(connectee ${SOURCES})
