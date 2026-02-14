// This #define tells header files to enable features in the 
// 2008 POSIX.1 standard, including XSI extensions
//
// It _must_ be _before_ any #include.
#define _XOPEN_SOURCE 700

#include <errno.h>
#include <limits.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>

#ifdef __APPLE__
#ifdef CLOCK_UPTIME_RAW
#define WHICH_CLOCK CLOCK_UPTIME_RAW
#elif CLOCK_MONOTONIC_RAW
#define WHICH_CLOCK CLOCK_MONOTONIC_RAW
#endif
#endif

#ifndef WHICH_CLOCK
#define WHICH_CLOCK CLOCK_MONOTONIC
#endif

static long nsec_difference(struct timespec *first, struct timespec *second) {
    return (second->tv_sec - first->tv_sec) * 1000000000L + (second->tv_nsec - first->tv_nsec);
}

static void measure_times_plain(int count, long *starts, long *durations) {
    struct timespec first, current, previous;
    clock_gettime(WHICH_CLOCK, &first);
    clock_gettime(WHICH_CLOCK, &previous);
    for (int i = 0; i < count; i += 1) {
        clock_gettime(WHICH_CLOCK, &current);
        starts[i] = nsec_difference(&first, &previous);
        durations[i] = nsec_difference(&previous, &current);
        previous = current;
    }
}

static void measure_times_threshold(int count, long *starts, long *durations) {
    const char *threshold_string = getenv("THRESHOLD");
    if (!threshold_string) {
        fprintf(stderr, "THRESHOLD environment variable not set (example: THRESHOLD=100 ./empty-time ...)\n");
        exit(EXIT_FAILURE);
    }
    long threshold = atoi(threshold_string);
    struct timespec first, current, previous;
    clock_gettime(WHICH_CLOCK, &first);
    clock_gettime(WHICH_CLOCK, &previous);
    int i = 0;
    while (i < count) {
        clock_gettime(WHICH_CLOCK, &current);
        long difference = nsec_difference(&previous, &current);
        if (difference > threshold) {
            starts[i] = nsec_difference(&first, &previous);
            durations[i] = difference;
            i += 1;
        }
        previous = current;
    }
}

static void output_times(const char *output_file, int count, long *starts, long *durations) {
    FILE *fh = fopen(output_file, "w");
    fprintf(fh, "start,duration\n");
    for (int i = 0; i < count; i += 1) {
        fprintf(fh, "%ld,%ld\n", starts[i], durations[i]);
    }
}

static void *malloc_or_fail(size_t bytes) {
    void *result = malloc(bytes);
    if (!result) {
        fprintf(stderr, "error allocating %ld bytes: %s",
                bytes, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return result;
}

#ifdef __linux__
static void save_proc_interrupts(const char *filename) {
    FILE *out = fopen(filename, "w");
    if (!out) {
        fprintf(stderr, "Error opening '%s': %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    FILE *in = fopen("/proc/interrupts", "r");
    if (!in) {
        fprintf(stderr, "Error opening '/proc/interrupts': %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    char buffer[4096];
    while (1) {
        ssize_t count = fread(buffer, 1, sizeof buffer, in);
        if (count <= 0) break;
        fwrite(buffer, 1, count, out);
    }
    fclose(in);
    fclose(out);
}
#endif

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s LOOP-VERSION COUNT OUTPUTBASE\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *version = argv[1];
    long count = atoi(argv[2]);
    const char *output_base = argv[3];
    char output_file_name[PATH_MAX];
    long *starts = malloc_or_fail(count * sizeof(long));
    long *durations = malloc_or_fail(count * sizeof(long));
    memset(starts , 0, count * sizeof(long));
    memset(durations, 0, count * sizeof(long));
#ifdef __linux__
    /* request that the system keep everything loaded in memory */
    mlockall(MCL_CURRENT | MCL_FUTURE);
#endif
#ifdef __linux__
    snprintf(output_file_name, sizeof output_file_name, "%s-before-int", output_base);
    save_proc_interrupts(output_file_name);
#endif
    if (0 == strcmp(version, "plain")) {
        measure_times_plain(count, starts, durations);
    } else if (0 == strcmp(version, "threshold")) {
        measure_times_threshold(count, starts, durations);
    } else {
        fprintf(stderr, "Unsupported loop version %s (use 'plain' or 'threshold')\n", version);
        return EXIT_FAILURE;
    }
#ifdef __linux__
    snprintf(output_file_name, sizeof output_file_name, "%s-after-int", output_base);
    save_proc_interrupts(output_file_name);
#endif
    snprintf(output_file_name, sizeof output_file_name, "%s-times.csv", output_base);
    output_times(output_file_name, count, starts, durations);
    return EXIT_SUCCESS;
}
