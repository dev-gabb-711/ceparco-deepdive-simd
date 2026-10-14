#include <stddef.h>

double dotProductC(
    const double* a,
    const double* b,
    size_t N
) {
    double sum = 0.0;

    for (size_t i = 0; i < count; i++) {
        sum += src[i] * dest[i];
    }

    return sum;
}