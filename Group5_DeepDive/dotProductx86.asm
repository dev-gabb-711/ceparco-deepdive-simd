global dotProductx86
section .text

dotProductx86:
	mov rax, 0
	movq xmm0, rax
	cmp r8, 0
	je done

loop_start:
	movsd xmm1, [rcx + rax * 8]
	mulsd xmm1, [rdx + rax *8]
	addsd xmm0, xmm1
	inc rax
	cmp rax, r8
	jb loop_start

done:
	xor rax, rax
	ret