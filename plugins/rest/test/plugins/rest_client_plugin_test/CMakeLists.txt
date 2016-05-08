include(${CMAKE_SOURCE_DIR}/link/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/link/common/config.cmake)
include(${CMAKE_SOURCE_DIR}/base/config.cmake)

include_directories(
  ${CMAKE_SOURCE_DIR}/link/link/plugins/rest/include
)

set( SOURCES main.cpp)

plugin_framework(rest_client_plugin_test ${SOURCES})
target_link_libraries(rest_client_plugin_test rest_client)
