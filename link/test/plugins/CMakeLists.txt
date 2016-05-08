

include_directories (${CMAKE_SOURCE_DIR}/link/plugins)

add_subdirectory(framework)
add_subdirectory(dummy_plugin)
add_subdirectory(raw_plugin)
add_subdirectory(connectee)
add_subdirectory(connector)
add_subdirectory(tcp_connect)
