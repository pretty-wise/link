include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/extern/jsmn/config.cmake)

include_directories (
  ${CMAKE_SOURCE_DIR}/link/include
  ${CMAKE_SOURCE_DIR}/plugins/rest/include
)

#
# monitor plugin.
#

set( SOURCES
  pluginsrc/monitor_plugin.h
  pluginsrc/monitor_plugin.cpp
	pluginsrc/cpu_stats_linux.cpp
)

plugin_framework(monitor ${SOURCES})
target_link_libraries (monitor rest_client link_plugin jsmn)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
