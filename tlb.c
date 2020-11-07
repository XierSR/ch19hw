#define _GNU_SOURCE 
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>

#define SAMPLES 0x09000000
#define BASE_10 10
#define MS_PER_SEC 1000000

double run_trials(int page_size, int num_pages, int trials);

int main(int argc, char *argv[]) {
    long pages, numtrials;
    double total;
    int page_size;
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset)) {
        fprintf(stderr, "Error setting CPU affinity\n");
        exit(EXIT_FAILURE);
    }

    if (argc != 3) {
        fprintf(stderr, "Usage: ./tlb <int: pages-to-touch> <int: numtrials>\n");
        exit(EXIT_FAILURE);
    }
    
    page_size = getpagesize();
    pages = strtol(argv[1], NULL, BASE_10);
    trials = strtol(argv[2], NULL, BASE_10);

    if (!pages || !numtrials) {
        fprintf(stderr, "Usage: ./tlb <int: pages-to-touch> <int: numtrials>\n");
        exit(EXIT_FAILURE);
    }

    total = run_trials(page_size, pages, numtrials);
    printf("%ld\t%ld\t%f\n", pages, numtrials, total);
    return 0;
}

double run_trials(int page_size, int num_pages, int numtrials) {
    struct timeval start, end;
    uint64_t trial_time = 0;
    int tod1, tod2, elems, jump, i, j;
    int *arr, arr_size;
 
    elems = (page_size * num_pages) / sizeof(int);
    arr = (int*) calloc(elems, sizeof(int));

    jump = page_size / sizeof(int);

    tod1 = gettimeofday(&start, NULL);
    for (j = 0; j < trials; j++) {
        for (i = 0; i < num_pages * jump; i += jump) {
            arr[i] += 1;                        
        }
    }
    tod2 = gettimeofday(&end, NULL);
    if (tod1 == -1 || tod2 == -1) {
        fprintf(stderr, "failed benchmarking time\n");
        exit(EXIT_FAILURE);
    }
    trial_time = (end.tv_sec - start.tv_sec) * MS_PER_SEC + end.tv_usec - start.tv_usec;
    free(arr);
    return trial_time / (double) (num_pages * numtrials);
}
