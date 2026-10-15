#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

#define NUM_RUNS 30
#define NUM_SIZES 4

extern double dotProductC(const double* a, const double* b, size_t N);
extern double dotProductx86(const double* a, const double* b, size_t N);

// Test sizes: Boundary size 1003, 2^20, 2^24, 2^28
static const size_t test_sizes[NUM_SIZES] = {
    1003,
    (size_t)1 << 20,
    (size_t)1 << 24,
    (size_t)1 << 28
};

void init_vectors(double* a, double* b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        a[i] = sin((double)i * 0.001);
        b[i] = cos((double)i * 0.002);
    }
}

double time_kernel(double (*kernel)(const double*, const double*, size_t), const double* a, const double* b, size_t n, double* out_res) {
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);

    double total_ms = 0.0;
    *out_res = kernel(a, b, n); // Warm-up run

    for (int r = 0; r < NUM_RUNS; r++) {
        QueryPerformanceCounter(&start);
        *out_res = kernel(a, b, n);
        QueryPerformanceCounter(&end);
        total_ms += ((double)(end.QuadPart - start.QuadPart) / freq.QuadPart) * 1000.0;
    }
    return total_ms / NUM_RUNS;
}

int main(void) {
#ifdef _DEBUG
    printf("========================================================================================\n");
    printf("                          BUILD CONFIGURATION: DEBUG                                    \n");
    printf("========================================================================================\n\n");
#else
    printf("========================================================================================\n");
    printf("                          BUILD CONFIGURATION: RELEASE                                  \n");
    printf("========================================================================================\n\n");
#endif

    for (int s = 0; s < NUM_SIZES; s++) {
        size_t N = test_sizes[s];
        size_t bytes = N * sizeof(double);

        double* a = (double*)malloc(bytes);
        double* b = (double*)malloc(bytes);
        if (!a || !b) {
            printf("Allocation failed for N = %zu\n", N);
            if (a) free(a);
            if (b) free(b);
            continue;
        }

        init_vectors(a, b, N);

        double resC = 0.0, resX86 = 0.0;
        double timeC = time_kernel(dotProductC, a, b, N, &resC);
        double timeX86 = time_kernel(dotProductx86, a, b, N, &resX86);

        int pass = (fabs(resX86 - resC) < 1e-3) || (fabs(resX86 - resC) / fabs(resC) < 1e-4);

        printf("----------------------------------------------------------------------------------------\n");
        printf(" Vector Size N = %zu\n", N);
        printf("----------------------------------------------------------------------------------------\n");
        printf(" %-15s | %-15s | %-20s | %-10s\n", "Kernel", "Avg Time (ms)", "Result Z", "Match");
        printf("----------------------------------------------------------------------------------------\n");
        printf(" %-15s | %-15.4f | %-20.6f | %-10s\n", "C Baseline", timeC, resC, "REF");
        printf(" %-15s | %-15.4f | %-20.6f | %-10s\n", "x86-64 Scalar", timeX86, resX86, pass ? "PASS" : "FAIL");
        printf("----------------------------------------------------------------------------------------\n\n");

        free(a);
        free(b);
    }
    return 0;
}