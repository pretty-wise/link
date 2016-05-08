include(${CMAKE_SOURCE_DIR}/link/config.cmake)
include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/common/config.cmake)
include(${CMAKE_SOURCE_DIR}/rush/config.cmake)
include(${CMAKE_SOURCE_DIR}/extern/tinyxml2/config.cmake)

include_directories (
  ${CMAKE_SOURCE_DIR}/link/plugins/rest/include
)

set( SOURCES
  pluginsrc/udp_node.h
  pluginsrc/udp_node.cpp
)

plugin_framework(udp_node ${SOURCES})
target_link_libraries (udp_node rest_client link_plugin jsmn tinyxml2 rush_transport)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
