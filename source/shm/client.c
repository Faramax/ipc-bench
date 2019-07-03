#include <assert.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#define __USE_GNU
#include <pthread.h>

#include "common/common.h"

void cleanup(char* shared_memory) {
	// Detach the shared memory from this process' address space.
	// If this is the last process using this shared memory, it is removed.
	shmdt(shared_memory);
}

void shm_wait(cvar_t* cvar) {
   if (pthread_cond_wait(&cvar->cv, &cvar->m))
      perror("pthread_cond_wait");
   //while (atomic_load(guard) != 'c')
   //   pthread_yield();;
}


void shm_notify(cvar_t* cvar) {
   if (pthread_cond_signal(&cvar->cv))
      perror("pthread_cond_signal");
   //atomic_store(guard, 's');
}

void shm_notifyinit(atomic_char* guard) {
   atomic_store(guard, 's');
}

void shm_wait_for_finish(atomic_char* guard) {
   while (atomic_load(guard) != 'c')
      ;
}

void communicate(char* shared_memory, struct Arguments* args) {
	// Buffer into which to read data
	void* buffer = malloc(args->size);

   atomic_char* guard = (atomic_char*)(shared_memory + sizeof(cvar_t));
   atomic_init(guard, 't');
   assert(sizeof(atomic_char) == 1);

   /* Initialise attribute to mutex. */
   pthread_mutexattr_t attrmutex;
   pthread_mutexattr_init(&attrmutex);
   if (pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED) < 0) {
      perror("mutex_setpshared");
      exit(EXIT_FAILURE);
   }
   /* Initialise attribute to condition. */
   pthread_condattr_t attrcond;
   pthread_condattr_init(&attrcond);
   if (pthread_condattr_setpshared(&attrcond, PTHREAD_PROCESS_SHARED) < 0) {
      perror("cv_setpshared");
      exit(EXIT_FAILURE);
   }

   cvar_t * cvar= (cvar_t *) shared_memory;

   if (pthread_mutex_init(&cvar->m, &attrmutex) < 0) {
      perror("mutex_init");
      exit(EXIT_FAILURE);
   }

   if (pthread_cond_init(&cvar->cv, &attrcond) < 0) {
      perror("cv_init");
      exit(EXIT_FAILURE);
   }

   pthread_mutex_lock(&cvar->m);

   shm_notifyinit(guard);

   size_t sz = args->size - sizeof(cvar_t) - sizeof(atomic_char);
   for (; args->count > 0; --args->count) {
      shm_wait(cvar);
		// Read
      memcpy(buffer, shared_memory + sizeof(cvar_t) + sizeof(atomic_char), sz);

		// Write back
      memset(shared_memory + sizeof(cvar_t)+ sizeof(atomic_char), '*', sz);

      shm_notify(cvar);
	}
   shm_wait_for_finish(guard);
   pthread_cond_destroy(&cvar->cv);
   pthread_condattr_destroy(&attrcond);
   pthread_mutex_destroy(&cvar->m);
   pthread_mutexattr_destroy(&attrmutex);
	free(buffer);
}

int main(int argc, char* argv[]) {
	// The identifier for the shared memory segment
	int segment_id;

	// The *actual* shared memory, that this and other
	// processes can read and write to as if it were
	// any other plain old memory
	char* shared_memory;

	// Key for the memory segment
	key_t segment_key;

	// Fetch command-line arguments
	struct Arguments args;

	parse_arguments(&args, argc, argv);

	segment_key = generate_key("shm");

	/*
		The call that actually allocates the shared memory segment.
		Arguments:
			1. The shared memory key. This must be unique across the OS.
			2. The number of bytes to allocate. This will be rounded up to the OS'
				 pages size for alignment purposes.
			3. The creation flags and permission bits, we pass IPC_CREAT to ensure
				 that the segment will be created if it does not yet exist. Using
				 0666 for permission flags means read + write permission for the user,
				 group and world.
		The call will return the segment ID if the key was valid,
		else the call fails.
	*/
   segment_id = shmget(segment_key, args.size, IPC_CREAT | 0666);

	if (segment_id < 0) {
		throw("Could not get segment");
	}

	/*
	Once the shared memory segment has been created, it must be
	attached to the address space of each process that wishes to
	use it. For this, we pass:
		1. The segment ID returned by shmget
		2. A pointer at which to attach the shared memory segment. This
			 address must be page-aligned. If you simply pass NULL, the OS
			 will find a suitable region to attach the segment.
		3. Flags, such as:
			 - SHM_RND: round the second argument (the address at which to
				 attach) down to a multiple of the page size. If you don't
				 pass this flag but specify a non-null address as second argument
				 you must ensure page-alignment yourself.
			 - SHM_RDONLY: attach for reading only (independent of access bits)
	shmat will return a pointer to the address space at which it attached the
	shared memory. Children processes created with fork() inherit this segment.
*/
	shared_memory = (char*)shmat(segment_id, NULL, 0);

	if (shared_memory < (char*)0) {
		throw("Could not attach segment");
	}

	communicate(shared_memory, &args);

	cleanup(shared_memory);

	return EXIT_SUCCESS;
}
