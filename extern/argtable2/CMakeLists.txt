project( argtable2 )

# CMake-specifics
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0015 NEW)
endif(COMMAND cmake_policy)

add_library( argtable2
  src/arg_date.c
  src/arg_dbl.c
  src/arg_end.c
  src/arg_file.c
  src/arg_int.c
  src/arg_lit.c
  src/arg_rem.c
  src/arg_rex.c
  src/arg_str.c
  src/argtable2.c
  src/argtable2.h
  src/getopt.c
  src/getopt.h
  src/getopt1.c 
)

OPTION( HAVE_GETOPT_H "Have the getopt library" 0 )
IF( HAVE_GETOPT_H )
	target_compile_definitions(argtable2 PUBLIC HAVE_GETOPT_H)
ENDIF( HAVE_GETOPT_H )

OPTION( HAVE_INTTYPES_H "Have the file inttypes.h" 1 )
IF( HAVE_INTTYPES_H )
	target_compile_definitions(argtable2 PUBLIC HAVE_INTTYPES_H)
ENDIF( HAVE_INTTYPES_H )

OPTION( HAVE_MEMORY_H "Have the file memory.h" 1 )
IF( HAVE_MEMORY_H )
	target_compile_definitions(argtable2 PUBLIC HAVE_MEMORY_H)
ENDIF( HAVE_MEMORY_H )

OPTION( HAVE_STDLIB_H "Have the file stdlib.h" 1 )
IF( HAVE_STDLIB_H )
	target_compile_definitions(argtable2 PUBLIC HAVE_STDLIB_H)
ENDIF( HAVE_STDLIB_H )

OPTION( HAVE_STRINGS_H "Have the strings.h" 0 )
IF( HAVE_STRINGS_H )
	target_compile_definitions(argtable2 PUBLIC HAVE_STRINGS_H)
ENDIF( HAVE_STRINGS_H )

OPTION( HAVE_STRING_H "Have the file string.h" 1 )
IF( HAVE_STRING_H )
	target_compile_definitions(argtable2 PUBLIC HAVE_STRING_H)
ENDIF( HAVE_STRING_H )

OPTION( HAVE_STDC_HEADERS "Have std C header files" 1 )
IF( HAVE_STDC_HEADERS )
	target_compile_definitions(argtable2 PUBLIC STDC_HEADERS)
ENDIF( HAVE_STDC_HEADERS )

target_include_directories(argtable2 PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/extern/argtable2/src>
	$<INSTALL_INTERFACE:/extern/argtable2/src>
)
