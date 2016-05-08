project (protobuf)

include_directories(src)
include_directories(.)

add_library (protobuf
	src/google/protobuf/descriptor.cc
	src/google/protobuf/descriptor.pb.cc
	src/google/protobuf/descriptor_database.cc
	src/google/protobuf/dynamic_message.cc
	src/google/protobuf/extension_set.cc
	src/google/protobuf/extension_set_heavy.cc
	src/google/protobuf/generated_message_reflection.cc
	src/google/protobuf/generated_message_util.cc
	src/google/protobuf/io/coded_stream.cc
	src/google/protobuf/io/gzip_stream.cc
	src/google/protobuf/io/printer.cc
	src/google/protobuf/io/strtod.cc
	src/google/protobuf/io/tokenizer.cc
	src/google/protobuf/io/zero_copy_stream.cc
	src/google/protobuf/io/zero_copy_stream_impl.cc
	src/google/protobuf/io/zero_copy_stream_impl_lite.cc
	src/google/protobuf/message.cc
	src/google/protobuf/message_lite.cc
	src/google/protobuf/reflection_ops.cc
	src/google/protobuf/repeated_field.cc
	src/google/protobuf/service.cc
	src/google/protobuf/stubs/atomicops_internals_x86_gcc.cc
	src/google/protobuf/stubs/common.cc
	src/google/protobuf/stubs/once.cc
	src/google/protobuf/stubs/stringprintf.cc
	src/google/protobuf/stubs/structurally_valid.cc
	src/google/protobuf/stubs/strutil.cc
	src/google/protobuf/stubs/substitute.cc
	src/google/protobuf/text_format.cc
	src/google/protobuf/unknown_field_set.cc
	src/google/protobuf/wire_format.cc
	src/google/protobuf/wire_format_lite.cc
#	src/google/protobuf/descriptor_database_unittest.cc
#	src/google/protobuf/descriptor_unittest.cc
#	src/google/protobuf/dynamic_message_unittest.cc
#	src/google/protobuf/extension_set_unittest.cc
#	src/google/protobuf/generated_message_reflection_unittest.cc
#	src/google/protobuf/io/coded_stream_unittest.cc
#	src/google/protobuf/io/printer_unittest.cc
#	src/google/protobuf/io/tokenizer_unittest.cc
#	src/google/protobuf/io/zero_copy_stream_unittest.cc
#	src/google/protobuf/lite_unittest.cc
#	src/google/protobuf/message_unittest.cc
#	src/google/protobuf/reflection_ops_unittest.cc
#	src/google/protobuf/repeated_field_reflection_unittest.cc
#	src/google/protobuf/repeated_field_unittest.cc
#	src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc
#	src/google/protobuf/stubs/common_unittest.cc
#	src/google/protobuf/stubs/once_unittest.cc
#	src/google/protobuf/stubs/stringprintf_unittest.cc
#	src/google/protobuf/stubs/structurally_valid_unittest.cc
#	src/google/protobuf/stubs/strutil_unittest.cc
#	src/google/protobuf/stubs/template_util_unittest.cc
#	src/google/protobuf/stubs/type_traits_unittest.cc
#	src/google/protobuf/test_util.cc
#	src/google/protobuf/test_util_lite.cc
#	src/google/protobuf/testing/file.cc
#	src/google/protobuf/testing/googletest.cc
#	src/google/protobuf/testing/zcgunzip.cc
#	src/google/protobuf/testing/zcgzip.cc
#	src/google/protobuf/text_format_unittest.cc
#	src/google/protobuf/unknown_field_set_unittest.cc
#	src/google/protobuf/wire_format_unittest.cc
)

add_executable(protoc
	src/google/protobuf/compiler/code_generator.cc
	src/google/protobuf/compiler/command_line_interface.cc
	src/google/protobuf/compiler/cpp/cpp_enum.cc
	src/google/protobuf/compiler/cpp/cpp_enum_field.cc
	src/google/protobuf/compiler/cpp/cpp_extension.cc
	src/google/protobuf/compiler/cpp/cpp_field.cc
	src/google/protobuf/compiler/cpp/cpp_file.cc
	src/google/protobuf/compiler/cpp/cpp_generator.cc
	src/google/protobuf/compiler/cpp/cpp_helpers.cc
	src/google/protobuf/compiler/cpp/cpp_message.cc
	src/google/protobuf/compiler/cpp/cpp_message_field.cc
	src/google/protobuf/compiler/cpp/cpp_primitive_field.cc
	src/google/protobuf/compiler/cpp/cpp_service.cc
	src/google/protobuf/compiler/cpp/cpp_string_field.cc
	src/google/protobuf/compiler/importer.cc
	src/google/protobuf/compiler/main.cc
	src/google/protobuf/compiler/parser.cc
	src/google/protobuf/compiler/plugin.cc
	src/google/protobuf/compiler/plugin.pb.cc
	src/google/protobuf/compiler/python/python_generator.cc
	src/google/protobuf/compiler/subprocess.cc
	src/google/protobuf/compiler/zip_writer.cc
	src/google/protobuf/compiler/java/java_context.cc
	src/google/protobuf/compiler/java/java_doc_comment.cc
	src/google/protobuf/compiler/java/java_enum.cc
	src/google/protobuf/compiler/java/java_enum_field.cc
	src/google/protobuf/compiler/java/java_extension.cc
	src/google/protobuf/compiler/java/java_field.cc
	src/google/protobuf/compiler/java/java_file.cc
	src/google/protobuf/compiler/java/java_generator.cc
	src/google/protobuf/compiler/java/java_generator_factory.cc
	src/google/protobuf/compiler/java/java_helpers.cc
	src/google/protobuf/compiler/java/java_lazy_message_field.cc
	src/google/protobuf/compiler/java/java_message.cc
	src/google/protobuf/compiler/java/java_message_field.cc
	src/google/protobuf/compiler/java/java_name_resolver.cc
	src/google/protobuf/compiler/java/java_primitive_field.cc
	src/google/protobuf/compiler/java/java_service.cc
	src/google/protobuf/compiler/java/java_shared_code_generator.cc
	src/google/protobuf/compiler/java/java_string_field.cc
#	src/google/protobuf/compiler/java/java_doc_comment_unittest.cc
#	src/google/protobuf/compiler/java/java_plugin_unittest.cc
#	src/google/protobuf/compiler/command_line_interface_unittest.cc
#	src/google/protobuf/compiler/cpp/cpp_bootstrap_unittest.cc
#	src/google/protobuf/compiler/cpp/cpp_plugin_unittest.cc
#	src/google/protobuf/compiler/cpp/cpp_unittest.cc
#	src/google/protobuf/compiler/importer_unittest.cc
#	src/google/protobuf/compiler/mock_code_generator.cc
#	src/google/protobuf/compiler/parser_unittest.cc
#	src/google/protobuf/compiler/python/python_plugin_unittest.cc
#	src/google/protobuf/compiler/test_plugin.cc
)

target_link_libraries(protoc protobuf)