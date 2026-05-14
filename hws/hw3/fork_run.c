#define _XOPEN_SOURCE 700
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

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

int my_itoa(int n, char buf[static 11]) {
    if (n < 0) { buf[0] = '\0';  return -1; } // only allow + and 0 case
    // largest int: 2^31-1 = 2147483647 = 10 digits
    // total size = 10 chars + '\0' = 11 chars
    return snprintf(buf, 11, "%d", n);
}

/*  NOTE: here,
    char *argv_base[] is the same thing as 
    char **argv_base
    since arrays-as-parameters are converted into
    a pointer of the first element in C
*/  
int execv_with_index(const char *argv_base[], int index) 
{
    // need to add an additional argument before the NULL
    int count = 0;
    while (argv_base[count] != NULL) count++;

    const char **argv_modified = (const char**)malloc((count+2)*sizeof(const char*));
    for (int i = 0; i < count; i++) argv_modified[i] = argv_base[i];

    char index_str[11];
    my_itoa(index, index_str);
    argv_modified[count] = index_str;
    argv_modified[count+1] = NULL;

    //for (int i = 0; i < count; i++) printf("%s", argv_modified[i]);

    execv(argv_modified[0], (char *const *)argv_modified);

    // execv failed
    free(argv_modified);
    return -1;
}

void writeoutput(const char *command, const char *out_path, const char *err_path) {
    int fd1 = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    int fd2 = open(err_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    pid_t pid = fork();
    if (pid == 0) {
        dup2(fd1, STDOUT_FILENO);
        dup2(fd2, STDERR_FILENO);
        close(fd1);
        close(fd2);
        my_system(command);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        _exit(0);
    } else {
        close(fd1);
        close(fd2);
        // my_system(command);
        waitpid(pid, NULL, 0);
    }
}

void parallelwriteoutput(int count, const char **argv_base, const char *out_file) {
    /*  basically just run the same command and arguments
        <count> times with the index of the child as the
        last argument of each 
    */
    int fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            dup2(fd, STDOUT_FILENO);
            close(fd);
            int ok = execv_with_index(argv_base, i);
            CHECK(ok > 0); // check for execv fail
            close(STDOUT_FILENO);
            _exit(127);
        }
    }
    close(fd);
    while(waitpid(-1, NULL, 0) > 0); // wait for all child processes
}

//#define TESTING6

#ifdef TESTING
int main(int argc, char **argv) {
    writeoutput("echo 1 2 3; sleep 2; echo 4 5", "out.txt", "err.txt");
    return 0;
}

#elif defined(TESTING2)
int main(int argc, char **argv) {
    writeoutput("echo hello; echo error 1>&2", "out.txt", "err.txt");
    return 0;
}

#elif defined(TESTING3)
int main(int argc, char **argv) {
    const char *args[] = { "/bin/echo", "child", NULL };
    parallelwriteoutput(3, args, "par.txt");
    return 0;
}

#elif defined(TESTING4)
int main(int argc, char **argv) {
    printf("args = [");
    for (int i = 0; i < argc; i += 1)
        printf("'%s'%s", argv[i], i == argc - 1 ? "" : ", ");
    printf("]\n");

    
    if (argc < 8) {
        parallelwriteoutput(argc, (const char**) argv, "test.txt"); // fork bomb
    }
    else { // don't die
        dprintf(STDERR_FILENO, "PID %d: System out of resources!\n", getpid());
    }
    return 0;
}

#elif defined(TESTING5)
int main(int argc, char **argv) {
    const char *args[] = {"/bin/echo", NULL};
    parallelwriteoutput(100, args, "test2.txt");
    return 0;
}

#elif defined(TESTING6)
int main(int argc, char **argv) {
    int n = 999999;
    char *args[n];
    for (int i = 0; i < n; i++) args[i] = "0";
    args[0] = "/bin/echo";
    args[n-1] = NULL;
    parallelwriteoutput(100, (const char**) args, "test3.txt");
    return 0;
}

#else
int main(int argc, char **argv) {
    return 0;
}
#endif