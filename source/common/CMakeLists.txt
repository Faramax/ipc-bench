###########################################################
## SOURCES
###########################################################

set(IPC_BENCH_COMMON_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/utility.c
	${CMAKE_CURRENT_SOURCE_DIR}/benchmarks.c
	${CMAKE_CURRENT_SOURCE_DIR}/signals.c
	${CMAKE_CURRENT_SOURCE_DIR}/arguments.c
	${CMAKE_CURRENT_SOURCE_DIR}/process.c
	${CMAKE_CURRENT_SOURCE_DIR}/sockets.c
	${CMAKE_CURRENT_SOURCE_DIR}/parent.c
)

###########################################################
## TARGETS
###########################################################

add_library(ipc-bench-common STATIC ${IPC_BENCH_COMMON_SOURCES})
target_link_libraries(ipc-bench-common rt)
target_compile_options(ipc-bench-common PUBLIC -fPIC)
