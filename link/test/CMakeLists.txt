include_directories (
  ${CMAKE_SOURCE_DIR}/extern/gtest/include
  ${CMAKE_SOURCE_DIR}/link/link/include
  ${CMAKE_SOURCE_DIR}/link/link/src/core
  ${CMAKE_SOURCE_DIR}/link/link/src/plugin
)

add_subdirectory(plugins)

include(${CMAKE_SOURCE_DIR}/link/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/base/config.cmake)
include(${CMAKE_SOURCE_DIR}/link/common/config.cmake)

simple_test (test_gtest
  test_gtest.cpp
)

#simple_test (link_core
#  socket_watch_test.cpp
#)

simple_test (test_plugin_directory
  test_plugin_directory.cpp
)

simple_test (test_connection_manager
  test_connection_manager.cpp
)

simple_test (test_plugin_loading
  test_plugin_loading.cpp
)

simple_test (test_tcp_stream
  test_tcp_stream.cpp
)
#$<TARGET_FILE:rest>$<TARGET_FILE:tcp_connect>
add_test(NAME test_tcp_connection
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_tcp_connection.py
  $<TARGET_FILE:link_server>
  "--config=<link runtime=\"5000\"><plugin path=\"$<TARGET_FILE:rest>\"/><plugin path=\"$<TARGET_FILE:tcp_connect>\"/></link>"
)
