project (raw_plugin)

#include (${CMAKE_SOURCE_DIR}/link/LinkCore/config.cmake)
include (${CMAKE_SOURCE_DIR}/base/config.cmake)

# to bo include nie dziala
include_directories (${CMAKE_SOURCE_DIR}/link/LinkCore/include)

set (SOURCES main.cpp)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/plugin)
add_library (raw_plugin SHARED ${SOURCES})
target_link_libraries (raw_plugin link_plugin)
