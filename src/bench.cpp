#include "../include/benc.h"

#include <time.h>
#include <stdio.h>

static clock_t bench_mark_clock;
void begin_debug_benchmark() {
    bench_mark_clock = clock();
}

float end_debug_benchmark(const char* label) {
    clock_t end = clock();
    double time_spent = (double)(end - bench_mark_clock);
    printf("Benchmark time for %s is %f ms.\n", label, time_spent);

    return (float) time_spent;
}