set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# use relative path to src files to make __FILE__ macro more compact.
#set (CMAKE_USE_RELATIVE_PATHS ON)

#set (CMAKE_C_COMPILER 		  "/usr/bin/clang")
#set (CMAKE_CXX_COMPILER             "/usr/bin/clang++")

if(APPLE)
  set (CMAKE_C_FLAGS                "-Wall -m64 -std=c99 -fPIC -Wno-unused-function -DBASE_APPLE")
  set (CMAKE_C_FLAGS_DEBUG          "-g -DDEBUG")
  set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
  set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
  set (CMAKE_DEBUG_POSTFIX          "d")

  set (CMAKE_CXX_FLAGS                "-Wall -m64 -std=c++0x -fPIC -Wno-unused-function -Wno-unused-private-field -DBASE_APPLE")
  set (CMAKE_CXX_FLAGS_DEBUG          "-g -DDEBUG")
  set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
  set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
elseif(UNIX)
  set (CMAKE_C_FLAGS                "-Wall -m64 -std=c99 -ldl -pthread -fPIC -Wno-unused-function -DBASE_UNIX")
  set (CMAKE_C_FLAGS_DEBUG          "-g -DDEBUG")
  set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
  set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
  set (CMAKE_DEBUG_POSTFIX          "d")

  set (CMAKE_CXX_FLAGS                "-Wall -m64 -std=c++0x -ldl -pthread -fPIC -Wno-unused-private-field -Wno-unused-function -DBASE_UNIX")
  set (CMAKE_CXX_FLAGS_DEBUG          "-g -DDEBUG")
  set (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
  set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
endif()
