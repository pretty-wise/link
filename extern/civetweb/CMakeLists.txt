project (civetweb)

add_library (civetweb
  src/civetweb.c
  include/civetweb.h
)

target_include_directories(civetweb PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/extern/civetweb/include>
	$<INSTALL_INTERFACE:/extern/civetweb/inclue>
)
