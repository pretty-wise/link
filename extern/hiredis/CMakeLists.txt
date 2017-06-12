project (hiredis)

include_directories(.)

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
