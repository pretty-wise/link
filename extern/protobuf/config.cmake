function(compile_proto output)
	set(GENERATED_CPP_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/cpp)
  foreach(FILENAME ${ARGN})
    get_filename_component(FILE_NAME ${FILENAME} NAME_WE)
    get_filename_component(FILE_DIR ${FILENAME} PATH)
    get_filename_component(FILE_ABS_PATH ${FILENAME} ABSOLUTE)
    get_filename_component(FILE_ABS_DIR ${FILE_ABS_PATH} PATH)

    set(COMMANDS ${COMMANDS} COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_CPP_OUTPUT_DIR}/${FILE_DIR})
		
    set(COMMANDS ${COMMANDS} COMMAND protoc ARGS --proto_path ${FILE_ABS_DIR} --cpp_out ${GENERATED_CPP_OUTPUT_DIR}/${FILE_DIR} ${FILE_ABS_PATH})

    set(SRCS ${SRCS}
      ${GENERATED_CPP_OUTPUT_DIR}/${FILE_DIR}/${FILE_NAME}.pb.cc
      ${GENERATED_CPP_OUTPUT_DIR}/${FILE_DIR}/${FILE_NAME}.pb.h)

    set(DEPENDENCIES ${DEPENDENCIES} ${FILE_ABS_PATH})
  endforeach(FILENAME)

  set(${output} ${SRCS} PARENT_SCOPE)

  add_custom_command(OUTPUT ${SRCS}
    DEPENDS protoc ${DEPENDENCIES}
    ${COMMANDS}
    COMMENT "Running protoc...")
endfunction(compile_proto)
