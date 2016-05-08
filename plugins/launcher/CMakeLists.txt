include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)
include(${CMAKE_SOURCE_DIR}/extern/jsmn/config.cmake)
include(${CMAKE_SOURCE_DIR}/extern/tinyxml2/config.cmake)

include_directories (
  ${CMAKE_SOURCE_DIR}/plugins/rest/include
)

set( SOURCES
  pluginsrc/launcher.h
  pluginsrc/launcher.cpp
)

if((APPLE) OR (UNIX))
set (SOURCES ${SOURCES}
  pluginsrc/commands_posix.cpp
)
endif()

plugin_framework(launcher ${SOURCES})
target_link_libraries (launcher 
	rest_client 
	link_plugin 
	jsmn 
	tinyxml2 
)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
