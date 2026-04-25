#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include "sincos_neon.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define TEST_COUNT (260 * 257)
#define ITERATIONS 5000

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

void benchmark_sincos(int count, const float* in) {
    float *s_out = malloc(count * sizeof(float));
    float *c_out = malloc(count * sizeof(float));
    float *s_ref = malloc(count * sizeof(float));
    float *c_ref = malloc(count * sizeof(float));
    volatile double dummy = 0;

    // Accuracy
    sincos_acc_grid(in, s_out, c_out, count);
    double max_err = 0;
    for (int i = 0; i < count; i++) {
        double err_s = fabs((double)s_out[i] - sin((double)in[i]));
        double err_c = fabs((double)c_out[i] - cos((double)in[i]));
        if (err_s > max_err) max_err = err_s;
        if (err_c > max_err) max_err = err_c;
    }

    // Benchmark Scalar
    double start_ref = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 0; i < count; i++) {
            s_ref[i] = sinf(in[i]);
            c_ref[i] = cosf(in[i]);
        }
        dummy += s_ref[0] + c_ref[0];
    }
    double time_ref = (get_time() - start_ref) / ITERATIONS;

    // Benchmark NEON
    double start_neon = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        sincos_acc_grid(in, s_out, c_out, count);
        dummy += s_out[0] + c_out[0];
    }
    double time_neon = (get_time() - start_neon) / ITERATIONS;

    printf("| sincos | %e | %.6f | %.6f | %.2fx |\n", 
           max_err, time_ref, time_neon, time_ref / time_neon);

    free(s_out); free(c_out); free(s_ref); free(c_ref);
}

void benchmark_tan(int count, const float* in) {
    float *out = malloc(count * sizeof(float));
    float *ref = malloc(count * sizeof(float));
    volatile double dummy = 0;

    tan_acc_grid(in, out, count);
    double max_err = 0;
    for (int i = 0; i < count; i++) {
        double err = fabs((double)out[i] - tan((double)in[i]));
        if (err > max_err) max_err = err;
    }

    double start_ref = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 0; i < count; i++) {
            ref[i] = tanf(in[i]);
        }
        dummy += ref[0];
    }
    double time_ref = (get_time() - start_ref) / ITERATIONS;

    double start_neon = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        tan_acc_grid(in, out, count);
        dummy += out[0];
    }
    double time_neon = (get_time() - start_neon) / ITERATIONS;

    printf("| tan    | %e | %.6f | %.6f | %.2fx |\n", 
           max_err, time_ref, time_neon, time_ref / time_neon);

    free(out); free(ref);
}

void benchmark_asin(int count, const float* in) {
    float *out = malloc(count * sizeof(float));
    float *ref = malloc(count * sizeof(float));
    volatile double dummy = 0;

    asin_acc_grid(in, out, count);
    double max_err = 0;
    for (int i = 0; i < count; i++) {
        double err = fabs((double)out[i] - asin((double)in[i]));
        if (err > max_err) max_err = err;
    }

    double start_ref = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 0; i < count; i++) {
            ref[i] = asinf(in[i]);
        }
        dummy += ref[0];
    }
    double time_ref = (get_time() - start_ref) / ITERATIONS;

    double start_neon = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        asin_acc_grid(in, out, count);
        dummy += out[0];
    }
    double time_neon = (get_time() - start_neon) / ITERATIONS;

    printf("| asin   | %e | %.6f | %.6f | %.2fx |\n", 
           max_err, time_ref, time_neon, time_ref / time_neon);

    free(out); free(ref);
}

void benchmark_acos(int count, const float* in) {
    float *out = malloc(count * sizeof(float));
    float *ref = malloc(count * sizeof(float));
    volatile double dummy = 0;

    acos_acc_grid(in, out, count);
    double max_err = 0;
    for (int i = 0; i < count; i++) {
        double err = fabs((double)out[i] - acos((double)in[i]));
        if (err > max_err) max_err = err;
    }

    double start_ref = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 0; i < count; i++) {
            ref[i] = acosf(in[i]);
        }
        dummy += ref[0];
    }
    double time_ref = (get_time() - start_ref) / ITERATIONS;

    double start_neon = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        acos_acc_grid(in, out, count);
        dummy += out[0];
    }
    double time_neon = (get_time() - start_neon) / ITERATIONS;

    printf("| acos   | %e | %.6f | %.6f | %.2fx |\n", 
           max_err, time_ref, time_neon, time_ref / time_neon);

    free(out); free(ref);
}

void benchmark_atan2(int count, const float* y, const float* x) {
    float *out = malloc(count * sizeof(float));
    float *ref = malloc(count * sizeof(float));
    volatile double dummy = 0;

    atan2_acc_grid(y, x, out, count);
    double max_err = 0;
    for (int i = 0; i < count; i++) {
        double err = fabs((double)out[i] - atan2((double)y[i], (double)x[i]));
        if (err > max_err) max_err = err;
    }

    double start_ref = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (int i = 0; i < count; i++) {
            ref[i] = atan2f(y[i], x[i]);
        }
        dummy += ref[0];
    }
    double time_ref = (get_time() - start_ref) / ITERATIONS;

    double start_neon = get_time();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        atan2_acc_grid(y, x, out, count);
        dummy += out[0];
    }
    double time_neon = (get_time() - start_neon) / ITERATIONS;

    printf("| atan2  | %e | %.6f | %.6f | %.2fx |\n", 
           max_err, time_ref, time_neon, time_ref / time_neon);

    free(out); free(ref);
}

int main() {
    float *in_general = malloc(TEST_COUNT * sizeof(float));
    float *in_bounded = malloc(TEST_COUNT * sizeof(float));
    float *in_y = malloc(TEST_COUNT * sizeof(float));
    float *in_x = malloc(TEST_COUNT * sizeof(float));

    for (int i = 0; i < TEST_COUNT; i++) {
        in_general[i] = -2.0f * M_PI + (4.0f * M_PI * i) / TEST_COUNT;
        in_bounded[i] = -1.0f + (2.0f * i) / TEST_COUNT;
        in_y[i] = in_general[i];
        in_x[i] = in_general[TEST_COUNT - 1 - i]; // Mix it up
    }

    printf("\n### Accelerated Math Functions Benchmark Results\n\n");
    printf("| Function | Max Error | Scalar (s) | NEON (s) | Speedup |\n");
    printf("| :--- | :--- | :--- | :--- | :--- |\n");

    benchmark_sincos(TEST_COUNT, in_general);
    benchmark_tan(TEST_COUNT, in_general);
    benchmark_asin(TEST_COUNT, in_bounded);
    benchmark_acos(TEST_COUNT, in_bounded);
    benchmark_atan2(TEST_COUNT, in_y, in_x);

    free(in_general); free(in_bounded); free(in_y); free(in_x);
    return 0;
}
