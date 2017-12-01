include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)

compile_proto(PROTOCOL 
	protocol/directory.proto)
set( SOURCES
  pluginsrc/directory_plugin.h
  pluginsrc/directory_plugin.cpp
  ${PROTOCOL}
)

plugin_framework(directory ${SOURCES})
target_link_libraries (directory PRIVATE link_plugin link_common tinyxml2 rest_client protobuf)

target_include_directories(directory PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/cpp>
)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
