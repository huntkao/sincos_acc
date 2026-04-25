#ifndef __ARM_NEON
#error "ARM NEON support is required for this implementation."
#endif

#include "sincos_neon.h"
#include "neon_mathfun.h"
#include <arm_neon.h>
#include <math.h>

void sincos_acc_grid(const float* input, float* sin_out, float* cos_out, int count) {
    int i = 0;
    int vec_count = (count / 4) * 4;

    // Main Vector Loop
    for (; i < vec_count; i += 4) {
        float32x4_t v_in = vld1q_f32(&input[i]);
        float32x4_t v_sin, v_cos;
        
        // Using neon_mathfun's sincos_ps for efficiency
        sincos_ps(v_in, &v_sin, &v_cos);
        
        vst1q_f32(&sin_out[i], v_sin);
        vst1q_f32(&cos_out[i], v_cos);
    }

    // Scalar Tail Handling
    for (; i < count; i++) {
        sin_out[i] = sinf(input[i]);
        cos_out[i] = cosf(input[i]);
    }
}

void tan_acc_grid(const float* input, float* output, int count) {
    int i = 0;
    int vec_count = (count / 4) * 4;
    for (; i < vec_count; i += 4) {
        float32x4_t v_in = vld1q_f32(&input[i]);
        vst1q_f32(&output[i], tan_ps(v_in));
    }
    for (; i < count; i++) {
        output[i] = tanf(input[i]);
    }
}

void asin_acc_grid(const float* input, float* output, int count) {
    int i = 0;
    int vec_count = (count / 4) * 4;
    for (; i < vec_count; i += 4) {
        float32x4_t v_in = vld1q_f32(&input[i]);
        vst1q_f32(&output[i], asin_ps(v_in));
    }
    for (; i < count; i++) {
        output[i] = asinf(input[i]);
    }
}

void acos_acc_grid(const float* input, float* output, int count) {
    int i = 0;
    int vec_count = (count / 4) * 4;
    for (; i < vec_count; i += 4) {
        float32x4_t v_in = vld1q_f32(&input[i]);
        vst1q_f32(&output[i], acos_ps(v_in));
    }
    for (; i < count; i++) {
        output[i] = acosf(input[i]);
    }
}

void atan2_acc_grid(const float* y, const float* x, float* output, int count) {
    int i = 0;
    int vec_count = (count / 4) * 4;
    for (; i < vec_count; i += 4) {
        float32x4_t v_y = vld1q_f32(&y[i]);
        float32x4_t v_x = vld1q_f32(&x[i]);
        vst1q_f32(&output[i], atan2_ps(v_y, v_x));
    }
    for (; i < count; i++) {
        output[i] = atan2f(y[i], x[i]);
    }
}
