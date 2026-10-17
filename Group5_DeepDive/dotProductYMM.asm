global dotProductYMM
section .text

dotProductYMM:
    xor     rax, rax
    vxorpd  ymm0, ymm0, ymm0
    vxorpd  ymm4, ymm4, ymm4

    cmp     r8, 0
    je      done

loop_start:
    mov     r9, r8
    sub     r9, rax
    cmp     r9, 8
    jb      reduce

    ; First 4 elements
    vmovupd     ymm1, [rcx + rax*8]
    vfmadd231pd ymm0, ymm1, [rdx + rax*8]

    ; Next 4 elements
    vmovupd     ymm2, [rcx + rax*8 + 32]
    vfmadd231pd ymm4, ymm2, [rdx + rax*8 + 32]

    add     rax, 8
    jmp     loop_start

reduce:
    ; Combine the two accumulators
    vaddpd      ymm0, ymm0, ymm4

    ; Reduce four lanes to one scalar
    vextractf128 xmm1, ymm0, 1
    vaddpd      xmm0, xmm0, xmm1
    vhaddpd     xmm0, xmm0, xmm0

tail:
    cmp     rax, r8
    jae     done

    vmovsd      xmm1, [rcx + rax*8]
    vfmadd231sd xmm0, xmm1, [rdx + rax*8]

    inc     rax
    jmp     tail

done:
    vzeroupper
    ret