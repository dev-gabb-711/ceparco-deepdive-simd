global dotProductYMM
section .text

dotProductYMM:
    xor     rax, rax

    ; Four independent vector accumulators
    vxorpd  ymm0, ymm0, ymm0
    vxorpd  ymm1, ymm1, ymm1
    vxorpd  ymm2, ymm2, ymm2
    vxorpd  ymm3, ymm3, ymm3

    cmp     r8, 16
    jb      reduce

.loop16:
    ; Elements 0-3 of this block
    vmovupd     ymm4, [rcx + rax*8]
    vfmadd231pd ymm0, ymm4, [rdx + rax*8]

    ; Elements 4-7
    vmovupd     ymm5, [rcx + rax*8 + 32]
    vfmadd231pd ymm1, ymm5, [rdx + rax*8 + 32]

    ; Elements 8-11
    vmovupd     ymm4, [rcx + rax*8 + 64]
    vfmadd231pd ymm2, ymm4, [rdx + rax*8 + 64]

    ; Elements 12-15
    vmovupd     ymm5, [rcx + rax*8 + 96]
    vfmadd231pd ymm3, ymm5, [rdx + rax*8 + 96]

    add     rax, 16

    ; Continue only if at least 16 elements remain
    mov     r9, r8
    sub     r9, rax
    cmp     r9, 16
    jae     .loop16

reduce:
    ; Combine the four accumulators
    vaddpd  ymm0, ymm0, ymm1
    vaddpd  ymm2, ymm2, ymm3
    vaddpd  ymm0, ymm0, ymm2

    ; Reduce four lanes to one scalar
    vextractf128 xmm1, ymm0, 1
    vaddpd      xmm0, xmm0, xmm1
    vhaddpd     xmm0, xmm0, xmm0

.tail:
    cmp     rax, r8
    jae     .done

    ; Process remaining elements safely
    vmovsd      xmm1, [rcx + rax*8]
    vfmadd231sd xmm0, xmm1, [rdx + rax*8]

    inc     rax
    jmp     .tail

.done:
    vzeroupper
    ret
