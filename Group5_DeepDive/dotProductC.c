#include <stddef.h>

double dotProductC(const double* a, const double* b, size_t N) {
    double sum = 0.0;
    for (size_t i = 0; i < N; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}