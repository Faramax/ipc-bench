#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>

#include "common/utility.h"

char* str_r_str(char* str, const char* target)
{
   char* str_end = str;
   while(*str_end != 0) ++str_end;
   --str_end;
   char const* target_end = target;
   while(*target_end != 0) ++target_end;
   --target_end;
   do
   {
      char* str_local_end = str_end;
      char const* target_local_end = target_end;
      while(*str_local_end == *target_local_end)
      {
         if(target_local_end == target)
            return str_local_end;
         if(str_local_end == str)
            return NULL;
         --target_local_end;
         --str_local_end;
      }
      --str_end;
   }
   while(str_end != str);
   return NULL;
}

char *find_build_path()
{
   enum {max_path_size = 200};
   char* buffer = (char *)malloc(max_path_size);
   char* path = getcwd(buffer, max_path_size);
   if(path == NULL)
      throw("Error finding build path");

   char const marker[] = "/source";
   char* const path_end = str_r_str(path, marker);
   if(path_end)
   {
      *(path_end + sizeof(marker) - 1) = 0;
      return path;
   }
   return NULL;
}


void start_process(char *argv[]) {
	// Will need to set the group id
	const pid_t parent_pid = getpid();
   const pid_t pid = fork();

	if (pid == 0) {
		// Set group id of the children so that we
		// can send around signals
		if (setpgid(pid, parent_pid) == -1) {
			throw("Could not set group id for child process");
		}
		// Replace the current process with the command
		// we want to execute (child or server)
		// First argument is the command to call,
		// second is an array of arguments, where the
		// command path has to be included as well
		// (that's why argv[0] first)
		if (execv(argv[0], argv) == -1) {
			throw("Error opening child process");
		}
	}
}

void copy_arguments(char *arguments[], int argc, char *argv[]) {
	int i;
	assert(argc < 8);
	for (i = 1; i < argc; ++i) {
		arguments[i] = argv[i];
	}

	arguments[argc] = NULL;
}

void start_child(char *name, int argc, char *argv[]) {
	char *arguments[8] = {name};
	copy_arguments(arguments, argc, argv);
	start_process(arguments);
}

void start_children(char *prefix, int argc, char *argv[]) {
   char server_name[200];
   char client_name[200];

	char *build_path = find_build_path();

	// clang-format off
	sprintf(
		server_name,
		"%s/%s/%s-%s",
		build_path,
		prefix,
		prefix,
		"server"
	);

	sprintf(
		client_name,
		"%s/%s/%s-%s",
		build_path,
		prefix,
		prefix,
		"client"
	);
	// clang-format on

	start_child(server_name, argc, argv);
	start_child(client_name, argc, argv);

	free(build_path);
}
