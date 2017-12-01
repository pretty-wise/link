include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)

compile_proto(PROTOCOL 
	protocol/gate.proto
)

#
# gate plugin.
#

set( SOURCES
  pluginsrc/gate_plugin.h
  pluginsrc/gate_plugin.cpp
	pluginsrc/gate_commands.h
	pluginsrc/gate_commands.cpp
	pluginsrc/gate_users.h
	pluginsrc/gate_users.cpp
	${PROTOCOL}
)	

plugin_framework(gate ${SOURCES})
target_link_libraries (gate PRIVATE link_plugin link_common rest_client tinyxml2 protobuf hiredis)

target_include_directories(gate PRIVATE
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/plugins/gate/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/cpp>
)

#
# gate client
#

set( SOURCES
  clientsrc/gate_client.cpp
	include/plugin/gate/gate_client.h
	${PROTOCOL}
)

add_library(gate_client ${SOURCES})
target_link_libraries (gate_client PRIVATE link_common protobuf)

target_include_directories(gate_client PRIVATE
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/plugins/gate/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/cpp>
)


if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
