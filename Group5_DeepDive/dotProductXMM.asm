section .text
global dotProductXMM

; double dotProductXMM(const double* A, const double* B, long long n);
; 
; Registers (Windows x64 ABI):
;   RCX = pointer to array A
;   RDX = pointer to array B
;   R8  = n (length)
;   XMM0 = return value (scalar double)

dotProductXMM:
    vxorpd      xmm0, xmm0, xmm0        ; Accumulator 0 = [0.0, 0.0]
    vxorpd      xmm1, xmm1, xmm1        ; Accumulator 1 = [0.0, 0.0]

    cmp         r8, 0
    jle         .done

    ; Setup loop counter
    mov         rax, r8
    shr         rax, 2                  ; rax = n / 4
    jz          .check_remainder_2      ; Jump if n < 4

.loop_4:
    ; Load 4 double elements (32 bytes total)
    vmovdqu     xmm2, [rcx]             ; xmm2 = A[i ... i+1]
    vmovdqu     xmm3, [rcx + 16]        ; xmm3 = A[i+2 ... i+3]

    ; Multiply and accumulate
    vfmadd231pd xmm0, xmm2, [rdx]       
    vfmadd231pd xmm1, xmm3, [rdx + 16]  

    ; Advance pointers by 32 bytes
    add         rcx, 32
    add         rdx, 32
    dec         rax
    jnz         .loop_4

.check_remainder_2:
    ; Merge ccumulator xmm1 into xmm0
    vaddpd      xmm0, xmm0, xmm1

    ; Check if remaining elements >= 2 (n & 2)
    mov         rax, r8
    and         rax, 2
    cmp         rax, 0
    je          .check_remainder_1

    ; 2-element boundary block
    vmovdqu     xmm2, [rcx]
    vfmadd231pd xmm0, xmm2, [rdx]

    add         rcx, 16                 ; Advance pointers by 16 bytes
    add         rdx, 16

.check_remainder_1:
    ; Check if 1 element remains (n & 1)
    mov         rax, r8
    and         rax, 1
    cmp         rax, 0
    je          .reduce

    ; Scalar single boundary element
    vmovsd      xmm2, [rcx]
    vfmadd231sd xmm0, xmm2, [rdx]   

.reduce:
    ; Horizontal sum: xmm0[0] = xmm0[0] + xmm0[1]
    vhaddpd     xmm0, xmm0, xmm0

.done:
    ret
