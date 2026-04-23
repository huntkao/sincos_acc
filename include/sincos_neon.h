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

#ifdef __cplusplus
}
#endif

#endif // SINCOS_NEON_H
