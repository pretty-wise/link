include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/extern/tinyxml2/config.cmake)
include(${CMAKE_SOURCE_DIR}/extern/protobuf/config.cmake)

include_directories (
  ${CMAKE_SOURCE_DIR}/link/include
  ${CMAKE_SOURCE_DIR}/plugins/gate/include
	${CMAKE_SOURCE_DIR}/plugins/rest/include
)

compile_proto(PROTOCOL 
	""
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
target_link_libraries (gate link_plugin link_common rest_client tinyxml2 protobuf)

#
# gate client
#

set( SOURCES
  clientsrc/gate_client.cpp
	include/plugin/gate/gate_client.h
	${PROTOCOL}
)

add_library(gate_client ${SOURCES})
target_link_libraries (gate_client link_common protobuf)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
