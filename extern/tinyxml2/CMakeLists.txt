project (tinyxml2)

add_library (tinyxml2
  tinyxml2.cpp 
  tinyxml2.h
)

target_include_directories(tinyxml2 PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/extern/tinyxml2>
	$<INSTALL_INTERFACE:/extern/tinyxml2>
)
