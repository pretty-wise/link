include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/extern/tinyxml2/config.cmake)
include(${CMAKE_SOURCE_DIR}/extern/protobuf/config.cmake)

include_directories (
  ${CMAKE_SOURCE_DIR}/link/include
  ${CMAKE_SOURCE_DIR}/plugins/matchmaker/include
	${CMAKE_SOURCE_DIR}/plugins/rest/include
)

compile_proto(PROTOCOL 
	""
	protocol/matchmaker.proto
)

#
# matchmaker plugin.
#

set( SOURCES
  pluginsrc/matchmaker_plugin.h
  pluginsrc/matchmaker_plugin.cpp
	${PROTOCOL}
)	

plugin_framework(matchmaker ${SOURCES})
target_link_libraries (matchmaker link_plugin link_common rest_client tinyxml2 protobuf)

#
# matchmaker client
#

set( SOURCES
  clientsrc/matchmaker_client.cpp
	include/plugin/matchmaker/matchmaker_client.h
	${PROTOCOL}
)

add_library(matchmaker_client ${SOURCES})
target_link_libraries (matchmaker_client link_common protobuf)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
