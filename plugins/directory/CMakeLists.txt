include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/extern/tinyxml2/config.cmake)
include(${CMAKE_SOURCE_DIR}/extern/protobuf/config.cmake)

include_directories (
  ${CMAKE_SOURCE_DIR}/link/include
	${CMAKE_SOURCE_DIR}/plugins/common/include
  ${CMAKE_SOURCE_DIR}/plugins/rest/include
)

compile_proto(PROTOCOL 
	""
	protocol/directory.proto)
set( SOURCES
  pluginsrc/directory_plugin.h
  pluginsrc/directory_plugin.cpp
  ${PROTOCOL}
)

plugin_framework(directory ${SOURCES})
target_link_libraries (directory link_plugin tinyxml2 rest_client protobuf)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
