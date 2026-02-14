#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

int my_system(const char *command) {
	pid_t pid = fork();
	if (pid == 0) {
		char* path = "/bin/bash";
		char *ARGS[] = {path, "-c", strdup(command), NULL}; 
		execv(path, ARGS);
		_exit(0);	
	}
	else {
		int status;
		waitpid(pid, &status, 0);		
		return status;
	}
}


int main(int argc, char *argv[]) {
	int a1 = my_system("sleep 1; echo hi");
    int a2 = my_system("echo bye");
    int a3 = my_system("flibbertigibbet 23");
	return 0;
}

