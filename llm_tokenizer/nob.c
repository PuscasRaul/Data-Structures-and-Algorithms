
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX

#include "nob.h"

#define BUILD_FOLDER "build/"

int main (int argc, char * argv[]) {
	NOB_GO_REBUILD_URSELF(argc, argv);

	Nob_Cmd cmd = {0};
	if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) {
		return 1;
	}

	nob_cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-ggdb", "-o", BUILD_FOLDER"bpe", "bpe.c");
	if (!nob_cmd_run_sync_and_reset(&cmd)) {
		return 1;
	}

	return 0;
} 


