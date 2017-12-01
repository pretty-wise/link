include(${CMAKE_SOURCE_DIR}/utils/utils.cmake)

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
target_link_libraries (launcher PRIVATE
	rest_client 
	link_plugin 
	link_common
	tinyxml2 
)

if(ENABLE_TEST)
  add_subdirectory (test)
endif(ENABLE_TEST)
