include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)

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
target_link_libraries (rest PRIVATE civetweb rest_client link_plugin link_common)

#
# rest client
#

set( SOURCES
  clientsrc/rest_client.cpp
  include/plugin/rest/rest_client.h
  include/plugin/rest/rest_command.h
)

add_library(rest_client ${SOURCES})

target_link_libraries (rest_client PRIVATE link_plugin link_common)

target_include_directories(rest_client PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/plugins/rest/include>
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/plugins/common/include>
)


if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
