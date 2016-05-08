include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/common/config.cmake)
include(${CMAKE_SOURCE_DIR}/extern/tinyxml2/config.cmake)
include(${CMAKE_SOURCE_DIR}/extern/protobuf/config.cmake)

include_directories (
  ${CMAKE_SOURCE_DIR}/link/include
  ${CMAKE_SOURCE_DIR}/plugins/punch/include
	${CMAKE_SOURCE_DIR}/plugins/rest/include
	${CMAKE_SOURCE_DIR}/transport/include
)

#
# punch plugin.
#

set( SOURCES
  pluginsrc/punch_plugin.h
  pluginsrc/punch_plugin.cpp
)	

plugin_framework(punch ${SOURCES})
target_link_libraries (punch link_plugin punch_server link_common rest_client tinyxml2)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
