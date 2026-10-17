
global dotProductx86
section .text

dotProductx86:
    ; Four independent scalar accumulators
    vxorpd  xmm0, xmm0, xmm0
    vxorpd  xmm1, xmm1, xmm1
    vxorpd  xmm2, xmm2, xmm2
    vxorpd  xmm3, xmm3, xmm3

    xor     rax, rax

    ; Process 4 elements per iteration
    mov     r9, r8
    shr     r9, 2
    jz      .reduce

.loop4:
    movsd   xmm4, [rcx + rax*8]
    mulsd   xmm4, [rdx + rax*8]
    addsd   xmm0, xmm4

    movsd   xmm4, [rcx + rax*8 + 8]
    mulsd   xmm4, [rdx + rax*8 + 8]
    addsd   xmm1, xmm4

    movsd   xmm4, [rcx + rax*8 + 16]
    mulsd   xmm4, [rdx + rax*8 + 16]
    addsd   xmm2, xmm4

    movsd   xmm4, [rcx + rax*8 + 24]
    mulsd   xmm4, [rdx + rax*8 + 24]
    addsd   xmm3, xmm4

    add     rax, 4
    dec     r9
    jnz     .loop4

.reduce:
    addsd   xmm0, xmm1
    addsd   xmm2, xmm3
    addsd   xmm0, xmm2

.tail:
    cmp     rax, r8
    jae     .done

    movsd   xmm4, [rcx + rax*8]
    mulsd   xmm4, [rdx + rax*8]
    addsd   xmm0, xmm4

    inc     rax
    jmp     .tail

.done:
    ret
