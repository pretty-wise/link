include (${CMAKE_SOURCE_DIR}/link/utils/utils.cmake)
include (${CMAKE_SOURCE_DIR}/link/link/config.cmake)
include (${CMAKE_SOURCE_DIR}/base/config.cmake)


set( SOURCES
  main.cpp
  connector.h
  connector.cpp
)

plugin_framework(connector ${SOURCES})
