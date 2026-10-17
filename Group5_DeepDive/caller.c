#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

#define NUM_RUNS 30
#define NUM_SIZES 4

extern double dotProductC(const double* a, const double* b, size_t N);
extern double dotProductx86(const double* a, const double* b, size_t N);
extern double dotProductXMM(const double* a, const double* b, size_t N);
extern double dotProductYMM(const double* a, const double* b, size_t N);


static const size_t test_sizes[NUM_SIZES] = {
    1003,
    (size_t)1 << 20,
    (size_t)1 << 26,
    (size_t)1 << 27
};

void init_vectors(double* a, double* b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        a[i] = sin((double)i * 0.001);
        b[i] = cos((double)i * 0.002);
    }
}

double time_kernel(double (*kernel)(const double*, const double*, size_t),
    const double* a, const double* b, size_t n,
    double* out_res) {
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);

    double total_ms = 0.0;
    *out_res = kernel(a, b, n); // Warm-up run

    for (int r = 0; r < NUM_RUNS; r++) {
        QueryPerformanceCounter(&start);
        *out_res = kernel(a, b, n);
        QueryPerformanceCounter(&end);

        total_ms += ((double)(end.QuadPart - start.QuadPart)
            / freq.QuadPart) * 1000.0;
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

        double resC = 0.0, resX86 = 0.0, resXMM = 0.0, resYMM = 0.0;

        double timeC = time_kernel(dotProductC, a, b, N, &resC);
        double timeX86 = time_kernel(dotProductx86, a, b, N, &resX86);
        double timeXMM = time_kernel(dotProductXMM, a, b, N, &resXMM);
        double timeYMM = time_kernel(dotProductYMM, a, b, N, &resYMM);

        int pass = fabs(resX86 - resC)
            <= 1e-8 + 1e-6 * fabs(resC);
        int passXMM = fabs(resXMM - resC)
            <= 1e-8 + 1e-6 * fabs(resC);
        int passYMM = fabs(resYMM - resC)
            <= 1e-8 + 1e-6 * fabs(resC);

        printf("----------------------------------------------------------------------------------------\n");
        printf(" Vector Size N = %zu\n", N);
        printf("----------------------------------------------------------------------------------------\n");
        printf(" %-15s | %-15s | %-20s | %-10s\n",
            "Kernel", "Avg Time (ms)", "Result Z", "Match");
        printf("----------------------------------------------------------------------------------------\n");
        printf(" %-15s | %-15.4f | %-20.6f | %-10s\n",
            "C Baseline", timeC, resC, "REF");
        printf(" %-15s | %-15.4f | %-20.6f | %-10s\n",
            "x86-64 Scalar", timeX86, resX86, pass ? "PASS" : "FAIL");
        printf(" %-15s | %-15.4f | %-20.6f | %-10s\n",
            "SIMD XMM", timeXMM, resXMM, passXMM ? "PASS" : "FAIL");
        printf(" %-15s | %-15.4f | %-20.6f | %-10s\n",
            "SIMD YMM", timeYMM, resYMM, passYMM ? "PASS" : "FAIL");
        printf("----------------------------------------------------------------------------------------\n\n");

        free(a);
        free(b);
    }

    size_t boundary_N = 1003;
    size_t b_bytes = boundary_N * sizeof(double);

    double* ba = (double*)malloc(b_bytes);
    double* bb = (double*)malloc(b_bytes);

    if (ba != NULL && bb != NULL) {
        init_vectors(ba, bb, boundary_N);

        double b_resC = dotProductC(ba, bb, boundary_N);
        double b_resX86 = dotProductx86(ba, bb, boundary_N);
        double b_resXMM = dotProductXMM(ba, bb, boundary_N);
        double b_resYMM = dotProductYMM(ba, bb, boundary_N);

        int passX86 = fabs(b_resX86 - b_resC)
            <= 1e-8 + 1e-6 * fabs(b_resC);
        int passXMM = fabs(b_resXMM - b_resC)
            <= 1e-8 + 1e-6 * fabs(b_resC);
        int passYMM = fabs(b_resYMM - b_resC)
            <= 1e-8 + 1e-6 * fabs(b_resC);

        printf("====================================================\n");
        printf("       BOUNDARY CONDITION CHECK (N = %zu)\n", boundary_N);
        printf("====================================================\n");
        printf("Reference C : %.6f\n", b_resC);
        printf("x86-64 Scalar: %.6f [%s]\n",
            b_resX86, passX86 ? "PASSED" : "FAILED");
        printf("SIMD XMM    : %.6f [%s]\n",
            b_resXMM, passXMM ? "PASSED" : "FAILED");
        printf("SIMD YMM    : %.6f [%s]\n",
            b_resYMM, passYMM ? "PASSED" : "FAILED");
        printf("====================================================\n\n");

        free(ba);
        free(bb);
    }
    else {
        printf("Boundary test allocation failed.\n");
        free(ba);
        free(bb);
    }

    return 0;
}

