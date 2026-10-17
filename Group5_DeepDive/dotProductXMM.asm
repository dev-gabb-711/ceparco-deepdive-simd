section .text
global dotProductXMM


dotProductXMM:
    vxorpd      xmm0, xmm0, xmm0
    vxorpd      xmm1, xmm1, xmm1
    vxorpd      xmm2, xmm2, xmm2
    vxorpd      xmm3, xmm3, xmm3

    xor         rax, rax

    ; Process 8 doubles per iteration
    mov         r9, r8
    shr         r9, 3              ; n / 8
    jz          .reduce

.loop8:
    ; First 2 doubles
    vmovupd     xmm4, [rcx]
    vfmadd231pd xmm0, xmm4, [rdx]

    ; Next 2 doubles
    vmovupd     xmm4, [rcx + 16]
    vfmadd231pd xmm1, xmm4, [rdx + 16]

    ; Next 2 doubles
    vmovupd     xmm4, [rcx + 32]
    vfmadd231pd xmm2, xmm4, [rdx + 32]

    ; Last 2 doubles
    vmovupd     xmm4, [rcx + 48]
    vfmadd231pd xmm3, xmm4, [rdx + 48]

    add         rcx, 64
    add         rdx, 64
    add         rax, 8

    dec         r9
    jnz         .loop8

.reduce:
    ; Combine all accumulators
    vaddpd      xmm0, xmm0, xmm1
    vaddpd      xmm2, xmm2, xmm3
    vaddpd      xmm0, xmm0, xmm2

    ; Process remaining elements
.tail:
    cmp         rax, r8
    jae         .horizontal_sum

    vmovsd      xmm4, [rcx]
    vfmadd231sd xmm0, xmm4, [rdx]

    add         rcx, 8
    add         rdx, 8
    inc         rax
    jmp         .tail

.horizontal_sum:
    vhaddpd     xmm0, xmm0, xmm0

.done:
    ret
