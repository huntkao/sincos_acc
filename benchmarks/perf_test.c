#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "sincos_neon.h"

#define GRID_SIZE (65 * 65)
#define TOLERANCE 1e-5f

void verify_accuracy(int count, const float* in, const float* s_out, const float* c_out) {
    float max_err_s = 0, max_err_c = 0;
    for (int i = 0; i < count; i++) {
        float err_s = fabsf(s_out[i] - sinf(in[i]));
        float err_c = fabsf(c_out[i] - cosf(in[i]));
        if (err_s > max_err_s) max_err_s = err_s;
        if (err_c > max_err_c) max_err_c = err_c;
    }
    printf("Max Error Sin: %e, Cos: %e\n", max_err_s, max_err_c);
    if (max_err_s > TOLERANCE || max_err_c > TOLERANCE) {
        printf("FAILED: Precision target not met!\n");
    } else {
        printf("PASSED: Precision target met.\n");
    }
}

int main() {
    float *in = malloc(GRID_SIZE * sizeof(float));
    float *s_out = malloc(GRID_SIZE * sizeof(float));
    float *c_out = malloc(GRID_SIZE * sizeof(float));

    if (!in || !s_out || !c_out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < GRID_SIZE; i++) in[i] = (float)i * 0.01f;

    clock_t start = clock();
    sincos_acc_grid(in, s_out, c_out, GRID_SIZE);
    clock_t end = clock();

    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time for %d elements: %f s\n", GRID_SIZE, time_spent);

    verify_accuracy(GRID_SIZE, in, s_out, c_out);

    free(in); free(s_out); free(c_out);
    return 0;
}
