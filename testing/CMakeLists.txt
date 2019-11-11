
find_package(GTest REQUIRED)

#
# general
#

add_executable(test_gtest "general/test_gtest.cpp")
target_link_libraries(test_gtest PRIVATE ${GTEST_BOTH_LIBRARIES})
target_include_directories(test_gtest PUBLIC
	${GTEST_INCLUDE_DIRS}
)
add_test(NAME test_gtest COMMAND $<TARGET_FILE:test_gtest>)

add_executable(test_connection_manager "general/test_connection_manager.cpp")
target_link_libraries(test_connection_manager PRIVATE ${GTEST_BOTH_LIBRARIES} link_core)
target_include_directories(test_connection_manager PUBLIC
	${GTEST_INCLUDE_DIRS}
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/src/core>
)
add_test(NAME test_connection_manager COMMAND $<TARGET_FILE:test_connection_manager>)

add_executable(test_plugin_loading "general/test_plugin_loading.cpp")
target_link_libraries(test_plugin_loading PRIVATE ${GTEST_BOTH_LIBRARIES} link_core)
target_include_directories(test_plugin_loading PUBLIC
	${GTEST_INCLUDE_DIRS}
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/src/core>
)
add_test(NAME test_plugin_loading COMMAND $<TARGET_FILE:test_plugin_loading>)

add_executable(test_plugin_directory "general/test_plugin_directory.cpp")
target_link_libraries(test_plugin_directory PRIVATE ${GTEST_BOTH_LIBRARIES} link_core)
target_include_directories(test_plugin_directory PUBLIC
	${GTEST_INCLUDE_DIRS}
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/src/core>
)
add_test(NAME test_plugin_directory COMMAND $<TARGET_FILE:test_plugin_directory>)

add_executable(test_tcp_stream "general/test_tcp_stream.cpp")
target_link_libraries(test_tcp_stream PRIVATE ${GTEST_BOTH_LIBRARIES} link_core)
target_include_directories(test_tcp_stream PUBLIC
	${GTEST_INCLUDE_DIRS}
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/src/core>
)
add_test(NAME test_tcp_stream COMMAND $<TARGET_FILE:test_tcp_stream>)

add_executable(test_plugin_watch "general/test_plugin_watch.cpp")
target_link_libraries(test_plugin_watch PRIVATE ${GTEST_BOTH_LIBRARIES} link_core)
target_include_directories(test_plugin_watch PUBLIC
	${GTEST_INCLUDE_DIRS}
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/src/core>
)
add_test(NAME test_plugin_watch COMMAND $<TARGET_FILE:test_plugin_watch>)

add_executable(test_socket_watch "general/test_socket_watch.cpp")
target_link_libraries(test_socket_watch PRIVATE ${GTEST_BOTH_LIBRARIES} link_core)
target_include_directories(test_socket_watch PUBLIC
	${GTEST_INCLUDE_DIRS}
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/src/core>
)
add_test(NAME test_socket_watch COMMAND $<TARGET_FILE:test_socket_watch>)

#
# link
#

add_executable(test_startup "link/test_startup.cpp")
target_link_libraries(test_startup PRIVATE ${GTEST_BOTH_LIBRARIES} base)
target_include_directories(test_startup PUBLIC
	${GTEST_INCLUDE_DIRS}
)
add_test(NAME test_startup COMMAND $<TARGET_FILE:test_startup> $<TARGET_FILE:link_server>)
