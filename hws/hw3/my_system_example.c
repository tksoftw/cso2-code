#define _XOPEN_SOURCE 700
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHECK(x) check_internal(#x, x)

void check_internal(const char *msg, bool okay) {
    if (!okay) {
        fprintf(stderr, "CHECK(%s) failed\n", msg);
        abort();
    }
}

int my_system(const char *command) {
    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, (char *) NULL);
        _exit(127);
    } else if (pid > 0) {
        int status;
        pid_t result = waitpid(pid, &status, 0);
        CHECK(result == pid);
        return status;
    } else {
        return -1;
    }
}

#ifdef TESTING
int main(int argc, char **argv) {
    my_system("echo 1 2 3; sleep 2; echo 5 5");
}
#endif
