include_directories(
  ${CMAKE_SOURCE_DIR}/extern/gtest/include
)

add_executable(test_gate_client test_gate_client.cpp link_process.cpp)
target_link_libraries(test_gate_client gtest gtest_main base gate_client)

add_test(NAME test_gate_client 
	COMMAND $<TARGET_FILE:test_gate_client>
	$<TARGET_FILE:link_server>
  "--config=<link runtime=\"0\"><plugin path=\"$<TARGET_FILE:gate>\"><config><gate port=\"0\" max_connections=\"100\"/></config></plugin></link>"
)

add_test(NAME test_gate_plugin
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_gate_plugin.py
  $<TARGET_FILE:link_server>
  "--config=<link runtime=\"5000\"><plugin path=\"$<TARGET_FILE:gate>\"><config><gate port=\"0\" max_connections=\"100\"/></config></plugin></link>"
)
