#define _XOPEN_SOURCE 700
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>

__attribute__((noinline)) void empty_function() {
	__asm__("");
}

long long nsecs() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec*1000000000LL + t.tv_nsec;
}

long long get_single_timing(int mode) {
	long long start, end;

	switch (mode) {
		case 0: // overhead measurement
			start = nsecs();
			// do absolutely nothing
			end = nsecs();
			break;
		case 1: 
			start = nsecs();
			empty_function();
			end = nsecs();
			break;
			
		case 2: 
			start = nsecs();
			drand48();
			end = nsecs();
			break;

		case 3: 
			start = nsecs();
			getppid();
			end = nsecs();
			break;

		case 4: {
			start = nsecs();
			pid_t pid = fork();
			end = nsecs();
			if (pid == 0) { _exit(0); }
			waitpid(pid, NULL, 0);
			break;
		}

		case 5: {
			struct timespec sleep_req = {0, 5000000}; // 5ms sleep
			pid_t pid = fork();
			if (pid == 0) { _exit(1337); }
			int status;
			nanosleep(&sleep_req, NULL); 
			start = nsecs();
			waitpid(pid, &status, 0);
			end = nsecs();
			break;
		}

		case 6: {
			start = nsecs();
			pid_t pid = fork();
			if (pid == 0) { 
				_exit(0); // don't exec
			}
			int status;
			waitpid(pid, &status, 0);
			end = nsecs();
			break;
		}

		case 7: {
			int status;
			start = nsecs();
			status = system("/usr/bin/true");
			end = nsecs();
			(void)status;  // suppress unused variable warning
			break;
		}

		case 8:
			start = nsecs();
			mkdir("/tmp/temp_nkq6bw", 0777);
			rmdir("/tmp/temp_nkq6bw");
			end = nsecs();
			break;

		default:
			return 1;
	}
	long long total_time = end - start;
	return total_time;
}

void get_timings(int mode) {
	int N = (mode < 4 && mode > 0) ? 1000000 : 1000;
	int W = N / 10; // warmup iterations

	long double noop_sum = 0.0L;
	long double mode_sum = 0.0L;
	
	for (int i=0; i<W; i++) (void)get_single_timing(0); // warmup
	for (int i=0; i<N; i++) noop_sum += (long double)get_single_timing(0); // use 0
	long double avg_noop = noop_sum / N;

	for (int i=0; i<W; i++) (void)get_single_timing(mode); // warmup
	for (int i=0; i<N; i++) mode_sum += (long double)get_single_timing(mode);
	long double avg_mode = mode_sum / N;

	long double final_time = avg_mode - avg_noop;
	printf("%Lf\n", final_time);
}

int main(int argc, char *argv[]) {
	if (argc != 2) return 1;
	get_timings(atoi(argv[1]));
	return 0;
}
