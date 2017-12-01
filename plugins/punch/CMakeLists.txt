include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)

find_package(rush REQUIRED)

#
# punch plugin.
#

set( SOURCES
  pluginsrc/punch_plugin.h
  pluginsrc/punch_plugin.cpp
)	

plugin_framework(punch ${SOURCES})
target_link_libraries (punch PRIVATE link_plugin punch_server link_common rest_client tinyxml2)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
