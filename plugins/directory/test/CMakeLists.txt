#add_subdirectory(plugins/rest_client_plugin_test)

add_test(NAME test_directory 
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_directory.py
  $<TARGET_FILE:link_server>
  $<TARGET_FILE:directory>
  $<TARGET_FILE:rest>
)
