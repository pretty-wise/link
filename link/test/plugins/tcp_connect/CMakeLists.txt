include (${CMAKE_SOURCE_DIR}/link/utils/utils.cmake)
include (${CMAKE_SOURCE_DIR}/link/link/config.cmake)
include (${CMAKE_SOURCE_DIR}/link/common/config.cmake)
include (${CMAKE_SOURCE_DIR}/base/config.cmake)

include_directories (
  ${CMAKE_SOURCE_DIR}/link/plugins/rest/include
)

set( SOURCES
  main.cpp
  tcp_connect.h
  tcp_connect.cpp
)

plugin_framework(tcp_connect ${SOURCES})
target_link_libraries (tcp_connect rest_client)
