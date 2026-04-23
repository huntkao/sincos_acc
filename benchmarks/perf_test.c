#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "sincos_neon.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#include <sys/time.h>

#define TEST_COUNT (260 * 257)
#define ITERATIONS 10000        // Increased iterations

typedef struct {
    double max_err;
    double mean_abs_err;
    double rmse;
} stats_t;

stats_t calculate_stats(int count, const float* in, const float* s_out, const float* c_out) {
    stats_t s = {0, 0, 0};
    double sum_abs_err = 0;
    double sum_sq_err = 0;

    for (int i = 0; i < count; i++) {
        double err_s = fabs((double)s_out[i] - sin((double)in[i]));
        double err_c = fabs((double)c_out[i] - cos((double)in[i]));
        double err = (err_s > err_c) ? err_s : err_c;

        if (err > s.max_err) s.max_err = err;
        sum_abs_err += err;
        sum_sq_err += err * err;
    }
    s.mean_abs_err = sum_abs_err / count;
    s.rmse = sqrt(sum_sq_err / count);
    return s;
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

int main() {
    float *in = malloc(TEST_COUNT * sizeof(float));
    float *s_out = malloc(TEST_COUNT * sizeof(float));
    float *c_out = malloc(TEST_COUNT * sizeof(float));
    float *s_ref = malloc(TEST_COUNT * sizeof(float));
    float *c_ref = malloc(TEST_COUNT * sizeof(float));

    if (!in || !s_out || !c_out || !s_ref || !c_ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Coverage: -2*PI to 2*PI
    for (int i = 0; i < TEST_COUNT; i++) {
        in[i] = -2.0f * M_PI + (4.0f * M_PI * i) / TEST_COUNT;
    }

    printf("Benchmarking... please wait.\n");

    // Benchmark Scalar (Reference)
    double dummy = 0;
    double start_ref = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 0; i < TEST_COUNT; i++) {
            s_ref[i] = sinf(in[i]);
            c_ref[i] = cosf(in[i]);
            dummy += (double)s_ref[i] + (double)c_ref[i];
        }
    }
    double end_ref = get_time();
    double time_ref = (end_ref - start_ref) / ITERATIONS;

    // Benchmark NEON
    double dummy_neon = 0;
    double start_neon = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        sincos_acc_grid(in, s_out, c_out, TEST_COUNT);
        dummy_neon += (double)s_out[0] + (double)c_out[0];
    }
    double end_neon = get_time();
    double time_neon = (end_neon - start_neon) / ITERATIONS;
    
    // Use dummy to prevent optimized out
    if (dummy == 1.23456789) printf(" "); 
    if (dummy_neon == 1.23456789) printf(" ");

    // Output Results
    stats_t res = calculate_stats(TEST_COUNT, in, s_out, c_out);
    printf("\n--- Precision Analysis (-2PI to 2PI) ---\n");
    printf("Max Absolute Error:  %e\n", res.max_err);
    printf("Mean Absolute Error: %e\n", res.mean_abs_err);
    printf("RMSE:                %e\n", res.rmse);

    printf("\n--- Performance Analysis (%d elements) ---\n", TEST_COUNT);
    printf("Scalar Time: %f s (Throughput: %.2f M/s)\n", time_ref, (TEST_COUNT / time_ref) / 1e6);
    printf("NEON Time:   %f s (Throughput: %.2f M/s)\n", time_neon, (TEST_COUNT / time_neon) / 1e6);
    printf("Speedup Ratio: %.2fx\n", time_ref / time_neon);

    free(in); free(s_out); free(c_out); free(s_ref); free(c_ref);
    return 0;
}
