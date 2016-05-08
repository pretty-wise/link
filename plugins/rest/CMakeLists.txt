include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/extern/jsmn/config.cmake)

include_directories (
  ${CMAKE_SOURCE_DIR}/link/include
  ${CMAKE_SOURCE_DIR}/extern/civetweb/include
  ${CMAKE_SOURCE_DIR}/plugins/rest/include
)

#
# rest plugin.
#

set( SOURCES
  pluginsrc/rest_plugin.h
  pluginsrc/rest_plugin.cpp
  pluginsrc/rest_server.h
  pluginsrc/rest_server.cpp
)

plugin_framework(rest ${SOURCES})
target_link_libraries (rest civetweb rest_client link_plugin jsmn)

#
# rest client
#

set( SOURCES
  clientsrc/rest_client.cpp
  include/plugin/rest/rest_client.h
  include/plugin/rest/rest_command.h
)

add_library(rest_client ${SOURCES})
target_link_libraries (rest_client link_common)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
