# CEPARCO: Deep Dive SIMD Project Term 1 AY 26-27
# Performance Report & Benchmark Analysis

> **Execution Environment Notes:**  
> * **Maximum Machine Capacity:** The maximum vector size supported by the test machine without encountering issues was **N = 2^27 ($134,217,728$ elements)**.  
> * **4 Accumulator Strategy:** All x86-64 assembly kernels (Scalar, XMM, and YMM) were implemented using **4 independent accumulators**.

## 1. Screenshot of results (with execution time and correctness check for all programs)
### a. Execution & Performance Summary
![Benchmark Execution Results](./assets/screenshot1.png)
*Figure 1: Performance output showing execution times for C Baseline, x86-64 Scalar, SIMD XMM, and SIMD YMM across vector sizes up to $N = 2^{27}$.*

### b. Boundary Condition Check ($N = 1003$)
![Boundary Check Screenshot](./assets/screenshot2.png)
*Figure 2: Correctness verification of all program results, confirming the exact match with Reference C output*

---

## 2. Comparative Execution Time

| Vector Size ($N$) | Case / Type | C Baseline (ms) | x86-64 Scalar (ms) | SIMD XMM (ms) | SIMD YMM (ms) | Speedup (YMM vs C) |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **1,003** | Boundary Check | 0.0010 | 0.0003 | 0.0001 | 0.0001 | **10.00x** |
| **1,048,576** ($2^{20}$) | Medium Vector | 1.1521 | 0.7752 | 0.7046 | 0.6651 | **1.73x** |
| **67,108,864** ($2^{26}$) | Large Vector | 102.9805 | 91.8301 | 57.7411 | 57.1489 | **1.80x** |
| **134,217,728** ($2^{27}$) | Max Tested ($2^{27}$) | 226.2914 | 126.4229 | 117.1613 | 128.9579 | **1.75x** |

---

## 3. Performance Analysis

1. x86 had a faster execution time than baseline C since it deals with the registers directly. C has to store and read intermediate values back and forth from the stack and memory; it reads and writes temporary variables back to system RAM/stack memory, which slows down execution.
2. The SIMD (XMM/YMM) programs had the fastest results compared to baseline C and x86-64 because scalar x86-64 processes 1 element per loop cycle, while SIMD is able to process multiple elements within one instruction. Meanwhile, SIMD XMM can process 2 double-precision floats simultaneously, and SIMD YMM can process 4 double-precision floats simultaneously.
3. Instead of accumulating the sum into a single register—which forces the CPU to wait for each addition to finish before starting the next—all assembly kernels split the workload across 4 independent registers. This eliminates data dependency delays and allows the CPU to perform multiple additions concurrently in every clock cycle.
4. At 134 million elements (over 1 GB of data), the vector becomes too big to fit inside the cache. Instead of being slow at math, it gets stuck waiting for RAM to deliver the numbers. Because the memory is completely clogged with traffic, wider 256-bit YMM registers cannot load data any faster than 128-bit XMM registers, causing the performance to slow down compared to XMM at huge array sizes.

---

## 4. Implementation Details & Methodology

The project implements a vector dot product benchmark comparing C, x86-64 scalar assembly, SIMD XMM, and SIMD YMM implementations. To avoid data-dependency stalls, all assembly kernels unroll their main loops across four independent accumulator registers (`xmm0`–`xmm3` / `ymm0`–`ymm3`) and use fused multiply-add instructions (`vfmadd231pd`) for vector operations. We handle arbitrary, non-power-of-two array sizes (such as N = 1003) with a scalar tail cleanup loop that processes trailing elements individually before performing a horizontal reduction (`vhaddpd`) to ensure precision. For the evaluation of performance, execution times are averaged over 30 runs using `QueryPerformanceCounter`, with memory allocation capped at N = 2^27 elements (134,217,728 doubles, or ~2.15 GB total per test pair) to fit within the test machine's physical RAM constraints.

---

## 5. Problems Encountered & Solutions
During initial testing, the YMM implementation unexpectedly underperformed compared to XMM. Upon reviewing the assembly code, we realized that the XMM kernel was using four accumulators while the YMM kernel only used two, causing YMM to suffer from data dependency delays. To fix this, we standardized all assembly kernels (Scalar, XMM, and YMM) to use four independent accumulators (xmm0–xmm3 / ymm0–ymm3). This eliminated data-dependency bottlenecks, allowed the CPU to perform additions concurrently across all register lanes, and established a fair, standardized benchmark across all implementations.
Additionally, attempting to benchmark array sizes beyond N = 2^27 caused allocation failures. Pushing past N = 2^27 exceeded the available memory limits of the testing machine. To solve this, we capped the maximum benchmark threshold strictly at N = 2^27, ensuring that all vector allocations fit securely within physical RAM.
