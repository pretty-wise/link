add_library( jsmn
  jsmn.h
  jsmn.c
)

target_include_directories(jsmn PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/extern/jsmn>
	$<INSTALL_INTERFACE:/extern/jsmn>
)
target_compile_definitions(jsmn PUBLIC JSMN_PARENT_LINKS)

