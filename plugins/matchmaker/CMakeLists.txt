include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)

compile_proto(PROTOCOL 
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
target_link_libraries (matchmaker PRIVATE link_plugin link_common rest_client tinyxml2 protobuf)

target_include_directories(matchmaker PRIVATE
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/plugins/matchmaker/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/cpp>
)

#
# matchmaker client
#

set( SOURCES
  clientsrc/matchmaker_client.cpp
	include/plugin/matchmaker/matchmaker_client.h
	${PROTOCOL}
)

add_library(matchmaker_client ${SOURCES})

target_link_libraries (matchmaker_client PRIVATE link_common protobuf)

target_include_directories(matchmaker_client PRIVATE
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/plugins/matchmaker/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/cpp>
)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
