
# creates a simple test from provided source files.
function (simple_test TEST_NAME)
  #message("creating " ${TEST_NAME} " with " ${ARGN})
  set (SRC_LIST)
  foreach (SRC_FILE ${ARGN})
    LIST(APPEND SRC_LIST ${SRC_FILE})
  endforeach ()
  add_executable (${TEST_NAME} ${SRC_LIST})
  target_link_libraries (${TEST_NAME} gtest gtest_main link_core link_common)
  add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
endfunction (simple_test)


function (plugin_framework PLUGIN_NAME)
  #message("creating " ${PLUGIN_NAME} " with " ${ARGN})

  # plugin destination folder.
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/plugin)

  set (SRC_LIST)
  foreach (SRC_FILE ${ARGN})
    LIST(APPEND SRC_LIST ${SRC_FILE})
  endforeach ()

  set (FRAMEWORK_SRC
    ${CMAKE_SOURCE_DIR}/plugins/common/include/plugin_interface.h
    ${CMAKE_SOURCE_DIR}/plugins/common/src/plugin_main.cpp
  )

  add_library (${PLUGIN_NAME} SHARED
    ${SRC_LIST}
    ${FRAMEWORK_SRC}
  )

	target_include_directories(${PLUGIN_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/plugins/common/include)
endfunction (plugin_framework)
