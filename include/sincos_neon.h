#ifndef SINCOS_NEON_H
#define SINCOS_NEON_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Accelerated sine and cosine for arbitrary grid sizes.
 * @param input   Pointer to input float array.
 * @param sin_out Pointer to output sine array.
 * @param cos_out Pointer to output cosine array.
 * @param count   Total number of elements.
 */
void sincos_acc_grid(const float* input, float* sin_out, float* cos_out, int count);

/**
 * @brief Accelerated tangent for arbitrary grid sizes.
 */
void tan_acc_grid(const float* input, float* output, int count);

/**
 * @brief Accelerated arcsine for arbitrary grid sizes.
 */
void asin_acc_grid(const float* input, float* output, int count);

/**
 * @brief Accelerated arccosine for arbitrary grid sizes.
 */
void acos_acc_grid(const float* input, float* output, int count);

/**
 * @brief Accelerated arctangent2 for arbitrary grid sizes.
 * @param y      Pointer to y-coordinate array.
 * @param x      Pointer to x-coordinate array.
 * @param output Pointer to output angle array.
 * @param count  Total number of elements.
 */
void atan2_acc_grid(const float* y, const float* x, float* output, int count);

#ifdef __cplusplus
}
#endif

#endif // SINCOS_NEON_H
