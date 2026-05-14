#include <stdio.h> // fopen, fread, fclose, printf, fseek, ftell
#include <math.h> // log, exp
#include <stdlib.h> // free, realloc
#include <time.h> // struct timespec, clock_gettime, CLOCK_REALTIME
#include <errno.h>
#include <string.h>
#include <omp.h>

// computes the geometric mean of a set of values.
// You should use OpenMP to make faster versions of this.
// Keep the underlying sum-of-logs approach.
double geomean0(unsigned char *s, size_t n) {
    double answer = 0;
	for(int i=0; i<n; i+=1) {
        if (s[i] > 0) answer += log(s[i]) / n;
    }
    return exp(answer);
}

double geomean1(unsigned char *s, size_t n) {
    double answer = 0;
	#pragma omp parallel for reduction(+:answer)
	for(int i=0; i<n; i+=1) {
        if (s[i] > 0) answer += log(s[i]);
    }
    return exp(answer/n);
}

double geomean2(unsigned char *s, size_t n) {
    double answer = 0;
	int j = 0;
	#pragma omp parallel
	while (1) {
		int i;
		#pragma omp atomic capture
		i = j++;
		if (i >= n) break;
		if (s[i] > 0) {
			double v = log(s[i]);
			#pragma omp atomic
			answer += v;
		}
	}
    return exp(answer/n);
}


double geomean3(unsigned char *s, size_t n) {
    double answer = 0;
	int j = 0;
	int K = 1024;
	#pragma omp parallel
	{
		double local = 0;
		while (1) {
			int i;
			#pragma omp atomic capture
			i = j += K;
			if (i - K >= n) break;
			int start = i - K;
			int end = i;
			if (end > n) end = n;

			for (int idx = start; idx < end; idx++) {
				if (s[idx] > 0) local += log(s[idx]);
			}
		}
		#pragma omp atomic
		answer += local;
	}
	return exp(answer/n);
}

double geomean4(unsigned char *s, size_t n) {
    double answer = 0;

    #pragma omp parallel for reduction(+:answer) schedule(dynamic)
    for (int i = 0; i < n; i++) {
        if (s[i] > 0) answer += log(s[i]);
    }

    return exp(answer / n);
}

double geomean5(unsigned char *s, size_t n) {
    double answer = 0;

    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (s[i] > 0) {
            double val = log(s[i]);
            #pragma omp atomic
            answer += val;
        }
    }

    return exp(answer / n);
}

double (*get_geomean())(unsigned char*, size_t) {
    char *env = getenv("GEOMEAN");

    if (!env) return geomean0;

    if (strcmp(env, "0") == 0) return geomean0;
    if (strcmp(env, "1") == 0) return geomean1;
    if (strcmp(env, "2") == 0) return geomean2;
    if (strcmp(env, "3") == 0) return geomean3; 
    if (strcmp(env, "4") == 0) return geomean4;	
    if (strcmp(env, "5") == 0) return geomean5;
	
    return geomean0;
}

// nanoseconds that have elapsed since 1970-01-01 00:00:00 UTC
long long nsecs() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec*1000000000 + t.tv_nsec;
}


/// reads arguments and invokes geomean; should not require editing
int main(int argc, char *argv[]) {
    // step 1: get the input array (the bytes in this file)
    char *s = NULL;
    size_t n = 0;
    double (*geomean)(unsigned char*, size_t) = get_geomean();
	for(int i=1; i<argc; i+=1) {
        // add argument i's file contents (or string value) to s
        FILE *f = fopen(argv[i], "rb");
        if (f) { // was a file; read it
            fseek(f, 0, SEEK_END); // go to end of file
            size_t size = ftell(f); // find out how many bytes in that was
            fseek(f, 0, SEEK_SET); // go back to beginning
            s = realloc(s, n+size); // make room
            fread(s+n, 1, size, f); // append this file on end of others
            fclose(f);
            n += size; // not new size
        } else { // not a file; treat as a string
            errno = 0; // clear the read error
        }
    }

    // step 2: invoke and time the geometric mean function
    long long t0 = nsecs();
    double answer = geomean((unsigned char*) s,n);
    long long t1 = nsecs();

    free(s);

    // step 3: report result
    printf("%lld ns to process %zd characters: %g\n", t1-t0, n, answer);
}
