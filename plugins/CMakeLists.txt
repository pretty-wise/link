function (plugin_framework PLUGIN_NAME)
  #message("creating " ${PLUGIN_NAME} " with " ${ARGN})
  set (SRC_LIST)
  foreach (SRC_FILE ${ARGN})
    LIST(APPEND SRC_LIST ${SRC_FILE})
  endforeach ()

  set (FRAMEWORK_SRC
    common/include/plugin_interface.h
    common/src/plugin_interface.cpp
    common/src/plugin_main.cpp
  )

  add_executable (${PLUGIN_NAME}
    ${SRC_LIST}
    ${FRAMEWORK_SRC}
  )
  target_link_libraries (${PLUGIN_NAME} link_plugin)
endfunction (plugin_framework)

#
# plugins:
#

add_subdirectory(rest)
add_subdirectory(directory)
add_subdirectory(monitor)
add_subdirectory(launcher)
#add_subdirectory(udp_node)
add_subdirectory(gate)
add_subdirectory(matchmaker)
#add_subdirectory(punch)
