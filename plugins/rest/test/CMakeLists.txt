add_subdirectory(plugins/rest_client_plugin_test)

add_test(NAME test_rest_plugin 
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_rest_plugin.py
  $<TARGET_FILE:link_server>
  "--config=<link runtime=\"5000\"><plugin path=\"$<TARGET_FILE:rest_client_plugin_test>\"/><plugin path=\"$<TARGET_FILE:rest>\"/></link>"
)
