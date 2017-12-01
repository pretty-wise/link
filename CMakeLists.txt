cmake_minimum_required (VERSION 3.0)
project (link)
set (link_VERSION_MAJOR 0)
set (link_VERSION_MINOR 1)
set (link_VERSION 1.0)
include (settings.cmake)
include (extern/protobuf/config.cmake)

set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries.")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for binaries.")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for includes.")
set(INSTALL_CMAKE_DIR cmake CACHE PATH "Installation directory for cmake files.")

find_package(Base REQUIRED)

add_subdirectory(extern)

#target_include_directories(link PUBLIC
#	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/common/include>
#	$<INSTALL_INTERFACE:common/include>
#	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/link/src/common>
#	$<INSTALL_INTERFACE:link/src/common>
#	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/link/include>
#	$<INSTALL_INTERFACE:link/include>
#)

#target_include_directories(link PRIVATE
#	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}link/src/common>
#	$<INSTALL_INTERFACE:link/src/common>
#)

#
# common code for link and plugins. mostly utilities.
#
project(link_common)

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

target_include_directories(link_common 
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common/include>
	$<INSTALL_INTERFACE:/common/include>
)

target_link_libraries (link_common 
	PUBLIC Base::Base jsmn
)

#
# link server code. includes plugin shared object interface.
#

project (link_core)

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

target_include_directories(link_core 
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/include>
	$<INSTALL_INTERFACE:${CMAKE_SOURCE_DIR}/link/include>
	PRIVATE
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/src/common>
)

target_link_libraries (link_core PUBLIC Base::Base link_common PRIVATE tinyxml2 protobuf)

#
#
#
project (link_plugin)

set (SOURCES 
	link/src/plugin/link_private.c
)

add_library (link_plugin ${SOURCES})

target_link_libraries (link_plugin PUBLIC Base::Base)

target_include_directories(link_plugin PRIVATE 
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/src/common>
)

#
#
#
project (link_server)

add_executable (link_server 
	link/src/server/main.cpp
)

target_include_directories (link_server PUBLIC
	PRIVATE
	link/src/core
)
#
target_link_libraries(link_server PRIVATE link_core link_common tinyxml2 argtable2)

#if(ENABLE_TEST)
#	 add_subdirectory (link/test)
#endif(ENABLE_TEST)

##
# various plugins to be used with link.
#
add_subdirectory (plugins)
#[[
install(TARGETS 
	link_common 
	link_core 
	link_plugin 
	EXPORT link_targets
	LIBRARY DESTINATION ${INSTALL_LIB_DIR}
	ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
)

install(EXPORT link_targets
	FILE LinkTargets.cmake
	NAMESPACE link::
	DESTINATION lib/cmake/link
)	

#install(TARGETS link_server EXPORT link_targets
#RUNTIME DESTINATION ${INSTALL_BIN_DIR}
#)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("../LinkConfigVersion.cmake"
	VERSION ${link_VERSION}
	COMPATIBILITY SameMajorVersion
)	

install(FILES "LinkConfig.cmake" "LinkVersionConfig.cmake"
	DESTINATION lib/cmake/link
)
]]
#export(PACKAGE link)

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
if(NOT CMAKE_GENERATOR MATCHES "Xcode")
execute_process(COMMAND curl ifconfig.co OUTPUT_VARIABLE HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE)
file(GENERATE OUTPUT config/link.txt CONTENT 
"<link runtime=\"0\" hostname=\"${HOSTNAME}\">
	<plugin path=\"$<TARGET_FILE:gate>\">
		<config>
			<gate port=\"0\" max_connections=\"5\">
				<redis hostname=\"localhost\" port=\"6379\"/>
			</gate>
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
endif()
