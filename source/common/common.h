#ifndef IPC_BENCH_COMMON_H
#define IPC_BENCH_COMMON_H

#include "common/arguments.h"
#include "common/benchmarks.h"
#include "common/signals.h"
#include "common/utility.h"

typedef struct {
   pthread_cond_t cv;
   pthread_mutex_t m;
} cvar_t;

#endif /* IPC_BENCH_COMMON_H */
