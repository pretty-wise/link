include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)

#
# monitor plugin.
#

set( SOURCES
  pluginsrc/monitor_plugin.h
  pluginsrc/monitor_plugin.cpp
)

if(APPLE)
	set( EXTRA_SOURCES
		pluginsrc/cpu_stats_mac.cpp
	)
else()
	set( EXTRA_SOURCES
		pluginsrc/cpu_stats_linux.cpp
	)
endif()

plugin_framework(monitor ${SOURCES} ${EXTRA_SOURCES})
target_link_libraries (monitor rest_client link_plugin link_common)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
