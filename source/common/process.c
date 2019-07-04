#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>

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

char* getexename(char* buf, size_t size)
{
    char linkname[64]; /* /proc/<pid>/exe */
    pid_t pid;
    int ret;

    /* Get our PID and build the name of the link in /proc */
    pid = getpid();

    if (snprintf(linkname, sizeof(linkname), "/proc/%i/exe", pid) < 0)
        {
        /* This should only happen on large word systems. I'm not sure
           what the proper response is here.
           Since it really is an assert-like condition, aborting the
           program seems to be in order. */
        abort();
        }


    /* Now read the symbolic link */
    ret = readlink(linkname, buf, size);

    /* In case of an error, leave the handling up to the caller */
    if (ret == -1)
        return NULL;

    /* Report insufficient buffer size */
    if (ret >= size)
        {
        errno = ERANGE;
        return NULL;
        }

    /* Ensure proper NUL termination */
    buf[ret] = 0;

    return buf;
}

char *find_build_path()
{
   enum {max_path_size = 200};
   char* buffer = (char *)malloc(max_path_size);
   char* exe_name = getexename(buffer, max_path_size);
   if(exe_name == NULL)
      return NULL;
   char const marker[] = "/source";
   char* const path_end = str_r_str(exe_name, marker);
   if(path_end)
   {
      *(path_end + sizeof(marker) - 1) = 0;
      return exe_name;
   }
   return NULL;
}


void start_process(char *argv[]) {
   // Set group id of parent and children so that we
   // can send around signals
   if (setpgid(0, 0) == -1) {
      throw("Could not set group id for child process");
   }
   const pid_t pid = fork();

	if (pid == 0) {
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

   assert(argc);
   char *build_path = find_build_path();
   if(build_path == NULL)
      throw("Error finding build path");

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
