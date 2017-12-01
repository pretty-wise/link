project (hiredis)

add_library (hiredis
	async.c
	async.h
	dict.c
	dict.h
	fmacros.h
	hiredis.c
	hiredis.h
	net.c
	net.h
	read.c
	read.h
	sds.c
	sds.h
	sdsalloc.h
	test.c
)

target_include_directories(hiredis PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/extern/hiredis>
	$<INSTALL_INTERFACE:/extern/hiredis>
)
