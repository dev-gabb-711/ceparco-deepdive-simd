
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <windows.h>

#define NUM_SIZES 3
#define RUNS 3

extern double dotProductC(
    const double* a,
    const double* b,
    size_t N
);

static const size_t test_sizes[NUM_SIZES] = {
    (size_t)1 << 20,
    (size_t)1 << 26,
    (size_t)1 << 30
};

void init_vectors(double* a, double* b, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        a[i] = (double)(i % 100 + 1) * 0.01;
        b[i] = (double)(i % 100 + 1) * 0.1;
    }
}

/* Return elapsed time in milliseconds. */
double benchmark(
    const double* a,
    const double* b,
    size_t n,
    double* result
) {
    LARGE_INTEGER frequency, start, end;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    *result = dotProductC(a, b, n);

    QueryPerformanceCounter(&end);

    return ((double)(end.QuadPart - start.QuadPart)
        / (double)frequency.QuadPart) * 1000.0;
}


int main(void)
{
    for (int s = 0; s < NUM_SIZES; s++) {
        size_t N = test_sizes[s];

        if (N > SIZE_MAX / sizeof(double)) {
            fprintf(stderr, "Input size overflow.\n");
            return 1;
        }

        size_t bytes = N * sizeof(double);
        double* arrA = malloc(bytes);
        double* arrB = malloc(bytes);

        if (arrA == NULL || arrB == NULL) {
            fprintf(stderr,
                "Allocation failed for N = %zu.\n", N);
            free(arrA);
            free(arrB);
            continue;
        }

        init_vectors(arrA, arrB, N);

        double times[RUNS];
        double results[RUNS];

        for (int r = 0; r < RUNS; r++) {
            times[r] = benchmark(
                arrA, arrB, N, &results[r]
            );

            printf("Run %d: %.4f ms | Result: %.6f\n",
                r + 1, times[r], results[r]);
        }

        free(arrA);
        free(arrB);
    }

    return 0;
}
