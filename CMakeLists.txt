cmake_minimum_required (VERSION 2.6)
project (link)
set (link_VERSION_MAJOR 0)
set (link_VERSION_MINOR 1)
include (settings.cmake)

set(TARGET_EXPORT_NAME "link-targets")
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries.")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for binaries.")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for includes.")
set(INSTALL_CMAKE_DIR cmake CACHE PATH "Installation directory for cmake files.")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
	set(var INSTALL_${p}_DIR)
	if(NOT IS_ABSOLUTE "${${var}}")
		set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
	endif()
endforeach()

find_package(base REQUIRED)
if(NOT BASE_FOUND)
	message(FATAL_ERROR 
		"base library not found. searched in: ${base_CONSIDERED_CONFIGS}"
	)
endif()

add_subdirectory (extern)

include_directories (
	${BASE_INCLUDES}
	common/include
	link/src/common
	link/include
)

#
# common code for link and plugins. mostly utilities.
#
project(link_common)

add_definitions(-DJSMN_PARENT_LINKS)

include_directories (
  ${CMAKE_SOURCE_DIR}/common/include
  ${CMAKE_SOURCE_DIR}/extern/jsmn
)

set (SOURCES
  common/include/common/json/json_writer.h
  common/src/json/json_writer.cpp
  common/include/common/json/json_reader.h
  common/src/json/json_reader.cpp
	common/include/common/protobuf_stream.h
	common/include/common/message_stream.h
	common/src/message_stream.cpp
	common/include/common/tcp_server.h
	common/src/tcp_server.cpp
)

add_library (link_common ${SOURCES})
target_link_libraries (link_common base jsmn)

#
# link server code. includes plugin shared object interface.
#
project (link_core)

include (${CMAKE_SOURCE_DIR}/extern/tinyxml2/config.cmake)
include (${CMAKE_SOURCE_DIR}/extern/protobuf/config.cmake)

set (PUBLIC_FILES
  link/include/link/link.h
  link/include/link/plugin_log.h
)

set (PRIVATE_FILES
  link/src/core/connection_manager.h
  link/src/core/connection_manager.cpp
  link/src/core/notification_queue.h
  link/src/core/notification_queue.cpp
  link/src/core/plugin.h
  link/src/core/plugin.cpp
  link/src/core/plugin_directory.h
  link/src/core/plugin_directory.cpp
  link/src/core/plugin_manager.h
  link/src/core/plugin_manager.cpp
  link/src/core/watch_manager.h
  link/src/core/watch_manager.cpp
  link/src/core/message_queue.h
  link/src/core/message_queue.cpp
  link/src/core/log.h
  link/src/core/log.cpp
  link/src/core/config_parser.h
  link/src/core/config_parser.cpp
  link/src/common/link_private.h
)

compile_proto (PROTOCOL link/src/core
  link/src/core/protocol/handshake.proto
)

add_library (link_core ${PUBLIC_FILES} ${PRIVATE_FILES} ${PROTOCOL})
target_link_libraries (link_core base tinyxml2 protobuf)

#
#
#
project (link_plugin)

set (SOURCES 
	link/src/plugin/link_private.c
)

add_library (link_plugin ${SOURCES})
target_link_libraries (link_plugin base)

#
#
#
project (link_server)

include(${CMAKE_SOURCE_DIR}/extern/tinyxml2/config.cmake)

add_executable (link_server 
	link/src/server/main.cpp
)

target_include_directories (link_server PUBLIC
	${CMAKE_SOURCE_DIR}/extern/argtable2/src
	link/src/core
)

target_link_libraries(link_server link_core link_common argtable2)

if(ENABLE_TEST)
	#  add_subdirectory (link/test)
endif(ENABLE_TEST)

#
# various plugins to be used with link.
#
add_subdirectory (plugins)

install(TARGETS link_common link_core link_plugin
	DESTINATION ${INSTALL_LIB_DIR}
	EXPORT ${TARGET_EXPORT_NAME}
)

install(TARGETS link_server
	DESTINATION ${INSTALL_BIN_DIR}
	EXPORT ${TARGET_EXPORT_NAME}
)

export(PACKAGE link)

install(DIRECTORY link/include/
	DESTINATION ${INSTALL_INCLUDE_DIR}
)

# project hierarchy:
# - Link
# +- common         = common code for code and plugins, compiled into LinkCommon lib.
# +- link           = server code
# |+- include
# ||+- link
# |+- src
# ||+- common       = code shared between server core and plugin interface.
# ||+- core         = server core code, compiled into link_core lib.
# ||+- server       = server code app, compiled into link_server app.
# ||+- plugin       = plugin interface code, compiled into LinkPlugin lib.
# |+- test          = various server core tests.
# | +- plugins      = plugin test code.
# +- plugins        = plugins.
#  +- common
#  +- pluginname    = source code of a single plugin.
#   +- include      = client header files, to be used by other plugins.
#   |+- plugin
#   | +- pluginname
#   +- clientsrc    = client source files, compiled into static lib, to be used by other plugins.
#   +- pluginsrc    = plugin source files, compiled into shared object.
#   +- test         = plugin tests.


#########################

execute_process(COMMAND curl ifconfig.co OUTPUT_VARIABLE HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE)

file(GENERATE OUTPUT config/link.txt CONTENT 
"<link runtime=\"0\" hostname=\"${HOSTNAME}\">
  <plugin path=\"$<TARGET_FILE:gate>\">
    <config>
      <gate port=\"0\" max_connections=\"5\"/>
    </config>
  </plugin>
  <plugin path=\"$<TARGET_FILE:directory>\">
  </plugin>
  <plugin path=\"$<TARGET_FILE:rest>\">
  </plugin>
  <plugin path=\"$<TARGET_FILE:launcher>\">
  </plugin>
  <plugin path=\"$<TARGET_FILE:monitor>\">
    <config>
      <punch port=\"0\" hostname=\"${HOSTNAME}\"/>
    </config>
  </plugin>
</link>
")
