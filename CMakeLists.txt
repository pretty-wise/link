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

set (BASE_CORE_FILES 
	base/include/base/core/types.h
  base/include/base/core/macro.h
  base/include/base/core/handle.h
  base/include/base/core/ref_counted.h
  base/include/base/core/service_locator.h
  base/include/base/core/shared_object.h
  base/include/base/core/shared_ptr.h
  base/include/base/core/string_id.h
  base/include/base/core/time_utils.h
  base/include/base/core/types.h
  base/include/base/core/str.h
  base/include/base/core/crash.h
 	base/src/core/time_utils.cpp
  base/src/core/string_id.cpp
  base/src/core/shared_object.cpp
  base/src/core/str.cpp
  base/src/core/crash.cpp
	base/include/base/core/log.h
	base/src/core/log.cpp
	base/include/base/core/log_file.h
	base/src/core/log_file.cpp
)

set (BASE_MATH_FILES 
	base/include/base/math/crc.h
  base/include/base/math/math.h
  base/include/base/math/matrix4x4.h
  base/include/base/math/quaternion.h
  base/include/base/math/ray.h
  base/include/base/math/vec2.h
  base/include/base/math/vec3.h
  base/include/base/math/vec4.h
)

set (BASE_NETWORK_FILES 
	base/include/base/network/address.h
  base/include/base/network/url.h
  base/include/base/network/socket.h
  base/include/base/network/epoll.h
  base/src/network/address.cpp
  base/src/network/url.cpp
  base/src/network/socket.cpp
)

if(APPLE)
	set (BASE_NETWORK_FILES 
		${BASE_NETWORK_FILES}
  	base/src/network/epoll_macosx.cpp
	)
elseif(UNIX)
	set (BASE_NETWORK_FILES 
		${BASE_NETWORK_FILES}
  	base/src/network/epoll_linux.cpp
  )
endif()

set (BASE_RTTI_FILES 
	base/include/base/rtti/dynamic_types.h
  base/include/base/rtti/rtti.h
)

set (BASE_IO_FILES 
  base/include/base/io/binary_reader.h
  base/include/base/io/binary_writer.h
  base/include/base/io/bit_reader.h
  base/include/base/io/bit_writer.h
  base/include/base/io/memory_stream.h
  base/include/base/io/reader.h
  base/include/base/io/stream.h
  base/include/base/io/writer.h
	base/include/base/io/base_file.h
  base/src/io/binary_writer.cpp
  base/src/io/binary_reader.cpp
  base/src/io/bit_writer.cpp
  base/src/io/bit_reader.cpp
  base/src/io/memory_stream.cpp
  base/src/io/reader.cpp
	base/src/io/base_file_linux.cpp
)

set (BASE_MEMORY_FILES 
	base/include/base/memory/allocator.h
  base/include/base/memory/ring_allocator.h
  base/src/memory/ring_allocator.cpp
  base/include/base/memory/ring_buffer.h
  base/src/memory/ring_buffer.cpp
)

set (BASE_THREAD_FILES 
	base/include/base/threading/mutex.h
  base/include/base/threading/condition_variable.h
  base/include/base/threading/thread.h
  base/src/threading/condition_variable.cpp
  base/src/threading/mutex.cpp
  base/src/threading/thread.cpp
)

set (BASE_PROCESS_FILES
	base/include/base/process/process.h
	base/src/process/process.cpp
)

add_library (base STATIC
	${BASE_CORE_FILES}
	${BASE_MATH_FILES}
	${BASE_NETWORK_FILES}
	${BASE_RTTI_FILES}
	${BASE_IO_FILES}
	${BASE_THREAD_FILES}
	${BASE_MEMORY_FILES}
	${BASE_PROCESS_FILES}
)

target_include_directories (base PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/base/include>
	$<INSTALL_INTERFACE:base/include>
)
target_include_directories (base PRIVATE src)

target_link_libraries(base PUBLIC ${CMAKE_DL_LIBS})

add_subdirectory(extern)

#
# common code for link and plugins. mostly utilities.
#
project(link_common)

add_library (link_common 
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

target_include_directories(link_common 
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common/include>
	$<INSTALL_INTERFACE:/common/include>
)

target_link_libraries (link_common 
	PUBLIC base jsmn
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
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/cpp/link/src/core>
)

target_link_libraries (link_core PUBLIC base link_common PRIVATE tinyxml2 protobuf)

#
# plugin core library
#
project (link_plugin)

set (SOURCES 
	link/src/plugin/link_private.c
)

add_library (link_plugin ${SOURCES})

target_link_libraries (link_plugin PUBLIC base)

target_include_directories(link_plugin PRIVATE 
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/src/common>
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/link/include>
)

#
# server executable
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
target_link_libraries(link_server PRIVATE ${CMAKE_DL_LIBS} link_core link_common tinyxml2 argtable2)

##
# various plugins to be used with link.
#
add_subdirectory (plugins)

#
# tests
#
#enable_testing()
#add_subdirectory(testing)

#
# install the library
#
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

include(CMakePackageConfigHelpers)
write_basic_package_version_file("../LinkConfigVersion.cmake"
	VERSION ${link_VERSION}
	COMPATIBILITY SameMajorVersion
)	

install(FILES "LinkConfig.cmake" "LinkVersionConfig.cmake"
	DESTINATION lib/cmake/link
)
]]

#
# generating sample config file
#
if(NOT CMAKE_GENERATOR MATCHES "Xcode")
  execute_process(COMMAND curl ifconfig.co 
		OUTPUT_VARIABLE HOSTNAME 
		OUTPUT_STRIP_TRAILING_WHITESPACE)
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
