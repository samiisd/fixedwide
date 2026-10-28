	.file	"batch_codegen.cpp"
	.text
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.type	_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE, @function
_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE:
.LFB1665:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$280, %rsp
	.cfi_def_cfa_offset 336
	movq	%fs:40, %rbp
	movq	%rbp, 264(%rsp)
	movq	%rdi, %rbp
	testq	%rcx, %rcx
	je	.L29
	movq	%r8, %rax
	movq	%r8, %xmm4
	movq	%r8, 64(%rsp)
	xorl	%ebx, %ebx
	sarq	$63, %rax
	movq	%rbp, 8(%rsp)
	movq	%rcx, %r15
	movq	%rsi, %rbp
	movq	%rax, %xmm0
	movq	%r8, %rax
	movq	%r8, %r14
	movq	%rdx, %r13
	sarq	$63, %rax
	movdqa	%xmm0, %xmm5
	punpcklqdq	%xmm0, %xmm4
	movq	%rbx, %r12
	movq	%rax, 72(%rsp)
	punpcklqdq	%xmm5, %xmm5
	movaps	%xmm4, 16(%rsp)
	movaps	%xmm5, 32(%rsp)
	.p2align 4
	.p2align 3
.L28:
	movdqu	0(%r13), %xmm0
	xorl	%edi, %edi
	movq	%xmm0, %rcx
	movhlps	%xmm0, %xmm1
	movq	%rcx, %rax
	movq	%xmm1, %rsi
	sarq	$63, %rax
	movq	%rsi, %rbx
	cmpq	%rsi, %rax
	movq	%rcx, %rax
	jne	.L6
	imulq	%r14
.L4:
	testq	%rdi, %rdi
	je	.L14
	sarq	$63, %rsi
	movdqa	32(%rsp), %xmm2
	movaps	%xmm0, 160(%rsp)
	leaq	224(%rsp), %rdi
	movq	%rsi, %xmm0
	subq	$64, %rsp
	.cfi_def_cfa_offset 400
	movl	$12, %ecx
	movl	$128, %r9d
	punpcklqdq	%xmm0, %xmm0
	movl	$5, %r8d
	movl	$10, %edx
	movaps	%xmm2, 272(%rsp)
	movaps	%xmm0, 240(%rsp)
	movdqa	80(%rsp), %xmm3
	movdqa	272(%rsp), %xmm0
	movl	$12, %esi
	movups	%xmm0, 48(%rsp)
	movdqa	224(%rsp), %xmm0
	movups	%xmm3, 32(%rsp)
	movups	%xmm0, (%rsp)
	movdqa	240(%rsp), %xmm0
	movups	%xmm0, 16(%rsp)
	call	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	addq	$64, %rsp
	.cfi_def_cfa_offset 336
	cmpb	$0, 256(%rsp)
	jne	.L15
	movzbl	224(%rsp), %eax
	movq	8(%rsp), %rbp
	movq	%r12, %rbx
	movb	%al, 80(%rsp)
.L26:
	movzbl	80(%rsp), %eax
	movq	%rbx, 0(%rbp)
	movb	$0, 16(%rbp)
	movb	%al, 8(%rbp)
.L1:
	movq	264(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L36
	addq	$280, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%rbp, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L14:
	.cfi_restore_state
	movq	%rax, %rdi
	testq	%rdx, %rdx
	js	.L17
	movq	%rdx, %rsi
	testq	%rdx, %rdx
	jne	.L18
	movabsq	$5739826786856214345, %rax
	imulq	%rdi, %rax
	rorq	$10, %rax
	cmpq	$1844674407, %rax
	ja	.L19
	movabsq	$-2601111570856684097, %rax
	mulq	%rdi
	shrq	$33, %rdx
	movq	%rdx, %rax
	xorl	%edx, %edx
.L20:
	movq	%rdx, 88(%rsp)
	movq	%rax, 80(%rsp)
	movl	$1, %eax
.L24:
	movdqa	80(%rsp), %xmm0
	movb	%al, 96(%rsp)
	movaps	%xmm0, 112(%rsp)
	testb	%al, %al
	jne	.L25
	movzbl	112(%rsp), %eax
	movq	8(%rsp), %rbp
	movq	%r12, %rbx
	movb	%al, 80(%rsp)
	jmp	.L26
	.p2align 4,,10
	.p2align 3
.L15:
	movdqa	224(%rsp), %xmm0
	movaps	%xmm0, 80(%rsp)
.L27:
	movq	%r12, %rax
	movdqa	80(%rsp), %xmm0
	addq	$1, %r12
	addq	$16, %r13
	salq	$4, %rax
	movups	%xmm0, 0(%rbp,%rax)
	cmpq	%r12, %r15
	jne	.L28
	movq	8(%rsp), %rbp
.L29:
	movb	$1, 16(%rbp)
	jmp	.L1
	.p2align 4,,10
	.p2align 3
.L25:
	movq	112(%rsp), %rax
	movq	%rax, 80(%rsp)
	movq	120(%rsp), %rax
	movq	%rax, 88(%rsp)
	jmp	.L27
	.p2align 4,,10
	.p2align 3
.L17:
	movq	%rax, %rsi
	movq	%rdx, %rdi
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
	movq	%rdi, %r8
	jne	.L21
	movabsq	$5739826786856214345, %rax
	imulq	%rsi, %rax
	rorq	$10, %rax
	cmpq	$1844674407, %rax
	jbe	.L37
.L19:
	movb	$2, 80(%rsp)
	xorl	%eax, %eax
	jmp	.L24
	.p2align 4,,10
	.p2align 3
.L6:
	mulq	%r14
	movq	%rax, 48(%rsp)
	movq	%r14, %rax
	movq	%rdx, 56(%rsp)
	mulq	%rsi
	movq	%rax, %r8
	movq	%rdx, %r9
	testq	%rsi, %rsi
	jns	.L10
	xorl	%eax, %eax
	subq	%rax, %r8
	sbbq	%r14, %r9
.L10:
	testq	%r14, %r14
	jns	.L11
	subq	%rcx, %r8
	sbbq	%rbx, %r9
.L11:
	movq	56(%rsp), %r11
	xorl	%edx, %edx
	movq	48(%rsp), %r10
	movq	%r11, %rax
	addq	%r8, %rax
	adcq	%r9, %rdx
	movq	%rax, %r9
	sarq	$63, %r9
	cmpq	%rdx, %r9
	jne	.L12
	movq	%rax, %rdx
	movq	%r10, %rax
	jmp	.L4
	.p2align 4,,10
	.p2align 3
.L18:
	movabsq	$-2601111570856684097, %rax
	movabsq	$10000000000, %rbx
	mulq	%rdx
	movabsq	$10000000000, %rax
	shrq	$33, %rdx
	imulq	%rdx, %rax
	movq	%rdx, %rcx
	subq	%rax, %rsi
	movq	%rdi, %rax
	movq	%rsi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbx
# 0 "" 2
#NO_APP
	testq	%rdx, %rdx
	jne	.L19
	movq	%rcx, %rdx
	jmp	.L20
	.p2align 4,,10
	.p2align 3
.L21:
	movabsq	$-2601111570856684097, %rax
	mulq	%rdi
	movabsq	$10000000000, %rdi
	shrq	$33, %rdx
	movq	%rdx, %rax
	movq	%rdx, %rcx
	movq	%r8, %rdx
	imulq	%rdi, %rax
	subq	%rax, %rdx
	movq	%rsi, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rdi
# 0 "" 2
#NO_APP
	testq	%rdx, %rdx
	jne	.L19
	movq	%rcx, %rdx
.L23:
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
	jmp	.L20
	.p2align 4,,10
	.p2align 3
.L37:
	movabsq	$-2601111570856684097, %rax
	mulq	%rsi
	shrq	$33, %rdx
	movq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L23
.L12:
	movq	72(%rsp), %rax
	movq	%r14, %rdi
	imulq	%rbx, %rdi
	imulq	%rcx, %rax
	addq	%rax, %rdi
	movq	%rcx, %rax
	mulq	%r14
	addq	%rdx, %rdi
	movq	%rdi, %rdx
	movl	$1, %edi
	jmp	.L4
.L36:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1665:
	.size	_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE, .-_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.type	_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE, @function
_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE:
.LFB1669:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movq	%r8, %rax
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movq	%rdx, %r14
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rdi, %rbx
	subq	$152, %rsp
	.cfi_def_cfa_offset 208
	movq	%rsi, 56(%rsp)
	movq	%rcx, 32(%rsp)
	movq	%fs:40, %rbp
	movq	%rbp, 136(%rsp)
	movq	%r8, %rbp
	testq	%r8, %r8
	jns	.L39
	negq	%rax
.L39:
	movq	%rax, %rdi
	movabsq	$-2601111570856684097, %rax
	mulq	%rdi
	movabsq	$10000000000, %rax
	movq	%rax, %r12
	movq	%rdx, %rcx
	shrq	$33, %rcx
	movq	%rcx, %rdx
	movq	%rdi, %rcx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	movq	%r12, %rdx
	movq	%rcx, %rax
	.p2align 4
	.p2align 4
	.p2align 3
.L40:
	movq	%rdx, %rcx
	xorl	%edx, %edx
	divq	%rcx
	movq	%rcx, %rax
	testq	%rdx, %rdx
	jne	.L40
	movq	$0, 8(%rsp)
	movq	%rdx, %r12
	movq	%rdi, %rax
	xorl	%edx, %edx
	divq	%rcx
	xorl	%edx, %edx
	movq	%rax, (%rsp)
	movabsq	$10000000000, %rax
	divq	%rcx
	movq	%rax, %r13
	cmpq	$1, %rax
	je	.L92
	xorl	%ecx, %ecx
	xorl	%edi, %edi
	movq	%rax, %rdx
	movl	$1, %esi
	call	__udivti3@PLT
	shrq	$63, %rbp
	cmpq	$0, 32(%rsp)
	movq	%rax, 64(%rsp)
	movq	%rdx, 72(%rsp)
	je	.L43
	movq	%rbx, 40(%rsp)
	xorl	%r15d, %r15d
	movb	%bpl, 55(%rsp)
	movq	56(%rsp), %rbp
.L60:
	movq	(%r14,%r15), %rcx
	movq	8(%r14,%r15), %rbx
	movq	%rcx, %rax
	movq	%rbx, %rdx
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rcx, %rax
	cmovs	%rbx, %rdx
	movq	%rax, %r8
	movq	%rdx, %r9
	movq	%rax, %r10
	testq	%rdx, %rdx
	je	.L44
	cmpq	%r13, %rdx
	jb	.L45
	movq	64(%rsp), %rax
	xorl	%esi, %esi
	movq	(%rsp), %r10
	mulq	%rdx
	movq	%r13, %rax
	imulq	%rdx, %rax
	subq	%rax, %r9
	cmpq	%r13, %r9
	movq	%r9, %rax
	setnb	%sil
	addq	%rdx, %rsi
	subq	%r13, %rax
	cmpq	%r13, %r9
	cmovnb	%rax, %r9
	movq	%r8, %rax
	xorl	%edi, %edi
	movq	%r9, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r13
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	%r10, %rax
	movq	%rdx, 80(%rsp)
	mulq	%r8
	movq	%rax, %r8
	movq	%rsi, %rax
	movq	%rdx, %r9
	mulq	%r10
	movq	%r9, %rsi
	addq	%rax, %rsi
	adcq	%rdx, %rdi
	xorl	%edx, %edx
	movq	%rdi, %rax
	orq	%rdx, %rax
	je	.L93
.L56:
	movb	$0, 112(%rsp)
	movq	40(%rsp), %rbx
.L72:
	movzbl	112(%rsp), %eax
	movq	%r12, (%rbx)
	movb	$0, 16(%rbx)
	movb	%al, 8(%rbx)
.L38:
	movq	136(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L94
	addq	$152, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%rbx, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L44:
	.cfi_restore_state
	movq	72(%rsp), %rsi
	imulq	%rax, %rsi
	movq	64(%rsp), %rax
	mulq	%r8
	addq	%rdx, %rsi
	movq	%rsi, %rax
	imulq	%r13, %rax
	subq	%rax, %r10
	xorl	%eax, %eax
	cmpq	%r13, %r10
	setnb	%al
	addq	%rax, %rsi
	cmpq	%r13, %r10
	jnb	.L50
.L91:
	movq	%rbx, %rdi
	movq	(%rsp), %rax
	xorl	%r9d, %r9d
	movq	%r10, %r8
	shrq	$63, %rdi
	movq	$-1, %rcx
	movabsq	$9223372036854775807, %rbx
	movq	$0, 16(%rsp)
	mulq	%rsi
	movb	%dil, 56(%rsp)
	xorb	55(%rsp), %dil
	movzbl	%dil, %edi
	addq	%rdi, %rcx
	adcq	$0, %rbx
	movq	%rdx, 24(%rsp)
	orq	16(%rsp), %rax
	xorl	%edx, %edx
	orq	24(%rsp), %rdx
	movq	%rax, %r10
	movq	%rdx, %r11
.L77:
	movq	%r8, %rax
	orq	%r9, %rax
	jne	.L95
	movzbl	55(%rsp), %ebx
	cmpb	%bl, 56(%rsp)
	je	.L61
.L69:
	negq	%r10
	adcq	$0, %r11
	negq	%r11
.L61:
	movq	%r10, 112(%rsp)
	addq	$1, %r12
	movq	%r11, 120(%rsp)
	movdqa	112(%rsp), %xmm0
	movups	%xmm0, 0(%rbp,%r15)
	addq	$16, %r15
	cmpq	%r12, 32(%rsp)
	jne	.L60
	movq	40(%rsp), %rbx
.L43:
	movb	$1, 16(%rbx)
	jmp	.L38
	.p2align 4,,10
	.p2align 3
.L95:
	movq	(%rsp), %rax
	mulq	%r8
	movq	%rax, 80(%rsp)
	movq	%rdx, %r9
	movq	%rax, %r8
	movq	%rdx, 88(%rsp)
	testq	%rdx, %rdx
	jne	.L96
	movq	80(%rsp), %rax
	movq	72(%rsp), %rdi
	imulq	%rax, %rdi
	movq	64(%rsp), %rax
	mulq	80(%rsp)
	leaq	(%rdi,%rdx), %rsi
	xorl	%edi, %edi
	movq	%rsi, %rax
	imulq	%r13, %rax
	subq	%rax, %r8
	xorl	%eax, %eax
	cmpq	%r13, %r8
	setnb	%al
	addq	%rax, %rsi
	movq	%r8, %rax
	adcq	$0, %rdi
	subq	%r13, %rax
	cmpq	%r13, %r8
	cmovnb	%rax, %r8
	xorl	%r9d, %r9d
.L63:
	movq	%rcx, %rax
	movq	%rbx, %rdx
	subq	%r10, %rax
	sbbq	%r11, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L56
	addq	%rsi, %r10
	movq	%rbx, %rax
	adcq	%rdi, %r11
	cmpq	%r10, %rcx
	sbbq	%r11, %rax
	jc	.L56
	movq	%r8, %rsi
	orq	%r9, %rsi
	je	.L97
	movb	$2, 112(%rsp)
	movq	40(%rsp), %rbx
	jmp	.L72
	.p2align 4,,10
	.p2align 3
.L50:
	subq	%r13, %r10
	jmp	.L91
	.p2align 4,,10
	.p2align 3
.L45:
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r13
# 0 "" 2
#NO_APP
	movq	%rbx, %rsi
	movq	%rax, %r10
	movq	(%rsp), %rax
	movq	%rdx, %r8
	shrq	$63, %rsi
	xorl	%r9d, %r9d
	movq	$-1, %rcx
	movabsq	$9223372036854775807, %rbx
	mulq	%r10
	movb	%sil, 56(%rsp)
	xorb	55(%rsp), %sil
	movzbl	%sil, %esi
	addq	%rsi, %rcx
	movq	%rax, %r10
	movq	%rdx, %r11
	adcq	$0, %rbx
	jmp	.L77
	.p2align 4,,10
	.p2align 3
.L93:
	movq	%rbx, %r10
	movq	$-1, %rcx
	movq	%rsi, %rdi
	movabsq	$9223372036854775807, %rbx
	shrq	$63, %r10
	movb	%r10b, 56(%rsp)
	xorb	55(%rsp), %r10b
	movzbl	%r10b, %r10d
	addq	%r10, %rcx
	movq	%r8, %r10
	adcq	$0, %rbx
	xorl	%r9d, %r9d
	xorl	%esi, %esi
	movq	%r9, %r11
	orq	%rsi, %r10
	movq	%rbx, %rax
	orq	%rdi, %r11
	cmpq	%r10, %rcx
	sbbq	%r11, %rax
	jc	.L56
	movq	80(%rsp), %r8
	xorl	%r9d, %r9d
	jmp	.L77
	.p2align 4,,10
	.p2align 3
.L92:
	shrq	$63, %rbp
	xorl	%r9d, %r9d
	cmpq	$0, 32(%rsp)
	je	.L43
	movq	56(%rsp), %r13
	movq	32(%rsp), %r15
	movq	%r9, %r10
	movq	%rbx, %r11
	.p2align 4
	.p2align 3
.L42:
	movq	(%r14,%r10), %r8
	movq	8(%r14,%r10), %r9
	movq	(%rsp), %rax
	movq	%r8, %rsi
	movq	%r9, %rcx
	negq	%rsi
	adcq	$0, %rcx
	negq	%rcx
	cmovs	%r8, %rsi
	cmovs	%r9, %rcx
	xorl	%ebx, %ebx
	mulq	%rsi
	movq	%rax, %rsi
	movq	(%rsp), %rax
	movq	%rdx, %rdi
	mulq	%rcx
	movq	%rdi, %rcx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	xorl	%edx, %edx
	movq	%rbx, %rax
	orq	%rdx, %rax
	jne	.L73
	movq	%r9, %r8
	movq	%rcx, %rbx
	xorl	%edi, %edi
	xorl	%ecx, %ecx
	shrq	$63, %r8
	orq	%rcx, %rsi
	orq	%rbx, %rdi
	xorl	%eax, %eax
	movabsq	$-9223372036854775808, %rbx
	cmpb	%bpl, %r8b
	sete	%al
	negq	%rax
	sbbq	%rdx, %rdx
	xorl	%ecx, %ecx
	xorq	%rcx, %rax
	xorq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L73
	cmpb	%bpl, %r8b
	je	.L75
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
.L75:
	movq	%rsi, 112(%rsp)
	addq	$1, %r12
	movq	%rdi, 120(%rsp)
	movdqa	112(%rsp), %xmm0
	movups	%xmm0, 0(%r13,%r10)
	addq	$16, %r10
	cmpq	%r12, %r15
	jne	.L42
	movq	%r11, %rbx
	jmp	.L43
	.p2align 4,,10
	.p2align 3
.L73:
	movb	$0, 112(%rsp)
	movq	%r11, %rbx
	jmp	.L72
	.p2align 4,,10
	.p2align 3
.L96:
	cmpq	%r13, %rdx
	jnb	.L62
	movq	$0, 96(%rsp)
	movq	$0, 104(%rsp)
.L65:
	movq	%r9, %rdx
	movq	80(%rsp), %rax
	xorl	%edi, %edi
	xorl	%r9d, %r9d
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r13
# 0 "" 2
#NO_APP
	orq	104(%rsp), %rdi
	movq	%rax, %rsi
	movq	%rdx, %r8
	orq	96(%rsp), %rsi
	jmp	.L63
.L97:
	movzbl	55(%rsp), %edi
	cmpb	%dil, 56(%rsp)
	jne	.L69
	jmp	.L61
.L62:
	movq	64(%rsp), %rax
	mulq	%rdx
	movq	%r13, %rax
	imulq	%rdx, %rax
	subq	%rax, %r9
	xorl	%eax, %eax
	cmpq	%r13, %r9
	movq	%r9, %r8
	setnb	%al
	addq	%rdx, %rax
	subq	%r13, %r8
	cmpq	%r13, %r9
	movq	%rax, %rdx
	cmovnb	%r8, %r9
	xorl	%eax, %eax
	movq	%rax, 96(%rsp)
	movq	%rdx, 104(%rsp)
	jmp	.L65
.L94:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1669:
	.size	_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE, .-_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.type	_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE, @function
_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE:
.LFB1690:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movq	%r8, %rax
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	movq	%rdi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$392, %rsp
	.cfi_def_cfa_offset 448
	movq	%rsi, 56(%rsp)
	movq	%rdx, 64(%rsp)
	movq	%rcx, 32(%rsp)
	movq	%fs:40, %r12
	movq	%r12, 376(%rsp)
	movq	%r8, %r12
	testq	%r8, %r8
	jns	.L99
	negq	%rax
.L99:
	movq	%rax, %rdi
	movabsq	$-2601111570856684097, %rax
	mulq	%rdi
	movabsq	$10000000000, %rax
	movq	%rax, %rbx
	movq	%rdx, %rcx
	shrq	$33, %rcx
	imulq	%rax, %rcx
	movq	%rcx, %rdx
	movq	%rdi, %rcx
	subq	%rdx, %rcx
	movq	%rbx, %rdx
	movq	%rcx, %rax
	.p2align 4
	.p2align 4
	.p2align 3
.L100:
	movq	%rdx, %rcx
	xorl	%edx, %edx
	divq	%rcx
	movq	%rcx, %rax
	testq	%rdx, %rdx
	jne	.L100
	movq	$0, 8(%rsp)
	movq	%rdx, %rbx
	movq	%rdi, %rax
	xorl	%edx, %edx
	divq	%rcx
	xorl	%edx, %edx
	movq	%rax, (%rsp)
	movabsq	$10000000000, %rax
	divq	%rcx
	movq	%rax, %r14
	cmpq	$1, %rax
	je	.L335
	xorl	%ecx, %ecx
	xorl	%edi, %edi
	movq	%rax, %rdx
	movl	$1, %esi
	call	__udivti3@PLT
	shrq	$63, %r12
	xorl	%r15d, %r15d
	cmpq	$3, 32(%rsp)
	movq	%r12, %r11
	movq	%rax, 16(%rsp)
	movq	64(%rsp), %r12
	movq	%rdx, 40(%rsp)
	movq	56(%rsp), %r13
	jbe	.L336
	movq	%rbx, 200(%rsp)
	movq	%rbp, 192(%rsp)
.L223:
	movq	(%r12), %rsi
	movq	8(%r12), %rdi
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	negq	%rcx
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rdi, %rdx
	cmovs	%rsi, %rcx
	movq	%rdx, %r10
	testq	%rdx, %rdx
	je	.L107
	cmpq	%r14, %rdx
	jnb	.L337
	movq	%rcx, %rax
	movq	%rdi, %rsi
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	(%rsp), %rax
	movq	%rdx, %rcx
	shrq	$63, %rsi
	movb	%sil, 48(%rsp)
	xorl	%r11d, %esi
	mulq	%r8
	movzbl	%sil, %esi
	movq	%rax, %r8
	movq	$-1, %rax
	movq	%rdx, %r9
	movabsq	$9223372036854775807, %rdx
	addq	%rsi, %rax
	adcq	$0, %rdx
	movq	%rax, %rdi
	movq	%rdx, %rbp
.L285:
	movq	%rcx, %rax
	orq	%rbx, %rax
	je	.L328
	.p2align 4
	.p2align 3
.L346:
	movq	(%rsp), %rax
	mulq	%rcx
	movq	%rax, %rcx
	movq	%rdx, %rsi
	movq	%rax, %r10
	testq	%rdx, %rdx
	jne	.L338
	movq	$0, 88(%rsp)
	movq	40(%rsp), %rsi
	xorl	%ebx, %ebx
	imulq	%rax, %rsi
	movq	16(%rsp), %rax
	mulq	%rcx
	leaq	(%rsi,%rdx), %rcx
	movq	%r10, %rsi
	movq	%rcx, %rax
	imulq	%r14, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r14, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%r14, %rax
	cmpq	%r14, %rsi
	cmovnb	%rax, %rsi
	movq	%rsi, 80(%rsp)
.L126:
	movq	%rdi, %rax
	movq	%rbp, %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L119
	addq	%rcx, %r8
	movq	%rbp, %rax
	adcq	%rbx, %r9
	cmpq	%r8, %rdi
	sbbq	%r9, %rax
	jc	.L119
	movq	80(%rsp), %rax
	movq	88(%rsp), %rdx
	movq	%rax, %rcx
	orq	%rdx, %rcx
	je	.L328
	movb	$2, 272(%rsp)
	movb	$0, 288(%rsp)
	jmp	.L115
	.p2align 4,,10
	.p2align 3
.L337:
	movq	16(%rsp), %rax
	xorl	%r8d, %r8d
	mulq	%rdx
	movq	%r14, %rax
	imulq	%rdx, %rax
	subq	%rax, %r10
	cmpq	%r14, %r10
	movq	%r10, %rax
	setnb	%r8b
	addq	%rdx, %r8
	subq	%r14, %rax
	cmpq	%r14, %r10
	cmovnb	%rax, %r10
	movq	%rcx, %rax
	xorl	%r9d, %r9d
	movq	%r10, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	(%rsp), %rax
	movq	%rdx, %r10
	mulq	%rcx
	movq	%rdx, %rbx
	movq	%rax, %rcx
	movq	%r8, %rax
	mulq	(%rsp)
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	je	.L339
.L119:
	movb	$0, 272(%rsp)
	movb	$0, 288(%rsp)
.L115:
	movb	$0, 48(%rsp)
.L123:
	movq	16(%r12), %rsi
	movq	24(%r12), %rdi
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	negq	%rcx
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rdi, %rdx
	cmovs	%rsi, %rcx
	movq	%rdx, %r10
	testq	%rdx, %rdx
	je	.L135
	cmpq	%r14, %rdx
	jb	.L136
	movq	16(%rsp), %rax
	xorl	%r8d, %r8d
	mulq	%rdx
	movq	%r14, %rax
	imulq	%rdx, %rax
	subq	%rax, %r10
	cmpq	%r14, %r10
	movq	%r10, %rax
	setnb	%r8b
	addq	%rdx, %r8
	subq	%r14, %rax
	cmpq	%r14, %r10
	cmovnb	%rax, %r10
	movq	%rcx, %rax
	xorl	%r9d, %r9d
	movq	%r10, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	(%rsp), %rax
	movq	%rdx, %r10
	mulq	%rcx
	movq	%rdx, %rbx
	movq	%rax, %rcx
	movq	%r8, %rax
	mulq	(%rsp)
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	je	.L340
.L147:
	movb	$0, 296(%rsp)
	movb	$0, 312(%rsp)
.L143:
	movb	$0, 80(%rsp)
.L151:
	movq	32(%r12), %rsi
	movq	40(%r12), %rdi
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	negq	%rcx
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rdi, %rdx
	cmovs	%rsi, %rcx
	movq	%rdx, %r10
	testq	%rdx, %rdx
	je	.L163
	cmpq	%r14, %rdx
	jb	.L164
	movq	16(%rsp), %rax
	xorl	%r8d, %r8d
	mulq	%rdx
	movq	%r14, %rax
	imulq	%rdx, %rax
	subq	%rax, %r10
	cmpq	%r14, %r10
	movq	%r10, %rax
	setnb	%r8b
	addq	%rdx, %r8
	subq	%r14, %rax
	cmpq	%r14, %r10
	cmovnb	%rax, %r10
	movq	%rcx, %rax
	xorl	%r9d, %r9d
	movq	%r10, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	(%rsp), %rax
	movq	%rdx, %r10
	mulq	%rcx
	movq	%rdx, %rbx
	movq	%rax, %rcx
	movq	%r8, %rax
	mulq	(%rsp)
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	je	.L341
.L175:
	movb	$0, 320(%rsp)
	movb	$0, 336(%rsp)
.L171:
	movb	$0, 96(%rsp)
.L179:
	movq	48(%r12), %rsi
	movq	56(%r12), %rdi
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	negq	%rcx
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rdi, %rdx
	cmovs	%rsi, %rcx
	movq	%rdx, %r10
	testq	%rdx, %rdx
	je	.L191
	cmpq	%r14, %rdx
	jb	.L192
	movq	16(%rsp), %rax
	xorl	%r8d, %r8d
	mulq	%rdx
	movq	%r14, %rax
	imulq	%rdx, %rax
	subq	%rax, %r10
	cmpq	%r14, %r10
	movq	%r10, %rax
	setnb	%r8b
	addq	%rdx, %r8
	subq	%r14, %rax
	cmpq	%r14, %r10
	cmovnb	%rax, %r10
	movq	%rcx, %rax
	xorl	%r9d, %r9d
	movq	%r10, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	(%rsp), %rax
	movq	%rdx, %r10
	mulq	%rcx
	movq	%rdx, %rbx
	movq	%rax, %rcx
	movq	%r8, %rax
	mulq	(%rsp)
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	je	.L342
.L203:
	movb	$0, 344(%rsp)
	xorl	%eax, %eax
	movb	$0, 360(%rsp)
.L348:
	cmpb	$0, 48(%rsp)
	jne	.L343
.L317:
	movq	200(%rsp), %rbx
	movq	192(%rsp), %rbp
	.p2align 4
	.p2align 3
.L219:
	leaq	(%rbx,%rbx,2), %rax
	movq	%r15, 0(%rbp)
	movzbl	272(%rsp,%rax,8), %eax
	movb	$0, 16(%rbp)
	movb	%al, 8(%rbp)
	jmp	.L98
.L341:
	movq	%rdi, %rsi
	movq	$-1, %rax
	movq	%r8, %r9
	movq	%rcx, %r8
	movabsq	$9223372036854775807, %rdx
	shrq	$63, %rsi
	movb	%sil, 96(%rsp)
	xorl	%r11d, %esi
	movzbl	%sil, %esi
	addq	%rsi, %rax
	adcq	$0, %rdx
	xorl	%ebx, %ebx
	movq	%rax, %rdi
	orq	%rbx, %r9
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	movq	%rdx, %rbp
	sbbq	%r9, %rax
	jc	.L175
	movq	%r10, %rcx
	xorl	%ebx, %ebx
	jmp	.L283
	.p2align 4,,10
	.p2align 3
.L342:
	movq	%rdi, %rsi
	movq	$-1, %rax
	movq	%r8, %r9
	movq	%rcx, %r8
	movabsq	$9223372036854775807, %rdx
	shrq	$63, %rsi
	movb	%sil, 176(%rsp)
	xorl	%r11d, %esi
	movzbl	%sil, %esi
	addq	%rsi, %rax
	adcq	$0, %rdx
	xorl	%ebx, %ebx
	movq	%rax, %rdi
	orq	%rbx, %r9
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	movq	%rdx, %rbp
	sbbq	%r9, %rax
	jc	.L203
	movq	%r10, %rcx
	xorl	%ebx, %ebx
	jmp	.L282
	.p2align 4,,10
	.p2align 3
.L339:
	movq	%rdi, %rsi
	movq	$-1, %rax
	movq	%r8, %r9
	movq	%rcx, %r8
	movabsq	$9223372036854775807, %rdx
	shrq	$63, %rsi
	movb	%sil, 48(%rsp)
	xorl	%r11d, %esi
	movzbl	%sil, %esi
	addq	%rsi, %rax
	adcq	$0, %rdx
	xorl	%ebx, %ebx
	movq	%rax, %rdi
	orq	%rbx, %r9
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	movq	%rdx, %rbp
	sbbq	%r9, %rax
	jc	.L119
	movq	%r10, %rcx
	xorl	%ebx, %ebx
	jmp	.L285
	.p2align 4,,10
	.p2align 3
.L340:
	movq	%rdi, %rsi
	movq	$-1, %rax
	movq	%r8, %r9
	movq	%rcx, %r8
	movabsq	$9223372036854775807, %rdx
	shrq	$63, %rsi
	movb	%sil, 80(%rsp)
	xorl	%r11d, %esi
	movzbl	%sil, %esi
	addq	%rsi, %rax
	adcq	$0, %rdx
	xorl	%ebx, %ebx
	movq	%rax, %rdi
	orq	%rbx, %r9
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	movq	%rdx, %rbp
	sbbq	%r9, %rax
	jc	.L147
	movq	%r10, %rcx
	xorl	%ebx, %ebx
	jmp	.L284
	.p2align 4,,10
	.p2align 3
.L135:
	movq	40(%rsp), %r9
	movq	16(%rsp), %rax
	xorl	%r10d, %r10d
	mulq	%rcx
	imulq	%rcx, %r9
	addq	%rdx, %r9
	movq	%r9, %rax
	imulq	%r14, %r9
	subq	%r9, %rcx
	cmpq	%r14, %rcx
	setnb	%r10b
	addq	%r10, %rax
	movq	%rax, %r8
	cmpq	%r14, %rcx
	jb	.L325
	subq	%r14, %rcx
.L325:
	movq	%rdi, %r9
	movq	(%rsp), %rax
	movq	$-1, %rsi
	xorl	%ebx, %ebx
	movabsq	$9223372036854775807, %rdi
	shrq	$63, %r9
	movq	$0, 144(%rsp)
	mulq	%r8
	movb	%r9b, 80(%rsp)
	xorl	%r11d, %r9d
	movq	%rdi, %rbp
	movzbl	%r9b, %r9d
	movq	%rsi, %rdi
	addq	%r9, %rdi
	adcq	$0, %rbp
	movq	%rdx, 152(%rsp)
	xorl	%edx, %edx
	orq	144(%rsp), %rax
	orq	152(%rsp), %rdx
	movq	%rax, %r8
	movq	%rdx, %r9
.L284:
	movq	%rcx, %rax
	orq	%rbx, %rax
	je	.L330
	movq	(%rsp), %rax
	mulq	%rcx
	movq	%rax, %rcx
	movq	%rdx, %rsi
	movq	%rax, %r10
	testq	%rdx, %rdx
	jne	.L344
	movq	$0, 104(%rsp)
	movq	40(%rsp), %rsi
	xorl	%ebx, %ebx
	imulq	%rax, %rsi
	movq	16(%rsp), %rax
	mulq	%rcx
	leaq	(%rsi,%rdx), %rcx
	movq	%r10, %rsi
	movq	%rcx, %rax
	imulq	%r14, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r14, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%r14, %rax
	cmpq	%r14, %rsi
	cmovnb	%rax, %rsi
	movq	%rsi, 96(%rsp)
.L154:
	movq	%rdi, %rax
	movq	%rbp, %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L147
	addq	%rcx, %r8
	movq	%rbp, %rax
	adcq	%rbx, %r9
	cmpq	%r8, %rdi
	sbbq	%r9, %rax
	jc	.L147
	movq	96(%rsp), %rax
	movq	104(%rsp), %rdx
	movq	%rax, %rcx
	orq	%rdx, %rcx
	je	.L330
	movb	$2, 296(%rsp)
	movb	$0, 312(%rsp)
	jmp	.L143
	.p2align 4,,10
	.p2align 3
.L163:
	movq	40(%rsp), %r9
	movq	16(%rsp), %rax
	xorl	%r10d, %r10d
	mulq	%rcx
	imulq	%rcx, %r9
	addq	%rdx, %r9
	movq	%r9, %rax
	imulq	%r14, %r9
	subq	%r9, %rcx
	cmpq	%r14, %rcx
	setnb	%r10b
	addq	%r10, %rax
	movq	%rax, %r8
	cmpq	%r14, %rcx
	jb	.L324
	subq	%r14, %rcx
.L324:
	movq	(%rsp), %rax
	movq	%rdi, %r9
	movq	$-1, %rsi
	xorl	%ebx, %ebx
	movabsq	$9223372036854775807, %rdi
	shrq	$63, %r9
	movq	$0, 128(%rsp)
	mulq	%r8
	movb	%r9b, 96(%rsp)
	xorl	%r11d, %r9d
	movq	%rdi, %rbp
	movzbl	%r9b, %r9d
	movq	%rsi, %rdi
	addq	%r9, %rdi
	movq	%rdx, %r8
	adcq	$0, %rbp
	xorl	%edx, %edx
	orq	128(%rsp), %rax
	movq	%r8, 136(%rsp)
	orq	136(%rsp), %rdx
	movq	%rax, %r8
	movq	%rdx, %r9
.L283:
	movq	%rcx, %rax
	orq	%rbx, %rax
	je	.L332
	movq	(%rsp), %rax
	mulq	%rcx
	movq	%rax, %rcx
	movq	%rdx, %rsi
	movq	%rax, %r10
	testq	%rdx, %rdx
	jne	.L345
	movq	40(%rsp), %rsi
	xorl	%ebx, %ebx
	movq	$0, 184(%rsp)
	imulq	%rax, %rsi
	movq	16(%rsp), %rax
	mulq	%rcx
	leaq	(%rsi,%rdx), %rcx
	movq	%r10, %rsi
	movq	%rcx, %rax
	imulq	%r14, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r14, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%r14, %rax
	cmpq	%r14, %rsi
	cmovnb	%rax, %rsi
	movq	%rsi, 176(%rsp)
.L182:
	movq	%rdi, %rax
	movq	%rbp, %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L175
	addq	%rcx, %r8
	movq	%rbp, %rax
	adcq	%rbx, %r9
	cmpq	%r8, %rdi
	sbbq	%r9, %rax
	jc	.L175
	movq	176(%rsp), %rax
	movq	184(%rsp), %rdx
	movq	%rax, %rcx
	orq	%rdx, %rcx
	je	.L332
	movb	$2, 320(%rsp)
	movb	$0, 336(%rsp)
	jmp	.L171
	.p2align 4,,10
	.p2align 3
.L107:
	movq	40(%rsp), %r9
	movq	16(%rsp), %rax
	xorl	%r10d, %r10d
	mulq	%rcx
	imulq	%rcx, %r9
	addq	%rdx, %r9
	movq	%r9, %rax
	imulq	%r14, %r9
	subq	%r9, %rcx
	cmpq	%r14, %rcx
	setnb	%r10b
	addq	%r10, %rax
	movq	%rax, %r8
	cmpq	%r14, %rcx
	jb	.L326
	subq	%r14, %rcx
.L326:
	movq	%rdi, %r9
	movq	(%rsp), %rax
	movq	$-1, %rsi
	xorl	%ebx, %ebx
	movabsq	$9223372036854775807, %rdi
	shrq	$63, %r9
	movq	$0, 160(%rsp)
	movb	%r9b, 48(%rsp)
	xorl	%r11d, %r9d
	mulq	%r8
	movq	%rdi, %rbp
	movzbl	%r9b, %r9d
	movq	%rsi, %rdi
	addq	%r9, %rdi
	adcq	$0, %rbp
	orq	160(%rsp), %rax
	movq	%rdx, 168(%rsp)
	xorl	%edx, %edx
	movq	%rax, %r8
	movq	%rcx, %rax
	orq	168(%rsp), %rdx
	orq	%rbx, %rax
	movq	%rdx, %r9
	jne	.L346
.L328:
	cmpb	%r11b, 48(%rsp)
	je	.L124
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L124:
	movq	%r8, 272(%rsp)
	movq	%r9, 280(%rsp)
	movb	$1, 288(%rsp)
	movb	$1, 48(%rsp)
	jmp	.L123
	.p2align 4,,10
	.p2align 3
.L191:
	movq	40(%rsp), %r9
	movq	16(%rsp), %rax
	xorl	%r10d, %r10d
	mulq	%rcx
	imulq	%rcx, %r9
	addq	%rdx, %r9
	movq	%r9, %rax
	imulq	%r14, %r9
	subq	%r9, %rcx
	cmpq	%r14, %rcx
	setnb	%r10b
	addq	%r10, %rax
	movq	%rax, %r8
	cmpq	%r14, %rcx
	jb	.L323
	subq	%r14, %rcx
.L323:
	movq	(%rsp), %rax
	movq	%rdi, %r9
	movq	$-1, %rsi
	xorl	%ebx, %ebx
	movabsq	$9223372036854775807, %rdi
	shrq	$63, %r9
	movq	$0, 112(%rsp)
	mulq	%r8
	movb	%r9b, 176(%rsp)
	xorl	%r11d, %r9d
	movq	%rdi, %rbp
	movzbl	%r9b, %r9d
	movq	%rsi, %rdi
	addq	%r9, %rdi
	movq	%rdx, %r8
	adcq	$0, %rbp
	xorl	%edx, %edx
	orq	112(%rsp), %rax
	movq	%r8, 120(%rsp)
	orq	120(%rsp), %rdx
	movq	%rax, %r8
	movq	%rdx, %r9
.L282:
	movq	%rcx, %rax
	orq	%rbx, %rax
	je	.L334
	movq	(%rsp), %rax
	mulq	%rcx
	movq	%rax, %rcx
	movq	%rdx, %rsi
	movq	%rax, %r10
	testq	%rdx, %rdx
	jne	.L347
	movq	40(%rsp), %rsi
	xorl	%ebx, %ebx
	movq	$0, 232(%rsp)
	imulq	%rax, %rsi
	movq	16(%rsp), %rax
	mulq	%rcx
	leaq	(%rsi,%rdx), %rcx
	movq	%r10, %rsi
	movq	%rcx, %rax
	imulq	%r14, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r14, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%r14, %rax
	cmpq	%r14, %rsi
	cmovnb	%rax, %rsi
	movq	%rsi, 224(%rsp)
.L210:
	movq	%rdi, %rax
	movq	%rbp, %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L203
	addq	%rcx, %r8
	movq	%rbp, %rax
	adcq	%rbx, %r9
	cmpq	%r8, %rdi
	sbbq	%r9, %rax
	jc	.L203
	movq	224(%rsp), %rax
	movq	232(%rsp), %rdx
	movq	%rax, %rcx
	orq	%rdx, %rcx
	je	.L334
	movb	$2, 344(%rsp)
	xorl	%eax, %eax
	movb	$0, 360(%rsp)
	jmp	.L348
	.p2align 4,,10
	.p2align 3
.L335:
	shrq	$63, %r12
	xorl	%r15d, %r15d
	cmpq	$3, 32(%rsp)
	movq	56(%rsp), %r13
	movq	%r12, %r14
	movq	64(%rsp), %r12
	jbe	.L349
	movq	%rbx, 80(%rsp)
	movq	(%rsp), %r10
	movq	%rbp, 48(%rsp)
.L244:
	movq	(%r12), %rsi
	movq	8(%r12), %rdi
	movq	%r10, %rax
	movq	%rsi, %rcx
	movq	%rdi, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rsi, %rcx
	cmovs	%rdi, %r8
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rax, %rcx
	movq	%r8, %rax
	movq	%rdx, %rbx
	mulq	%r10
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	jne	.L225
	movq	%rdi, %rsi
	movq	%r8, %r9
	xorl	%ebx, %ebx
	xorl	%r8d, %r8d
	shrq	$63, %rsi
	orq	%r8, %rcx
	orq	%r9, %rbx
	xorl	%eax, %eax
	movabsq	$-9223372036854775808, %rbp
	cmpb	%r14b, %sil
	sete	%al
	negq	%rax
	sbbq	%rdx, %rdx
	xorl	%edi, %edi
	xorq	%rdi, %rax
	xorq	%rbp, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L225
	cmpb	%r14b, %sil
	je	.L228
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L228:
	movq	%rcx, 272(%rsp)
	movl	$1, %ebp
	movq	%rbx, 280(%rsp)
	movb	$1, 288(%rsp)
	jmp	.L227
	.p2align 4,,10
	.p2align 3
.L225:
	movb	$0, 272(%rsp)
	xorl	%ebp, %ebp
	movb	$0, 288(%rsp)
.L227:
	movq	16(%r12), %rsi
	movq	24(%r12), %rdi
	movq	%r10, %rax
	movq	%rsi, %rcx
	movq	%rdi, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rsi, %rcx
	cmovs	%rdi, %r8
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rax, %rcx
	movq	%r8, %rax
	movq	%rdx, %rbx
	mulq	%r10
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	jne	.L229
	movq	%rdi, %rsi
	movq	%r8, %r9
	xorl	%ebx, %ebx
	xorl	%r8d, %r8d
	shrq	$63, %rsi
	orq	%r8, %rcx
	orq	%r9, %rbx
	xorl	%eax, %eax
	movabsq	$-9223372036854775808, %r9
	cmpb	%r14b, %sil
	sete	%al
	negq	%rax
	sbbq	%rdx, %rdx
	xorl	%r8d, %r8d
	xorq	%r8, %rax
	xorq	%r9, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L229
	cmpb	%r14b, %sil
	je	.L232
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L232:
	movq	%rcx, 296(%rsp)
	movq	%rbx, 304(%rsp)
	movb	$1, 312(%rsp)
	movb	$1, 16(%rsp)
	jmp	.L231
	.p2align 4,,10
	.p2align 3
.L229:
	movb	$0, 296(%rsp)
	movb	$0, 312(%rsp)
	movb	$0, 16(%rsp)
.L231:
	movq	32(%r12), %rsi
	movq	40(%r12), %rdi
	movq	%r10, %rax
	movq	%rsi, %rcx
	movq	%rdi, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rsi, %rcx
	cmovs	%rdi, %r8
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rax, %rcx
	movq	%r8, %rax
	movq	%rdx, %rbx
	mulq	%r10
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	jne	.L233
	movq	%rdi, %rsi
	movq	%r8, %r9
	xorl	%ebx, %ebx
	xorl	%r8d, %r8d
	shrq	$63, %rsi
	orq	%r8, %rcx
	orq	%r9, %rbx
	xorl	%eax, %eax
	movabsq	$-9223372036854775808, %r9
	cmpb	%r14b, %sil
	sete	%al
	negq	%rax
	sbbq	%rdx, %rdx
	xorl	%r8d, %r8d
	xorq	%r8, %rax
	xorq	%r9, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L233
	cmpb	%r14b, %sil
	je	.L236
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L236:
	movq	%rcx, 320(%rsp)
	movq	%rbx, 328(%rsp)
	movb	$1, 336(%rsp)
	movb	$1, 40(%rsp)
	jmp	.L235
	.p2align 4,,10
	.p2align 3
.L233:
	movb	$0, 320(%rsp)
	movb	$0, 336(%rsp)
	movb	$0, 40(%rsp)
.L235:
	movq	48(%r12), %rsi
	movq	56(%r12), %rdi
	movq	%rsi, %rcx
	movq	%rdi, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rsi, %rcx
	cmovs	%rdi, %r8
	xorl	%r9d, %r9d
	movq	%rcx, %rax
	mulq	%r10
	movq	%rax, %rcx
	movq	%r8, %rax
	movq	%rdx, %rbx
	mulq	%r10
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	jne	.L237
	movq	%rdi, %rsi
	movq	%r8, %r9
	xorl	%ebx, %ebx
	xorl	%r8d, %r8d
	shrq	$63, %rsi
	orq	%r8, %rcx
	orq	%r9, %rbx
	xorl	%eax, %eax
	movabsq	$-9223372036854775808, %r9
	cmpb	%r14b, %sil
	sete	%al
	negq	%rax
	sbbq	%rdx, %rdx
	xorl	%r8d, %r8d
	xorq	%r8, %rax
	xorq	%r9, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L237
	cmpb	%r14b, %sil
	je	.L240
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L240:
	movq	%rcx, 344(%rsp)
	movl	$1, %eax
	movq	%rbx, 352(%rsp)
	movb	$1, 360(%rsp)
	jmp	.L239
	.p2align 4,,10
	.p2align 3
.L237:
	movb	$0, 344(%rsp)
	xorl	%eax, %eax
	movb	$0, 360(%rsp)
.L239:
	testb	%bpl, %bpl
	je	.L318
	movdqa	272(%rsp), %xmm0
	cmpb	$0, 16(%rsp)
	movups	%xmm0, 0(%r13)
	je	.L350
	movdqu	296(%rsp), %xmm0
	cmpb	$0, 40(%rsp)
	movups	%xmm0, 16(%r13)
	je	.L351
	movdqa	320(%rsp), %xmm0
	movups	%xmm0, 32(%r13)
	testb	%al, %al
	je	.L352
	movq	32(%rsp), %rax
	addq	$4, %r15
	addq	$64, %r12
	addq	$64, %r13
	movdqu	344(%rsp), %xmm0
	subq	%r15, %rax
	movups	%xmm0, -16(%r13)
	cmpq	$3, %rax
	ja	.L244
	movq	32(%rsp), %rax
	movq	48(%rsp), %rbp
	cmpq	%rax, %r15
	jnb	.L106
.L247:
	movq	%r15, %r10
	xorl	%r12d, %r12d
	movq	56(%rsp), %r9
	movq	64(%rsp), %r8
	movabsq	$-9223372036854775808, %r13
	movq	%r12, 16(%rsp)
	salq	$4, %r10
	movq	%r13, 24(%rsp)
	movq	32(%rsp), %r13
	jmp	.L280
	.p2align 4,,10
	.p2align 3
.L353:
	movq	%rbx, %rsi
	movq	%rax, %rdx
	xorl	%ebx, %ebx
	xorl	%eax, %eax
	shrq	$63, %rsi
	orq	%rbx, %rdx
	orq	%r11, %rax
	xorl	%ecx, %ecx
	cmpb	%r14b, %sil
	sete	%cl
	negq	%rcx
	sbbq	%rbx, %rbx
	xorq	16(%rsp), %rcx
	xorq	24(%rsp), %rbx
	movq	%rbx, %rdi
	cmpq	%rax, %rcx
	sbbq	%rdx, %rdi
	jc	.L277
	cmpb	%r14b, %sil
	je	.L279
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
.L279:
	movq	%rax, 240(%rsp)
	addq	$1, %r15
	movq	%rdx, 248(%rsp)
	movdqa	240(%rsp), %xmm0
	movups	%xmm0, (%r9,%r10)
	addq	$16, %r10
	cmpq	%r15, %r13
	je	.L106
.L280:
	movq	(%r8,%r10), %rcx
	movq	8(%r8,%r10), %rbx
	movq	(%rsp), %rax
	movq	%rcx, %rdx
	movq	%rbx, %rsi
	negq	%rdx
	adcq	$0, %rsi
	negq	%rsi
	cmovs	%rcx, %rdx
	cmovs	%rbx, %rsi
	xorl	%edi, %edi
	mulq	%rdx
	movq	%rax, %r11
	movq	(%rsp), %rax
	movq	%rdx, %r12
	mulq	%rsi
	addq	%r12, %rax
	adcq	%rdi, %rdx
	xorl	%edi, %edi
	movq	%rdx, %rsi
	orq	%rdi, %rsi
	je	.L353
.L277:
	movb	$0, 240(%rsp)
	jmp	.L276
	.p2align 4,,10
	.p2align 3
.L330:
	cmpb	%r11b, 80(%rsp)
	je	.L152
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L152:
	movq	%r8, 296(%rsp)
	movq	%r9, 304(%rsp)
	movb	$1, 312(%rsp)
	movb	$1, 80(%rsp)
	jmp	.L151
	.p2align 4,,10
	.p2align 3
.L332:
	cmpb	%r11b, 96(%rsp)
	je	.L180
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L180:
	movq	%r8, 320(%rsp)
	movq	%r9, 328(%rsp)
	movb	$1, 336(%rsp)
	movb	$1, 96(%rsp)
	jmp	.L179
	.p2align 4,,10
	.p2align 3
.L334:
	cmpb	%r11b, 176(%rsp)
	jne	.L216
.L208:
	cmpb	$0, 48(%rsp)
	movq	%r8, 344(%rsp)
	movl	$1, %eax
	movq	%r9, 352(%rsp)
	movb	$1, 360(%rsp)
	je	.L317
.L343:
	movdqa	272(%rsp), %xmm0
	cmpb	$0, 80(%rsp)
	movups	%xmm0, 0(%r13)
	je	.L354
	movdqu	296(%rsp), %xmm0
	cmpb	$0, 96(%rsp)
	movups	%xmm0, 16(%r13)
	je	.L355
	movdqa	320(%rsp), %xmm0
	movups	%xmm0, 32(%r13)
	testb	%al, %al
	je	.L356
	movq	32(%rsp), %rax
	addq	$4, %r15
	addq	$64, %r12
	addq	$64, %r13
	movdqu	344(%rsp), %xmm0
	subq	%r15, %rax
	movups	%xmm0, -16(%r13)
	cmpq	$3, %rax
	ja	.L223
	movq	32(%rsp), %rax
	movq	192(%rsp), %rbp
	cmpq	%rax, %r15
	jnb	.L106
.L105:
	movq	%rbp, 48(%rsp)
	movq	%r15, %r13
	movq	56(%rsp), %rbp
	movl	%r11d, %r12d
	movq	64(%rsp), %r8
	salq	$4, %r13
	.p2align 4
	.p2align 3
.L264:
	movq	(%r8,%r13), %rcx
	movq	8(%r8,%r13), %rbx
	movq	%rcx, %rax
	movq	%rbx, %rdx
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rcx, %rax
	cmovs	%rbx, %rdx
	movq	%rax, %r9
	movq	%rdx, %r10
	movq	%rax, %r11
	testq	%rdx, %rdx
	je	.L248
	cmpq	%r14, %rdx
	jb	.L249
	movq	16(%rsp), %rax
	mulq	%rdx
	movq	%r14, %rax
	imulq	%rdx, %rax
	subq	%rax, %r10
	xorl	%eax, %eax
	cmpq	%r14, %r10
	setnb	%al
	movq	%rax, %rsi
	movq	%r10, %rax
	addq	%rdx, %rsi
	subq	%r14, %rax
	cmpq	%r14, %r10
	cmovnb	%rax, %r10
	movq	%r9, %rax
	xorl	%edi, %edi
	movq	%r10, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	%rax, %r9
	movq	(%rsp), %rax
	movq	%rdx, 64(%rsp)
	mulq	%r9
	movq	%rdx, %r11
	movq	%rax, %r10
	movq	%rsi, %rax
	mulq	(%rsp)
	movq	%r11, %rsi
	addq	%rax, %rsi
	adcq	%rdx, %rdi
	xorl	%edx, %edx
	movq	%rdi, %rax
	orq	%rdx, %rax
	je	.L357
.L260:
	movb	$0, 240(%rsp)
	movq	48(%rsp), %rbp
.L276:
	movzbl	240(%rsp), %eax
	movq	%r15, 0(%rbp)
	movb	$0, 16(%rbp)
	movb	%al, 8(%rbp)
	.p2align 4
	.p2align 3
.L98:
	movq	376(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L358
	addq	$392, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%rbp, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L136:
	.cfi_restore_state
	movq	%rcx, %rax
	movq	%rdi, %rsi
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	(%rsp), %rax
	movq	%rdx, %rcx
	shrq	$63, %rsi
	movb	%sil, 80(%rsp)
	xorl	%r11d, %esi
	mulq	%r8
	movzbl	%sil, %esi
	movq	%rax, %r8
	movq	$-1, %rax
	movq	%rdx, %r9
	movabsq	$9223372036854775807, %rdx
	addq	%rsi, %rax
	adcq	$0, %rdx
	movq	%rax, %rdi
	movq	%rdx, %rbp
	jmp	.L284
	.p2align 4,,10
	.p2align 3
.L192:
	movq	%rcx, %rax
	movq	%rdi, %rsi
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	(%rsp), %rax
	movq	%rdx, %rcx
	shrq	$63, %rsi
	movb	%sil, 176(%rsp)
	xorl	%r11d, %esi
	mulq	%r8
	movzbl	%sil, %esi
	movq	%rax, %r8
	movq	$-1, %rax
	movq	%rdx, %r9
	movabsq	$9223372036854775807, %rdx
	addq	%rsi, %rax
	adcq	$0, %rdx
	movq	%rax, %rdi
	movq	%rdx, %rbp
	jmp	.L282
	.p2align 4,,10
	.p2align 3
.L164:
	movq	%rcx, %rax
	movq	%rdi, %rsi
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	(%rsp), %rax
	movq	%rdx, %rcx
	shrq	$63, %rsi
	movb	%sil, 96(%rsp)
	xorl	%r11d, %esi
	mulq	%r8
	movzbl	%sil, %esi
	movq	%rax, %r8
	movq	$-1, %rax
	movq	%rdx, %r9
	movabsq	$9223372036854775807, %rdx
	addq	%rsi, %rax
	adcq	$0, %rdx
	movq	%rax, %rdi
	movq	%rdx, %rbp
	jmp	.L283
	.p2align 4,,10
	.p2align 3
.L216:
	negq	%r8
	adcq	$0, %r9
	negq	%r9
	jmp	.L208
	.p2align 4,,10
	.p2align 3
.L338:
	cmpq	%r14, %rdx
	jnb	.L125
	movq	$0, 80(%rsp)
	movq	$0, 88(%rsp)
.L128:
	movq	%rcx, %rax
	movq	%rsi, %rdx
	xorl	%ebx, %ebx
	orq	88(%rsp), %rbx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	$0, 88(%rsp)
	movq	%rax, %rcx
	orq	80(%rsp), %rcx
	movq	%rdx, 80(%rsp)
	jmp	.L126
	.p2align 4,,10
	.p2align 3
.L347:
	cmpq	%r14, %rdx
	jnb	.L209
	movq	$0, 224(%rsp)
	movq	$0, 232(%rsp)
.L212:
	movq	%rcx, %rax
	movq	%rsi, %rdx
	xorl	%ebx, %ebx
	orq	232(%rsp), %rbx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	$0, 232(%rsp)
	movq	%rax, %rcx
	orq	224(%rsp), %rcx
	movq	%rdx, 224(%rsp)
	jmp	.L210
	.p2align 4,,10
	.p2align 3
.L345:
	cmpq	%r14, %rdx
	jnb	.L181
	movq	$0, 176(%rsp)
	movq	$0, 184(%rsp)
.L184:
	movq	%rcx, %rax
	movq	%rsi, %rdx
	xorl	%ebx, %ebx
	orq	184(%rsp), %rbx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	$0, 184(%rsp)
	movq	%rax, %rcx
	orq	176(%rsp), %rcx
	movq	%rdx, 176(%rsp)
	jmp	.L182
	.p2align 4,,10
	.p2align 3
.L344:
	cmpq	%r14, %rdx
	jnb	.L153
	movq	$0, 96(%rsp)
	movq	$0, 104(%rsp)
.L156:
	movq	%rcx, %rax
	movq	%rsi, %rdx
	xorl	%ebx, %ebx
	orq	104(%rsp), %rbx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	$0, 104(%rsp)
	movq	%rax, %rcx
	orq	96(%rsp), %rcx
	movq	%rdx, 96(%rsp)
	jmp	.L154
	.p2align 4,,10
	.p2align 3
.L354:
	movq	192(%rsp), %rbp
	addq	$1, %r15
	movl	$1, %ebx
	jmp	.L219
	.p2align 4,,10
	.p2align 3
.L355:
	movq	192(%rsp), %rbp
	addq	$2, %r15
	movl	$2, %ebx
	jmp	.L219
	.p2align 4,,10
	.p2align 3
.L248:
	movq	40(%rsp), %rsi
	imulq	%rax, %rsi
	movq	16(%rsp), %rax
	mulq	%r9
	addq	%rdx, %rsi
	movq	%rsi, %rax
	imulq	%r14, %rax
	subq	%rax, %r11
	xorl	%eax, %eax
	cmpq	%r14, %r11
	setnb	%al
	addq	%rax, %rsi
	cmpq	%r14, %r11
	jb	.L322
	subq	%r14, %r11
.L322:
	movq	(%rsp), %rax
	movq	%r11, %r10
	movq	$-1, %rcx
	xorl	%r11d, %r11d
	movq	$0, 208(%rsp)
	mulq	%rsi
	movq	%rbx, %rsi
	movabsq	$9223372036854775807, %rbx
	shrq	$63, %rsi
	movb	%sil, 56(%rsp)
	xorl	%r12d, %esi
	movzbl	%sil, %esi
	movq	%rdx, %r9
	addq	%rsi, %rcx
	movq	%r9, 216(%rsp)
	movq	%rax, %rsi
	adcq	$0, %rbx
	xorl	%edx, %edx
	orq	208(%rsp), %rsi
	movq	%rdx, %rdi
	orq	216(%rsp), %rdi
.L281:
	movq	%r10, %rax
	orq	%r11, %rax
	je	.L257
	movq	(%rsp), %rax
	mulq	%r10
	movq	%rax, 64(%rsp)
	movq	%rax, %r10
	movq	%rdx, %r9
	testq	%rdx, %rdx
	je	.L258
	cmpq	%r14, %rdx
	jnb	.L266
	movq	$0, 64(%rsp)
	movq	$0, 72(%rsp)
.L269:
	movq	%r10, %rax
	xorl	%r11d, %r11d
	movq	%r9, %rdx
	orq	72(%rsp), %r11
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	$0, 72(%rsp)
	movq	%rax, %r10
	orq	64(%rsp), %r10
	movq	%rdx, 64(%rsp)
.L267:
	movq	%rcx, %rax
	movq	%rbx, %rdx
	subq	%rsi, %rax
	sbbq	%rdi, %rdx
	cmpq	%r10, %rax
	movq	%rdx, %rax
	sbbq	%r11, %rax
	jc	.L260
	addq	%rsi, %r10
	movq	%rbx, %rax
	adcq	%rdi, %r11
	cmpq	%r10, %rcx
	sbbq	%r11, %rax
	jc	.L260
	movq	64(%rsp), %rax
	movq	72(%rsp), %rdx
	movq	%rax, %rsi
	orq	%rdx, %rsi
	je	.L359
	movb	$2, 240(%rsp)
	movq	48(%rsp), %rbp
	jmp	.L276
	.p2align 4,,10
	.p2align 3
.L356:
	movq	192(%rsp), %rbp
	addq	$3, %r15
	movl	$3, %ebx
	jmp	.L219
.L318:
	movq	80(%rsp), %rbx
	movq	48(%rsp), %rbp
	jmp	.L219
.L350:
	movq	48(%rsp), %rbp
	addq	$1, %r15
	movl	$1, %ebx
	jmp	.L219
.L351:
	movq	48(%rsp), %rbp
	addq	$2, %r15
	movl	$2, %ebx
	jmp	.L219
.L357:
	movq	%rbx, %r9
	movq	$-1, %rcx
	movq	%rsi, %rdi
	movq	%r10, %rsi
	movabsq	$9223372036854775807, %rbx
	shrq	$63, %r9
	movb	%r9b, 56(%rsp)
	xorl	%r12d, %r9d
	movzbl	%r9b, %r9d
	addq	%r9, %rcx
	adcq	$0, %rbx
	xorl	%r11d, %r11d
	orq	%r11, %rdi
	movq	%rbx, %rax
	cmpq	%r10, %rcx
	sbbq	%rdi, %rax
	jc	.L260
	movq	64(%rsp), %r10
	xorl	%r11d, %r11d
	jmp	.L281
	.p2align 4,,10
	.p2align 3
.L249:
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r14
# 0 "" 2
#NO_APP
	movq	%rax, %r9
	movq	(%rsp), %rax
	movq	%rdx, %r10
	xorl	%r11d, %r11d
	movq	$-1, %rcx
	mulq	%r9
	movq	%rax, %rsi
	movq	%rbx, %rax
	movq	%rdx, %rdi
	movabsq	$9223372036854775807, %rbx
	shrq	$63, %rax
	movb	%al, 56(%rsp)
	xorl	%r12d, %eax
	movzbl	%al, %eax
	addq	%rax, %rcx
	adcq	$0, %rbx
	jmp	.L281
.L257:
	cmpb	%r12b, 56(%rsp)
	jne	.L273
.L275:
	movq	%rsi, %rax
	movq	%rdi, %rdx
.L274:
	movq	%rax, 240(%rsp)
	addq	$1, %r15
	movq	%rdx, 248(%rsp)
	movdqa	240(%rsp), %xmm0
	movups	%xmm0, 0(%rbp,%r13)
	addq	$16, %r13
	cmpq	%r15, 32(%rsp)
	jne	.L264
	movq	48(%rsp), %rbp
.L106:
	movb	$1, 16(%rbp)
	jmp	.L98
.L352:
	movq	48(%rsp), %rbp
	addq	$3, %r15
	movl	$3, %ebx
	jmp	.L219
.L273:
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
	jmp	.L275
.L258:
	movq	40(%rsp), %r9
	xorl	%r11d, %r11d
	movq	$0, 72(%rsp)
	imulq	%rax, %r9
	movq	16(%rsp), %rax
	mulq	%r10
	addq	%rdx, %r9
	movq	%r9, %rax
	movq	%r9, %r10
	movq	64(%rsp), %r9
	imulq	%r14, %rax
	subq	%rax, %r9
	xorl	%eax, %eax
	cmpq	%r14, %r9
	setnb	%al
	addq	%rax, %r10
	movq	%r9, %rax
	adcq	$0, %r11
	subq	%r14, %rax
	cmpq	%r14, %r9
	cmovnb	%rax, %r9
	movq	%r9, 64(%rsp)
	jmp	.L267
.L153:
	movq	16(%rsp), %rax
	mulq	%rdx
	movq	%rdx, %rax
	imulq	%r14, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r14, %rsi
	movq	%rsi, %r10
	setnb	%al
	addq	%rdx, %rax
	subq	%r14, %r10
	cmpq	%r14, %rsi
	movq	%rax, %rdx
	cmovnb	%r10, %rsi
	xorl	%eax, %eax
	movq	%rax, 96(%rsp)
	movq	%rdx, 104(%rsp)
	jmp	.L156
.L181:
	movq	16(%rsp), %rax
	mulq	%rdx
	movq	%rdx, %rax
	imulq	%r14, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r14, %rsi
	movq	%rsi, %r10
	setnb	%al
	addq	%rdx, %rax
	subq	%r14, %r10
	cmpq	%r14, %rsi
	movq	%rax, %rdx
	cmovnb	%r10, %rsi
	xorl	%eax, %eax
	movq	%rax, 176(%rsp)
	movq	%rdx, 184(%rsp)
	jmp	.L184
.L209:
	movq	16(%rsp), %rax
	mulq	%rdx
	movq	%rdx, %rax
	imulq	%r14, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r14, %rsi
	movq	%rsi, %r10
	setnb	%al
	addq	%rdx, %rax
	subq	%r14, %r10
	cmpq	%r14, %rsi
	movq	%rax, %rdx
	cmovnb	%r10, %rsi
	xorl	%eax, %eax
	movq	%rax, 224(%rsp)
	movq	%rdx, 232(%rsp)
	jmp	.L212
.L125:
	movq	16(%rsp), %rax
	mulq	%rdx
	movq	%rdx, %rax
	imulq	%r14, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r14, %rsi
	movq	%rsi, %r10
	setnb	%al
	addq	%rdx, %rax
	subq	%r14, %r10
	cmpq	%r14, %rsi
	movq	%rax, %rdx
	cmovnb	%r10, %rsi
	xorl	%eax, %eax
	movq	%rax, 80(%rsp)
	movq	%rdx, 88(%rsp)
	jmp	.L128
.L336:
	cmpq	$0, 32(%rsp)
	jne	.L105
	jmp	.L106
.L349:
	cmpq	$0, 32(%rsp)
	jne	.L247
	jmp	.L106
	.p2align 4,,10
	.p2align 3
.L359:
	cmpb	%r12b, 56(%rsp)
	je	.L265
	negq	%r10
	adcq	$0, %r11
	negq	%r11
.L265:
	movq	%r10, %rax
	movq	%r11, %rdx
	jmp	.L274
.L266:
	movq	16(%rsp), %rax
	mulq	%rdx
	movq	%rdx, %rax
	movq	%rdx, 64(%rsp)
	imulq	%r14, %rax
	subq	%rax, %r9
	xorl	%eax, %eax
	cmpq	%r14, %r9
	setnb	%al
	xorl	%edx, %edx
	addq	64(%rsp), %rax
	movq	%rax, 64(%rsp)
	movq	%r9, %rax
	adcq	$0, %rdx
	subq	%r14, %rax
	cmpq	%r14, %r9
	movq	%rdx, 72(%rsp)
	cmovnb	%rax, %r9
	movq	64(%rsp), %rax
	movq	%rax, %rdx
	xorl	%eax, %eax
	movq	%rax, 64(%rsp)
	movq	%rdx, 72(%rsp)
	jmp	.L269
.L358:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1690:
	.size	_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE, .-_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.type	_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_, @function
_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_:
.LFB1691:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rdi, %rbx
	subq	$296, %rsp
	.cfi_def_cfa_offset 352
	movq	%rcx, 32(%rsp)
	movq	%r8, 40(%rsp)
	movq	%r9, 48(%rsp)
	movq	%fs:40, %rax
	movq	%rax, 280(%rsp)
	xorl	%eax, %eax
	testq	%rcx, %rcx
	je	.L391
	movq	40(%rsp), %rdi
	movq	%r9, %rax
	movq	%r9, 8(%rsp)
	movq	%rdx, %r15
	sarq	$63, %rax
	movq	%rbx, 56(%rsp)
	xorl	%r14d, %r14d
	movq	%r9, %r13
	movq	%rax, %xmm3
	movq	%rdi, %rax
	movq	%rdi, (%rsp)
	movq	%rsi, %r12
	sarq	$63, %rax
	punpcklqdq	%xmm3, %xmm3
	movq	%rax, 24(%rsp)
	movaps	%xmm3, 64(%rsp)
	.p2align 4
	.p2align 3
.L390:
	movdqu	(%r15), %xmm0
	xorl	%r9d, %r9d
	movq	%xmm0, %rcx
	movhlps	%xmm0, %xmm1
	movq	%rcx, %rax
	movq	%xmm1, %r8
	sarq	$63, %rax
	movq	%r8, %rbx
	movq	%r8, %rsi
	cmpq	%r8, %rax
	jne	.L365
	cmpq	%r13, 24(%rsp)
	jne	.L366
	movq	(%rsp), %rax
	imulq	%rcx
.L363:
	testq	%r9, %r9
	je	.L373
	movq	40(%rsp), %rax
	sarq	$63, %r8
	movdqa	64(%rsp), %xmm2
	movaps	%xmm0, 176(%rsp)
	movq	%r8, %xmm0
	movl	$128, %r9d
	movl	$3, %r8d
	movl	$12, %ecx
	punpcklqdq	%xmm0, %xmm0
	movq	%rax, 208(%rsp)
	movq	48(%rsp), %rax
	leaq	240(%rsp), %rdi
	subq	$64, %rsp
	.cfi_def_cfa_offset 416
	movl	$12, %edx
	movl	$12, %esi
	movaps	%xmm2, 288(%rsp)
	movq	%rax, 280(%rsp)
	movaps	%xmm0, 256(%rsp)
	movdqa	272(%rsp), %xmm0
	movups	%xmm0, 32(%rsp)
	movdqa	288(%rsp), %xmm0
	movups	%xmm0, 48(%rsp)
	movdqa	240(%rsp), %xmm0
	movups	%xmm0, (%rsp)
	movdqa	256(%rsp), %xmm0
	movups	%xmm0, 16(%rsp)
	call	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	addq	$64, %rsp
	.cfi_def_cfa_offset 352
	cmpb	$0, 272(%rsp)
	jne	.L374
	movzbl	240(%rsp), %eax
	movq	56(%rsp), %rbx
	movb	%al, 96(%rsp)
.L388:
	movzbl	96(%rsp), %eax
	movq	%r14, (%rbx)
	movb	$0, 16(%rbx)
	movb	%al, 8(%rbx)
.L360:
	movq	280(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L406
	addq	$296, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%rbx, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L373:
	.cfi_restore_state
	movq	%rax, %r9
	movq	%rdx, %r10
	testq	%rdx, %rdx
	js	.L376
	movq	%rdx, %rcx
	testq	%rdx, %rdx
	jne	.L377
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	movabsq	$1000000000000, %r8
	mulq	%r9
	shrq	$37, %rdx
	movq	%rdx, %rsi
	imulq	%r8, %rdx
	subq	%rdx, %r9
	je	.L378
	movq	%r9, %rax
	xorl	%edx, %edx
	addq	%r9, %rax
	adcq	%rdx, %rdx
	cmpq	%rax, %r8
	movq	%rax, %r10
	sbbq	%rdx, %rcx
	jnc	.L379
	addq	$1, %rsi
	adcq	$0, %rdi
	.p2align 4
	.p2align 3
.L378:
	movb	$1, 112(%rsp)
	cmpb	$0, 112(%rsp)
	movq	%rsi, 96(%rsp)
	movq	%rdi, 104(%rsp)
	movq	%rsi, 128(%rsp)
	movq	%rdi, 136(%rsp)
	jne	.L387
	movzbl	128(%rsp), %eax
	movq	56(%rsp), %rbx
	movb	%al, 96(%rsp)
	jmp	.L388
	.p2align 4,,10
	.p2align 3
.L374:
	movdqa	240(%rsp), %xmm0
	movaps	%xmm0, 96(%rsp)
.L389:
	movq	%r14, %rax
	movdqa	96(%rsp), %xmm0
	addq	$1, %r14
	addq	$16, %r15
	salq	$4, %rax
	movups	%xmm0, (%r12,%rax)
	cmpq	%r14, 32(%rsp)
	jne	.L390
	movq	56(%rsp), %rbx
.L391:
	movb	$1, 16(%rbx)
	jmp	.L360
	.p2align 4,,10
	.p2align 3
.L387:
	movq	128(%rsp), %rax
	movq	%rax, 96(%rsp)
	movq	136(%rsp), %rax
	movq	%rax, 104(%rsp)
	jmp	.L389
	.p2align 4,,10
	.p2align 3
.L376:
	negq	%rax
	adcq	$0, %rdx
	movq	%rax, %r8
	movq	%rax, %r11
	negq	%rdx
	movq	%rdx, %rcx
	jne	.L380
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	mulq	%r8
	movabsq	$1000000000000, %r8
	shrq	$37, %rdx
	movq	%rdx, %rsi
	imulq	%r8, %rdx
	subq	%rdx, %r11
	je	.L381
	movq	%r11, %rax
	xorl	%edx, %edx
	addq	%r11, %rax
	adcq	%rdx, %rdx
	cmpq	%rax, %r8
	movq	%rax, %r10
	sbbq	%rdx, %rcx
	jnc	.L382
	addq	$1, %rsi
	adcq	$0, %rdi
.L381:
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
	jmp	.L378
	.p2align 4,,10
	.p2align 3
.L365:
	cmpq	%r13, 24(%rsp)
	jne	.L368
	movq	(%rsp), %rbp
	movq	%xmm0, 80(%rsp)
	movq	%xmm1, 88(%rsp)
.L367:
	movq	(%rsp), %rax
	mulq	%rcx
	movq	%rax, %r10
	movq	%rbp, %rax
	movq	%rdx, %r11
	mulq	%rsi
	testq	%rsi, %rsi
	jns	.L369
	xorl	%esi, %esi
	subq	%rsi, %rax
	sbbq	%rbp, %rdx
.L369:
	testq	%rbp, %rbp
	jns	.L370
	subq	80(%rsp), %rax
	sbbq	88(%rsp), %rdx
.L370:
	movq	%r11, %rsi
	xorl	%edi, %edi
	addq	%rax, %rsi
	adcq	%rdx, %rdi
	movq	%rsi, %rdx
	sarq	$63, %rdx
	cmpq	%rdi, %rdx
	jne	.L371
	movq	%r10, %rax
	movq	%rsi, %rdx
	jmp	.L363
	.p2align 4,,10
	.p2align 3
.L366:
	movdqa	(%rsp), %xmm4
	movq	%r13, %rsi
	movq	%rcx, %rbp
	movaps	%xmm4, 80(%rsp)
	jmp	.L367
	.p2align 4,,10
	.p2align 3
.L377:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %rdi
	mulq	%rdx
	movq	%rdx, %rsi
	movq	%r10, %rdx
	shrq	$37, %rsi
	movq	%rsi, %rax
	imulq	%rdi, %rax
	subq	%rax, %rdx
	movq	%r9, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rdi
# 0 "" 2
#NO_APP
	movq	%rsi, %rdi
	movq	%rax, %rsi
	movq	%rdx, %rax
	movq	%rdx, %rcx
	orq	%rbx, %rax
	je	.L378
.L383:
	xorl	%ebx, %ebx
	movl	$244140625, %eax
	addq	%rcx, %rcx
	adcq	%rbx, %rbx
	salq	$12, %rax
	xorl	%r8d, %r8d
	movq	%rcx, %rdx
	cmpq	%rcx, %rax
	sbbq	%rbx, %r8
	jc	.L407
	testq	%rbx, %rbx
	je	.L408
.L385:
	testq	%r10, %r10
	jns	.L378
	jmp	.L381
	.p2align 4,,10
	.p2align 3
.L380:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %rdi
	mulq	%rdx
	movq	%rdx, %rsi
	movq	%rcx, %rdx
	shrq	$37, %rsi
	movq	%rsi, %rax
	imulq	%rdi, %rax
	subq	%rax, %rdx
	movq	%r8, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rdi
# 0 "" 2
#NO_APP
	movq	%rsi, %rdi
	movq	%rax, %rsi
	movq	%rdx, %rax
	movq	%rdx, %rcx
	orq	%rbx, %rax
	jne	.L383
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
	jmp	.L378
	.p2align 4,,10
	.p2align 3
.L379:
	movabsq	$1000000000000, %rcx
	movq	%rcx, %rax
	xorq	%r10, %rax
	orq	%rdx, %rax
	jne	.L378
	addq	$1, %rsi
	adcq	$0, %rdi
	andq	$-2, %rsi
	jmp	.L378
	.p2align 4,,10
	.p2align 3
.L368:
	movq	(%rsp), %r10
	movq	8(%rsp), %r11
	movq	%r10, %rdi
	movq	%r11, %rdx
	movq	%r10, %rax
	imulq	%rcx, %rdx
	imulq	%r8, %rdi
	addq	%rdx, %rdi
	mulq	%rcx
	leaq	1(%r8), %rcx
	addq	%rdx, %rdi
	movq	%rdi, %rdx
	cmpq	$1, %rcx
	ja	.L364
	leaq	1(%r13), %rcx
	cmpq	$1, %rcx
	ja	.L364
	cmpq	%r13, %r8
	je	.L409
	testq	%rdi, %rdi
	js	.L363
	jmp	.L364
	.p2align 4,,10
	.p2align 3
.L382:
	movabsq	$1000000000000, %rcx
	movq	%rcx, %rax
	xorq	%r10, %rax
	orq	%rdx, %rax
	jne	.L381
	addq	$1, %rsi
	adcq	$0, %rdi
	andq	$-2, %rsi
	jmp	.L381
	.p2align 4,,10
	.p2align 3
.L409:
	cmpq	%rax, %r9
	movq	%r9, %rbx
	sbbq	%rdi, %rbx
	jl	.L363
	jmp	.L364
	.p2align 4,,10
	.p2align 3
.L371:
	movq	(%rsp), %r10
	movq	8(%rsp), %r11
	movq	%r10, %rdi
	movq	%r11, %rax
	imulq	%rcx, %rax
	imulq	%rbx, %rdi
	addq	%rax, %rdi
	movq	%r10, %rax
	mulq	%rcx
	addq	%rdx, %rdi
	movq	%rdi, %rdx
.L364:
	movl	$1, %r9d
	jmp	.L363
.L407:
	addq	$1, %rsi
	adcq	$0, %rdi
	jmp	.L385
.L406:
	call	__stack_chk_fail@PLT
.L408:
	cmpq	%rax, %rdx
	jne	.L385
	addq	$1, %rsi
	adcq	$0, %rdi
	andq	$-2, %rsi
	jmp	.L385
	.cfi_endproc
.LFE1691:
	.size	_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_, .-_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.type	_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_, @function
_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_:
.LFB1692:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movq	%r9, %xmm0
	movq	%r8, %xmm1
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movdqa	%xmm1, %xmm2
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	punpcklqdq	%xmm0, %xmm2
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rdi, %rbx
	subq	$344, %rsp
	.cfi_def_cfa_offset 400
	movq	%rdx, 64(%rsp)
	movq	%rcx, 48(%rsp)
	movq	%rsi, 80(%rsp)
	movq	%fs:40, %rax
	movq	%rax, 328(%rsp)
	xorl	%eax, %eax
	movaps	%xmm4, (%rsp)
	movq	8(%rsp), %rdx
	movq	%r8, (%rsp)
	movq	%rdx, 8(%rsp)
	movdqa	(%rsp), %xmm4
	punpcklqdq	%xmm0, %xmm4
	movaps	%xmm4, 32(%rsp)
	movq	32(%rsp), %rdx
	movq	40(%rsp), %rcx
	movq	%rdx, %rdi
	movq	%rcx, %rax
	negq	%rdi
	adcq	$0, %rax
	negq	%rax
	cmovs	%rdx, %rdi
	cmovs	%rcx, %rax
	movabsq	$2535301200456458803, %rdx
	movq	%rax, %r15
	movq	%rdi, %rbp
	testq	%rax, %rax
	jne	.L411
	movq	%rdi, %rax
	movq	%rdi, %rcx
	mulq	%rdx
	movabsq	$1000000000000, %rax
	shrq	$37, %rdx
	imulq	%rdx, %rax
	subq	%rax, %rcx
.L412:
	movabsq	$1000000000000, %r14
	movq	%rcx, %rax
	movq	%r14, %rdx
	.p2align 4
	.p2align 4
	.p2align 3
.L413:
	movq	%rdx, %rcx
	xorl	%edx, %edx
	divq	%rcx
	movq	%rcx, %rax
	testq	%rdx, %rdx
	jne	.L413
	movq	%rdx, %r14
	testq	%r15, %r15
	jne	.L414
	movq	$0, 24(%rsp)
	movq	%rbp, %rax
	xorl	%edx, %edx
	divq	%rcx
	xorl	%edx, %edx
	movq	%rax, 16(%rsp)
	movabsq	$1000000000000, %rax
	divq	%rcx
	movq	%rax, 56(%rsp)
	cmpq	$1, %rax
	je	.L415
	xorl	%ecx, %ecx
	xorl	%edi, %edi
	movq	%rax, %rdx
	movl	$1, %esi
	call	__udivti3@PLT
	cmpq	$0, 48(%rsp)
	movq	%rax, 96(%rsp)
	je	.L417
.L416:
	movq	%rbx, 88(%rsp)
	xorl	%r15d, %r15d
	.p2align 4
	.p2align 3
.L508:
	movq	64(%rsp), %rax
	movq	$-1, %rdi
	movabsq	$9223372036854775807, %rbp
	movq	(%rax,%r15), %r10
	movq	8(%rax,%r15), %r11
	movq	%r10, %rsi
	movq	%r11, %rcx
	negq	%rsi
	adcq	$0, %rcx
	negq	%rcx
	cmovs	%r11, %rcx
	cmovs	%r10, %rsi
	xorq	40(%rsp), %r11
	xorl	%edx, %edx
	movq	%r11, %rax
	shrq	$63, %rax
	movq	%rsi, %r12
	xorl	%esi, %esi
	movq	%rcx, %r13
	addq	%rax, %rdi
	movq	%rcx, %rax
	movq	%rcx, %r8
	adcq	%rdx, %rbp
	orq	%rsi, %rax
	movq	%rdi, (%rsp)
	movq	%rbp, 8(%rsp)
	jne	.L464
	movq	16(%rsp), %rdi
	movq	24(%rsp), %rbp
	movq	%rdi, %rsi
	movq	%rbp, %rax
	imulq	%rcx, %rsi
	imulq	%r12, %rax
	addq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r12
	addq	%rdx, %rsi
	cmpq	$1, 56(%rsp)
	movq	%rax, %rcx
	movq	%rsi, %rbx
	je	.L565
	movq	%rsi, %rdi
	movq	%rax, %rsi
	testq	%rbx, %rbx
	jne	.L468
	movq	96(%rsp), %rax
	movq	56(%rsp), %r9
	xorl	%ebx, %ebx
	mulq	%rcx
	movq	%r9, %rdi
	xorl	%eax, %eax
	imulq	%rdx, %rdi
	movq	%rdx, %rcx
	subq	%rdi, %rsi
	cmpq	%r9, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%r9, %rax
	cmpq	%r9, %rsi
	cmovnb	%rax, %rsi
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rsi
	je	.L466
	addq	%rax, %rax
	adcq	%rdx, %rdx
.L558:
	movq	%rax, %rsi
	movq	%r9, %r8
	movq	%r9, %rax
	xorl	%r9d, %r9d
	cmpq	%rsi, %rax
	movq	%r9, %rax
	movq	%rdx, %rdi
	sbbq	%rdx, %rax
	jc	.L470
.L471:
	movq	%r8, %rax
	movq	%r9, %rdx
	xorq	%rsi, %rax
	xorq	%rdi, %rdx
	orq	%rdx, %rax
	jne	.L466
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	orq	%rdx, %rax
	je	.L466
.L476:
	movq	(%rsp), %rdi
	movq	8(%rsp), %rbp
	movq	%rdi, %rax
	movq	%rbp, %rdx
	xorq	%rcx, %rax
	xorq	%rbx, %rdx
	orq	%rdx, %rax
	je	.L505
.L470:
	addq	$1, %rcx
	adcq	$0, %rbx
	.p2align 4
	.p2align 3
.L466:
	testq	%r11, %r11
	js	.L566
.L480:
	movq	%rcx, 192(%rsp)
	movq	%rbx, 200(%rsp)
.L481:
	movdqa	192(%rsp), %xmm0
	movq	80(%rsp), %rax
	addq	$1, %r14
	movups	%xmm0, (%rax,%r15)
	addq	$16, %r15
	cmpq	%r14, 48(%rsp)
	jne	.L508
	movq	88(%rsp), %rbx
.L417:
	movb	$1, 16(%rbx)
.L410:
	movq	328(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L567
	addq	$344, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%rbx, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L411:
	.cfi_restore_state
	movabsq	$1000000000000, %rsi
	mulq	%rdx
	shrq	$37, %rdx
	movq	%rdx, %rax
	movq	%r15, %rdx
	imulq	%rsi, %rax
	subq	%rax, %rdx
	movq	%rdi, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rsi
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	jmp	.L412
	.p2align 4,,10
	.p2align 3
.L414:
	movq	$0, 96(%rsp)
	movq	%r15, %rax
	xorl	%edx, %edx
	divq	%rcx
	movq	%rax, %rsi
	movq	%rdi, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rcx
# 0 "" 2
#NO_APP
	xorl	%edx, %edx
	movq	%rax, 16(%rsp)
	movabsq	$1000000000000, %rax
	divq	%rcx
	movq	%rsi, 24(%rsp)
	movq	%rax, 56(%rsp)
	cmpq	$1, %rax
	je	.L418
	xorl	%ecx, %ecx
	movq	%rax, %rdx
	xorl	%edi, %edi
	movl	$1, %esi
	movq	%xmm0, 88(%rsp)
	movaps	%xmm2, (%rsp)
	call	__udivti3@PLT
	movq	88(%rsp), %xmm0
	movdqa	(%rsp), %xmm2
	movq	%rax, 96(%rsp)
.L418:
	movq	48(%rsp), %rdi
	testq	%rdi, %rdi
	je	.L417
	movq	24(%rsp), %rax
	xorl	%edx, %edx
	orq	%rdx, %rax
	je	.L416
	movq	%rbp, 56(%rsp)
	movq	64(%rsp), %r12
	movq	%rdi, %r13
	movq	%rbx, 16(%rsp)
	movq	%r15, 48(%rsp)
	movq	80(%rsp), %r15
	.p2align 4
	.p2align 3
.L463:
	movdqu	(%r12), %xmm1
	movq	%xmm1, %rcx
	movhlps	%xmm1, %xmm5
	movq	%xmm5, %r8
	movq	%rcx, %rdi
	negq	%rdi
	movq	%r8, %rsi
	movq	%r8, %rbx
	adcq	$0, %rsi
	negq	%rsi
	cmovs	%r8, %rsi
	cmovs	%rcx, %rdi
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rax
	jne	.L419
	movq	48(%rsp), %r11
	xorl	%r9d, %r9d
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %rbp
	mulq	%r11
	movq	%rdx, %r8
	movq	%r11, %rdx
	shrq	$37, %r8
	movq	%r8, %rax
	movq	%r8, 8(%rsp)
	xorl	%r8d, %r8d
	imulq	%rbp, %rax
	imulq	%rdi, %r8
	subq	%rax, %rdx
	movq	56(%rsp), %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, (%rsp)
	movq	(%rsp), %r10
	movq	%rsi, %rax
	movq	%rdx, 80(%rsp)
	imulq	%r10, %rax
	addq	%rax, %r8
	movq	%r10, %rax
	mulq	%rdi
	addq	%rdx, %r8
	movq	%rax, %r10
	movq	8(%rsp), %rax
	movq	%r8, %r11
	movq	%rsi, %r8
	imulq	%rax, %r8
	mulq	%rdi
	addq	%rdx, %r8
	movq	%r8, %rdx
	movq	%r11, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, (%rsp)
	movq	(%rsp), %rax
	movq	%rdx, 8(%rsp)
	movq	8(%rsp), %rdx
	orq	%rdx, %rax
	jne	.L420
	xorq	40(%rsp), %rbx
	xorq	32(%rsp), %rcx
	movq	%r8, %r9
	movq	$-1, %rax
	movabsq	$9223372036854775807, %rdx
	movq	%rcx, 64(%rsp)
	movq	%rbx, %rcx
	shrq	$63, %rcx
	movq	%rbx, 72(%rsp)
	xorl	%ebx, %ebx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	xorl	%r8d, %r8d
	xorl	%r11d, %r11d
	orq	%r10, %r8
	orq	%r11, %r9
	movq	%rbx, %rax
	cmpq	%r8, %rcx
	sbbq	%r9, %rax
	jc	.L420
	cmpq	$0, 80(%rsp)
	je	.L563
	movq	80(%rsp), %rax
	imulq	%rax, %rsi
	mulq	%rdi
	addq	%rdx, %rsi
	movq	%rax, %r10
	movq	%rsi, %rax
	xorl	%esi, %esi
	movq	%rax, %rdx
	orq	%rsi, %rdx
	jne	.L424
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	xorl	%r11d, %r11d
	mulq	%r10
	movq	%rdx, %rax
	movq	%r10, %rdx
	shrq	$37, %rax
	movq	%rax, %rsi
	imulq	%rbp, %rax
	subq	%rax, %rdx
	movq	%rdx, %r10
.L425:
	movq	%rcx, %rax
	movq	%rbx, %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L420
	addq	%rsi, %r8
	movq	%rbx, %rax
	adcq	%rdi, %r9
	cmpq	%r8, %rcx
	sbbq	%r9, %rax
	jc	.L420
	movq	%r10, %rax
	orq	%r11, %rax
	je	.L563
	movq	%r10, %rax
	xorl	%edx, %edx
	movl	$244140625, %edi
	addq	%r10, %rax
	adcq	%rdx, %rdx
	movq	%rax, %rsi
	salq	$12, %rdi
	xorl	%eax, %eax
	cmpq	%rsi, %rdi
	sbbq	%rdx, %rax
	jc	.L431
	testq	%rdx, %rdx
	je	.L568
.L432:
	addq	(%rsp), %r8
	adcq	8(%rsp), %r9
.L563:
	cmpq	$0, 72(%rsp)
	jns	.L422
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L422:
	movq	%r8, 192(%rsp)
	movq	%r9, 200(%rsp)
.L423:
	movdqa	192(%rsp), %xmm1
	movq	%r14, %rax
	addq	$1, %r14
	addq	$16, %r12
	salq	$4, %rax
	movups	%xmm1, (%r15,%rax)
	cmpq	%r14, %r13
	jne	.L463
	movq	16(%rsp), %rbx
	jmp	.L417
	.p2align 4,,10
	.p2align 3
.L464:
	movq	56(%rsp), %rax
	xorl	%ecx, %ecx
	xorl	%ebx, %ebx
	cmpq	$1, %rax
	je	.L482
	movq	%r13, %rcx
	xorl	%esi, %esi
	xorl	%edi, %edi
	movq	%rax, %rbx
	cmpq	%rax, %r13
	jb	.L483
	movq	96(%rsp), %rax
	xorl	%esi, %esi
	mulq	%r13
	movq	%rbx, %rax
	imulq	%rdx, %rax
	subq	%rax, %rcx
	cmpq	%rbx, %rcx
	movq	%rcx, %rax
	setnb	%sil
	addq	%rdx, %rsi
	subq	%rbx, %rax
	cmpq	%rbx, %rcx
	cmovnb	%rax, %rcx
	movq	%rsi, %rdi
	xorl	%esi, %esi
.L483:
	xorl	%r13d, %r13d
	movq	56(%rsp), %rbx
	movq	%r12, %rax
	movq	%rcx, %rdx
	orq	%rdi, %r13
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbx
# 0 "" 2
#NO_APP
	movq	%rax, %r12
	movq	%rdx, %rcx
	xorl	%ebx, %ebx
	movq	%r13, %r8
	orq	%rsi, %r12
.L482:
	movq	16(%rsp), %rax
	xorl	%r9d, %r9d
	mulq	%r12
	movq	%rax, %rsi
	movq	16(%rsp), %rax
	movq	%rdx, %rdi
	mulq	%r8
	addq	%rdi, %rax
	adcq	%r9, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %rbp
	movq	%rdx, %r8
	orq	%r9, %rbp
	jne	.L505
	movq	%rax, %rdx
	xorl	%edi, %edi
	xorl	%eax, %eax
	orq	%rdx, %rdi
	orq	%rax, %rsi
	movq	8(%rsp), %rdx
	movq	(%rsp), %rax
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L505
	movq	%rcx, %rax
	orq	%rbx, %rax
	je	.L486
	movq	16(%rsp), %rax
	mulq	%rcx
	cmpq	$1, 56(%rsp)
	movq	%rax, %rcx
	movq	%rdx, %rbx
	je	.L569
	movq	%rdx, 104(%rsp)
	movq	%rdx, %rax
	movq	%rcx, %rbp
	testq	%rdx, %rdx
	je	.L570
	movq	56(%rsp), %rbp
	cmpq	%rbp, %rdx
	jb	.L497
	mulq	96(%rsp)
	movq	%rbp, %rax
	movq	%rbp, %r9
	imulq	%rdx, %rax
	subq	%rax, 104(%rsp)
	movq	104(%rsp), %rbp
	xorl	%eax, %eax
	cmpq	%r9, %rbp
	movq	%rbp, %r8
	setnb	%al
	addq	%rdx, %rax
	subq	%r9, %r8
	cmpq	%r9, %rbp
	cmovb	%rbp, %r8
	movq	%rax, %rdx
	xorl	%eax, %eax
	movq	%rdx, %r9
	movq	%r8, 104(%rsp)
	movq	%rax, %r8
.L497:
	movq	56(%rsp), %rbx
	movq	%rcx, %rax
	movq	104(%rsp), %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbx
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	xorl	%edx, %edx
	orq	%rax, %r8
	xorl	%ebx, %ebx
	orq	%rdx, %r9
.L496:
	movq	(%rsp), %rax
	movq	8(%rsp), %rdx
	subq	%rsi, %rax
	sbbq	%rdi, %rdx
	cmpq	%r8, %rax
	movq	%rdx, %rax
	sbbq	%r9, %rax
	jc	.L505
	movq	(%rsp), %rax
	movq	8(%rsp), %rdx
	addq	%r8, %rsi
	adcq	%r9, %rdi
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L505
	movq	%rcx, %rax
	orq	%rbx, %rax
	je	.L486
	movq	56(%rsp), %rax
	xorl	%ebx, %ebx
	addq	%rcx, %rcx
	adcq	%rbx, %rbx
	xorl	%r9d, %r9d
	movq	%rax, %r8
	cmpq	%rcx, %rax
	movq	%r9, %rax
	sbbq	%rbx, %rax
	jc	.L502
	movq	%rcx, %rdx
	movq	%rbx, %rax
	xorq	%r8, %rdx
	xorq	%r9, %rax
	orq	%rax, %rdx
	jne	.L486
	movq	%rsi, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L502
	testq	%rax, %rax
	je	.L486
.L502:
	movq	(%rsp), %rcx
	movq	8(%rsp), %rbx
	movq	%rcx, %rax
	movq	%rbx, %rdx
	xorq	%rsi, %rax
	xorq	%rdi, %rdx
	orq	%rdx, %rax
	je	.L505
	addq	$1, %rsi
	adcq	$0, %rdi
	.p2align 4
	.p2align 3
.L486:
	testq	%r11, %r11
	jns	.L489
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
.L489:
	movq	%rsi, 192(%rsp)
	movq	%rdi, 200(%rsp)
	jmp	.L481
	.p2align 4,,10
	.p2align 3
.L565:
	movq	(%rsp), %rax
	movq	8(%rsp), %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rsi, %rax
	jnc	.L466
	.p2align 4
	.p2align 3
.L505:
	movb	$0, 192(%rsp)
	movq	88(%rsp), %rbx
.L488:
	movzbl	192(%rsp), %eax
	movq	%r14, (%rbx)
	movb	$0, 16(%rbx)
	movb	%al, 8(%rbx)
	jmp	.L410
	.p2align 4,,10
	.p2align 3
.L468:
	movq	56(%rsp), %rbx
	cmpq	%rbx, %rdi
	jnb	.L571
	movq	%rdi, %rdx
	movq	%rbx, %r9
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbx
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rax
	xorl	%edx, %edx
	xorl	%ebx, %ebx
	movq	%rax, %rsi
	orq	%rdx, %rsi
	je	.L466
	addq	%rax, %rax
	adcq	%rdx, %rdx
	jmp	.L558
	.p2align 4,,10
	.p2align 3
.L415:
	movq	$0, 96(%rsp)
	cmpq	$0, 48(%rsp)
	jne	.L416
	jmp	.L417
	.p2align 4,,10
	.p2align 3
.L566:
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
	jmp	.L480
	.p2align 4,,10
	.p2align 3
.L571:
	movq	96(%rsp), %rax
	movq	8(%rsp), %rbp
	mulq	%rdi
	movq	%rbx, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	xorl	%eax, %eax
	cmpq	%rbx, %rdi
	setnb	%al
	movq	%rax, %r8
	movq	%rdi, %rax
	addq	%rdx, %r8
	subq	%rbx, %rax
	cmpq	%rbx, %rdi
	cmovnb	%rax, %rdi
	movq	%rcx, %rax
	movq	%rdi, %rdx
	movq	(%rsp), %rdi
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbx
# 0 "" 2
#NO_APP
	cmpq	%rax, %rdi
	movq	%rax, %rcx
	movq	%rbp, %rax
	movq	%r8, %rbx
	sbbq	%r8, %rax
	jc	.L505
	movq	%rdx, %rsi
	xorl	%edi, %edi
	testq	%rdx, %rdx
	je	.L466
	movq	56(%rsp), %rax
	addq	%rdx, %rsi
	adcq	%rdi, %rdi
	xorl	%r9d, %r9d
	movq	%rax, %r8
	cmpq	%rsi, %rax
	movq	%r9, %rax
	sbbq	%rdi, %rax
	jc	.L476
	jmp	.L471
	.p2align 4,,10
	.p2align 3
.L570:
	movq	96(%rsp), %rax
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	56(%rsp), %rcx
	movq	%rcx, %rbx
	imulq	%rdx, %rbx
	movq	%rdx, %r8
	movq	%rbx, %rax
	movq	%rbp, %rbx
	subq	%rax, %rbx
	xorl	%eax, %eax
	cmpq	%rcx, %rbx
	setnb	%al
	addq	%rax, %r8
	movq	%rbx, %rax
	adcq	$0, %r9
	subq	%rcx, %rax
	cmpq	%rcx, %rbx
	cmovnb	%rax, %rbx
	movq	%rbx, %rcx
	xorl	%ebx, %ebx
	jmp	.L496
	.p2align 4,,10
	.p2align 3
.L420:
	movb	$0, 192(%rsp)
	movq	16(%rsp), %rbx
	jmp	.L488
	.p2align 4,,10
	.p2align 3
.L569:
	movq	(%rsp), %r8
	movq	8(%rsp), %r9
	movq	%r8, %rax
	movq	%r9, %rdx
	subq	%rsi, %rax
	sbbq	%rdi, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L505
	addq	%rcx, %rsi
	movq	%r9, %rax
	adcq	%rbx, %rdi
	cmpq	%rsi, %r8
	sbbq	%rdi, %rax
	jc	.L505
	jmp	.L486
	.p2align 4,,10
	.p2align 3
.L419:
	movq	32(%rsp), %r10
	movq	40(%rsp), %r11
	movq	%rcx, %rax
	xorl	%esi, %esi
	sarq	$63, %rax
	movq	%r10, %rdx
	movq	%r11, %rdi
	sarq	$63, %rdx
	cmpq	%r11, %rdx
	jne	.L439
	cmpq	%r8, %rax
	jne	.L440
	movq	%r10, %rax
	imulq	%rcx
.L437:
	testq	%rsi, %rsi
	je	.L447
	movq	%xmm0, %rax
	sarq	$63, %r8
	movaps	%xmm1, 224(%rsp)
	leaq	288(%rsp), %rdi
	sarq	$63, %rax
	movq	%r8, %xmm1
	subq	$64, %rsp
	.cfi_def_cfa_offset 464
	movq	%xmm0, 128(%rsp)
	movq	%rax, %xmm3
	punpcklqdq	%xmm1, %xmm1
	movl	$12, %ecx
	movl	$128, %r9d
	punpcklqdq	%xmm3, %xmm3
	movl	$3, %r8d
	movl	$12, %edx
	movaps	%xmm1, 304(%rsp)
	movaps	%xmm3, 336(%rsp)
	movl	$12, %esi
	movdqa	336(%rsp), %xmm1
	movups	%xmm2, 32(%rsp)
	movups	%xmm1, 48(%rsp)
	movdqa	288(%rsp), %xmm1
	movaps	%xmm2, 64(%rsp)
	movups	%xmm1, (%rsp)
	movdqa	304(%rsp), %xmm1
	movups	%xmm1, 16(%rsp)
	call	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	addq	$64, %rsp
	.cfi_def_cfa_offset 400
	movdqa	(%rsp), %xmm2
	movq	64(%rsp), %xmm0
	cmpb	$0, 320(%rsp)
	jne	.L448
	movzbl	288(%rsp), %eax
	movq	16(%rsp), %rbx
	movb	%al, 192(%rsp)
	jmp	.L488
.L439:
	cmpq	%r8, %rax
	jne	.L442
	movdqa	32(%rsp), %xmm6
	movq	%rcx, %r9
	movaps	%xmm6, 64(%rsp)
.L441:
	movq	32(%rsp), %rax
	mulq	%rcx
	movq	%rax, (%rsp)
	movq	%r9, %rax
	movq	%rdx, 8(%rsp)
	mulq	%rdi
	movq	%rax, %r10
	movq	%rdx, %r11
	testq	%rdi, %rdi
	jns	.L443
	xorl	%eax, %eax
	subq	%rax, %r10
	sbbq	%r9, %r11
.L443:
	testq	%r9, %r9
	jns	.L444
	subq	64(%rsp), %r10
	sbbq	72(%rsp), %r11
.L444:
	movq	8(%rsp), %rdx
	movq	%rdx, %rax
	xorl	%edx, %edx
	addq	%r10, %rax
	movq	%rax, %r9
	adcq	%r11, %rdx
	sarq	$63, %r9
	cmpq	%rdx, %r9
	jne	.L445
	movq	%rax, %rdx
	movq	(%rsp), %rax
	jmp	.L437
.L447:
	movq	%rax, %rdi
	movq	%rdx, %r8
	testq	%rdx, %rdx
	js	.L449
	movq	%rdx, %rcx
	testq	%rdx, %rdx
	jne	.L450
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r8
	mulq	%rdi
	movq	%rdx, %r9
	xorl	%edx, %edx
	shrq	$37, %r9
	movq	%r9, %rax
	imulq	%r8, %r9
	subq	%r9, %rdi
	movq	%rdi, %rsi
	je	.L451
	xorl	%edi, %edi
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	cmpq	%rsi, %r8
	sbbq	%rdi, %rcx
	jnc	.L452
	addq	$1, %rax
	adcq	$0, %rdx
.L451:
	movb	$1, 128(%rsp)
	cmpb	$0, 128(%rsp)
	movq	%rax, 144(%rsp)
	movq	%rdx, 152(%rsp)
	jne	.L462
	movzbl	144(%rsp), %eax
	movq	16(%rsp), %rbx
	movb	%al, 192(%rsp)
	jmp	.L488
.L448:
	movdqa	288(%rsp), %xmm1
	movaps	%xmm1, 192(%rsp)
	jmp	.L423
.L462:
	movq	144(%rsp), %rax
	movq	%rax, 192(%rsp)
	movq	152(%rsp), %rax
	movq	%rax, 200(%rsp)
	jmp	.L423
.L449:
	negq	%rax
	adcq	$0, %rdx
	movq	%rax, %rbx
	negq	%rdx
	movq	%rdx, %rcx
	jne	.L454
	movabsq	$2535301200456458803, %rax
	movq	%rbx, %rsi
	movabsq	$1000000000000, %r8
	mulq	%rbx
	movq	%rdx, %rdi
	xorl	%edx, %edx
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r8, %rdi
	subq	%rdi, %rsi
	je	.L455
	xorl	%edi, %edi
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	cmpq	%rsi, %r8
	sbbq	%rdi, %rcx
	jnc	.L456
	addq	$1, %rax
	adcq	$0, %rdx
.L455:
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
	jmp	.L451
.L440:
	movq	32(%rsp), %r9
	movq	%xmm1, 64(%rsp)
	movq	%r8, %rdi
	movq	%xmm5, 72(%rsp)
	jmp	.L441
.L424:
	movq	%rax, %rdi
	xorl	%r11d, %r11d
	movabsq	$2535301200456458803, %rax
	mulq	%rdi
	shrq	$37, %rdx
	movq	%rdx, %rax
	movq	%rdx, %rsi
	imulq	%rbp, %rax
	subq	%rax, %rdi
	movq	%r10, %rax
	movq	%rdi, %rdx
	movq	%rsi, %rdi
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %r10
	jmp	.L425
.L450:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %r9
	mulq	%rdx
	shrq	$37, %rdx
	movq	%rdx, %rax
	movq	%rdx, %rsi
	imulq	%r9, %rax
	subq	%rax, %rcx
	movq	%rdi, %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r9
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	movq	%rsi, %rdx
	movq	%rcx, %rsi
	orq	%rbx, %rsi
	je	.L451
.L458:
	xorl	%ebx, %ebx
	addq	%rcx, %rcx
	movq	%rcx, %rsi
	movl	$244140625, %ecx
	adcq	%rbx, %rbx
	xorl	%r9d, %r9d
	salq	$12, %rcx
	cmpq	%rsi, %rcx
	sbbq	%rbx, %r9
	jnc	.L459
	addq	$1, %rax
	adcq	$0, %rdx
.L460:
	testq	%r8, %r8
	jns	.L451
	jmp	.L455
	.p2align 4,,10
	.p2align 3
.L454:
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r9
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r9, %rax
	subq	%rax, %rcx
	movq	%rbx, %rax
	xorl	%ebx, %ebx
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r9
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	movq	%rdi, %rdx
	movq	%rcx, %rsi
	orq	%rbx, %rsi
	je	.L455
	jmp	.L458
.L442:
	movq	32(%rsp), %rax
	movq	%r11, %r10
	imulq	%rcx, %r10
	imulq	%r8, %rax
	addq	%rax, %r10
	movq	32(%rsp), %rax
	mulq	%rcx
	leaq	1(%r11), %rcx
	addq	%rdx, %r10
	movq	%r10, %rdx
	cmpq	$1, %rcx
	ja	.L438
	leaq	1(%r8), %rcx
	cmpq	$1, %rcx
	ja	.L438
	cmpq	%r8, %r11
	jne	.L446
	cmpq	%rax, %rsi
	movq	%rsi, %rbx
	sbbq	%r10, %rbx
	jl	.L437
.L438:
	movl	$1, %esi
	jmp	.L437
	.p2align 4,,10
	.p2align 3
.L446:
	testq	%r10, %r10
	js	.L437
	jmp	.L438
.L456:
	testq	%rdi, %rdi
	jne	.L455
	cmpq	%r8, %rsi
	jne	.L455
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L455
.L568:
	cmpq	%rdi, %rsi
	jne	.L432
	movq	%r8, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	movq	%rdx, 8(%rsp)
	movq	%rax, (%rsp)
	testq	%rdx, %rdx
	jne	.L431
	testq	%rax, %rax
	je	.L432
.L431:
	movq	%rcx, %rax
	movq	%rbx, %rdx
	xorq	%r8, %rax
	xorq	%r9, %rdx
	orq	%rdx, %rax
	je	.L420
	xorl	%r10d, %r10d
	movq	$1, (%rsp)
	movq	%r10, 8(%rsp)
	jmp	.L432
.L452:
	testq	%rdi, %rdi
	jne	.L451
	cmpq	%r8, %rsi
	jne	.L451
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L451
.L459:
	testq	%rbx, %rbx
	jne	.L460
	cmpq	%rcx, %rsi
	jne	.L460
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L460
.L445:
	movq	32(%rsp), %rdi
	movq	40(%rsp), %rsi
	movq	%rdi, %rax
	imulq	%rcx, %rsi
	imulq	%rbx, %rax
	addq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%rcx
	movq	%rax, %rcx
	leaq	(%rsi,%rdx), %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	jmp	.L438
.L567:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1692:
	.size	_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_, .-_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m
	.type	_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m, @function
_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m:
.LFB1695:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	movq	%rdi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$328, %rsp
	.cfi_def_cfa_offset 384
	movq	%rsi, 72(%rsp)
	movq	%r8, 80(%rsp)
	movq	%fs:40, %rax
	movq	%rax, 312(%rsp)
	xorl	%eax, %eax
	testq	%r8, %r8
	je	.L573
	movq	%rdx, 24(%rsp)
	xorl	%r12d, %r12d
	movq	%rdi, 88(%rsp)
	movq	%r12, %r13
	movq	%rcx, %r12
	.p2align 4
	.p2align 3
.L641:
	movq	24(%rsp), %rax
	movdqu	(%r12), %xmm0
	movdqu	(%rax), %xmm1
	movq	%xmm0, %rcx
	movhlps	%xmm0, %xmm3
	movq	%xmm3, %rax
	movq	%rcx, %r11
	movq	%xmm3, 64(%rsp)
	movhlps	%xmm1, %xmm4
	movq	%xmm1, (%rsp)
	movq	(%rsp), %rdi
	movq	%rax, %r9
	movq	%xmm4, 8(%rsp)
	movq	8(%rsp), %rbp
	movq	%rax, %rbx
	movq	%rdi, %r10
	movq	%rdi, %rsi
	movq	%xmm4, 56(%rsp)
	negq	%r10
	movq	%rbp, %r8
	adcq	$0, %r8
	negq	%r8
	cmovs	%rbp, %r8
	cmovs	%rdi, %r10
	negq	%r11
	movq	%rbp, %rdi
	adcq	$0, %r9
	negq	%r9
	movq	%r8, %rdx
	cmovs	%rax, %r9
	cmovs	%rcx, %r11
	xorq	%rax, %rdi
	xorl	%r14d, %r14d
	movq	%rdi, %rax
	xorq	%rcx, %rsi
	movq	%rdi, 40(%rsp)
	movq	%r9, %rbp
	shrq	$63, %rax
	xorl	%r15d, %r15d
	movq	%rsi, 32(%rsp)
	orq	%r14, %rbp
	movq	%rax, 48(%rsp)
	jne	.L574
	movq	%r8, %rax
	orq	%r15, %rax
	jne	.L575
	imulq	%r10, %r9
	movq	%r10, %rax
	xorl	%edi, %edi
	imulq	%r11, %r8
	mulq	%r11
	addq	%r9, %r8
	addq	%rdx, %r8
	movq	%rax, %r10
	movq	%r8, %rdx
	movq	%r10, %r8
	movq	%rdx, %rsi
	orq	%rdi, %rsi
	jne	.L576
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	xorl	%r9d, %r9d
	mulq	%r10
	movabsq	$1000000000000, %rax
	shrq	$37, %rdx
	movq	%rdx, %rsi
	imulq	%rax, %rdx
	subq	%rdx, %r8
.L577:
	movq	%r8, %rax
	orq	%r9, %rax
	je	.L578
	movabsq	$1000000000000, %rcx
	movq	%r8, %rax
	xorl	%edx, %edx
	addq	%r8, %rax
	adcq	%rdx, %rdx
	cmpq	%rax, %rcx
	movl	$0, %ecx
	movq	%rax, %r14
	sbbq	%rdx, %rcx
	jc	.L676
	movabsq	$1000000000000, %rcx
	movq	%rcx, %rax
	xorq	%r14, %rax
	orq	%rdx, %rax
	jne	.L578
	addq	$1, %rsi
	adcq	$0, %rdi
	andq	$-2, %rsi
	.p2align 4
	.p2align 3
.L578:
	cmpq	$0, 40(%rsp)
	jns	.L581
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
.L581:
	movq	%rsi, 176(%rsp)
	movq	%rdi, 184(%rsp)
	jmp	.L582
	.p2align 4,,10
	.p2align 3
.L574:
	orq	%r15, %rdx
	jne	.L599
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %rdi
	mulq	%r9
	movabsq	$1000000000000, %rax
	movq	%rdx, %rsi
	shrq	$37, %rsi
	imulq	%rsi, %rax
	movq	%rsi, %rbp
	xorl	%esi, %esi
	imulq	%r10, %rsi
	subq	%rax, %r9
	movq	%r11, %rax
	movq	%r9, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rdi
# 0 "" 2
#NO_APP
	movq	%rax, %rdi
	movq	%r8, %rax
	movq	%rdx, %r9
	imulq	%rdi, %rax
	addq	%rax, %rsi
	movq	%rdi, %rax
	xorl	%edi, %edi
	mulq	%r10
	addq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%rbp, %rax
	movq	%rsi, %rbx
	mulq	%r10
	movq	%r8, %rsi
	imulq	%rbp, %rsi
	addq	%rdx, %rsi
	movq	%rsi, %rdx
	movq	%rbx, %rsi
	addq	%rax, %rsi
	adcq	%rdx, %rdi
	xorl	%edx, %edx
	movq	%rdi, %rax
	orq	%rdx, %rax
	jne	.L613
	movq	$-1, %r14
	addq	48(%rsp), %r14
	movq	%rsi, %rdi
	movabsq	$9223372036854775807, %r15
	adcq	$0, %r15
	xorl	%esi, %esi
	xorl	%ebx, %ebx
	orq	%rsi, %rcx
	orq	%rdi, %rbx
	movq	%r15, %rax
	cmpq	%rcx, %r14
	movq	%rcx, (%rsp)
	sbbq	%rbx, %rax
	movq	%rbx, 8(%rsp)
	jc	.L613
	testq	%r9, %r9
	jne	.L677
	cmpq	$0, 40(%rsp)
	js	.L678
.L603:
	movq	(%rsp), %rax
	movq	8(%rsp), %rdx
	movq	%rax, %rcx
	movq	%rdx, %rax
.L604:
	movq	%rcx, 176(%rsp)
	movq	%rax, 184(%rsp)
	jmp	.L582
	.p2align 4,,10
	.p2align 3
.L613:
	movb	$0, 176(%rsp)
	movq	88(%rsp), %rbp
	movq	%r13, %r12
.L586:
	movzbl	176(%rsp), %eax
	movq	%r12, 0(%rbp)
	movb	$0, 16(%rbp)
	movb	%al, 8(%rbp)
.L572:
	movq	312(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L679
	addq	$328, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%rbp, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L575:
	.cfi_restore_state
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %rbx
	mulq	%r8
	movabsq	$1000000000000, %rax
	movq	%rdx, %rsi
	shrq	$37, %rsi
	imulq	%rsi, %rax
	movq	%rsi, %rbp
	xorl	%esi, %esi
	imulq	%r11, %rsi
	subq	%rax, %r8
	movq	%r10, %rax
	movq	%r8, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbx
# 0 "" 2
#NO_APP
	movq	%rax, %rdi
	movq	%r9, %rax
	movq	%rdx, %r8
	imulq	%rdi, %rax
	addq	%rax, %rsi
	movq	%rdi, %rax
	xorl	%edi, %edi
	mulq	%r11
	addq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%rbp, %rax
	movq	%rsi, %rbx
	mulq	%r11
	movq	%r9, %rsi
	imulq	%rbp, %rsi
	addq	%rdx, %rsi
	movq	%rsi, %rdx
	movq	%rbx, %rsi
	addq	%rax, %rsi
	adcq	%rdx, %rdi
	xorl	%edx, %edx
	movq	%rdi, %rax
	orq	%rdx, %rax
	jne	.L613
	movq	$-1, %r14
	addq	48(%rsp), %r14
	movq	%rsi, %rdi
	movabsq	$9223372036854775807, %r15
	adcq	$0, %r15
	xorl	%esi, %esi
	xorl	%ebx, %ebx
	orq	%rsi, %rcx
	orq	%rdi, %rbx
	movq	%r15, %rax
	cmpq	%rcx, %r14
	sbbq	%rbx, %rax
	jc	.L613
	testq	%r8, %r8
	jne	.L680
	cmpq	$0, 40(%rsp)
	jns	.L587
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L587:
	movq	%rcx, 176(%rsp)
	movq	%rbx, 184(%rsp)
	jmp	.L582
	.p2align 4,,10
	.p2align 3
.L599:
	movq	(%rsp), %rsi
	movq	8(%rsp), %rdi
	movq	%rcx, %rax
	xorl	%ebp, %ebp
	sarq	$63, %rax
	movq	%rsi, %rdx
	movq	%rdi, %r9
	sarq	$63, %rdx
	cmpq	%rdi, %rdx
	jne	.L617
	cmpq	%rbx, %rax
	jne	.L618
	movq	%rsi, %rax
	imulq	%rcx
.L615:
	testq	%rbp, %rbp
	je	.L625
	movq	64(%rsp), %rax
	movaps	%xmm1, 208(%rsp)
	leaq	272(%rsp), %rdi
	subq	$64, %rsp
	.cfi_def_cfa_offset 448
	movl	$128, %r9d
	movl	$3, %r8d
	movl	$12, %ecx
	movl	$12, %edx
	sarq	$63, %rax
	movl	$12, %esi
	movq	%rax, %xmm2
	movq	120(%rsp), %rax
	punpcklqdq	%xmm2, %xmm2
	sarq	$63, %rax
	movaps	%xmm2, 320(%rsp)
	movq	%rax, %xmm1
	punpcklqdq	%xmm1, %xmm1
	movaps	%xmm1, 288(%rsp)
	movups	%xmm0, 32(%rsp)
	movdqa	320(%rsp), %xmm0
	movups	%xmm0, 48(%rsp)
	movdqa	272(%rsp), %xmm0
	movups	%xmm0, (%rsp)
	movdqa	288(%rsp), %xmm0
	movups	%xmm0, 16(%rsp)
	call	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	addq	$64, %rsp
	.cfi_def_cfa_offset 384
	cmpb	$0, 304(%rsp)
	je	.L681
	movdqa	272(%rsp), %xmm0
	movaps	%xmm0, 176(%rsp)
	.p2align 4
	.p2align 3
.L582:
	movq	%r13, %rax
	movq	72(%rsp), %rdi
	addq	$16, 24(%rsp)
	addq	$1, %r13
	movdqa	176(%rsp), %xmm0
	salq	$4, %rax
	addq	$16, %r12
	movups	%xmm0, (%rdi,%rax)
	cmpq	%r13, 80(%rsp)
	jne	.L641
	movq	88(%rsp), %rbp
.L573:
	movb	$1, 16(%rbp)
	jmp	.L572
	.p2align 4,,10
	.p2align 3
.L576:
	movabsq	$2535301200456458803, %rax
	movq	%rdx, %rcx
	xorl	%r9d, %r9d
	movabsq	$1000000000000, %rdi
	mulq	%rdx
	movq	%rdx, %rsi
	movq	%rcx, %rdx
	shrq	$37, %rsi
	movq	%rsi, %rax
	imulq	%rdi, %rax
	subq	%rax, %rdx
	movq	%r10, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rdi
# 0 "" 2
#NO_APP
	movq	%rsi, %rdi
	movq	%rdx, %r8
	movq	%rax, %rsi
	jmp	.L577
	.p2align 4,,10
	.p2align 3
.L625:
	movq	%rax, %r9
	movq	%rdx, %r8
	movq	%rax, %r10
	movq	%rdx, %r11
	testq	%rdx, %rdx
	js	.L628
	movq	%rdx, %rcx
	testq	%rdx, %rdx
	jne	.L629
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	movabsq	$1000000000000, %r8
	mulq	%r9
	movq	%r9, %rax
	shrq	$37, %rdx
	movq	%rdx, %rsi
	imulq	%r8, %rdx
	subq	%rdx, %rax
	je	.L630
	xorl	%edx, %edx
	addq	%rax, %rax
	adcq	%rdx, %rdx
	cmpq	%rax, %r8
	sbbq	%rdx, %rcx
	jc	.L682
	testq	%rdx, %rdx
	jne	.L630
	cmpq	%r8, %rax
	jne	.L630
	addq	$1, %rsi
	adcq	$0, %rdi
	andq	$-2, %rsi
	jmp	.L630
	.p2align 4,,10
	.p2align 3
.L680:
	movq	%r9, %rsi
	movq	%r11, %rax
	mulq	%r8
	imulq	%r8, %rsi
	movq	%rax, %r8
	addq	%rdx, %rsi
	movq	%r8, %r10
	movq	%rsi, %rax
	xorl	%esi, %esi
	movq	%rax, %rdx
	orq	%rsi, %rdx
	je	.L683
	movq	%rax, %rsi
	xorl	%r11d, %r11d
	movabsq	$2535301200456458803, %rax
	mulq	%rsi
	movabsq	$1000000000000, %rax
	movq	%rdx, %rdi
	movq	%rsi, %rdx
	movabsq	$1000000000000, %rsi
	shrq	$37, %rdi
	imulq	%rdi, %rax
	subq	%rax, %rdx
	movq	%r8, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rsi
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %r10
.L590:
	movq	%r14, %rax
	movq	%r15, %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L613
	movq	%rcx, %rax
	movq	%rbx, %rdx
	movq	%r15, %rbx
	addq	%rsi, %rax
	adcq	%rdi, %rdx
	cmpq	%rax, %r14
	sbbq	%rdx, %rbx
	jc	.L613
	movq	%r10, %rcx
	orq	%r11, %rcx
	je	.L594
	movabsq	$1000000000000, %rsi
	movq	%r10, %r8
	xorl	%r9d, %r9d
	addq	%r10, %r8
	adcq	%r9, %r9
	cmpq	%r8, %rsi
	movl	$0, %esi
	sbbq	%r9, %rsi
	jc	.L595
	movabsq	$1000000000000, %rsi
	movq	%rsi, %rcx
	xorq	%r8, %rcx
	orq	%r9, %rcx
	jne	.L594
	movq	%rax, %rcx
	xorl	%esi, %esi
	andl	$1, %ecx
	orq	%rsi, %rcx
	je	.L594
.L595:
	movq	%r14, %rcx
	movq	%r15, %rsi
	xorq	%rax, %rcx
	xorq	%rdx, %rsi
	orq	%rsi, %rcx
	je	.L613
	addq	$1, %rax
	adcq	$0, %rdx
.L594:
	cmpq	$0, 40(%rsp)
	jns	.L598
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
.L598:
	movq	%rax, 176(%rsp)
	movq	%rdx, 184(%rsp)
	jmp	.L582
	.p2align 4,,10
	.p2align 3
.L677:
	movq	%r10, %rax
	imulq	%r9, %r8
	xorl	%esi, %esi
	mulq	%r9
	addq	%rdx, %r8
	movq	%rax, %r10
	movq	%r8, %rax
	movq	%r10, %r8
	movq	%rax, %rdx
	orq	%rsi, %rdx
	je	.L684
	movabsq	$1000000000000, %rbx
	movq	%rax, %rsi
	xorl	%r9d, %r9d
	movabsq	$2535301200456458803, %rax
	mulq	%rsi
	movabsq	$1000000000000, %rax
	movq	%rdx, %rdi
	movq	%rsi, %rdx
	shrq	$37, %rdi
	imulq	%rdi, %rax
	subq	%rax, %rdx
	movq	%r10, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbx
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %r8
.L606:
	movq	%r14, %rax
	movq	%r15, %rdx
	subq	(%rsp), %rax
	sbbq	8(%rsp), %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L613
	movq	%rsi, %rax
	addq	(%rsp), %rax
	movq	%rdi, %rdx
	movq	%r15, %rbx
	adcq	8(%rsp), %rdx
	cmpq	%rax, %r14
	sbbq	%rdx, %rbx
	jc	.L613
	movq	%r8, %rcx
	orq	%r9, %rcx
	je	.L610
	movabsq	$1000000000000, %rsi
	xorl	%r9d, %r9d
	addq	%r8, %r8
	adcq	%r9, %r9
	cmpq	%r8, %rsi
	movl	$0, %esi
	sbbq	%r9, %rsi
	jc	.L611
	movabsq	$1000000000000, %rsi
	movq	%rsi, %rcx
	xorq	%r8, %rcx
	orq	%r9, %rcx
	jne	.L610
	movq	%rax, %rcx
	xorl	%esi, %esi
	andl	$1, %ecx
	orq	%rsi, %rcx
	je	.L610
.L611:
	movq	%r14, %rcx
	movq	%r15, %rsi
	xorq	%rax, %rcx
	xorq	%rdx, %rsi
	orq	%rsi, %rcx
	je	.L613
	addq	$1, %rax
	adcq	$0, %rdx
.L610:
	cmpq	$0, 40(%rsp)
	jns	.L614
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
.L614:
	movq	%rax, %rcx
	movq	%rdx, %rax
	jmp	.L604
	.p2align 4,,10
	.p2align 3
.L681:
	movzbl	272(%rsp), %eax
	movq	88(%rsp), %rbp
	movq	%r13, %r12
	movb	%al, 176(%rsp)
	jmp	.L586
	.p2align 4,,10
	.p2align 3
.L628:
	negq	%r10
	adcq	$0, %r11
	movq	%r10, %r9
	negq	%r11
	movq	%r11, %rcx
	je	.L685
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %rdi
	mulq	%r11
	movq	%rdx, %rsi
	movq	%r11, %rdx
	shrq	$37, %rsi
	movq	%rsi, %rax
	imulq	%rdi, %rax
	subq	%rax, %rdx
	movq	%r10, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rdi
# 0 "" 2
#NO_APP
	movq	%rsi, %rdi
	movq	%rax, %rsi
	movq	%rdx, %rax
	movq	%rdx, %rcx
	orq	%rbx, %rax
	jne	.L636
.L634:
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
.L630:
	movb	$1, 112(%rsp)
	cmpb	$0, 112(%rsp)
	movq	%rsi, 128(%rsp)
	movq	%rdi, 136(%rsp)
	jne	.L640
	movzbl	128(%rsp), %eax
	movq	88(%rsp), %rbp
	movq	%r13, %r12
	movb	%al, 176(%rsp)
	jmp	.L586
	.p2align 4,,10
	.p2align 3
.L617:
	cmpq	%rbx, %rax
	jne	.L620
	movq	%rcx, %r8
.L619:
	movq	(%rsp), %rax
	mulq	%rcx
	movq	%rax, %r14
	movq	%r8, %rax
	movq	%rdx, %r15
	mulq	%r9
	movq	%rax, %r10
	movq	%rdx, %r11
	testq	%r9, %r9
	jns	.L621
	xorl	%eax, %eax
	subq	%rax, %r10
	sbbq	%r8, %r11
.L621:
	testq	%r8, %r8
	jns	.L622
	subq	%rsi, %r10
	sbbq	%rdi, %r11
.L622:
	movq	%r15, %rax
	xorl	%edx, %edx
	addq	%r10, %rax
	movq	%rax, %r9
	adcq	%r11, %rdx
	sarq	$63, %r9
	cmpq	%rdx, %r9
	jne	.L623
	movq	%rax, %rdx
	movq	%r14, %rax
	jmp	.L615
	.p2align 4,,10
	.p2align 3
.L678:
	negq	(%rsp)
	adcq	$0, 8(%rsp)
	negq	8(%rsp)
	jmp	.L603
	.p2align 4,,10
	.p2align 3
.L684:
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	xorl	%r9d, %r9d
	mulq	%r10
	movabsq	$1000000000000, %rax
	shrq	$37, %rdx
	movq	%rdx, %rsi
	imulq	%rax, %rdx
	subq	%rdx, %r8
	jmp	.L606
	.p2align 4,,10
	.p2align 3
.L683:
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	xorl	%r11d, %r11d
	mulq	%r8
	movabsq	$1000000000000, %rax
	shrq	$37, %rdx
	movq	%rdx, %rsi
	imulq	%rax, %rdx
	subq	%rdx, %r10
	jmp	.L590
	.p2align 4,,10
	.p2align 3
.L629:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %rdi
	mulq	%rdx
	movq	%rdx, %rsi
	movq	%r8, %rdx
	shrq	$37, %rsi
	movq	%rsi, %rax
	imulq	%rdi, %rax
	subq	%rax, %rdx
	movq	%r9, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rdi
# 0 "" 2
#NO_APP
	movq	%rsi, %rdi
	movq	%rax, %rsi
	movq	%rdx, %rax
	movq	%rdx, %rcx
	orq	%rbx, %rax
	je	.L630
.L636:
	xorl	%ebx, %ebx
	movl	$244140625, %eax
	addq	%rcx, %rcx
	adcq	%rbx, %rbx
	salq	$12, %rax
	xorl	%r9d, %r9d
	movq	%rcx, %rdx
	cmpq	%rcx, %rax
	sbbq	%rbx, %r9
	jc	.L686
	testq	%rbx, %rbx
	je	.L687
.L638:
	testq	%r8, %r8
	jns	.L630
	jmp	.L634
	.p2align 4,,10
	.p2align 3
.L685:
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	movabsq	$1000000000000, %r8
	mulq	%r10
	shrq	$37, %rdx
	movq	%rdx, %rsi
	imulq	%r8, %rdx
	subq	%rdx, %r9
	je	.L634
	movq	%r9, %rax
	xorl	%edx, %edx
	addq	%r9, %rax
	adcq	%rdx, %rdx
	cmpq	%rax, %r8
	movq	%rax, %r14
	sbbq	%rdx, %rcx
	jc	.L688
	movabsq	$1000000000000, %rcx
	movq	%rcx, %rax
	xorq	%r14, %rax
	orq	%rdx, %rax
	jne	.L634
	addq	$1, %rsi
	adcq	$0, %rdi
	andq	$-2, %rsi
	jmp	.L634
	.p2align 4,,10
	.p2align 3
.L640:
	movq	128(%rsp), %rax
	movq	%rax, 176(%rsp)
	movq	136(%rsp), %rax
	movq	%rax, 184(%rsp)
	jmp	.L582
	.p2align 4,,10
	.p2align 3
.L618:
	movq	(%rsp), %r8
	movq	%rcx, %rsi
	movq	%rbx, %rdi
	movq	%rbx, %r9
	jmp	.L619
.L620:
	movq	%rbx, %rax
	imulq	%rcx, %rdi
	imulq	%rsi, %rax
	addq	%rax, %rdi
	movq	%rsi, %rax
	mulq	%rcx
	leaq	1(%r9), %rcx
	addq	%rdx, %rdi
	movq	%rdi, %rdx
	cmpq	$1, %rcx
	ja	.L616
	leaq	1(%rbx), %rcx
	cmpq	$1, %rcx
	ja	.L616
	cmpq	%rbx, %r9
	jne	.L624
	xorl	%ebx, %ebx
	cmpq	%rax, %rbx
	sbbq	%rdi, %rbx
	jl	.L615
.L616:
	movl	$1, %ebp
	jmp	.L615
	.p2align 4,,10
	.p2align 3
.L676:
	addq	$1, %rsi
	adcq	$0, %rdi
	jmp	.L578
.L688:
	addq	$1, %rsi
	adcq	$0, %rdi
	jmp	.L634
.L624:
	testq	%rdi, %rdi
	js	.L615
	jmp	.L616
.L679:
	call	__stack_chk_fail@PLT
.L682:
	addq	$1, %rsi
	adcq	$0, %rdi
	jmp	.L630
.L686:
	addq	$1, %rsi
	adcq	$0, %rdi
	jmp	.L638
.L623:
	movq	(%rsp), %r14
	movq	8(%rsp), %rdi
	movq	%rbx, %rax
	imulq	%r14, %rax
	imulq	%rcx, %rdi
	addq	%rax, %rdi
	movq	%r14, %rax
	mulq	%rcx
	addq	%rdx, %rdi
	movq	%rdi, %rdx
	jmp	.L616
.L687:
	cmpq	%rax, %rdx
	jne	.L638
	addq	$1, %rsi
	adcq	$0, %rdi
	andq	$-2, %rsi
	jmp	.L638
	.cfi_endproc
.LFE1695:
	.size	_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m, .-_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.type	_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE, @function
_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE:
.LFB1700:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	movq	%rdi, %rbp
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$312, %rsp
	.cfi_def_cfa_offset 368
	movq	%rcx, 8(%rsp)
	movq	%r9, 32(%rsp)
	movq	%fs:40, %r14
	movq	%r14, 296(%rsp)
	movq	368(%rsp), %r14
	testq	%r9, %r9
	je	.L690
	movq	%r14, %rax
	movq	%r14, %xmm1
	movq	%r14, 64(%rsp)
	movq	%r14, %r15
	sarq	$63, %rax
	movq	%rsi, %r13
	xorl	%r12d, %r12d
	movq	%r8, %r10
	movq	%rax, %xmm0
	movq	%r14, %rax
	movq	%rdx, %r14
	sarq	$63, %rax
	movdqa	%xmm0, %xmm5
	punpcklqdq	%xmm0, %xmm1
	movq	%rax, 72(%rsp)
	punpcklqdq	%xmm5, %xmm5
	movaps	%xmm5, 48(%rsp)
	.p2align 4
	.p2align 3
.L722:
	movdqu	0(%r13), %xmm0
	xorl	%edi, %edi
	movq	%xmm0, %rcx
	movhlps	%xmm0, %xmm2
	movq	%rcx, %rax
	movq	%xmm2, %rsi
	sarq	$63, %rax
	movq	%rsi, %rbx
	cmpq	%rsi, %rax
	movq	%rcx, %rax
	jne	.L693
	imulq	%r15
.L691:
	testq	%rdi, %rdi
	je	.L701
	sarq	$63, %rsi
	movdqa	48(%rsp), %xmm3
	movaps	%xmm0, 192(%rsp)
	leaq	256(%rsp), %rdi
	movq	%rsi, %xmm0
	movq	%r10, 40(%rsp)
	subq	$64, %rsp
	.cfi_def_cfa_offset 432
	movl	$128, %r9d
	punpcklqdq	%xmm0, %xmm0
	movl	$12, %ecx
	movl	$10, %edx
	movaps	%xmm1, 288(%rsp)
	movaps	%xmm3, 304(%rsp)
	movl	$5, %r8d
	movl	$12, %esi
	movaps	%xmm0, 272(%rsp)
	movdqa	304(%rsp), %xmm0
	movups	%xmm1, 32(%rsp)
	movups	%xmm0, 48(%rsp)
	movdqa	256(%rsp), %xmm0
	movaps	%xmm1, 80(%rsp)
	movups	%xmm0, (%rsp)
	movdqa	272(%rsp), %xmm0
	movups	%xmm0, 16(%rsp)
	call	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	addq	$64, %rsp
	.cfi_def_cfa_offset 368
	movdqa	16(%rsp), %xmm1
	movq	40(%rsp), %r10
	cmpb	$0, 288(%rsp)
	jne	.L702
	movzbl	256(%rsp), %eax
	movb	%al, 80(%rsp)
.L713:
	movzbl	80(%rsp), %eax
	movq	%r12, 0(%rbp)
	movb	$0, 16(%rbp)
	movb	%al, 8(%rbp)
	.p2align 4
	.p2align 3
.L689:
	movq	296(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L733
	addq	$312, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%rbp, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L701:
	.cfi_restore_state
	movq	%rax, %rdi
	movq	%rdx, %rbx
	testq	%rdx, %rdx
	js	.L704
	testq	%rdx, %rdx
	jne	.L705
	movabsq	$5739826786856214345, %rax
	imulq	%rdi, %rax
	rorq	$10, %rax
	cmpq	$1844674407, %rax
	ja	.L706
	movabsq	$-2601111570856684097, %rax
	mulq	%rdi
	shrq	$33, %rdx
	movq	%rdx, %rax
	xorl	%edx, %edx
.L707:
	movq	%rdx, 120(%rsp)
	movq	%rax, 112(%rsp)
	movl	$1, %eax
.L711:
	movb	%al, 128(%rsp)
	movdqa	112(%rsp), %xmm0
	movdqa	128(%rsp), %xmm4
	movaps	%xmm0, 144(%rsp)
	movaps	%xmm4, 160(%rsp)
	testb	%al, %al
	jne	.L712
	movzbl	144(%rsp), %eax
	movb	%al, 80(%rsp)
	jmp	.L713
	.p2align 4,,10
	.p2align 3
.L702:
	movdqa	256(%rsp), %xmm0
	movaps	%xmm0, 80(%rsp)
.L714:
	movq	80(%rsp), %rax
	movq	(%r14), %rcx
	movq	88(%rsp), %rdx
	movq	8(%r14), %rbx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jo	.L732
	movq	%rcx, 112(%rsp)
	movl	(%r10,%r12,4), %ecx
	movq	%rbx, 120(%rsp)
	movq	8(%rsp), %rbx
	salq	$4, %rcx
	addq	%rbx, %rcx
	movq	(%rcx), %rsi
	movq	8(%rcx), %rdi
	addq	%rsi, %rax
	adcq	%rdi, %rdx
	jno	.L721
.L732:
	movq	%r12, 0(%rbp)
	movb	$0, 8(%rbp)
	movb	$0, 16(%rbp)
	jmp	.L689
	.p2align 4,,10
	.p2align 3
.L721:
	movdqa	112(%rsp), %xmm0
	addq	$1, %r12
	addq	$16, %r13
	addq	$16, %r14
	movq	%rax, 144(%rsp)
	movups	%xmm0, -16(%r14)
	movq	%rdx, 152(%rsp)
	movdqa	144(%rsp), %xmm0
	movups	%xmm0, (%rcx)
	cmpq	%r12, 32(%rsp)
	jne	.L722
.L690:
	movb	$1, 16(%rbp)
	jmp	.L689
	.p2align 4,,10
	.p2align 3
.L712:
	movq	144(%rsp), %rax
	movq	%rax, 80(%rsp)
	movq	152(%rsp), %rax
	movq	%rax, 88(%rsp)
	jmp	.L714
	.p2align 4,,10
	.p2align 3
.L704:
	movq	%rax, %rsi
	movq	%rdx, %rdi
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
	movq	%rdi, %r8
	jne	.L708
	movabsq	$5739826786856214345, %rax
	imulq	%rsi, %rax
	rorq	$10, %rax
	cmpq	$1844674407, %rax
	jbe	.L734
.L706:
	movb	$2, 112(%rsp)
	xorl	%eax, %eax
	jmp	.L711
	.p2align 4,,10
	.p2align 3
.L693:
	mulq	%r15
	movq	%rax, 16(%rsp)
	movq	%r15, %rax
	movq	%rdx, 24(%rsp)
	mulq	%rsi
	movq	%rax, %r8
	movq	%rdx, %r9
	testq	%rsi, %rsi
	jns	.L697
	xorl	%eax, %eax
	subq	%rax, %r8
	sbbq	%r15, %r9
.L697:
	testq	%r15, %r15
	jns	.L698
	subq	%rcx, %r8
	sbbq	%rbx, %r9
.L698:
	movq	24(%rsp), %rdx
	movq	%rdx, %rax
	xorl	%edx, %edx
	addq	%r8, %rax
	adcq	%r9, %rdx
	movq	%rax, %r9
	sarq	$63, %r9
	cmpq	%rdx, %r9
	jne	.L699
	movq	%rax, %rdx
	movq	16(%rsp), %rax
	jmp	.L691
	.p2align 4,,10
	.p2align 3
.L705:
	movabsq	$-2601111570856684097, %rax
	mulq	%rdx
	movabsq	$10000000000, %rax
	shrq	$33, %rdx
	imulq	%rdx, %rax
	movq	%rdx, %rcx
	movq	%rbx, %rdx
	movabsq	$10000000000, %rbx
	subq	%rax, %rdx
	movq	%rdi, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbx
# 0 "" 2
#NO_APP
	testq	%rdx, %rdx
	jne	.L706
	movq	%rcx, %rdx
	jmp	.L707
	.p2align 4,,10
	.p2align 3
.L708:
	movabsq	$-2601111570856684097, %rax
	mulq	%rdi
	movabsq	$10000000000, %rdi
	shrq	$33, %rdx
	movq	%rdx, %rax
	movq	%rdx, %rcx
	movq	%r8, %rdx
	imulq	%rdi, %rax
	subq	%rax, %rdx
	movq	%rsi, %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rdi
# 0 "" 2
#NO_APP
	testq	%rdx, %rdx
	jne	.L706
	movq	%rcx, %rdx
.L710:
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
	jmp	.L707
	.p2align 4,,10
	.p2align 3
.L734:
	movabsq	$-2601111570856684097, %rax
	mulq	%rsi
	shrq	$33, %rdx
	movq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L710
.L699:
	movq	72(%rsp), %rax
	movq	%r15, %rdi
	imulq	%rbx, %rdi
	imulq	%rcx, %rax
	addq	%rax, %rdi
	movq	%rcx, %rax
	mulq	%r15
	addq	%rdx, %rdi
	movq	%rdi, %rdx
	movl	$1, %edi
	jmp	.L691
.L733:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1700:
	.size	_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE, .-_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m
	.type	_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m, @function
_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m:
.LFB1705:
	.cfi_startproc
	endbr64
	movq	%rdi, %r10
	testq	%r8, %r8
	je	.L736
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L737:
	movl	(%rdx,%rax,4), %edi
	movl	(%rcx,%rax,4), %r9d
	cmpl	%r9d, %edi
	cmovg	%r9d, %edi
	movl	%edi, (%rsi,%rax,4)
	addq	$1, %rax
	cmpq	%rax, %r8
	jne	.L737
.L736:
	movb	$1, 16(%r10)
	movq	%r10, %rax
	ret
	.cfi_endproc
.LFE1705:
	.size	_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m, .-_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m
	.type	_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m, @function
_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m:
.LFB1728:
	.cfi_startproc
	endbr64
	movq	%rdi, %r11
	testq	%r8, %r8
	je	.L753
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rdx, %r9
	salq	$4, %r8
	xorl	%eax, %eax
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	movq	%rsi, %rbx
	jmp	.L746
	.p2align 4,,10
	.p2align 3
.L744:
	jl	.L745
	movq	%r10, %rsi
	movq	%rdi, %rdx
.L745:
	movq	%rdx, (%rbx,%rax)
	movq	%rsi, 8(%rbx,%rax)
	addq	$16, %rax
	cmpq	%rax, %r8
	je	.L756
.L746:
	movq	8(%r9,%rax), %rsi
	movq	8(%rcx,%rax), %r10
	movq	(%r9,%rax), %rdx
	movq	(%rcx,%rax), %rdi
	cmpq	%r10, %rsi
	jne	.L744
	cmpq	%rdi, %rdx
	movq	%rdi, %rbp
	cmovbe	%rdx, %rbp
	cmovne	%rbp, %rdx
	jmp	.L745
	.p2align 4,,10
	.p2align 3
.L756:
	movq	%r11, %rax
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	movb	$1, 16(%r11)
	ret
	.p2align 4,,10
	.p2align 3
.L753:
	.cfi_restore 3
	.cfi_restore 6
	movb	$1, 16(%rdi)
	movq	%rdi, %rax
	ret
	.cfi_endproc
.LFE1728:
	.size	_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m, .-_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m
	.section	.text._ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE,"axG",@progbits,_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE,comdat
	.p2align 4
	.weak	_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	.type	_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE, @function
_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE:
.LFB1834:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movq	%rdx, %r14
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movq	%rsi, %rbx
	subq	$216, %rsp
	.cfi_def_cfa_offset 272
	movq	%rdi, 24(%rsp)
	movq	%rcx, 48(%rsp)
	movq	%r8, 56(%rsp)
	movq	%r9, 16(%rsp)
	movq	%fs:40, %rbp
	movq	%rbp, 200(%rsp)
	movq	272(%rsp), %rbp
	movq	%rbp, %rax
	testq	%rbp, %rbp
	jns	.L758
	negq	%rax
.L758:
	movq	%rax, %rdi
	movabsq	$-2601111570856684097, %rax
	mulq	%rdi
	movq	%rdi, %rcx
	movabsq	$10000000000, %rax
	shrq	$33, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	movq	%rax, %rdx
	movq	%rcx, %rax
	.p2align 4
	.p2align 4
	.p2align 3
.L759:
	movq	%rdx, %rcx
	xorl	%edx, %edx
	divq	%rcx
	movq	%rcx, %rax
	testq	%rdx, %rdx
	jne	.L759
	movq	$0, 8(%rsp)
	movq	%rdx, %r12
	movq	%rdi, %rax
	xorl	%edx, %edx
	divq	%rcx
	xorl	%edx, %edx
	movq	%rax, (%rsp)
	movabsq	$10000000000, %rax
	divq	%rcx
	movq	%rax, %r13
	cmpq	$1, %rax
	je	.L836
	xorl	%ecx, %ecx
	xorl	%edi, %edi
	movq	%rax, %rdx
	movl	$1, %esi
	call	__udivti3@PLT
	shrq	$63, %rbp
	cmpq	$0, 16(%rsp)
	movq	%rax, 72(%rsp)
	movq	%rdx, 88(%rsp)
	movq	%rbp, 64(%rsp)
	je	.L762
	movq	%r14, %rbp
	leaq	8(%rbx), %r14
	.p2align 4
	.p2align 3
.L799:
	movq	-8(%r14), %rcx
	movq	(%r14), %rbx
	movq	%rcx, %rax
	movq	%rbx, %rdx
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rcx, %rax
	cmovs	%rbx, %rdx
	movq	%rax, %r8
	movq	%rdx, %r9
	movq	%rax, %r10
	testq	%rdx, %rdx
	je	.L763
	cmpq	%r13, %rdx
	jb	.L764
	movq	72(%rsp), %rax
	movq	(%rsp), %r10
	mulq	%rdx
	movq	%r13, %rax
	imulq	%rdx, %rax
	subq	%rax, %r9
	xorl	%eax, %eax
	cmpq	%r13, %r9
	setnb	%al
	movq	%rax, %rsi
	movq	%r9, %rax
	addq	%rdx, %rsi
	subq	%r13, %rax
	cmpq	%r13, %r9
	cmovnb	%rax, %r9
	movq	%r8, %rax
	xorl	%edi, %edi
	movq	%r9, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r13
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	%r10, %rax
	movq	%rdx, 96(%rsp)
	mulq	%r8
	movq	%rax, %r8
	movq	%rsi, %rax
	movq	%rdx, %r9
	mulq	%r10
	movq	%r9, %rsi
	addq	%rax, %rsi
	adcq	%rdx, %rdi
	xorl	%edx, %edx
	movq	%rdi, %rax
	orq	%rdx, %rax
	je	.L837
	.p2align 4
	.p2align 3
.L800:
	movb	$0, 176(%rsp)
.L794:
	movq	24(%rsp), %rdi
	movzbl	176(%rsp), %eax
	movq	%r12, (%rdi)
	movb	%al, 8(%rdi)
	movb	$0, 16(%rdi)
.L757:
	movq	200(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L838
	movq	24(%rsp), %rax
	addq	$216, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L763:
	.cfi_restore_state
	movq	88(%rsp), %rsi
	imulq	%rax, %rsi
	movq	72(%rsp), %rax
	mulq	%r8
	addq	%rdx, %rsi
	movq	%rsi, %rax
	imulq	%r13, %rax
	subq	%rax, %r10
	xorl	%eax, %eax
	cmpq	%r13, %r10
	setnb	%al
	addq	%rax, %rsi
	cmpq	%r13, %r10
	jnb	.L769
.L833:
	movq	(%rsp), %rax
	movq	%rbx, %rcx
	movq	%r10, %r8
	xorl	%r9d, %r9d
	shrq	$63, %rcx
	movq	$-1, %r10
	movabsq	$9223372036854775807, %r11
	movq	$0, 32(%rsp)
	mulq	%rsi
	movb	%cl, 87(%rsp)
	xorb	64(%rsp), %cl
	movzbl	%cl, %ecx
	addq	%rcx, %r10
	adcq	$0, %r11
	movq	%rdx, 40(%rsp)
	movq	%rax, %rsi
	xorl	%edi, %edi
	orq	32(%rsp), %rsi
	orq	40(%rsp), %rdi
.L814:
	movq	%r8, %rax
	orq	%r9, %rax
	jne	.L839
.L835:
	movzbl	87(%rsp), %ebx
	cmpb	%bl, 64(%rsp)
	je	.L783
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
.L783:
	movq	0(%rbp), %rcx
	movq	8(%rbp), %rbx
	movq	%rdi, %rdx
	movq	%rsi, %rax
	movq	%rsi, 176(%rsp)
	addq	%rsi, %rcx
	movq	%rdi, 184(%rsp)
	adcq	%rdi, %rbx
	jo	.L781
	movq	56(%rsp), %rdi
	movq	%rbx, 120(%rsp)
	movq	48(%rsp), %rbx
	movq	%rcx, 112(%rsp)
	movl	(%rdi,%r12,4), %esi
	salq	$4, %rsi
	addq	%rbx, %rsi
	movq	(%rsi), %rcx
	movq	8(%rsi), %rbx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jo	.L781
	movq	%rbx, 152(%rsp)
	movl	$1, %eax
	movl	%r15d, %ebx
	movdqa	112(%rsp), %xmm0
	movb	%al, %bh
	movq	%rcx, 144(%rsp)
	movups	%xmm0, 0(%rbp)
	movl	%ebx, %eax
	movl	%ebx, %r15d
	movdqa	144(%rsp), %xmm0
	shrw	$8, %ax
	movups	%xmm0, (%rsi)
	je	.L798
.L840:
	addq	$1, %r12
	addq	$16, %rbp
	addq	$16, %r14
	cmpq	%r12, 16(%rsp)
	jne	.L799
.L762:
	movq	24(%rsp), %rax
	movb	$1, 16(%rax)
	jmp	.L757
	.p2align 4,,10
	.p2align 3
.L769:
	subq	%r13, %r10
	jmp	.L833
	.p2align 4,,10
	.p2align 3
.L764:
	movq	%rbx, %rcx
	xorl	%r9d, %r9d
	movq	$-1, %r10
	movabsq	$9223372036854775807, %r11
	shrq	$63, %rcx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r13
# 0 "" 2
#NO_APP
	movq	%rdx, %r8
	mulq	(%rsp)
	movb	%cl, 87(%rsp)
	xorb	64(%rsp), %cl
	movzbl	%cl, %ecx
	addq	%rcx, %r10
	movq	%rax, %rsi
	movq	%r8, %rax
	movq	%rdx, %rdi
	adcq	$0, %r11
	orq	%r9, %rax
	je	.L835
.L839:
	movq	(%rsp), %rax
	mulq	%r8
	movq	%rax, %rcx
	movq	%rdx, %r9
	movq	%rax, %r8
	testq	%rdx, %rdx
	je	.L773
	cmpq	%r13, %rdx
	jnb	.L784
	movq	$0, 96(%rsp)
	movq	$0, 104(%rsp)
.L787:
	movq	%rcx, %rax
	movq	%r9, %rdx
	xorl	%ebx, %ebx
	xorl	%r9d, %r9d
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r13
# 0 "" 2
#NO_APP
	orq	104(%rsp), %rbx
	movq	%rax, %rcx
	movq	%rdx, %r8
	orq	96(%rsp), %rcx
.L785:
	movq	%r10, %rax
	movq	%r11, %rdx
	subq	%rsi, %rax
	sbbq	%rdi, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L800
	addq	%rcx, %rsi
	movq	%r11, %rax
	adcq	%rbx, %rdi
	cmpq	%rsi, %r10
	sbbq	%rdi, %rax
	jc	.L800
	movq	%r8, %rcx
	orq	%r9, %rcx
	je	.L835
	movb	$2, 176(%rsp)
	jmp	.L794
	.p2align 4,,10
	.p2align 3
.L781:
	xorl	%eax, %eax
	movl	%r15d, %ebx
	movb	%al, %bh
	movl	%ebx, %eax
	movl	%ebx, %r15d
	shrw	$8, %ax
	jne	.L840
.L798:
	movq	24(%rsp), %rax
	movq	%r12, (%rax)
	movb	$0, 8(%rax)
	movb	$0, 16(%rax)
	jmp	.L757
	.p2align 4,,10
	.p2align 3
.L837:
	movq	%rbx, %rcx
	movq	$-1, %r10
	movq	%rsi, %rdi
	movq	%r8, %rsi
	movabsq	$9223372036854775807, %r11
	shrq	$63, %rcx
	movb	%cl, 87(%rsp)
	xorb	64(%rsp), %cl
	movzbl	%cl, %ecx
	addq	%rcx, %r10
	adcq	$0, %r11
	xorl	%r9d, %r9d
	orq	%r9, %rdi
	movq	%r11, %rax
	cmpq	%r8, %r10
	sbbq	%rdi, %rax
	jc	.L800
	movq	96(%rsp), %r8
	xorl	%r9d, %r9d
	jmp	.L814
	.p2align 4,,10
	.p2align 3
.L836:
	shrq	$63, %rbp
	xorl	%r13d, %r13d
	cmpq	$0, 16(%rsp)
	movq	%r14, %r11
	movabsq	$-9223372036854775808, %r14
	leaq	8(%rbx), %r10
	je	.L762
	.p2align 4
	.p2align 3
.L812:
	movq	-8(%r10), %rsi
	movq	(%r10), %rdi
	movq	(%rsp), %rax
	movq	%rsi, %rcx
	movq	%rdi, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rsi, %rcx
	cmovs	%rdi, %r8
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rax, %rcx
	movq	(%rsp), %rax
	movq	%rdx, %rbx
	mulq	%r8
	addq	%rbx, %rax
	adcq	%r9, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %r8
	orq	%r9, %r8
	jne	.L800
	movq	%rdi, %rsi
	movq	%rax, %rdx
	xorl	%ebx, %ebx
	xorl	%eax, %eax
	shrq	$63, %rsi
	orq	%rax, %rcx
	orq	%rdx, %rbx
	xorl	%eax, %eax
	cmpb	%bpl, %sil
	sete	%al
	negq	%rax
	sbbq	%rdx, %rdx
	xorq	%r13, %rax
	xorq	%r14, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L800
	cmpb	%bpl, %sil
	je	.L802
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L802:
	movq	(%r11), %rax
	movq	8(%r11), %rdx
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	jo	.L805
	movq	%rax, 112(%rsp)
	movq	56(%rsp), %rax
	movq	%rdx, 120(%rsp)
	movl	(%rax,%r12,4), %esi
	movq	48(%rsp), %rax
	salq	$4, %rsi
	addq	%rax, %rsi
	movq	(%rsi), %rax
	movq	8(%rsi), %rdx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jo	.L805
	movdqa	112(%rsp), %xmm0
	movq	%rcx, 144(%rsp)
	movl	$1, %eax
	movq	%rbx, 152(%rsp)
	movups	%xmm0, (%r11)
	movdqa	144(%rsp), %xmm0
	movups	%xmm0, (%rsi)
.L808:
	movl	%r15d, %ebx
	movb	%al, %bh
	movl	%ebx, %eax
	movl	%ebx, %r15d
	shrw	$8, %ax
	je	.L798
	addq	$1, %r12
	addq	$16, %r11
	addq	$16, %r10
	cmpq	%r12, 16(%rsp)
	jne	.L812
	jmp	.L762
	.p2align 4,,10
	.p2align 3
.L805:
	xorl	%eax, %eax
	jmp	.L808
	.p2align 4,,10
	.p2align 3
.L773:
	movq	88(%rsp), %r9
	xorl	%ebx, %ebx
	imulq	%rax, %r9
	movq	72(%rsp), %rax
	mulq	%rcx
	leaq	(%r9,%rdx), %rcx
	movq	%rcx, %rax
	imulq	%r13, %rax
	subq	%rax, %r8
	xorl	%eax, %eax
	cmpq	%r13, %r8
	setnb	%al
	addq	%rax, %rcx
	movq	%r8, %rax
	adcq	$0, %rbx
	subq	%r13, %rax
	cmpq	%r13, %r8
	cmovnb	%rax, %r8
	xorl	%r9d, %r9d
	jmp	.L785
.L784:
	movq	72(%rsp), %rax
	mulq	%rdx
	movq	%r13, %rax
	imulq	%rdx, %rax
	subq	%rax, %r9
	xorl	%eax, %eax
	cmpq	%r13, %r9
	movq	%r9, %r8
	setnb	%al
	addq	%rdx, %rax
	subq	%r13, %r8
	cmpq	%r13, %r9
	movq	%rax, %rdx
	cmovnb	%r8, %r9
	xorl	%eax, %eax
	movq	%rax, 96(%rsp)
	movq	%rdx, 104(%rsp)
	jmp	.L787
.L838:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1834:
	.size	_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE, .-_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	.text
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.type	_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE, @function
_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE:
.LFB1703:
	.cfi_startproc
	endbr64
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	subq	$24, %rsp
	.cfi_def_cfa_offset 40
	movq	%fs:40, %rbx
	movq	%rbx, 16(%rsp)
	movq	%rdi, %rbx
	pushq	40(%rsp)
	.cfi_def_cfa_offset 48
	call	_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	popq	%rax
	.cfi_def_cfa_offset 40
	popq	%rdx
	.cfi_def_cfa_offset 32
	movq	8(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L844
	addq	$16, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 16
	movq	%rbx, %rax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
.L844:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1703:
	.size	_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE, .-_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.section	.text._ZNK9fixedwide6detail5batch20broadcast_multiplierINS_11basic_fixedILm128ELj12EEES4_EclIS4_EESt8expectedIS4_NS_15ArithmeticErrorEET_,"axG",@progbits,_ZNK9fixedwide6detail5batch20broadcast_multiplierINS_11basic_fixedILm128ELj12EEES4_EclIS4_EESt8expectedIS4_NS_15ArithmeticErrorEET_,comdat
	.align 2
	.p2align 4
	.weak	_ZNK9fixedwide6detail5batch20broadcast_multiplierINS_11basic_fixedILm128ELj12EEES4_EclIS4_EESt8expectedIS4_NS_15ArithmeticErrorEET_
	.type	_ZNK9fixedwide6detail5batch20broadcast_multiplierINS_11basic_fixedILm128ELj12EEES4_EclIS4_EESt8expectedIS4_NS_15ArithmeticErrorEET_, @function
_ZNK9fixedwide6detail5batch20broadcast_multiplierINS_11basic_fixedILm128ELj12EEES4_EclIS4_EESt8expectedIS4_NS_15ArithmeticErrorEET_:
.LFB1959:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movq	%rdx, %xmm1
	xorl	%r9d, %r9d
	movq	%rcx, %xmm0
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movdqa	%xmm1, %xmm2
	movq	%rdi, %r14
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	punpcklqdq	%xmm0, %xmm2
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$280, %rsp
	.cfi_def_cfa_offset 336
	movq	56(%rsi), %rbx
	movq	48(%rsi), %rdi
	movq	%fs:40, %rax
	movq	%rax, 264(%rsp)
	xorl	%eax, %eax
	movq	%rdx, (%rsp)
	movq	(%rsp), %r10
	movq	%rcx, 8(%rsp)
	movq	8(%rsp), %r11
	movq	%r10, %rdx
	movq	%rbx, 16(%rsp)
	movzbl	16(%rsi), %r15d
	negq	%rdx
	movq	%r11, %rax
	movq	%r9, 24(%rsp)
	adcq	$0, %rax
	negq	%rax
	cmovs	8(%rsp), %rax
	cmovs	%r10, %rdx
	xorl	%r11d, %r11d
	movq	%rax, %r13
	movq	%rax, %r10
	movq	%rbx, %rax
	orq	%r9, %rax
	movq	%rdx, %r12
	jne	.L846
	movq	8(%rsp), %rcx
	movzbl	64(%rsi), %eax
	movabsq	$9223372036854775807, %rdx
	shrq	$63, %rcx
	movb	%al, 32(%rsp)
	xorl	%ecx, %eax
	movq	%rcx, 48(%rsp)
	movb	%al, 63(%rsp)
	movzbl	%al, %ecx
	movq	$-1, %rax
	addq	%rcx, %rax
	movq	24(%rsi), %rcx
	adcq	$0, %rdx
	movq	%rax, (%rsp)
	movq	%rdx, 8(%rsp)
	cmpb	$5, %r15b
	je	.L847
	movq	%r13, %rax
	orq	%r11, %rax
	jne	.L848
	movq	%r13, %rax
	imulq	%r12, %rbx
	imulq	%rdi, %rax
	addq	%rax, %rbx
	movq	%rdi, %rax
	mulq	%r12
	addq	%rdx, %rbx
	movq	%rax, %r8
	movq	%rbx, %r9
	cmpq	$1, %rcx
	je	.L1100
	movq	%rbx, %rdi
	movq	%rax, %r10
	testq	%rbx, %rbx
	jne	.L852
	movq	32(%rsi), %rax
	xorl	%r9d, %r9d
	mulq	%r8
	movq	%r10, %rax
	movq	%rdx, %r8
	imulq	%rcx, %rdx
	subq	%rdx, %rax
	xorl	%edx, %edx
	cmpq	%rcx, %rax
	setnb	%dl
	addq	%rdx, %r8
	movq	%rax, %rdx
	adcq	$0, %r9
	subq	%rcx, %rdx
	cmpq	%rcx, %rax
	cmovnb	%rdx, %rax
	xorl	%edx, %edx
.L854:
	movq	%rax, %rsi
	orq	%rdx, %rsi
	je	.L884
	cmpb	$3, %r15b
	je	.L858
.L1099:
	ja	.L894
	cmpb	$1, %r15b
	je	.L895
	xorb	$1, 63(%rsp)
	cmpb	$2, %r15b
	jne	.L884
.L895:
	movq	(%rsp), %rbx
	movq	8(%rsp), %rsi
	movq	%rbx, %rax
	movq	%rsi, %rdx
	xorq	%r8, %rax
	xorq	%r9, %rdx
	orq	%rdx, %rax
	jne	.L1021
	cmpb	$0, 63(%rsp)
	jne	.L1083
.L1021:
	movzbl	63(%rsp), %eax
	addq	%rax, %r8
	adcq	$0, %r9
	.p2align 4
	.p2align 3
.L884:
	movzbl	32(%rsp), %eax
	cmpb	%al, 48(%rsp)
	je	.L945
.L1085:
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L945:
	movq	%r8, (%r14)
	movq	%r9, 8(%r14)
	movb	$1, 16(%r14)
	jmp	.L845
	.p2align 4,,10
	.p2align 3
.L847:
	cmpq	$1, %rcx
	je	.L869
	movq	%r13, %rbp
	movq	%r12, %r11
	testq	%r13, %r13
	jne	.L870
	movq	32(%rsi), %rax
	movq	32(%rsi), %r8
	xorl	%r9d, %r9d
	movq	%rbx, %r10
	movq	$0, 72(%rsp)
	mulq	%r12
	imulq	%r13, %r8
	addq	%rdx, %r8
	movq	%r8, %rax
	imulq	%rcx, %rax
	subq	%rax, %r11
	xorl	%eax, %eax
	cmpq	%rcx, %r11
	setnb	%al
	addq	%rax, %r8
	movq	%r11, %rax
	adcq	$0, %r9
	subq	%rcx, %rax
	cmpq	%rcx, %r11
	cmovnb	%rax, %r11
	imulq	%r8, %r10
	movq	%r9, %rax
	imulq	%rdi, %rax
	movq	%r11, 64(%rsp)
	addq	%rax, %r10
	movq	%rdi, %rax
	mulq	%r8
	addq	%rdx, %r10
	movq	%rax, %r8
	movq	%r10, %r12
.L874:
	movq	(%rsp), %rax
	movq	%r12, %r13
	movq	8(%rsp), %rdx
	xorl	%r12d, %r12d
	xorl	%r9d, %r9d
	orq	%r12, %r8
	orq	%r13, %r9
	cmpq	%r8, %rax
	movq	%rdx, %rax
	sbbq	%r9, %rax
	jc	.L1083
	movq	64(%rsp), %r11
	movq	72(%rsp), %r12
	movq	%r11, %rax
	orq	%r12, %rax
	je	.L884
	movq	%r12, %rax
	imulq	%r11, %rbx
	imulq	%rdi, %rax
	addq	%rax, %rbx
	movq	%r11, %rax
	mulq	%rdi
	addq	%rdx, %rbx
	movq	%rax, %r10
	movq	%rbx, %r11
	cmpq	$1, %rcx
	je	.L1101
	movq	%rax, %rdi
	testq	%rbx, %rbx
	jne	.L885
	movq	32(%rsi), %rax
	xorl	%r11d, %r11d
	mulq	%r10
	movq	%rdi, %rax
	movq	%r11, %rdi
	movq	%rdx, %r10
	imulq	%rcx, %rdx
	movq	%r10, %rsi
	subq	%rdx, %rax
	xorl	%edx, %edx
	cmpq	%rcx, %rax
	setnb	%dl
	addq	%rdx, %rsi
	movq	%rax, %rdx
	adcq	$0, %rdi
	subq	%rcx, %rdx
	cmpq	%rcx, %rax
	cmovnb	%rdx, %rax
	xorl	%edx, %edx
.L887:
	movq	(%rsp), %r10
	movq	8(%rsp), %r11
	subq	%r8, %r10
	sbbq	%r9, %r11
	cmpq	%rsi, %r10
	movq	%r11, %rbx
	sbbq	%rdi, %rbx
	jc	.L1083
	addq	%rsi, %r8
	movq	(%rsp), %rbx
	movq	8(%rsp), %rsi
	adcq	%rdi, %r9
	cmpq	%r8, %rbx
	movq	%rsi, %rbx
	sbbq	%r9, %rbx
	jc	.L1083
	movq	%rax, %rsi
	orq	%rdx, %rsi
	je	.L884
	cmpb	$5, %r15b
	je	.L1084
	cmpb	$3, %r15b
	jne	.L1099
	xorl	%edx, %edx
	addq	%rax, %rax
	adcq	%rdx, %rdx
	xorl	%edi, %edi
	cmpq	%rax, %rcx
	movq	%rdi, %rbx
	sbbq	%rdx, %rbx
	jc	.L1009
	testq	%rdx, %rdx
	jne	.L884
	cmpq	%rcx, %rax
	jne	.L884
	movq	%r8, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1009
	testq	%rax, %rax
	jne	.L1009
	jmp	.L884
	.p2align 4,,10
	.p2align 3
.L846:
	movdqa	(%rsi), %xmm1
	movhlps	%xmm1, %xmm4
	movq	%xmm1, 16(%rsp)
	movq	16(%rsp), %rax
	movq	%xmm4, 24(%rsp)
	movq	24(%rsp), %rdx
	movq	%xmm4, %rcx
	movq	%rax, %rdi
	negq	%rdi
	movq	%rdx, %rsi
	adcq	$0, %rsi
	negq	%rsi
	cmovs	%rdx, %rsi
	cmovs	%rax, %rdi
	xorq	8(%rsp), %rdx
	xorq	(%rsp), %rax
	movq	%rdx, %rbp
	movq	%rdx, 40(%rsp)
	xorl	%edx, %edx
	movq	%rsi, %rbx
	shrq	$63, %rbp
	movq	%rax, 32(%rsp)
	orq	%rdx, %rbx
	movb	%bpl, 48(%rsp)
	jne	.L906
	movq	%r13, %rax
	cmpb	$5, %r15b
	je	.L907
	orq	%r11, %rax
	jne	.L908
	movq	%r13, %rax
	imulq	%r12, %rsi
	imulq	%rdi, %rax
	addq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r12
	addq	%rdx, %rsi
	movq	%rax, %r12
	movq	%rsi, %rdx
	xorl	%esi, %esi
	movq	%r12, %rcx
	movq	%rdx, %rax
	orq	%rsi, %rax
	je	.L1102
	movabsq	$2535301200456458803, %rax
	movq	%rdx, %rcx
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %rdi
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	imulq	%rdi, %rax
	subq	%rax, %rcx
	movq	%r12, %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rdi
# 0 "" 2
#NO_APP
	movq	%rsi, %rdi
	movq	%rdx, %rcx
	movq	%rax, %rsi
.L910:
	movq	%rcx, %rax
	orq	%rbx, %rax
	je	.L911
	cmpb	$3, %r15b
	je	.L912
	ja	.L913
	cmpb	$1, %r15b
	je	.L914
	cmpb	$2, %r15b
	jne	.L911
	movq	40(%rsp), %rdx
	notq	%rdx
	movq	%rdx, %rax
	shrq	$63, %rax
	movq	%rax, 48(%rsp)
.L914:
	movzbl	48(%rsp), %eax
	addq	%rax, %rsi
	adcq	$0, %rdi
	.p2align 4
	.p2align 3
.L911:
	cmpq	$0, 40(%rsp)
	jns	.L921
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
.L921:
	movq	%rsi, (%r14)
	movq	%rdi, 8(%r14)
	movb	$1, 16(%r14)
	jmp	.L845
	.p2align 4,,10
	.p2align 3
.L848:
	movq	%r13, %rbp
	cmpq	$1, %rcx
	je	.L869
.L870:
	xorl	%r8d, %r8d
	xorl	%r9d, %r9d
	cmpq	%rcx, %rbp
	jb	.L875
	movq	32(%rsi), %rax
	xorl	%r8d, %r8d
	mulq	%r10
	movq	%rcx, %rax
	imulq	%rdx, %rax
	subq	%rax, %rbp
	cmpq	%rcx, %rbp
	movq	%rbp, %rax
	setnb	%r8b
	addq	%rdx, %r8
	subq	%rcx, %rax
	cmpq	%rcx, %rbp
	cmovnb	%rax, %rbp
	movq	%r8, %r9
	xorl	%r8d, %r8d
.L875:
	movq	%r12, %rax
	movq	%rbp, %rdx
	xorl	%r13d, %r13d
	movq	$0, 72(%rsp)
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rcx
# 0 "" 2
#NO_APP
	movq	%rdx, %rbp
	xorl	%edx, %edx
	orq	%r8, %rax
	xorl	%r11d, %r11d
	movq	%rbp, 64(%rsp)
	orq	%r9, %rdx
	movq	%rax, %r12
	movq	%rdx, %r10
.L872:
	movq	%r13, %rax
	imulq	%rdi, %r11
	movq	%rbx, %rbp
	xorl	%r13d, %r13d
	imulq	%r12, %rbp
	imulq	%rdi, %rax
	addq	%rax, %rbp
	movq	%rdi, %rax
	mulq	%r12
	movq	%rax, %r8
	movq	%rbx, %rax
	addq	%rdx, %rbp
	imulq	%r10, %rax
	movq	%rbp, %r12
	addq	%rax, %r11
	movq	%rdi, %rax
	mulq	%r10
	addq	%rdx, %r11
	addq	%rax, %r12
	adcq	%r11, %r13
	xorl	%edx, %edx
	movq	%r13, %rax
	orq	%rdx, %rax
	je	.L874
.L1083:
	movb	$0, (%r14)
	movb	$0, 16(%r14)
.L845:
	movq	264(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L1103
	addq	$280, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%r14, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L906:
	.cfi_restore_state
	orq	%r11, %r10
	jne	.L947
	movabsq	$2535301200456458803, %r9
	movq	%rsi, %rax
	movq	%rsi, %rcx
	movabsq	$1000000000000, %r8
	mulq	%r9
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movq	%rsi, 8(%rsp)
	xorl	%esi, %esi
	imulq	%r8, %rax
	imulq	%r12, %rsi
	subq	%rax, %rcx
	movq	%rdi, %rax
	xorl	%edi, %edi
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r8
# 0 "" 2
#NO_APP
	movq	%rax, (%rsp)
	movq	(%rsp), %rcx
	movq	%r13, %rax
	movq	%rdx, %r10
	imulq	%rcx, %rax
	addq	%rax, %rsi
	movq	%rcx, %rax
	mulq	%r12
	addq	%rdx, %rsi
	movq	%rax, %rcx
	movq	8(%rsp), %rax
	movq	%rsi, %rbx
	movq	%r13, %rsi
	imulq	%rax, %rsi
	mulq	%r12
	addq	%rdx, %rsi
	movq	%rsi, %rdx
	movq	%rbx, %rsi
	addq	%rax, %rsi
	adcq	%rdx, %rdi
	xorl	%edx, %edx
	movq	%rdi, %rax
	orq	%rdx, %rax
	jne	.L1083
	movq	$-1, %rax
	movq	%rsi, %rdi
	movabsq	$9223372036854775807, %rdx
	addq	%rbp, %rax
	adcq	$0, %rdx
	xorl	%esi, %esi
	xorl	%ebx, %ebx
	movq	%rax, (%rsp)
	orq	%rsi, %rcx
	orq	%rdi, %rbx
	movq	%rdx, 8(%rsp)
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1083
	testq	%r10, %r10
	je	.L957
	movq	%r10, %r11
	movq	%r10, %rax
	xorl	%r10d, %r10d
	mulq	%r12
	imulq	%r13, %r11
	movq	%rax, %r12
	addq	%rdx, %r11
	movq	%r11, %rax
	movq	%r12, %r11
	movq	%rax, %rdx
	orq	%r10, %rdx
	jne	.L953
	movq	%r12, %rax
	xorl	%edi, %edi
	mulq	%r9
	movq	%rdx, %rsi
	xorl	%edx, %edx
	shrq	$37, %rsi
	movq	%rsi, %rax
	imulq	%r8, %rsi
	subq	%rsi, %r11
	movq	%r11, %rsi
.L954:
	movq	(%rsp), %r8
	movq	8(%rsp), %r9
	subq	%rcx, %r8
	sbbq	%rbx, %r9
	cmpq	%rax, %r8
	sbbq	%rdx, %r9
	jc	.L1083
	addq	%rax, %rcx
	movq	(%rsp), %rax
	adcq	%rdx, %rbx
	movq	8(%rsp), %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1083
	movq	%rsi, %rax
	orq	%rdi, %rax
	je	.L957
	cmpb	$5, %r15b
	je	.L1084
	cmpb	$3, %r15b
	je	.L959
	ja	.L960
	cmpb	$1, %r15b
	je	.L961
	cmpb	$2, %r15b
	jne	.L1016
	movq	40(%rsp), %rdx
	notq	%rdx
	movq	%rdx, %rax
	shrq	$63, %rax
	movq	%rax, 48(%rsp)
.L961:
	movq	(%rsp), %rdi
	movq	8(%rsp), %rbp
	movq	%rdi, %rax
	movq	%rbp, %rdx
	xorq	%rcx, %rax
	xorq	%rbx, %rdx
	orq	%rdx, %rax
	jne	.L963
	cmpb	$0, 48(%rsp)
	jne	.L1083
.L963:
	movzbl	48(%rsp), %eax
	addq	%rax, %rcx
	adcq	$0, %rbx
	.p2align 4
	.p2align 3
.L957:
	cmpq	$0, 40(%rsp)
	jns	.L968
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L968:
	movq	%rcx, (%r14)
	movq	%rbx, 8(%r14)
	movb	$1, 16(%r14)
	jmp	.L845
	.p2align 4,,10
	.p2align 3
.L869:
	movq	$0, 64(%rsp)
	xorl	%r13d, %r13d
	movl	$1, %ecx
	movq	$0, 72(%rsp)
	jmp	.L872
	.p2align 4,,10
	.p2align 3
.L1100:
	movq	(%rsp), %rax
	movq	8(%rsp), %rdx
	cmpq	%r8, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1083
	jmp	.L884
	.p2align 4,,10
	.p2align 3
.L907:
	orq	%r11, %rax
	je	.L1104
.L908:
	movabsq	$2535301200456458803, %rax
	movq	%r10, %rcx
	xorl	%r9d, %r9d
	mulq	%r10
	movabsq	$1000000000000, %r10
	movq	%rdx, %r8
	shrq	$37, %r8
	movq	%r8, %rax
	movq	%r8, %r11
	movq	%r9, %r8
	movq	%rsi, %r9
	imulq	%r10, %rax
	imulq	%rdi, %r8
	imulq	%r11, %r9
	subq	%rax, %rcx
	movq	%r12, %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r10
# 0 "" 2
#NO_APP
	movq	%rax, %r10
	movq	%rsi, %rax
	movq	%rdx, %rcx
	imulq	%r10, %rax
	addq	%rax, %r8
	movq	%r10, %rax
	mulq	%rdi
	movq	%rax, %r12
	movq	%r11, %rax
	addq	%rdx, %r8
	mulq	%rdi
	addq	%rdx, %r9
	movq	%r9, %rdx
	xorl	%r9d, %r9d
	addq	%r8, %rax
	adcq	%r9, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %r8
	orq	%r9, %r8
	jne	.L1083
	movq	$-1, %r10
	movq	%rax, %rdx
	movabsq	$9223372036854775807, %r11
	addq	%rbp, %r10
	adcq	$0, %r11
	xorl	%eax, %eax
	xorl	%r13d, %r13d
	orq	%r12, %rax
	orq	%r13, %rdx
	movq	%rax, %r8
	cmpq	%rax, %r10
	movq	%r11, %rax
	movq	%rdx, %r9
	sbbq	%rdx, %rax
	jc	.L1083
	movq	%rcx, %r12
	xorl	%r13d, %r13d
.L924:
	movq	%r12, %rax
	orq	%r13, %rax
	je	.L934
	movq	%r13, %rax
	imulq	%r12, %rsi
	imulq	%rdi, %rax
	addq	%rax, %rsi
	movq	%rdi, %rax
	xorl	%edi, %edi
	mulq	%r12
	addq	%rdx, %rsi
	movq	%rax, %r12
	movq	%rsi, %rdx
	movq	%r12, %rsi
	movq	%rdx, %rcx
	orq	%rdi, %rcx
	jne	.L930
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	mulq	%r12
	movabsq	$1000000000000, %rax
	shrq	$37, %rdx
	movq	%rdx, %rcx
	imulq	%rax, %rdx
	subq	%rdx, %rsi
	xorl	%edx, %edx
	movq	%rsi, %rax
.L931:
	movq	%r10, %rsi
	movq	%r11, %rdi
	subq	%r8, %rsi
	sbbq	%r9, %rdi
	cmpq	%rcx, %rsi
	sbbq	%rbx, %rdi
	jc	.L1083
	addq	%rcx, %r8
	adcq	%rbx, %r9
	cmpq	%r8, %r10
	movq	%r11, %rbx
	sbbq	%r9, %rbx
	jc	.L1083
	movq	%rax, %rcx
	orq	%rdx, %rcx
	je	.L934
	cmpb	$5, %r15b
	je	.L1084
	cmpb	$3, %r15b
	je	.L936
	ja	.L937
	cmpb	$1, %r15b
	je	.L938
	cmpb	$2, %r15b
	jne	.L1013
	movq	40(%rsp), %rdx
	notq	%rdx
	movq	%rdx, %rax
	shrq	$63, %rax
	movq	%rax, 48(%rsp)
.L938:
	movq	%r8, %rax
	movq	%r11, %rdx
	xorq	%r10, %rax
	xorq	%r9, %rdx
	orq	%rdx, %rax
	jne	.L940
	cmpb	$0, 48(%rsp)
	jne	.L1083
.L940:
	movzbl	48(%rsp), %eax
	addq	%rax, %r8
	adcq	$0, %r9
	.p2align 4
	.p2align 3
.L934:
	cmpq	$0, 40(%rsp)
	jns	.L945
	jmp	.L1085
	.p2align 4,,10
	.p2align 3
.L852:
	cmpq	%rcx, %rbx
	jb	.L855
	movq	32(%rsi), %rax
	xorl	%r8d, %r8d
	movq	8(%rsp), %rsi
	mulq	%rbx
	movq	%rcx, %rax
	movq	(%rsp), %rbx
	imulq	%rdx, %rax
	subq	%rax, %rdi
	cmpq	%rcx, %rdi
	movq	%rdi, %rax
	setnb	%r8b
	addq	%rdx, %r8
	subq	%rcx, %rax
	cmpq	%rcx, %rdi
	cmovnb	%rax, %rdi
	movq	%r8, %r9
	movq	%r10, %rax
	movq	%rdi, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rcx
# 0 "" 2
#NO_APP
	cmpq	%rax, %rbx
	movq	%rax, %r8
	movq	%rsi, %rax
	sbbq	%r9, %rax
	jc	.L1083
	movq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L854
	.p2align 4,,10
	.p2align 3
.L947:
	movq	16(%rsp), %r9
	movq	(%rsp), %r11
	xorl	%esi, %esi
	movq	24(%rsp), %r10
	movq	8(%rsp), %r12
	movq	%r9, %rax
	movq	%r11, %rdx
	sarq	$63, %rax
	movq	%r10, %rbx
	movq	%r12, %rdi
	sarq	$63, %rdx
	cmpq	%r10, %rax
	jne	.L971
	cmpq	%r12, %rdx
	jne	.L972
	movq	%r9, %rax
	imulq	%r11
	movq	%rax, %r8
	movq	%rdx, %r9
.L969:
	andl	$1, %esi
	je	.L979
	movq	%rcx, %rax
	movl	%r15d, %r8d
	movl	$12, %ecx
	movl	$12, %edx
	sarq	$63, %rax
	movaps	%xmm2, 160(%rsp)
	movl	$12, %esi
	leaq	224(%rsp), %rdi
	movq	%rax, %xmm3
	movq	%xmm0, %rax
	subq	$64, %rsp
	.cfi_def_cfa_offset 400
	movl	$128, %r9d
	sarq	$63, %rax
	punpcklqdq	%xmm3, %xmm3
	movq	%rax, %xmm0
	movaps	%xmm3, 272(%rsp)
	punpcklqdq	%xmm0, %xmm0
	movaps	%xmm0, 240(%rsp)
	movdqa	272(%rsp), %xmm0
	movups	%xmm1, 32(%rsp)
	movups	%xmm0, 48(%rsp)
	movdqa	224(%rsp), %xmm0
	movups	%xmm0, (%rsp)
	movdqa	240(%rsp), %xmm0
	movups	%xmm0, 16(%rsp)
	call	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	addq	$64, %rsp
	.cfi_def_cfa_offset 336
	cmpb	$0, 256(%rsp)
	jne	.L980
	movzbl	224(%rsp), %eax
	movb	$0, 16(%r14)
	movb	%al, (%r14)
	jmp	.L845
	.p2align 4,,10
	.p2align 3
.L855:
	movq	%rbx, %rdx
	xorl	%r9d, %r9d
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rcx
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L854
	.p2align 4,,10
	.p2align 3
.L1104:
	movabsq	$2535301200456458803, %rax
	xorl	%r13d, %r13d
	movabsq	$1000000000000, %r8
	movabsq	$9223372036854775807, %r11
	mulq	%r12
	movq	%r12, %rax
	shrq	$37, %rdx
	imulq	%rdx, %r8
	movq	%rdx, %r10
	imulq	%rsi, %r10
	subq	%r8, %rax
	movq	%rax, %r12
	movq	%rdx, %rax
	mulq	%rdi
	addq	%rdx, %r10
	movq	%rax, %r8
	movq	%r10, %r9
	movq	$-1, %r10
	addq	%rbp, %r10
	adcq	$0, %r11
	jmp	.L924
	.p2align 4,,10
	.p2align 3
.L1102:
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	xorl	%ebx, %ebx
	mulq	%r12
	movabsq	$1000000000000, %rax
	shrq	$37, %rdx
	movq	%rdx, %rsi
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	jmp	.L910
	.p2align 4,,10
	.p2align 3
.L1101:
	movq	(%rsp), %rbx
	movq	8(%rsp), %rsi
	movq	%rbx, %rax
	movq	%rsi, %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%r10, %rax
	movq	%rdx, %rax
	sbbq	%r11, %rax
	jc	.L1083
	addq	%r10, %r8
	movq	%rsi, %rax
	adcq	%r11, %r9
	cmpq	%r8, %rbx
	sbbq	%r9, %rax
	jc	.L1083
	jmp	.L884
	.p2align 4,,10
	.p2align 3
.L971:
	cmpq	%r12, %rdx
	jne	.L974
	movdqa	16(%rsp), %xmm5
	movq	(%rsp), %rdi
	movaps	%xmm5, 32(%rsp)
	movq	32(%rsp), %r12
	movq	40(%rsp), %r13
.L973:
	movq	16(%rsp), %rax
	mulq	(%rsp)
	movq	%rax, %r8
	movq	%rdi, %rax
	movq	%rdx, %r9
	mulq	%rbx
	movq	%rax, %r10
	movq	%rdx, %r11
	testq	%rbx, %rbx
	jns	.L975
	xorl	%eax, %eax
	subq	%rax, %r10
	sbbq	%rdi, %r11
.L975:
	testq	%rdi, %rdi
	jns	.L976
	subq	%r12, %r10
	sbbq	%r13, %r11
.L976:
	movq	%r9, %rax
	xorl	%edx, %edx
	addq	%r10, %rax
	movq	%rax, %rdi
	adcq	%r11, %rdx
	sarq	$63, %rdi
	cmpq	%rdx, %rdi
	jne	.L977
	movq	%rax, %r9
	jmp	.L969
.L858:
	xorl	%edx, %edx
	addq	%rax, %rax
	adcq	%rdx, %rdx
	xorl	%edi, %edi
	movq	%rax, %r11
	cmpq	%rax, %rcx
	movq	%rdi, %rax
	sbbq	%rdx, %rax
	jc	.L1009
	movq	%r11, %rax
	xorq	%rdi, %rdx
	xorq	%rcx, %rax
	orq	%rdx, %rax
	jne	.L884
	movq	%r8, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	orq	%rdx, %rax
	je	.L884
.L1009:
	movb	$1, 63(%rsp)
	jmp	.L895
	.p2align 4,,10
	.p2align 3
.L979:
	testq	%r9, %r9
	js	.L982
	movq	%r9, %rdi
	movq	%r8, %r10
	testq	%r9, %r9
	jne	.L983
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	mulq	%r8
	movabsq	$1000000000000, %rax
	shrq	$37, %rdx
	movq	%rdx, %rcx
	imulq	%rax, %rdx
	subq	%rdx, %r10
	je	.L985
	movq	%r10, %rax
.L988:
	cmpb	$5, %r15b
	je	.L1105
	cmpb	$3, %r15b
	je	.L991
	ja	.L992
	cmpb	$1, %r15b
	je	.L993
	cmpb	$2, %r15b
	jne	.L995
	movq	%r9, %rdx
	notq	%rdx
	movq	%rdx, %rax
	shrq	$63, %rax
	movq	%rax, %rsi
.L995:
	movzbl	%sil, %esi
	addq	%rsi, %rcx
	adcq	$0, %rbx
	testq	%r9, %r9
	js	.L987
	.p2align 4
	.p2align 3
.L985:
	movq	%rcx, 80(%rsp)
	movl	$1, %eax
	movq	%rbx, 88(%rsp)
.L990:
	movdqa	80(%rsp), %xmm0
	movb	%al, 96(%rsp)
	movaps	%xmm0, 112(%rsp)
	testb	%al, %al
	jne	.L1001
	movzbl	112(%rsp), %eax
	movb	$0, 16(%r14)
	movb	%al, (%r14)
	jmp	.L845
	.p2align 4,,10
	.p2align 3
.L885:
	cmpq	%rcx, %rbx
	jb	.L888
	movq	32(%rsi), %rax
	mulq	%rbx
	movq	%rcx, %rax
	imulq	%rdx, %rax
	subq	%rax, %rbx
	xorl	%eax, %eax
	cmpq	%rcx, %rbx
	movq	%rbx, %rsi
	setnb	%al
	addq	%rdx, %rax
	subq	%rcx, %rsi
	cmpq	%rcx, %rbx
	movq	%rax, %rdx
	cmovnb	%rsi, %rbx
	xorl	%eax, %eax
	movq	%rax, 16(%rsp)
	movq	%rdx, 24(%rsp)
.L888:
	movq	%rdi, %rax
	movq	%rbx, %rdx
	xorl	%edi, %edi
	orq	24(%rsp), %rdi
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rcx
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %rax
	orq	16(%rsp), %rsi
	xorl	%edx, %edx
	jmp	.L887
	.p2align 4,,10
	.p2align 3
.L980:
	movdqa	224(%rsp), %xmm0
	movb	$1, 16(%r14)
	movups	%xmm0, (%r14)
	jmp	.L845
	.p2align 4,,10
	.p2align 3
.L1001:
	movq	112(%rsp), %rax
	movq	%rax, (%r14)
	movq	120(%rsp), %rax
	movb	$1, 16(%r14)
	movq	%rax, 8(%r14)
	jmp	.L845
	.p2align 4,,10
	.p2align 3
.L894:
	cmpb	$4, %r15b
	jne	.L884
	xorl	%edx, %edx
	addq	%rax, %rax
	adcq	%rdx, %rdx
	xorl	%edi, %edi
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	setnc	63(%rsp)
	jmp	.L895
	.p2align 4,,10
	.p2align 3
.L982:
	movq	%r8, %r10
	movq	%r9, %r11
	negq	%r10
	adcq	$0, %r11
	movq	%r10, %rbp
	negq	%r11
	movq	%r11, %rdi
	jne	.L986
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	mulq	%r10
	movq	%rdx, %rax
	movabsq	$1000000000000, %rdx
	shrq	$37, %rax
	movq	%rax, %rcx
	imulq	%rdx, %rax
	subq	%rax, %rbp
	movq	%rbp, %rax
	jne	.L988
.L987:
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
	jmp	.L985
.L930:
	movabsq	$2535301200456458803, %rax
	movq	%rdx, %rdi
	movabsq	$1000000000000, %rsi
	mulq	%rdx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movq	%rcx, %rbx
	imulq	%rsi, %rax
	subq	%rax, %rdi
	movq	%r12, %rax
	movq	%rdi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rsi
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L931
.L972:
	movq	%r12, %r13
	movq	%rdi, %rbx
	movq	%r11, %r12
	movq	16(%rsp), %rdi
	jmp	.L973
.L983:
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r10
	mulq	%r9
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movq	%rcx, %rbx
	imulq	%r10, %rax
	subq	%rax, %rdi
	movq	%r8, %rax
	movq	%rdi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r10
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rax
	xorl	%edx, %edx
	movq	%rax, %rdi
	orq	%rdx, %rdi
	jne	.L988
	jmp	.L985
.L953:
	movq	%rax, %rdi
	mulq	%r9
	movq	%rdx, %r9
	shrq	$37, %r9
	movq	%r9, %rax
	imulq	%r8, %rax
	subq	%rax, %rdi
	movq	%r12, %rax
	movq	%rdi, %rdx
	xorl	%edi, %edi
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r8
# 0 "" 2
#NO_APP
	movq	%rdx, %rsi
	movq	%r9, %rdx
	jmp	.L954
.L986:
	movabsq	$2535301200456458803, %rax
	mulq	%r11
	movabsq	$1000000000000, %r11
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movq	%rcx, %rbx
	imulq	%r11, %rax
	subq	%rax, %rdi
	movq	%r10, %rax
	movq	%rdi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r11
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rax
	xorl	%edx, %edx
	movq	%rax, %rdi
	orq	%rdx, %rdi
	jne	.L988
	jmp	.L987
.L1084:
	movb	$2, (%r14)
	movb	$0, 16(%r14)
	jmp	.L845
.L913:
	cmpb	$4, %r15b
	jne	.L911
	movq	%rcx, %rax
	xorl	%edx, %edx
	addq	%rcx, %rax
	movabsq	$999999999999, %rcx
	movq	%rax, %r9
	adcq	%rdx, %rdx
	xorl	%eax, %eax
	cmpq	%r9, %rcx
	sbbq	%rdx, %rax
	setc	48(%rsp)
	jmp	.L914
.L1105:
	movb	$2, 80(%rsp)
	xorl	%eax, %eax
	jmp	.L990
.L974:
	movq	(%rsp), %rax
	movq	8(%rsp), %r8
	movq	%r10, %rdx
	movq	%r9, %r11
	imulq	%rax, %rdx
	movq	%r9, %rax
	imulq	%r8, %rax
	leaq	(%rdx,%rax), %r9
	movq	%r11, %rax
	mulq	(%rsp)
	leaq	(%r9,%rdx), %r8
	movq	%r8, %r9
	movq	%rax, %r8
	leaq	1(%r10), %rax
	cmpq	$1, %rax
	ja	.L970
	leaq	1(%r12), %rax
	cmpq	$1, %rax
	ja	.L970
	cmpq	%r12, %r10
	jne	.L978
	cmpq	%r8, %rsi
	movq	%rsi, %rax
	sbbq	%r9, %rax
	jl	.L969
.L970:
	movl	$1, %esi
	jmp	.L969
	.p2align 4,,10
	.p2align 3
.L912:
	movq	%rcx, %rax
	xorl	%edx, %edx
	addq	%rcx, %rax
	movabsq	$1000000000000, %rcx
	adcq	%rdx, %rdx
	xorl	%r8d, %r8d
	cmpq	%rax, %rcx
	sbbq	%rdx, %r8
	jc	.L1011
	testq	%rdx, %rdx
	jne	.L911
	cmpq	%rcx, %rax
	jne	.L911
	movq	%rsi, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1011
	testq	%rax, %rax
	je	.L911
.L1011:
	movb	$1, 48(%rsp)
	jmp	.L914
.L992:
	cmpb	$4, %r15b
	jne	.L995
	movabsq	$999999999999, %rsi
	xorl	%edx, %edx
	addq	%rax, %rax
	movq	%rax, %rdi
	adcq	%rdx, %rdx
	xorl	%eax, %eax
	cmpq	%rdi, %rsi
	sbbq	%rdx, %rax
	setc	%sil
	jmp	.L995
.L978:
	testq	%r9, %r9
	js	.L969
	jmp	.L970
.L991:
	xorl	%edx, %edx
	addq	%rax, %rax
	movq	%rax, %rdi
	movl	$244140625, %eax
	adcq	%rdx, %rdx
	xorl	%r10d, %r10d
	salq	$12, %rax
	cmpq	%rdi, %rax
	sbbq	%rdx, %r10
	jc	.L1019
	testq	%rdx, %rdx
	jne	.L995
	cmpq	%rax, %rdi
	jne	.L995
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1019
	testq	%rax, %rax
	je	.L995
.L1019:
	movl	$1, %esi
	jmp	.L995
.L993:
	movq	%r9, %rax
	shrq	$63, %rax
	movq	%rax, %rsi
	jmp	.L995
.L937:
	cmpb	$4, %r15b
	jne	.L1013
	movabsq	$999999999999, %rcx
	xorl	%edx, %edx
	addq	%rax, %rax
	movq	%rax, %rbx
	adcq	%rdx, %rdx
	xorl	%eax, %eax
	cmpq	%rbx, %rcx
	sbbq	%rdx, %rax
	setc	48(%rsp)
	jmp	.L938
.L1013:
	movb	$0, 48(%rsp)
	jmp	.L940
.L1103:
	call	__stack_chk_fail@PLT
.L1016:
	movb	$0, 48(%rsp)
	jmp	.L963
.L960:
	cmpb	$4, %r15b
	jne	.L1016
	movabsq	$999999999999, %rax
	xorl	%edi, %edi
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	cmpq	%rsi, %rax
	movl	$0, %eax
	sbbq	%rdi, %rax
	setc	48(%rsp)
	jmp	.L961
.L936:
	xorl	%edx, %edx
	addq	%rax, %rax
	movq	%rax, %rcx
	movl	$244140625, %eax
	adcq	%rdx, %rdx
	xorl	%esi, %esi
	salq	$12, %rax
	cmpq	%rcx, %rax
	sbbq	%rdx, %rsi
	jc	.L1014
	testq	%rdx, %rdx
	jne	.L1013
	cmpq	%rax, %rcx
	jne	.L1013
	movq	%r8, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1014
	testq	%rax, %rax
	je	.L1013
.L1014:
	movb	$1, 48(%rsp)
	jmp	.L938
.L959:
	xorl	%edi, %edi
	movl	$244140625, %eax
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	movq	%rsi, %rdx
	salq	$12, %rax
	movq	%rdi, %rsi
	xorl	%edi, %edi
	cmpq	%rdx, %rax
	sbbq	%rsi, %rdi
	jc	.L1017
	testq	%rsi, %rsi
	jne	.L1016
	cmpq	%rax, %rdx
	jne	.L1016
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1017
	testq	%rax, %rax
	je	.L1016
.L1017:
	movb	$1, 48(%rsp)
	jmp	.L961
.L977:
	movq	16(%rsp), %rdi
	movq	(%rsp), %r9
	movq	24(%rsp), %rdx
	movq	8(%rsp), %r10
	movq	%rdi, %rax
	imulq	%r9, %rdx
	imulq	%r10, %rax
	leaq	(%rdx,%rax), %rsi
	movq	%rdi, %rax
	mulq	%r9
	addq	%rdx, %rsi
	movq	%rax, %r8
	movq	%rsi, %r9
	jmp	.L970
	.cfi_endproc
.LFE1959:
	.size	_ZNK9fixedwide6detail5batch20broadcast_multiplierINS_11basic_fixedILm128ELj12EEES4_EclIS4_EESt8expectedIS4_NS_15ArithmeticErrorEET_, .-_ZNK9fixedwide6detail5batch20broadcast_multiplierINS_11basic_fixedILm128ELj12EEES4_EclIS4_EESt8expectedIS4_NS_15ArithmeticErrorEET_
	.text
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.type	_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_, @function
_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_:
.LFB1694:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movq	%rdx, %r10
	movq	%r8, %xmm0
	movq	%rsi, %r11
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movdqa	%xmm0, %xmm1
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movq	%rdi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$808, %rsp
	.cfi_def_cfa_offset 864
	movq	%rcx, 160(%rsp)
	movq	%r9, 96(%rsp)
	movq	%fs:40, %rax
	movq	%rax, 792(%rsp)
	xorl	%eax, %eax
	movq	%r8, 112(%rsp)
	movq	112(%rsp), %rdi
	movhps	96(%rsp), %xmm1
	movq	%r9, 120(%rsp)
	movq	120(%rsp), %rbp
	movq	%rdi, %rdx
	movb	$3, 320(%rsp)
	negq	%rdx
	movq	%rbp, %rax
	movaps	%xmm1, 304(%rsp)
	adcq	$0, %rax
	movq	%rdx, %rcx
	movabsq	$2535301200456458803, %rdx
	negq	%rax
	cmovs	%rbp, %rax
	cmovs	%rdi, %rcx
	xorl	%r13d, %r13d
	movq	%rcx, 144(%rsp)
	movq	%rax, %rbx
	movq	%rax, 192(%rsp)
	movq	%rax, 168(%rsp)
	movq	%rcx, 72(%rsp)
	testq	%rax, %rax
	jne	.L1107
	movq	%rcx, %rax
	movq	72(%rsp), %rcx
	mulq	%rdx
	movabsq	$1000000000000, %rax
	shrq	$37, %rdx
	imulq	%rdx, %rax
	subq	%rax, %rcx
.L1108:
	movabsq	$1000000000000, %r9
	movq	%rcx, %rax
	movq	%r9, %rdx
	.p2align 4
	.p2align 4
	.p2align 3
.L1109:
	movq	%rdx, %rsi
	xorl	%edx, %edx
	divq	%rsi
	movq	%rsi, %rax
	testq	%rdx, %rdx
	jne	.L1109
	cmpq	$0, 168(%rsp)
	movq	%rdx, %r9
	jne	.L1110
	movq	72(%rsp), %rax
	xorl	%edx, %edx
	xorl	%r15d, %r15d
	divq	%rsi
	movq	%rax, %r14
.L1111:
	movabsq	$1000000000000, %rax
	xorl	%edx, %edx
	movq	%r14, 352(%rsp)
	divq	%rsi
	movq	%r15, 360(%rsp)
	movq	$0, 128(%rsp)
	movq	%rax, %rbp
	cmpq	$1, %rax
	je	.L1112
	xorl	%ecx, %ecx
	movq	%rax, %rdx
	xorl	%edi, %edi
	movl	$1, %esi
	movq	%r10, 208(%rsp)
	movq	%r11, 200(%rsp)
	movq	%r9, 80(%rsp)
	movaps	%xmm1, 176(%rsp)
	call	__udivti3@PLT
	movq	208(%rsp), %r10
	movq	80(%rsp), %r9
	movq	%rax, 128(%rsp)
	movq	200(%rsp), %r11
	movdqa	176(%rsp), %xmm1
.L1112:
	movq	128(%rsp), %rax
	movq	%rbp, 328(%rsp)
	movq	%rax, 336(%rsp)
	movq	120(%rsp), %rax
	shrq	$63, %rax
	cmpq	$3, 160(%rsp)
	movb	%al, 80(%rsp)
	movb	%al, 368(%rsp)
	jbe	.L1612
	movq	%r15, %rax
	xorl	%edx, %edx
	orq	%rdx, %rax
	jne	.L1826
	movq	%r11, 72(%rsp)
	movq	$0, 96(%rsp)
	movq	%r9, 176(%rsp)
	movq	%r11, 200(%rsp)
	movq	%r10, 208(%rsp)
	movq	%r12, 192(%rsp)
	movq	%r10, %r12
.L1604:
	movq	(%r12), %r10
	movq	8(%r12), %r11
	movabsq	$9223372036854775807, %rcx
	movq	8(%rsp), %rsi
	movq	%r10, %rdx
	movq	%r11, %rax
	negq	%rdx
	movq	%rsi, 8(%rsp)
	adcq	$0, %rax
	negq	%rax
	cmovs	%r11, %rax
	cmovs	%r10, %rdx
	xorl	%r8d, %r8d
	movq	%rax, 8(%rsp)
	movq	%r11, %rax
	movq	8(%rsp), %rsi
	shrq	$63, %rax
	xorb	80(%rsp), %al
	movq	%rdx, (%rsp)
	movq	$-1, %rdx
	movzbl	%al, %eax
	movq	(%rsp), %rbx
	movq	%rsi, %r9
	addq	%rax, %rdx
	movq	%rsi, %rax
	adcq	$0, %rcx
	orq	%r8, %rax
	movq	%rbx, (%rsp)
	movq	(%rsp), %rbx
	movq	%rdx, 144(%rsp)
	movq	%rcx, 152(%rsp)
	jne	.L1438
	movq	%rbx, %rax
	imulq	%r14, %rsi
	imulq	%r15, %rax
	addq	%rax, %rsi
	movq	%rbx, %rax
	mulq	%r14
	addq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%rsi, %rbx
	cmpq	$1, %rbp
	je	.L1827
	movq	%rsi, %rdi
	movq	%rax, %rsi
	testq	%rbx, %rbx
	jne	.L1442
	movq	128(%rsp), %rax
	xorl	%ebx, %ebx
	mulq	%rcx
	movq	%rdx, %rax
	movq	%rdx, %rcx
	imulq	%rbp, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%rbp, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%rbp, %rax
	cmpq	%rbp, %rsi
	cmovnb	%rax, %rsi
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rsi
	je	.L1460
	addq	%rax, %rax
	adcq	%rdx, %rdx
	jmp	.L1796
	.p2align 4,,10
	.p2align 3
.L1438:
	xorl	%esi, %esi
	xorl	%edi, %edi
	cmpq	$1, %rbp
	je	.L1456
	movq	%r9, %rsi
	xorl	%ecx, %ecx
	xorl	%ebx, %ebx
	cmpq	%rbp, %r9
	jb	.L1457
	movq	128(%rsp), %rax
	xorl	%ecx, %ecx
	mulq	%r9
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %rsi
	cmpq	%rbp, %rsi
	movq	%rsi, %rax
	setnb	%cl
	addq	%rdx, %rcx
	subq	%rbp, %rax
	cmpq	%rbp, %rsi
	cmovnb	%rax, %rsi
	movq	%rcx, %rbx
	xorl	%ecx, %ecx
.L1457:
	movq	(%rsp), %rax
	movq	%rsi, %rdx
	xorl	%edi, %edi
	xorl	%r8d, %r8d
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rdx, %rsi
	xorl	%edx, %edx
	orq	%rcx, %rax
	orq	%rbx, %rdx
	movq	%rax, (%rsp)
	movq	%rdx, 8(%rsp)
	movq	%rdx, %r9
.L1456:
	movq	(%rsp), %rcx
	xorl	%r13d, %r13d
	imulq	%r14, %r8
	movq	%r15, %rax
	imulq	%r14, %r13
	imulq	%rcx, %rax
	addq	%rax, %r13
	movq	%rcx, %rax
	mulq	%r14
	movq	%rax, %rcx
	movq	%r15, %rax
	addq	%rdx, %r13
	imulq	%r9, %rax
	addq	%rax, %r8
	movq	%r9, %rax
	xorl	%r9d, %r9d
	mulq	%r14
	addq	%rdx, %r8
	addq	%r13, %rax
	movq	%r8, %rdx
	adcq	%r9, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %r13
	movq	%rdx, %r8
	orq	%r9, %r13
	jne	.L1459
	movq	%rax, %rdx
	xorl	%ebx, %ebx
	xorl	%eax, %eax
	orq	%rdx, %rbx
	orq	%rax, %rcx
	movq	152(%rsp), %rdx
	movq	144(%rsp), %rax
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jnc	.L1828
.L1459:
	movb	$0, 688(%rsp)
	movb	$0, 704(%rsp)
	movb	$0, 168(%rsp)
.L1455:
	movq	16(%r12), %r10
	movq	24(%r12), %r11
	movabsq	$9223372036854775807, %rcx
	movq	56(%rsp), %rsi
	movq	%r10, %rdx
	movq	%r11, %rax
	negq	%rdx
	movq	%rsi, 56(%rsp)
	adcq	$0, %rax
	negq	%rax
	cmovs	%r11, %rax
	cmovs	%r10, %rdx
	xorl	%r8d, %r8d
	movq	%rax, 56(%rsp)
	movq	%r11, %rax
	movq	56(%rsp), %rsi
	shrq	$63, %rax
	xorb	80(%rsp), %al
	movq	%rdx, 48(%rsp)
	movq	$-1, %rdx
	movzbl	%al, %eax
	movq	48(%rsp), %rbx
	movq	%rsi, %r9
	addq	%rax, %rdx
	movq	%rsi, %rax
	adcq	$0, %rcx
	orq	%r8, %rax
	movq	%rbx, 48(%rsp)
	movq	48(%rsp), %rbx
	movq	%rdx, 144(%rsp)
	movq	%rcx, 152(%rsp)
	jne	.L1481
	movq	%rbx, %rax
	imulq	%r14, %rsi
	imulq	%r15, %rax
	addq	%rax, %rsi
	movq	%rbx, %rax
	mulq	%r14
	addq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%rsi, %rbx
	cmpq	$1, %rbp
	je	.L1829
	movq	%rsi, %rdi
	movq	%rax, %rsi
	testq	%rbx, %rbx
	jne	.L1485
	movq	128(%rsp), %rax
	xorl	%ebx, %ebx
	mulq	%rcx
	movq	%rdx, %rax
	movq	%rdx, %rcx
	imulq	%rbp, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%rbp, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%rbp, %rax
	cmpq	%rbp, %rsi
	cmovnb	%rax, %rsi
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rsi
	je	.L1508
	addq	%rax, %rax
	adcq	%rdx, %rdx
	jmp	.L1799
	.p2align 4,,10
	.p2align 3
.L1481:
	xorl	%esi, %esi
	xorl	%edi, %edi
	cmpq	$1, %rbp
	je	.L1498
	movq	%r9, %rsi
	xorl	%ecx, %ecx
	xorl	%ebx, %ebx
	cmpq	%rbp, %r9
	jb	.L1499
	movq	128(%rsp), %rax
	xorl	%ecx, %ecx
	mulq	%r9
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %rsi
	cmpq	%rbp, %rsi
	movq	%rsi, %rax
	setnb	%cl
	addq	%rdx, %rcx
	subq	%rbp, %rax
	cmpq	%rbp, %rsi
	cmovnb	%rax, %rsi
	movq	%rcx, %rbx
	xorl	%ecx, %ecx
.L1499:
	movq	48(%rsp), %rax
	movq	%rsi, %rdx
	xorl	%edi, %edi
	xorl	%r8d, %r8d
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rdx, %rsi
	xorl	%edx, %edx
	orq	%rcx, %rax
	orq	%rbx, %rdx
	movq	%rax, 48(%rsp)
	movq	%rdx, 56(%rsp)
	movq	%rdx, %r9
.L1498:
	movq	48(%rsp), %rcx
	xorl	%edx, %edx
	imulq	%r14, %r8
	movq	%r15, %rax
	imulq	%r14, %rdx
	imulq	%rcx, %rax
	leaq	(%rdx,%rax), %r13
	movq	%rcx, %rax
	mulq	%r14
	movq	%rax, %rcx
	leaq	0(%r13,%rdx), %rax
	movq	%rax, %rbx
	movq	%r15, %rax
	imulq	%r9, %rax
	addq	%rax, %r8
	movq	%r9, %rax
	xorl	%r9d, %r9d
	mulq	%r14
	addq	%rdx, %r8
	addq	%rbx, %rax
	movq	%r8, %rdx
	adcq	%r9, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %r13
	movq	%rdx, %r8
	orq	%r9, %r13
	jne	.L1519
	movq	%rax, %rdx
	xorl	%ebx, %ebx
	xorl	%eax, %eax
	orq	%rdx, %rbx
	orq	%rax, %rcx
	movq	152(%rsp), %rdx
	movq	144(%rsp), %rax
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jnc	.L1830
.L1519:
	movb	$0, 712(%rsp)
	movb	$0, 728(%rsp)
.L1497:
	movq	32(%r12), %r10
	movq	40(%r12), %r11
	movabsq	$9223372036854775807, %rcx
	movq	40(%rsp), %rsi
	movq	%r10, %rdx
	movq	%r11, %rax
	negq	%rdx
	movq	%rsi, 40(%rsp)
	adcq	$0, %rax
	negq	%rax
	cmovs	%r11, %rax
	cmovs	%r10, %rdx
	xorl	%r8d, %r8d
	movq	%rax, 40(%rsp)
	movq	%r11, %rax
	movq	40(%rsp), %rsi
	shrq	$63, %rax
	xorb	80(%rsp), %al
	movq	%rdx, 32(%rsp)
	movq	$-1, %rdx
	movzbl	%al, %eax
	movq	32(%rsp), %rbx
	movq	%rsi, %r9
	addq	%rax, %rdx
	movq	%rsi, %rax
	adcq	$0, %rcx
	orq	%r8, %rax
	movq	%rbx, 32(%rsp)
	movq	32(%rsp), %rbx
	movq	%rdx, 144(%rsp)
	movq	%rcx, 152(%rsp)
	jne	.L1521
	movq	%rbx, %rax
	imulq	%r14, %rsi
	imulq	%r15, %rax
	addq	%rax, %rsi
	movq	%rbx, %rax
	mulq	%r14
	addq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%rsi, %rbx
	cmpq	$1, %rbp
	je	.L1831
	movq	%rsi, %rdi
	movq	%rax, %rsi
	testq	%rbx, %rbx
	jne	.L1525
	movq	128(%rsp), %rax
	xorl	%ebx, %ebx
	mulq	%rcx
	movq	%rdx, %rax
	movq	%rdx, %rcx
	imulq	%rbp, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%rbp, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%rbp, %rax
	cmpq	%rbp, %rsi
	cmovnb	%rax, %rsi
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rsi
	je	.L1548
	addq	%rax, %rax
	adcq	%rdx, %rdx
	jmp	.L1801
	.p2align 4,,10
	.p2align 3
.L1521:
	xorl	%esi, %esi
	xorl	%edi, %edi
	cmpq	$1, %rbp
	je	.L1538
	movq	%r9, %rsi
	xorl	%ecx, %ecx
	xorl	%ebx, %ebx
	cmpq	%rbp, %r9
	jb	.L1539
	movq	128(%rsp), %rax
	xorl	%ecx, %ecx
	mulq	%r9
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %rsi
	cmpq	%rbp, %rsi
	movq	%rsi, %rax
	setnb	%cl
	addq	%rdx, %rcx
	subq	%rbp, %rax
	cmpq	%rbp, %rsi
	cmovnb	%rax, %rsi
	movq	%rcx, %rbx
	xorl	%ecx, %ecx
.L1539:
	movq	32(%rsp), %rax
	movq	%rsi, %rdx
	xorl	%edi, %edi
	xorl	%r8d, %r8d
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rdx, %rsi
	xorl	%edx, %edx
	orq	%rcx, %rax
	orq	%rbx, %rdx
	movq	%rax, 32(%rsp)
	movq	%rdx, 40(%rsp)
	movq	%rdx, %r9
.L1538:
	movq	32(%rsp), %rcx
	xorl	%edx, %edx
	imulq	%r14, %r8
	movq	%r15, %rax
	imulq	%r14, %rdx
	imulq	%rcx, %rax
	leaq	(%rdx,%rax), %r13
	movq	%rcx, %rax
	mulq	%r14
	movq	%rax, %rcx
	leaq	0(%r13,%rdx), %rax
	movq	%rax, %rbx
	movq	%r15, %rax
	imulq	%r9, %rax
	addq	%rax, %r8
	movq	%r9, %rax
	xorl	%r9d, %r9d
	mulq	%r14
	addq	%rdx, %r8
	addq	%rbx, %rax
	movq	%r8, %rdx
	adcq	%r9, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %r13
	movq	%rdx, %r8
	orq	%r9, %r13
	jne	.L1559
	movq	%rax, %rdx
	xorl	%ebx, %ebx
	xorl	%eax, %eax
	orq	%rdx, %rbx
	orq	%rax, %rcx
	movq	152(%rsp), %rdx
	movq	144(%rsp), %rax
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jnc	.L1832
.L1559:
	movb	$0, 736(%rsp)
	movb	$0, 752(%rsp)
.L1537:
	movq	48(%r12), %r10
	movq	56(%r12), %r11
	movabsq	$9223372036854775807, %rcx
	movq	24(%rsp), %rsi
	movq	%r10, %rdx
	movq	%r11, %rax
	negq	%rdx
	movq	%rsi, 24(%rsp)
	adcq	$0, %rax
	negq	%rax
	cmovs	%r11, %rax
	cmovs	%r10, %rdx
	xorl	%r8d, %r8d
	movq	%rax, 24(%rsp)
	movq	%r11, %rax
	movq	24(%rsp), %rsi
	shrq	$63, %rax
	xorb	80(%rsp), %al
	movq	%rdx, 16(%rsp)
	movq	$-1, %rdx
	movzbl	%al, %eax
	movq	16(%rsp), %rbx
	movq	%rsi, %r9
	addq	%rax, %rdx
	movq	%rsi, %rax
	adcq	$0, %rcx
	orq	%r8, %rax
	movq	%rbx, 16(%rsp)
	movq	16(%rsp), %rbx
	movq	%rdx, 144(%rsp)
	movq	%rcx, 152(%rsp)
	jne	.L1561
	movq	%rbx, %rax
	imulq	%r14, %rsi
	imulq	%r15, %rax
	addq	%rax, %rsi
	movq	%rbx, %rax
	mulq	%r14
	addq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%rsi, %rbx
	cmpq	$1, %rbp
	je	.L1833
	movq	%rsi, %rdi
	movq	%rax, %rsi
	testq	%rbx, %rbx
	jne	.L1565
	movq	128(%rsp), %rax
	xorl	%ebx, %ebx
	mulq	%rcx
	movq	%rdx, %rax
	movq	%rdx, %rcx
	imulq	%rbp, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%rbp, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%rbp, %rax
	cmpq	%rbp, %rsi
	cmovnb	%rax, %rsi
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rsi
	je	.L1588
	addq	%rax, %rax
	adcq	%rdx, %rdx
	jmp	.L1803
	.p2align 4,,10
	.p2align 3
.L1561:
	xorl	%esi, %esi
	xorl	%edi, %edi
	cmpq	$1, %rbp
	je	.L1578
	movq	%r9, %rsi
	xorl	%ecx, %ecx
	xorl	%ebx, %ebx
	cmpq	%rbp, %r9
	jb	.L1579
	movq	128(%rsp), %rax
	xorl	%ecx, %ecx
	mulq	%r9
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %rsi
	cmpq	%rbp, %rsi
	movq	%rsi, %rax
	setnb	%cl
	addq	%rdx, %rcx
	subq	%rbp, %rax
	cmpq	%rbp, %rsi
	cmovnb	%rax, %rsi
	movq	%rcx, %rbx
	xorl	%ecx, %ecx
.L1579:
	movq	16(%rsp), %rax
	movq	%rsi, %rdx
	xorl	%edi, %edi
	xorl	%r8d, %r8d
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rdx, %rsi
	xorl	%edx, %edx
	orq	%rcx, %rax
	orq	%rbx, %rdx
	movq	%rax, 16(%rsp)
	movq	%rdx, 24(%rsp)
	movq	%rdx, %r9
.L1578:
	movq	16(%rsp), %rcx
	xorl	%edx, %edx
	imulq	%r14, %r8
	movq	%r15, %rax
	imulq	%r14, %rdx
	imulq	%rcx, %rax
	leaq	(%rdx,%rax), %r13
	movq	%rcx, %rax
	mulq	%r14
	movq	%rax, %rcx
	leaq	0(%r13,%rdx), %rax
	movq	%rax, %rbx
	movq	%r15, %rax
	imulq	%r9, %rax
	addq	%rax, %r8
	movq	%r9, %rax
	xorl	%r9d, %r9d
	mulq	%r14
	addq	%rdx, %r8
	addq	%rbx, %rax
	movq	%r8, %rdx
	adcq	%r9, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %r13
	movq	%rdx, %r8
	orq	%r9, %r13
	jne	.L1599
	movq	%rax, %rdx
	xorl	%ebx, %ebx
	xorl	%eax, %eax
	orq	%rdx, %rbx
	orq	%rax, %rcx
	movq	152(%rsp), %rdx
	movq	144(%rsp), %rax
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jnc	.L1834
.L1599:
	cmpb	$0, 168(%rsp)
	movb	$0, 760(%rsp)
	movb	$0, 776(%rsp)
	je	.L1786
.L1842:
	movdqa	688(%rsp), %xmm0
	movq	72(%rsp), %rax
	cmpb	$0, 728(%rsp)
	movups	%xmm0, (%rax)
	je	.L1835
	movdqu	712(%rsp), %xmm0
	cmpb	$0, 752(%rsp)
	movups	%xmm0, 16(%rax)
	je	.L1836
	movdqa	736(%rsp), %xmm0
	cmpb	$0, 776(%rsp)
	movups	%xmm0, 32(%rax)
	je	.L1837
	addq	$64, %rax
	addq	$4, 96(%rsp)
	movq	96(%rsp), %rdi
	addq	$64, %r12
	movdqu	760(%rsp), %xmm0
	movups	%xmm0, -16(%rax)
	movq	%rax, 72(%rsp)
	movq	160(%rsp), %rax
	subq	%rdi, %rax
	cmpq	$3, %rax
	ja	.L1604
	movq	192(%rsp), %r12
	movq	200(%rsp), %r11
	movq	208(%rsp), %r10
.L1113:
	movq	160(%rsp), %rdi
	cmpq	%rdi, 96(%rsp)
	jnb	.L1609
	movq	96(%rsp), %r13
	movq	%r12, (%rsp)
	movq	%r11, %r14
	leaq	256(%rsp), %rbp
	leaq	304(%rsp), %rbx
	movq	%r13, %rax
	salq	$4, %rax
	leaq	(%r10,%rax), %r15
	movq	%r15, %r12
	movq	160(%rsp), %r15
	jmp	.L1608
	.p2align 4,,10
	.p2align 3
.L1607:
	movdqa	256(%rsp), %xmm0
	movq	%r13, %rdx
	addq	$1, %r13
	addq	$16, %r12
	salq	$4, %rdx
	movups	%xmm0, (%r14,%rdx)
	cmpq	%r13, %r15
	je	.L1838
.L1608:
	movq	(%r12), %rdx
	movq	8(%r12), %rcx
	movq	%rbx, %rsi
	movq	%rbp, %rdi
	call	_ZNK9fixedwide6detail5batch20broadcast_multiplierINS_11basic_fixedILm128ELj12EEES4_EclIS4_EESt8expectedIS4_NS_15ArithmeticErrorEET_
	cmpb	$0, 272(%rsp)
	jne	.L1607
	movq	(%rsp), %r12
	movzbl	256(%rsp), %eax
	movq	%r13, (%r12)
	movb	%al, 8(%r12)
	movb	$0, 16(%r12)
	.p2align 4
	.p2align 3
.L1106:
	movq	792(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L1839
	addq	$808, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%r12, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L1110:
	.cfi_restore_state
	movq	168(%rsp), %rax
	xorl	%edx, %edx
	divq	%rsi
	movq	%rax, %r14
	movq	144(%rsp), %rax
	movq	%r14, %r15
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rsi
# 0 "" 2
#NO_APP
	movq	%rax, %r14
	jmp	.L1111
	.p2align 4,,10
	.p2align 3
.L1107:
	movabsq	$1000000000000, %rdi
	mulq	%rdx
	movq	%rbx, %rax
	shrq	$37, %rdx
	movq	%rdx, %rsi
	imulq	%rdi, %rsi
	subq	%rsi, %rax
	movq	%rax, %rdx
	movq	144(%rsp), %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rdi
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	jmp	.L1108
	.p2align 4,,10
	.p2align 3
.L1834:
	movq	%rsi, %rax
	orq	%rdi, %rax
	je	.L1588
	movq	%rdi, %rdx
	movq	%r15, %rax
	imulq	%rsi, %rax
	imulq	%r14, %rdx
	leaq	(%rdx,%rax), %r13
	movq	%rsi, %rax
	mulq	%r14
	movq	%rax, %rsi
	leaq	0(%r13,%rdx), %rax
	movq	%rax, %rdi
	cmpq	$1, %rbp
	je	.L1840
	movq	%rax, 216(%rsp)
	movq	%rsi, %r13
	testq	%rax, %rax
	je	.L1841
	cmpq	%rbp, %rax
	jb	.L1592
	mulq	128(%rsp)
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, 216(%rsp)
	xorl	%eax, %eax
	movq	216(%rsp), %rdi
	cmpq	%rbp, %rdi
	setnb	%al
	movq	%rax, %r8
	movq	%rdi, %rax
	addq	%rdx, %r8
	subq	%rbp, %rax
	cmpq	%rbp, %rdi
	cmovb	%rdi, %rax
	movq	%r8, %r9
	xorl	%r8d, %r8d
	movq	%rax, 216(%rsp)
.L1592:
	xorl	%edi, %edi
	movq	%r13, %rax
	movq	216(%rsp), %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	orq	%r9, %rdi
	xorl	%r9d, %r9d
	orq	%r8, %rsi
	movq	%rdx, %r8
.L1591:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1599
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	addq	%rsi, %rcx
	adcq	%rdi, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1599
	movq	%r8, %rax
	orq	%r9, %rax
	je	.L1588
	movq	%r8, %rsi
	xorl	%edi, %edi
	addq	%r8, %rsi
	adcq	%rdi, %rdi
	xorl	%r9d, %r9d
	cmpq	%rsi, %rbp
	movq	%r9, %rax
	sbbq	%rdi, %rax
	jc	.L1596
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%rbp, %rax
	xorq	%r9, %rdx
	orq	%rdx, %rax
	jne	.L1588
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1596
	testq	%rax, %rax
	je	.L1588
.L1596:
	movq	144(%rsp), %rsi
	movq	152(%rsp), %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%rcx, %rax
	xorq	%rbx, %rdx
	orq	%rdx, %rax
	je	.L1599
.L1822:
	addq	$1, %rcx
	adcq	$0, %rbx
	.p2align 4
	.p2align 3
.L1588:
	xorq	120(%rsp), %r11
	jns	.L1600
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1600:
	cmpb	$0, 168(%rsp)
	movq	%rcx, 760(%rsp)
	movq	%rbx, 768(%rsp)
	movb	$1, 776(%rsp)
	jne	.L1842
.L1786:
	movq	176(%rsp), %r9
	movq	192(%rsp), %r12
	.p2align 4
	.p2align 3
.L1290:
	leaq	(%r9,%r9,2), %rax
	movq	96(%rsp), %rdi
	movzbl	688(%rsp,%rax,8), %eax
	movb	$0, 16(%r12)
	movq	%rdi, (%r12)
	movb	%al, 8(%r12)
	jmp	.L1106
	.p2align 4,,10
	.p2align 3
.L1832:
	movq	%rsi, %rax
	orq	%rdi, %rax
	je	.L1548
	movq	%rdi, %rdx
	movq	%r15, %rax
	imulq	%rsi, %rax
	imulq	%r14, %rdx
	leaq	(%rdx,%rax), %r13
	movq	%rsi, %rax
	mulq	%r14
	movq	%rax, %rsi
	leaq	0(%r13,%rdx), %rax
	movq	%rax, %rdi
	cmpq	$1, %rbp
	je	.L1843
	movq	%rsi, 216(%rsp)
	movq	%rax, %r13
	testq	%rax, %rax
	je	.L1844
	cmpq	%rbp, %rax
	jb	.L1552
	movq	128(%rsp), %rax
	mulq	%rdi
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %r13
	xorl	%eax, %eax
	cmpq	%rbp, %r13
	setnb	%al
	movq	%rax, %r8
	movq	%r13, %rax
	addq	%rdx, %r8
	subq	%rbp, %rax
	cmpq	%rbp, %r13
	cmovnb	%rax, %r13
	movq	%r8, %r9
	xorl	%r8d, %r8d
.L1552:
	xorl	%edi, %edi
	movq	216(%rsp), %rax
	movq	%r13, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	orq	%r9, %rdi
	xorl	%r9d, %r9d
	orq	%r8, %rsi
	movq	%rdx, %r8
.L1551:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1559
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	addq	%rsi, %rcx
	adcq	%rdi, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1559
	movq	%r8, %rax
	orq	%r9, %rax
	je	.L1548
	movq	%r8, %rsi
	xorl	%edi, %edi
	addq	%r8, %rsi
	adcq	%rdi, %rdi
	xorl	%r9d, %r9d
	cmpq	%rsi, %rbp
	movq	%r9, %rax
	sbbq	%rdi, %rax
	jc	.L1556
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%rbp, %rax
	xorq	%r9, %rdx
	orq	%rdx, %rax
	jne	.L1548
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1556
	testq	%rax, %rax
	je	.L1548
.L1556:
	movq	144(%rsp), %rsi
	movq	152(%rsp), %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%rcx, %rax
	xorq	%rbx, %rdx
	orq	%rdx, %rax
	je	.L1559
.L1821:
	addq	$1, %rcx
	adcq	$0, %rbx
	.p2align 4
	.p2align 3
.L1548:
	xorq	120(%rsp), %r11
	jns	.L1560
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1560:
	movq	%rcx, 736(%rsp)
	movq	%rbx, 744(%rsp)
	movb	$1, 752(%rsp)
	jmp	.L1537
	.p2align 4,,10
	.p2align 3
.L1830:
	movq	%rsi, %rax
	orq	%rdi, %rax
	je	.L1508
	movq	%rdi, %rax
	movq	%r15, %rdx
	imulq	%rsi, %rdx
	imulq	%r14, %rax
	leaq	(%rdx,%rax), %r13
	movq	%r14, %rax
	mulq	%rsi
	movq	%rax, %rsi
	leaq	0(%r13,%rdx), %rax
	movq	%rax, %rdi
	cmpq	$1, %rbp
	je	.L1845
	movq	%rsi, 216(%rsp)
	movq	%rax, %r13
	testq	%rax, %rax
	je	.L1846
	cmpq	%rbp, %rax
	jb	.L1512
	movq	128(%rsp), %rax
	mulq	%rdi
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %r13
	xorl	%eax, %eax
	cmpq	%rbp, %r13
	setnb	%al
	movq	%rax, %r8
	movq	%r13, %rax
	addq	%rdx, %r8
	subq	%rbp, %rax
	cmpq	%rbp, %r13
	cmovnb	%rax, %r13
	movq	%r8, %r9
	xorl	%r8d, %r8d
.L1512:
	xorl	%edi, %edi
	movq	216(%rsp), %rax
	movq	%r13, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	orq	%r9, %rdi
	xorl	%r9d, %r9d
	orq	%r8, %rsi
	movq	%rdx, %r8
.L1511:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1519
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	addq	%rsi, %rcx
	adcq	%rdi, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1519
	movq	%r8, %rax
	orq	%r9, %rax
	je	.L1508
	movq	%r8, %rsi
	xorl	%edi, %edi
	addq	%r8, %rsi
	adcq	%rdi, %rdi
	xorl	%r9d, %r9d
	cmpq	%rsi, %rbp
	movq	%r9, %rax
	sbbq	%rdi, %rax
	jc	.L1516
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%rbp, %rax
	xorq	%r9, %rdx
	orq	%rdx, %rax
	jne	.L1508
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1516
	testq	%rax, %rax
	je	.L1508
.L1516:
	movq	144(%rsp), %rsi
	movq	152(%rsp), %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%rcx, %rax
	xorq	%rbx, %rdx
	orq	%rdx, %rax
	je	.L1519
.L1820:
	addq	$1, %rcx
	adcq	$0, %rbx
	.p2align 4
	.p2align 3
.L1508:
	xorq	120(%rsp), %r11
	jns	.L1520
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1520:
	movq	%rcx, 712(%rsp)
	movq	%rbx, 720(%rsp)
	movb	$1, 728(%rsp)
	jmp	.L1497
	.p2align 4,,10
	.p2align 3
.L1828:
	movq	%rsi, %rax
	orq	%rdi, %rax
	je	.L1460
	movq	%rdi, %r13
	movq	%r15, %rax
	imulq	%rsi, %rax
	imulq	%r14, %r13
	addq	%rax, %r13
	movq	%rsi, %rax
	mulq	%r14
	movq	%rax, %rsi
	addq	%rdx, %r13
	cmpq	$1, %rbp
	je	.L1847
	movq	%rax, 168(%rsp)
	testq	%r13, %r13
	je	.L1848
	cmpq	%rbp, %r13
	jb	.L1470
	movq	128(%rsp), %rax
	mulq	%r13
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %r13
	xorl	%eax, %eax
	cmpq	%rbp, %r13
	setnb	%al
	movq	%rax, %r8
	movq	%r13, %rax
	addq	%rdx, %r8
	subq	%rbp, %rax
	cmpq	%rbp, %r13
	cmovnb	%rax, %r13
	movq	%r8, %r9
	xorl	%r8d, %r8d
.L1470:
	xorl	%edi, %edi
	movq	168(%rsp), %rax
	movq	%r13, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	orq	%r9, %rdi
	xorl	%r9d, %r9d
	orq	%r8, %rsi
	movq	%rdx, %r8
.L1469:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1459
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	addq	%rsi, %rcx
	adcq	%rdi, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1459
	movq	%r8, %rax
	orq	%r9, %rax
	je	.L1460
	movq	%r8, %rsi
	xorl	%edi, %edi
	addq	%r8, %rsi
	adcq	%rdi, %rdi
	xorl	%r9d, %r9d
	cmpq	%rsi, %rbp
	movq	%r9, %rax
	sbbq	%rdi, %rax
	jc	.L1475
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%rbp, %rax
	xorq	%r9, %rdx
	orq	%rdx, %rax
	jne	.L1460
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1475
	testq	%rax, %rax
	je	.L1460
.L1475:
	movq	144(%rsp), %rsi
	movq	152(%rsp), %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%rcx, %rax
	xorq	%rbx, %rdx
	orq	%rdx, %rax
	je	.L1459
.L1819:
	addq	$1, %rcx
	adcq	$0, %rbx
	.p2align 4
	.p2align 3
.L1460:
	xorq	120(%rsp), %r11
	jns	.L1462
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1462:
	movq	%rcx, 688(%rsp)
	movq	%rbx, 696(%rsp)
	movb	$1, 704(%rsp)
	movb	$1, 168(%rsp)
	jmp	.L1455
	.p2align 4,,10
	.p2align 3
.L1826:
	movq	%rbx, %rbp
	orq	%r13, %rbp
	je	.L1115
	movq	%r9, 32(%rsp)
	movq	%r10, %r14
	xorl	%r13d, %r13d
	movq	%r11, 48(%rsp)
	movq	%r10, 80(%rsp)
	movq	%r12, (%rsp)
	movq	%r11, %r12
.L1294:
	movdqu	(%r14), %xmm0
	movq	%xmm0, %rcx
	movhlps	%xmm0, %xmm3
	movq	%xmm3, %r8
	movq	%rcx, %rdi
	negq	%rdi
	movq	%r8, %rsi
	movq	%r8, %rbx
	adcq	$0, %rsi
	negq	%rsi
	cmovs	%r8, %rsi
	cmovs	%rcx, %rdi
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rax
	jne	.L1116
	movq	168(%rsp), %rax
	xorl	%r11d, %r11d
	movabsq	$2535301200456458803, %r15
	movabsq	$1000000000000, %rbp
	mulq	%r15
	movq	%rdx, %r10
	movq	168(%rsp), %rdx
	shrq	$37, %r10
	movq	%r10, %rax
	movq	%r10, 24(%rsp)
	imulq	%rbp, %rax
	subq	%rax, %rdx
	movq	72(%rsp), %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, 16(%rsp)
	movq	16(%rsp), %r10
	movq	%rsi, %rax
	movq	%rdx, 128(%rsp)
	movq	%r10, %r8
	movq	%r11, %r10
	xorl	%r11d, %r11d
	imulq	%r8, %rax
	imulq	%rdi, %r10
	addq	%rax, %r10
	movq	%r8, %rax
	mulq	%rdi
	addq	%rdx, %r10
	movq	%rax, %r8
	movq	24(%rsp), %rax
	movq	%r10, %r9
	movq	%rsi, %r10
	imulq	%rax, %r10
	mulq	%rdi
	addq	%rdx, %r10
	movq	%r10, %rdx
	movq	%r9, %r10
	addq	%rax, %r10
	adcq	%rdx, %r11
	xorl	%edx, %edx
	movq	%r11, %rax
	orq	%rdx, %rax
	jne	.L1788
	xorq	120(%rsp), %rbx
	xorq	112(%rsp), %rcx
	movq	%r10, %r11
	movq	$-1, %rax
	movabsq	$9223372036854775807, %rdx
	movq	%rcx, 16(%rsp)
	movq	%rbx, %rcx
	shrq	$63, %rcx
	movq	%rbx, 24(%rsp)
	xorl	%ebx, %ebx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	xorl	%r10d, %r10d
	movq	%rcx, %rax
	xorl	%r9d, %r9d
	movq	%rcx, 144(%rsp)
	movq	%r10, %rcx
	movq	%rbx, %rdx
	movq	%rbx, 152(%rsp)
	orq	%r8, %rcx
	movq	%r11, %rbx
	orq	%r9, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1788
	cmpq	$0, 128(%rsp)
	jne	.L1118
	cmpq	$0, 24(%rsp)
	jns	.L1119
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1119:
	movq	%rcx, 688(%rsp)
	movq	%rbx, 696(%rsp)
	movb	$1, 704(%rsp)
.L1133:
	movl	$1, %r15d
.L1134:
	movdqu	16(%r14), %xmm0
	movq	%xmm0, %rcx
	movhlps	%xmm0, %xmm4
	movq	%xmm4, %r8
	movq	%rcx, %rdi
	negq	%rdi
	movq	%r8, %rsi
	movq	%r8, %rbx
	adcq	$0, %rsi
	negq	%rsi
	cmovs	%r8, %rsi
	cmovs	%rcx, %rdi
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rax
	jne	.L1161
	movq	168(%rsp), %r11
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %rbp
	mulq	%r11
	movq	%rdx, %r10
	movq	%r11, %rdx
	xorl	%r11d, %r11d
	shrq	$37, %r10
	movq	%r10, %rax
	movq	%r10, 24(%rsp)
	imulq	%rbp, %rax
	subq	%rax, %rdx
	movq	72(%rsp), %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, 16(%rsp)
	movq	16(%rsp), %r10
	movq	%rsi, %rax
	movq	%rdx, 128(%rsp)
	movq	%r10, %r8
	movq	%r11, %r10
	xorl	%r11d, %r11d
	imulq	%r8, %rax
	imulq	%rdi, %r10
	addq	%rax, %r10
	movq	%r8, %rax
	mulq	%rdi
	addq	%rdx, %r10
	movq	%rax, %r8
	movq	24(%rsp), %rax
	movq	%r10, %r9
	movq	%rsi, %r10
	imulq	%rax, %r10
	mulq	%rdi
	addq	%rdx, %r10
	movq	%r10, %rdx
	movq	%r9, %r10
	addq	%rax, %r10
	adcq	%rdx, %r11
	xorl	%edx, %edx
	movq	%r11, %rax
	orq	%rdx, %rax
	jne	.L1162
	xorq	120(%rsp), %rbx
	xorq	112(%rsp), %rcx
	movq	%r10, %r11
	movq	$-1, %rax
	movabsq	$9223372036854775807, %rdx
	movq	%rcx, 16(%rsp)
	movq	%rbx, %rcx
	shrq	$63, %rcx
	movq	%rbx, 24(%rsp)
	xorl	%ebx, %ebx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	xorl	%r10d, %r10d
	movq	%rcx, %rax
	xorl	%r9d, %r9d
	movq	%rcx, 144(%rsp)
	movq	%r10, %rcx
	movq	%rbx, %rdx
	movq	%rbx, 152(%rsp)
	orq	%r8, %rcx
	movq	%r11, %rbx
	orq	%r9, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1162
	cmpq	$0, 128(%rsp)
	jne	.L1163
	cmpq	$0, 24(%rsp)
	jns	.L1164
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1164:
	movq	%rcx, 712(%rsp)
	movq	%rbx, 720(%rsp)
	movb	$1, 728(%rsp)
.L1177:
	movdqu	32(%r14), %xmm0
	movq	%xmm0, %rcx
	movhlps	%xmm0, %xmm5
	movq	%xmm5, %r8
	movq	%rcx, %rdi
	negq	%rdi
	movq	%r8, %rsi
	movq	%r8, %rbx
	adcq	$0, %rsi
	negq	%rsi
	cmovs	%r8, %rsi
	cmovs	%rcx, %rdi
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rax
	jne	.L1204
	movq	168(%rsp), %r11
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %rbp
	mulq	%r11
	movq	%rdx, %r10
	movq	%r11, %rdx
	xorl	%r11d, %r11d
	shrq	$37, %r10
	movq	%r10, %rax
	movq	%r10, 24(%rsp)
	imulq	%rbp, %rax
	subq	%rax, %rdx
	movq	72(%rsp), %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, 16(%rsp)
	movq	16(%rsp), %r10
	movq	%rsi, %rax
	movq	%rdx, 128(%rsp)
	movq	%r10, %r8
	movq	%r11, %r10
	xorl	%r11d, %r11d
	imulq	%r8, %rax
	imulq	%rdi, %r10
	addq	%rax, %r10
	movq	%r8, %rax
	mulq	%rdi
	addq	%rdx, %r10
	movq	%rax, %r8
	movq	24(%rsp), %rax
	movq	%r10, %r9
	movq	%rsi, %r10
	imulq	%rax, %r10
	mulq	%rdi
	addq	%rdx, %r10
	movq	%r10, %rdx
	movq	%r9, %r10
	addq	%rax, %r10
	adcq	%rdx, %r11
	xorl	%edx, %edx
	movq	%r11, %rax
	orq	%rdx, %rax
	jne	.L1205
	xorq	120(%rsp), %rbx
	xorq	112(%rsp), %rcx
	movq	%r10, %r11
	movq	$-1, %rax
	movabsq	$9223372036854775807, %rdx
	movq	%rcx, 16(%rsp)
	movq	%rbx, %rcx
	shrq	$63, %rcx
	movq	%rbx, 24(%rsp)
	xorl	%ebx, %ebx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	xorl	%r10d, %r10d
	movq	%rcx, %rax
	xorl	%r9d, %r9d
	movq	%rcx, 144(%rsp)
	movq	%r10, %rcx
	movq	%rbx, %rdx
	movq	%rbx, 152(%rsp)
	orq	%r8, %rcx
	movq	%r11, %rbx
	orq	%r9, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1205
	cmpq	$0, 128(%rsp)
	jne	.L1206
	cmpq	$0, 24(%rsp)
	jns	.L1207
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1207:
	movq	%rcx, 736(%rsp)
	movq	%rbx, 744(%rsp)
	movb	$1, 752(%rsp)
.L1220:
	movdqu	48(%r14), %xmm0
	movq	%xmm0, %rcx
	movhlps	%xmm0, %xmm6
	movq	%xmm6, %r8
	movq	%rcx, %rdi
	negq	%rdi
	movq	%r8, %rsi
	movq	%r8, %rbx
	adcq	$0, %rsi
	negq	%rsi
	cmovs	%r8, %rsi
	cmovs	%rcx, %rdi
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rax
	jne	.L1247
	movq	168(%rsp), %r11
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %rbp
	mulq	%r11
	movq	%rdx, %r10
	movq	%r11, %rdx
	xorl	%r11d, %r11d
	shrq	$37, %r10
	movq	%r10, %rax
	movq	%r10, 24(%rsp)
	imulq	%rbp, %rax
	subq	%rax, %rdx
	movq	72(%rsp), %rax
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, 16(%rsp)
	movq	16(%rsp), %r10
	movq	%rsi, %rax
	movq	%rdx, 128(%rsp)
	movq	%r10, %r8
	movq	%r11, %r10
	xorl	%r11d, %r11d
	imulq	%r8, %rax
	imulq	%rdi, %r10
	addq	%rax, %r10
	movq	%r8, %rax
	mulq	%rdi
	addq	%rdx, %r10
	movq	%rax, %r8
	movq	24(%rsp), %rax
	movq	%r10, %r9
	movq	%rsi, %r10
	imulq	%rax, %r10
	mulq	%rdi
	addq	%rdx, %r10
	movq	%r10, %rdx
	movq	%r9, %r10
	addq	%rax, %r10
	adcq	%rdx, %r11
	xorl	%edx, %edx
	movq	%r11, %rax
	orq	%rdx, %rax
	jne	.L1248
	xorq	120(%rsp), %rbx
	xorq	112(%rsp), %rcx
	movq	%r10, %r11
	movq	$-1, %rax
	movabsq	$9223372036854775807, %rdx
	movq	%rcx, 16(%rsp)
	movq	%rbx, %rcx
	shrq	$63, %rcx
	movq	%rbx, 24(%rsp)
	xorl	%ebx, %ebx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	xorl	%r10d, %r10d
	movq	%rcx, %rax
	xorl	%r9d, %r9d
	movq	%rcx, 144(%rsp)
	movq	%r10, %rcx
	movq	%rbx, %rdx
	movq	%rbx, 152(%rsp)
	orq	%r8, %rcx
	movq	%r11, %rbx
	orq	%r9, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1248
	cmpq	$0, 128(%rsp)
	jne	.L1249
	cmpq	$0, 24(%rsp)
	jns	.L1250
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1250:
	movq	%rcx, 760(%rsp)
	movq	%rbx, 768(%rsp)
	movb	$1, 776(%rsp)
.L1263:
	testb	%r15b, %r15b
	je	.L1785
	movdqa	688(%rsp), %xmm0
	cmpb	$0, 728(%rsp)
	movups	%xmm0, (%r12)
	je	.L1849
	movdqu	712(%rsp), %xmm0
	cmpb	$0, 752(%rsp)
	movups	%xmm0, 16(%r12)
	je	.L1850
	movdqa	736(%rsp), %xmm0
	cmpb	$0, 776(%rsp)
	movups	%xmm0, 32(%r12)
	je	.L1851
	movq	160(%rsp), %rax
	addq	$4, %r13
	addq	$64, %r14
	addq	$64, %r12
	movdqu	760(%rsp), %xmm0
	subq	%r13, %rax
	movups	%xmm0, -16(%r12)
	cmpq	$3, %rax
	ja	.L1294
	movq	(%rsp), %r12
	movq	48(%rsp), %r11
	movq	%r13, 96(%rsp)
	movq	80(%rsp), %r10
	jmp	.L1113
	.p2align 4,,10
	.p2align 3
.L1115:
	movq	%r9, 32(%rsp)
	xorl	%eax, %eax
	movq	120(%rsp), %r13
	movq	%r11, %rbp
	movq	%r10, 72(%rsp)
	movq	%r10, %r15
	movq	%r12, (%rsp)
	movq	112(%rsp), %r12
	movq	%r11, 48(%rsp)
	movq	%rax, %r11
.L1437:
	movq	(%r15), %rsi
	movq	8(%r15), %rdi
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	negq	%rcx
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rdi, %rdx
	cmovs	%rsi, %rcx
	xorl	%r8d, %r8d
	xorq	%r13, %rdi
	movq	%rdi, %r10
	movq	%rdx, %rsi
	orq	%r8, %rsi
	jne	.L1852
	movq	144(%rsp), %rax
	movq	192(%rsp), %rsi
	imulq	%rax, %rdx
	imulq	%rcx, %rsi
	addq	%rdx, %rsi
	mulq	%rcx
	addq	%rdx, %rsi
	movq	%rax, %rcx
	xorl	%edx, %edx
	movq	%rsi, %rax
	movq	%rsi, %rdi
	orq	%rdx, %rax
	je	.L1423
	movabsq	$2535301200456458803, %rax
	movq	%rdi, %rsi
	movabsq	$1000000000000, %r8
	mulq	%rdi
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movq	%rdi, 24(%rsp)
	imulq	%r8, %rax
	subq	%rax, %rsi
	movq	%rcx, %rax
	movq	%rsi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r8
# 0 "" 2
#NO_APP
	movq	%rax, 16(%rsp)
	movq	%rdx, %rax
	xorl	%edx, %edx
.L1433:
	movq	%rax, %rcx
	orq	%rdx, %rcx
	je	.L1430
	xorl	%edx, %edx
	movl	$244140625, %esi
	addq	%rax, %rax
	adcq	%rdx, %rdx
	movq	%rax, %rcx
	salq	$12, %rsi
	xorl	%eax, %eax
	cmpq	%rcx, %rsi
	sbbq	%rdx, %rax
	jc	.L1428
	testq	%rdx, %rdx
	je	.L1853
.L1430:
	testq	%r10, %r10
	jns	.L1427
	negq	16(%rsp)
	adcq	$0, 24(%rsp)
	negq	24(%rsp)
.L1427:
	movq	16(%rsp), %rax
	movq	24(%rsp), %rdx
	movb	$1, 704(%rsp)
	movq	%rax, 688(%rsp)
	movq	%rdx, 696(%rsp)
.L1421:
	movl	$1, %r14d
	jmp	.L1410
	.p2align 4,,10
	.p2align 3
.L1827:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rsi, %rax
	jc	.L1459
	jmp	.L1460
	.p2align 4,,10
	.p2align 3
.L1833:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rsi, %rax
	jc	.L1599
	jmp	.L1588
	.p2align 4,,10
	.p2align 3
.L1829:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rsi, %rax
	jc	.L1519
	jmp	.L1508
	.p2align 4,,10
	.p2align 3
.L1831:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rsi, %rax
	jc	.L1559
	jmp	.L1548
	.p2align 4,,10
	.p2align 3
.L1442:
	cmpq	%rbp, %rbx
	jnb	.L1854
	movq	%rbx, %rdx
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rax
	xorl	%edx, %edx
	movq	%rax, %rsi
	orq	%rdx, %rsi
	je	.L1460
	addq	%rax, %rax
	adcq	%rdx, %rdx
.L1796:
	xorl	%r9d, %r9d
	movq	%rax, %rsi
	cmpq	%rax, %rbp
	movq	%rdx, %rdi
	movq	%r9, %rax
	movq	%rbp, %r8
	sbbq	%rdx, %rax
	jc	.L1819
.L1445:
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%r8, %rax
	xorq	%r9, %rdx
	orq	%rdx, %rax
	jne	.L1460
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	orq	%rdx, %rax
	je	.L1460
	jmp	.L1475
	.p2align 4,,10
	.p2align 3
.L1565:
	cmpq	%rbp, %rbx
	jnb	.L1855
	movq	%rbx, %rdx
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rax
	xorl	%edx, %edx
	movq	%rax, %rsi
	orq	%rdx, %rsi
	je	.L1588
	addq	%rax, %rax
	adcq	%rdx, %rdx
.L1803:
	xorl	%r9d, %r9d
	movq	%rax, %rsi
	cmpq	%rax, %rbp
	movq	%rdx, %rdi
	movq	%r9, %rax
	movq	%rbp, %r8
	sbbq	%rdx, %rax
	jc	.L1822
.L1568:
	movq	%r8, %rax
	movq	%r9, %rdx
	xorq	%rsi, %rax
	xorq	%rdi, %rdx
	orq	%rdx, %rax
	jne	.L1588
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	orq	%rdx, %rax
	je	.L1588
	jmp	.L1596
	.p2align 4,,10
	.p2align 3
.L1525:
	cmpq	%rbp, %rbx
	jnb	.L1856
	movq	%rbx, %rdx
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rax
	xorl	%edx, %edx
	movq	%rax, %rsi
	orq	%rdx, %rsi
	je	.L1548
	addq	%rax, %rax
	adcq	%rdx, %rdx
.L1801:
	xorl	%r9d, %r9d
	movq	%rax, %rsi
	cmpq	%rax, %rbp
	movq	%rdx, %rdi
	movq	%r9, %rax
	movq	%rbp, %r8
	sbbq	%rdx, %rax
	jc	.L1821
.L1528:
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%r8, %rax
	xorq	%r9, %rdx
	orq	%rdx, %rax
	jne	.L1548
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	orq	%rdx, %rax
	je	.L1548
	jmp	.L1556
	.p2align 4,,10
	.p2align 3
.L1485:
	cmpq	%rbp, %rbx
	jnb	.L1857
	movq	%rbx, %rdx
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rax
	xorl	%edx, %edx
	movq	%rax, %rsi
	orq	%rdx, %rsi
	je	.L1508
	addq	%rax, %rax
	adcq	%rdx, %rdx
.L1799:
	xorl	%r9d, %r9d
	movq	%rax, %rsi
	cmpq	%rax, %rbp
	movq	%rdx, %rdi
	movq	%r9, %rax
	movq	%rbp, %r8
	sbbq	%rdx, %rax
	jc	.L1820
.L1488:
	movq	%rsi, %rax
	movq	%rdi, %rdx
	xorq	%r8, %rax
	xorq	%r9, %rdx
	orq	%rdx, %rax
	jne	.L1508
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	orq	%rdx, %rax
	je	.L1508
	jmp	.L1516
	.p2align 4,,10
	.p2align 3
.L1835:
	addq	$1, 96(%rsp)
	movq	192(%rsp), %r12
	movl	$1, %r9d
	jmp	.L1290
	.p2align 4,,10
	.p2align 3
.L1836:
	addq	$2, 96(%rsp)
	movq	192(%rsp), %r12
	movl	$2, %r9d
	jmp	.L1290
	.p2align 4,,10
	.p2align 3
.L1837:
	addq	$3, 96(%rsp)
	movq	192(%rsp), %r12
	movl	$3, %r9d
	jmp	.L1290
.L1857:
	movq	128(%rsp), %rax
	xorl	%ecx, %ecx
	mulq	%rbx
	movq	%rbp, %rax
	imulq	%rdx, %rax
	movq	%rdx, %r8
	subq	%rax, %rdi
	cmpq	%rbp, %rdi
	movq	%rdi, %rax
	movq	%rdi, %rdx
	setnb	%cl
	addq	%r8, %rcx
	subq	%rbp, %rax
	cmpq	%rbp, %rdi
	movq	152(%rsp), %rdi
	cmovnb	%rax, %rdx
	movq	%rsi, %rax
	movq	144(%rsp), %rsi
	movq	%rcx, %rbx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	cmpq	%rax, %rsi
	movq	%rax, %rcx
	movq	%rdi, %rax
	sbbq	%rbx, %rax
	jc	.L1519
	movq	%rdx, %rsi
	xorl	%edi, %edi
	testq	%rdx, %rdx
	je	.L1508
	addq	%rdx, %rsi
	movq	%rbp, %r8
	adcq	%rdi, %rdi
	xorl	%r9d, %r9d
	cmpq	%rsi, %rbp
	movq	%r9, %rax
	sbbq	%rdi, %rax
	jc	.L1516
	jmp	.L1488
	.p2align 4,,10
	.p2align 3
.L1854:
	movq	128(%rsp), %rax
	mulq	%rbx
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	xorl	%eax, %eax
	cmpq	%rbp, %rdi
	setnb	%al
	movq	%rax, %rcx
	movq	%rdi, %rax
	addq	%rdx, %rcx
	subq	%rbp, %rax
	cmpq	%rbp, %rdi
	cmovnb	%rax, %rdi
	movq	%rsi, %rax
	movq	144(%rsp), %rsi
	movq	%rcx, %rbx
	movq	%rdi, %rdx
	movq	152(%rsp), %rdi
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	cmpq	%rax, %rsi
	movq	%rax, %rcx
	movq	%rdi, %rax
	sbbq	%rbx, %rax
	jc	.L1459
	movq	%rdx, %rsi
	xorl	%edi, %edi
	testq	%rdx, %rdx
	je	.L1460
	addq	%rdx, %rsi
	movq	%rbp, %r8
	adcq	%rdi, %rdi
	xorl	%r9d, %r9d
	cmpq	%rsi, %rbp
	movq	%r9, %rax
	sbbq	%rdi, %rax
	jc	.L1475
	jmp	.L1445
	.p2align 4,,10
	.p2align 3
.L1855:
	movq	128(%rsp), %rax
	xorl	%ecx, %ecx
	mulq	%rbx
	movq	%rbp, %rax
	imulq	%rdx, %rax
	movq	%rdx, %r8
	subq	%rax, %rdi
	cmpq	%rbp, %rdi
	movq	%rdi, %rax
	movq	%rdi, %rdx
	setnb	%cl
	addq	%r8, %rcx
	subq	%rbp, %rax
	cmpq	%rbp, %rdi
	movq	152(%rsp), %rdi
	cmovnb	%rax, %rdx
	movq	%rsi, %rax
	movq	144(%rsp), %rsi
	movq	%rcx, %rbx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	cmpq	%rax, %rsi
	movq	%rax, %rcx
	movq	%rdi, %rax
	sbbq	%rbx, %rax
	jc	.L1599
	movq	%rdx, %rsi
	xorl	%edi, %edi
	testq	%rdx, %rdx
	je	.L1588
	addq	%rdx, %rsi
	movq	%rbp, %r8
	adcq	%rdi, %rdi
	xorl	%r9d, %r9d
	cmpq	%rsi, %rbp
	movq	%r9, %rax
	sbbq	%rdi, %rax
	jc	.L1596
	jmp	.L1568
	.p2align 4,,10
	.p2align 3
.L1856:
	movq	128(%rsp), %rax
	xorl	%ecx, %ecx
	mulq	%rbx
	movq	%rbp, %rax
	imulq	%rdx, %rax
	movq	%rdx, %r8
	subq	%rax, %rdi
	cmpq	%rbp, %rdi
	movq	%rdi, %rax
	movq	%rdi, %rdx
	setnb	%cl
	addq	%r8, %rcx
	subq	%rbp, %rax
	cmpq	%rbp, %rdi
	movq	152(%rsp), %rdi
	cmovnb	%rax, %rdx
	movq	%rsi, %rax
	movq	144(%rsp), %rsi
	movq	%rcx, %rbx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	cmpq	%rax, %rsi
	movq	%rax, %rcx
	movq	%rdi, %rax
	sbbq	%rbx, %rax
	jc	.L1559
	movq	%rdx, %rsi
	xorl	%edi, %edi
	testq	%rdx, %rdx
	je	.L1548
	addq	%rdx, %rsi
	movq	%rbp, %r8
	adcq	%rdi, %rdi
	xorl	%r9d, %r9d
	cmpq	%rsi, %rbp
	movq	%r9, %rax
	sbbq	%rdi, %rax
	jc	.L1556
	jmp	.L1528
	.p2align 4,,10
	.p2align 3
.L1838:
	movq	(%rsp), %r12
.L1609:
	movb	$1, 16(%r12)
	jmp	.L1106
.L1612:
	movq	$0, 96(%rsp)
	jmp	.L1113
	.p2align 4,,10
	.p2align 3
.L1852:
	movq	%rdx, %rax
	movq	%rdx, %rdi
	xorl	%ebx, %ebx
	movabsq	$2535301200456458803, %r14
	movabsq	$1000000000000, %r8
	mulq	%r14
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movq	%rsi, 24(%rsp)
	imulq	%r8, %rax
	subq	%rax, %rdi
	movq	%rcx, %rax
	movq	%rdi, %rdx
	movq	192(%rsp), %rdi
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r8
# 0 "" 2
#NO_APP
	movq	%rdx, 80(%rsp)
	movq	144(%rsp), %rdx
	movq	%rax, 16(%rsp)
	movq	16(%rsp), %rcx
	movq	%rdi, %rax
	movq	%rdx, %rsi
	imulq	%rcx, %rax
	imulq	%rbx, %rsi
	addq	%rax, %rsi
	movq	%rdx, %rax
	mulq	%rcx
	movq	%rax, %rcx
	movq	24(%rsp), %rax
	addq	%rdx, %rsi
	movq	%rsi, %rbx
	imulq	%rax, %rdi
	mulq	144(%rsp)
	movq	%rdi, %rsi
	xorl	%edi, %edi
	addq	%rdx, %rsi
	addq	%rbx, %rax
	movq	%rsi, %rdx
	adcq	%rdi, %rdx
	xorl	%edi, %edi
	movq	%rdx, %rsi
	orq	%rdi, %rsi
	jne	.L1406
	movq	%r10, %rsi
	xorl	%edi, %edi
	movq	%rax, %rdx
	shrq	$63, %rsi
	movq	%rdi, 24(%rsp)
	movabsq	$9223372036854775807, %rdi
	movq	%rsi, 16(%rsp)
	movq	$-1, %rsi
	addq	16(%rsp), %rsi
	adcq	24(%rsp), %rdi
	xorl	%eax, %eax
	xorl	%ebx, %ebx
	movq	%rsi, 16(%rsp)
	orq	%rcx, %rax
	orq	%rbx, %rdx
	movq	%rdi, 24(%rsp)
	movq	%rax, %rsi
	movq	%rdx, %rdi
	movq	16(%rsp), %rax
	movq	24(%rsp), %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1406
	movq	80(%rsp), %rbx
	testq	%rbx, %rbx
	je	.L1403
	movq	192(%rsp), %rax
	movq	%rbx, %rcx
	imulq	%rax, %rcx
	movq	144(%rsp), %rax
	mulq	%rbx
	addq	%rdx, %rcx
	movq	%rax, 80(%rsp)
	movq	80(%rsp), %rax
	xorl	%edx, %edx
	movq	%rcx, 88(%rsp)
	movq	88(%rsp), %rcx
	movq	%rax, 96(%rsp)
	movq	%rcx, %rax
	orq	%rdx, %rax
	jne	.L1858
	movq	$0, 88(%rsp)
	movq	96(%rsp), %rax
	xorl	%ebx, %ebx
	mulq	%r14
	shrq	$37, %rdx
	movq	%rdx, %rax
	movq	%rdx, %rcx
	movq	96(%rsp), %rdx
	imulq	%r8, %rax
	subq	%rax, %rdx
	movq	%rdx, 80(%rsp)
.L1419:
	movq	16(%rsp), %rax
	movq	24(%rsp), %rdx
	subq	%rsi, %rax
	sbbq	%rdi, %rdx
	cmpq	%rcx, %rax
	sbbq	%rbx, %rdx
	jc	.L1406
	movq	16(%rsp), %rax
	movq	24(%rsp), %rdx
	addq	%rsi, %rcx
	adcq	%rdi, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1406
	movq	80(%rsp), %rsi
	movq	88(%rsp), %rdi
	movq	%rsi, %rax
	orq	%rdi, %rax
	je	.L1415
	movq	80(%rsp), %rax
	xorl	%edx, %edx
	movl	$244140625, %edi
	addq	%rax, %rax
	adcq	%rdx, %rdx
	movq	%rax, %rsi
	salq	$12, %rdi
	xorl	%eax, %eax
	cmpq	%rsi, %rdi
	sbbq	%rdx, %rax
	jc	.L1417
	testq	%rdx, %rdx
	je	.L1859
.L1632:
	xorl	%eax, %eax
	jmp	.L1413
	.p2align 4,,10
	.p2align 3
.L1848:
	movq	128(%rsp), %rax
	xorl	%edi, %edi
	mulq	%rsi
	movq	%rdx, %rax
	movq	%rdx, %rsi
	movq	168(%rsp), %rdx
	imulq	%rbp, %rax
	subq	%rax, %rdx
	xorl	%eax, %eax
	cmpq	%rbp, %rdx
	setnb	%al
	addq	%rax, %rsi
	movq	%rdx, %rax
	adcq	$0, %rdi
	subq	%rbp, %rax
	cmpq	%rbp, %rdx
	cmovnb	%rax, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %r8
	jmp	.L1469
.L1844:
	movq	128(%rsp), %rax
	xorl	%edi, %edi
	mulq	%rsi
	movq	%rdx, %rax
	movq	%rdx, %rsi
	movq	216(%rsp), %rdx
	imulq	%rbp, %rax
	subq	%rax, %rdx
	xorl	%eax, %eax
	cmpq	%rbp, %rdx
	setnb	%al
	addq	%rax, %rsi
	movq	%rdx, %rax
	adcq	$0, %rdi
	subq	%rbp, %rax
	cmpq	%rbp, %rdx
	cmovnb	%rax, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %r8
	jmp	.L1551
.L1846:
	movq	128(%rsp), %rax
	xorl	%edi, %edi
	mulq	%rsi
	movq	%rdx, %rax
	movq	%rdx, %rsi
	movq	216(%rsp), %rdx
	imulq	%rbp, %rax
	subq	%rax, %rdx
	xorl	%eax, %eax
	cmpq	%rbp, %rdx
	setnb	%al
	addq	%rax, %rsi
	movq	%rdx, %rax
	adcq	$0, %rdi
	subq	%rbp, %rax
	cmpq	%rbp, %rdx
	cmovnb	%rax, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %r8
	jmp	.L1511
.L1841:
	movq	128(%rsp), %rax
	xorl	%edi, %edi
	mulq	%rsi
	movq	%rdx, %rax
	movq	%rdx, %rsi
	movq	%r13, %rdx
	imulq	%rbp, %rax
	subq	%rax, %rdx
	xorl	%eax, %eax
	cmpq	%rbp, %rdx
	setnb	%al
	addq	%rax, %rsi
	movq	%rdx, %rax
	adcq	$0, %rdi
	subq	%rbp, %rax
	cmpq	%rbp, %rdx
	cmovnb	%rax, %rdx
	xorl	%r9d, %r9d
	movq	%rdx, %r8
	jmp	.L1591
.L1884:
	cmpq	%rcx, 16(%rsp)
	jne	.L1631
.L1406:
	movb	$0, 688(%rsp)
	xorl	%r14d, %r14d
	movb	$0, 704(%rsp)
.L1410:
	movq	16(%r15), %rax
	movq	24(%r15), %rdx
	movq	%rax, %rcx
	movq	%rdx, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rdx, %r8
	cmovs	%rax, %rcx
	xorq	%r12, %rax
	xorq	%r13, %rdx
	movq	%rax, 16(%rsp)
	movq	%rdx, 24(%rsp)
	movq	%r8, %rax
	xorl	%edx, %edx
	movq	%r8, %rdi
	orq	%rdx, %rax
	jne	.L1860
	movq	144(%rsp), %rax
	movq	192(%rsp), %rsi
	imulq	%rax, %r8
	imulq	%rcx, %rsi
	mulq	%rcx
	addq	%r8, %rsi
	addq	%rdx, %rsi
	movq	%rax, %rcx
	xorl	%edx, %edx
	movq	%rsi, %rax
	movq	%rsi, %rdi
	orq	%rdx, %rax
	jne	.L1861
	movabsq	$2535301200456458803, %rax
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rdx, %rax
	movabsq	$1000000000000, %rdx
	shrq	$37, %rax
	imulq	%rax, %rdx
	movq	%rax, %r8
	movq	%rcx, %rax
	subq	%rdx, %rax
	xorl	%edx, %edx
.L1397:
	movq	%rax, %rcx
	orq	%rdx, %rcx
	je	.L1394
	xorl	%edx, %edx
	movl	$244140625, %esi
	addq	%rax, %rax
	adcq	%rdx, %rdx
	movq	%rax, %rcx
	salq	$12, %rsi
	xorl	%eax, %eax
	cmpq	%rcx, %rsi
	sbbq	%rdx, %rax
	jc	.L1392
	testq	%rdx, %rdx
	je	.L1862
.L1394:
	cmpq	$0, 24(%rsp)
	jns	.L1391
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L1391:
	movq	%r8, 712(%rsp)
	movq	%r9, 720(%rsp)
	movb	$1, 728(%rsp)
.L1367:
	movq	32(%r15), %rax
	movq	40(%r15), %rdx
	movq	%rax, %rcx
	movq	%rdx, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rdx, %r8
	cmovs	%rax, %rcx
	xorq	%r12, %rax
	xorq	%r13, %rdx
	movq	%rax, 16(%rsp)
	movq	%rdx, 24(%rsp)
	movq	%r8, %rax
	xorl	%edx, %edx
	movq	%r8, %rdi
	orq	%rdx, %rax
	jne	.L1863
	movq	144(%rsp), %rax
	movq	192(%rsp), %rsi
	imulq	%rax, %r8
	imulq	%rcx, %rsi
	mulq	%rcx
	addq	%r8, %rsi
	addq	%rdx, %rsi
	movq	%rax, %rcx
	xorl	%edx, %edx
	movq	%rsi, %rax
	movq	%rsi, %rdi
	orq	%rdx, %rax
	jne	.L1864
	movabsq	$2535301200456458803, %rax
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rdx, %rax
	movabsq	$1000000000000, %rdx
	shrq	$37, %rax
	imulq	%rax, %rdx
	movq	%rax, %r8
	movq	%rcx, %rax
	subq	%rdx, %rax
	xorl	%edx, %edx
.L1363:
	movq	%rax, %rcx
	orq	%rdx, %rcx
	je	.L1360
	xorl	%edx, %edx
	movl	$244140625, %esi
	addq	%rax, %rax
	adcq	%rdx, %rdx
	movq	%rax, %rcx
	salq	$12, %rsi
	xorl	%eax, %eax
	cmpq	%rcx, %rsi
	sbbq	%rdx, %rax
	jc	.L1358
	testq	%rdx, %rdx
	je	.L1865
.L1360:
	cmpq	$0, 24(%rsp)
	jns	.L1357
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L1357:
	movq	%r8, 736(%rsp)
	movq	%r9, 744(%rsp)
	movb	$1, 752(%rsp)
.L1333:
	movq	48(%r15), %rax
	movq	56(%r15), %rdx
	movq	%rax, %rcx
	movq	%rdx, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rdx, %r8
	cmovs	%rax, %rcx
	xorq	%r12, %rax
	xorq	%r13, %rdx
	movq	%rax, 16(%rsp)
	movq	%rdx, 24(%rsp)
	movq	%r8, %rax
	xorl	%edx, %edx
	movq	%r8, %rdi
	orq	%rdx, %rax
	jne	.L1866
	movq	144(%rsp), %rax
	movq	192(%rsp), %rsi
	imulq	%rax, %r8
	imulq	%rcx, %rsi
	mulq	%rcx
	addq	%r8, %rsi
	addq	%rdx, %rsi
	movq	%rax, %rcx
	xorl	%edx, %edx
	movq	%rsi, %rax
	movq	%rsi, %rdi
	orq	%rdx, %rax
	jne	.L1867
	movabsq	$2535301200456458803, %rax
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rdx, %rax
	movabsq	$1000000000000, %rdx
	shrq	$37, %rax
	imulq	%rax, %rdx
	movq	%rax, %r8
	movq	%rcx, %rax
	subq	%rdx, %rax
	xorl	%edx, %edx
.L1329:
	movq	%rax, %rcx
	orq	%rdx, %rcx
	je	.L1326
	xorl	%edx, %edx
	movl	$244140625, %esi
	addq	%rax, %rax
	adcq	%rdx, %rdx
	movq	%rax, %rcx
	salq	$12, %rsi
	xorl	%eax, %eax
	cmpq	%rcx, %rsi
	sbbq	%rdx, %rax
	jc	.L1324
	testq	%rdx, %rdx
	je	.L1868
.L1326:
	cmpq	$0, 24(%rsp)
	jns	.L1323
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L1323:
	movq	%r8, 760(%rsp)
	movq	%r9, 768(%rsp)
	movb	$1, 776(%rsp)
.L1299:
	testb	%r14b, %r14b
	je	.L1869
	movdqa	688(%rsp), %xmm0
	cmpb	$0, 728(%rsp)
	movups	%xmm0, 0(%rbp)
	je	.L1870
	movdqu	712(%rsp), %xmm0
	cmpb	$0, 752(%rsp)
	movups	%xmm0, 16(%rbp)
	je	.L1871
	movdqa	736(%rsp), %xmm0
	cmpb	$0, 776(%rsp)
	movups	%xmm0, 32(%rbp)
	je	.L1872
	movq	160(%rsp), %rax
	addq	$4, %r11
	addq	$64, %r15
	addq	$64, %rbp
	movdqu	760(%rsp), %xmm0
	subq	%r11, %rax
	movups	%xmm0, -16(%rbp)
	cmpq	$3, %rax
	ja	.L1437
	movq	(%rsp), %r12
	movq	72(%rsp), %r10
	movq	%r11, 96(%rsp)
	movq	48(%rsp), %r11
	jmp	.L1113
.L1888:
	cmpq	%rax, 144(%rsp)
	jne	.L1614
.L1788:
	movb	$0, 688(%rsp)
	xorl	%r15d, %r15d
	movb	$0, 704(%rsp)
	jmp	.L1134
.L1423:
	movabsq	$2535301200456458803, %rax
	movq	$0, 24(%rsp)
	mulq	%rcx
	movq	%rdx, %rax
	movabsq	$1000000000000, %rdx
	shrq	$37, %rax
	imulq	%rax, %rdx
	movq	%rax, 16(%rsp)
	movq	%rcx, %rax
	subq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L1433
.L1860:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %r10
	mulq	%rdi
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movq	%rsi, %r9
	imulq	%r10, %rax
	subq	%rax, %rdi
	movq	%rcx, %rax
	movq	%rdi, %rdx
	movq	192(%rsp), %rdi
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r10
# 0 "" 2
#NO_APP
	movq	%rdx, 96(%rsp)
	movq	144(%rsp), %rdx
	movq	%rax, %r8
	movq	%rdi, %rax
	imulq	%r8, %rax
	movq	%rdx, %rsi
	imulq	%r9, %rdi
	imulq	%rbx, %rsi
	addq	%rax, %rsi
	movq	%rdx, %rax
	mulq	%r8
	addq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%r9, %rax
	mulq	144(%rsp)
	movq	%rsi, %rbx
	movq	%rdi, %rsi
	xorl	%edi, %edi
	addq	%rdx, %rsi
	addq	%rbx, %rax
	movq	%rsi, %rdx
	adcq	%rdi, %rdx
	xorl	%edi, %edi
	movq	%rdx, %rsi
	orq	%rdi, %rsi
	jne	.L1372
	movq	24(%rsp), %r9
	movq	$-1, %rsi
	movq	%rax, %rdx
	movabsq	$9223372036854775807, %rdi
	movq	%r9, %r8
	xorl	%r9d, %r9d
	shrq	$63, %r8
	addq	%rsi, %r8
	adcq	%rdi, %r9
	xorl	%eax, %eax
	movq	%r8, %rsi
	xorl	%ebx, %ebx
	orq	%rax, %rcx
	movq	%r9, %rdi
	orq	%rdx, %rbx
	movq	%r8, 80(%rsp)
	cmpq	%rcx, %rsi
	movq	%rdi, %rax
	movq	%r9, 88(%rsp)
	movq	%rcx, %r8
	sbbq	%rbx, %rax
	movq	%rbx, %r9
	jc	.L1372
	movq	96(%rsp), %rdi
	testq	%rdi, %rdi
	je	.L1394
	movq	192(%rsp), %rax
	movq	%rdi, %rcx
	imulq	%rax, %rcx
	movq	144(%rsp), %rax
	mulq	%rdi
	addq	%rdx, %rcx
	movq	%rax, 96(%rsp)
	xorl	%edx, %edx
	movq	96(%rsp), %rbx
	movq	%rcx, 104(%rsp)
	movq	104(%rsp), %rcx
	movq	%rbx, %rdi
	movq	%rcx, %rax
	orq	%rdx, %rax
	jne	.L1873
	movabsq	$2535301200456458803, %rax
	mulq	%rbx
	xorl	%ebx, %ebx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	imulq	%r10, %rax
	subq	%rax, %rdi
	movq	%rdi, %rsi
	xorl	%edi, %edi
.L1384:
	movq	80(%rsp), %rax
	movq	88(%rsp), %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rcx, %rax
	sbbq	%rbx, %rdx
	jc	.L1372
	movq	80(%rsp), %rax
	movq	88(%rsp), %rdx
	addq	%r8, %rcx
	adcq	%r9, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1372
	movq	%rsi, %rax
	orq	%rdi, %rax
	je	.L1380
	movq	%rsi, %rax
	xorl	%edx, %edx
	movl	$244140625, %edi
	addq	%rsi, %rax
	adcq	%rdx, %rdx
	movq	%rax, %rsi
	salq	$12, %rdi
	xorl	%eax, %eax
	cmpq	%rsi, %rdi
	sbbq	%rdx, %rax
	jc	.L1382
	testq	%rdx, %rdx
	je	.L1874
.L1630:
	xorl	%eax, %eax
	jmp	.L1378
	.p2align 4,,10
	.p2align 3
.L1863:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %r10
	mulq	%rdi
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movq	%rsi, %r9
	imulq	%r10, %rax
	subq	%rax, %rdi
	movq	%rcx, %rax
	movq	%rdi, %rdx
	movq	192(%rsp), %rdi
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r10
# 0 "" 2
#NO_APP
	movq	%rdx, 96(%rsp)
	movq	144(%rsp), %rdx
	movq	%rax, %r8
	movq	%rdi, %rax
	imulq	%r8, %rax
	movq	%rdx, %rsi
	imulq	%r9, %rdi
	imulq	%rbx, %rsi
	addq	%rax, %rsi
	movq	%rdx, %rax
	mulq	%r8
	addq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%r9, %rax
	mulq	144(%rsp)
	movq	%rsi, %rbx
	movq	%rdi, %rsi
	xorl	%edi, %edi
	addq	%rdx, %rsi
	addq	%rbx, %rax
	movq	%rsi, %rdx
	adcq	%rdi, %rdx
	xorl	%edi, %edi
	movq	%rdx, %rsi
	orq	%rdi, %rsi
	jne	.L1338
	movq	24(%rsp), %r9
	movq	$-1, %rsi
	movq	%rax, %rdx
	movabsq	$9223372036854775807, %rdi
	movq	%r9, %r8
	xorl	%r9d, %r9d
	shrq	$63, %r8
	addq	%rsi, %r8
	adcq	%rdi, %r9
	xorl	%eax, %eax
	movq	%r8, %rsi
	xorl	%ebx, %ebx
	orq	%rax, %rcx
	movq	%r9, %rdi
	orq	%rdx, %rbx
	movq	%r8, 80(%rsp)
	cmpq	%rcx, %rsi
	movq	%rdi, %rax
	movq	%r9, 88(%rsp)
	movq	%rcx, %r8
	sbbq	%rbx, %rax
	movq	%rbx, %r9
	jc	.L1338
	movq	96(%rsp), %rdi
	testq	%rdi, %rdi
	je	.L1360
	movq	192(%rsp), %rax
	movq	%rdi, %rcx
	imulq	%rax, %rcx
	movq	144(%rsp), %rax
	mulq	%rdi
	addq	%rdx, %rcx
	movq	%rax, 96(%rsp)
	xorl	%edx, %edx
	movq	96(%rsp), %rbx
	movq	%rcx, 104(%rsp)
	movq	104(%rsp), %rcx
	movq	%rbx, %rdi
	movq	%rcx, %rax
	orq	%rdx, %rax
	jne	.L1875
	movabsq	$2535301200456458803, %rax
	mulq	%rbx
	xorl	%ebx, %ebx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	imulq	%r10, %rax
	subq	%rax, %rdi
	movq	%rdi, %rsi
	xorl	%edi, %edi
.L1350:
	movq	80(%rsp), %rax
	movq	88(%rsp), %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rcx, %rax
	sbbq	%rbx, %rdx
	jc	.L1338
	movq	80(%rsp), %rax
	movq	88(%rsp), %rdx
	addq	%r8, %rcx
	adcq	%r9, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1338
	movq	%rsi, %rax
	orq	%rdi, %rax
	je	.L1346
	movq	%rsi, %rax
	xorl	%edx, %edx
	movl	$244140625, %edi
	addq	%rsi, %rax
	adcq	%rdx, %rdx
	movq	%rax, %rsi
	salq	$12, %rdi
	xorl	%eax, %eax
	cmpq	%rsi, %rdi
	sbbq	%rdx, %rax
	jc	.L1348
	testq	%rdx, %rdx
	je	.L1876
.L1628:
	xorl	%eax, %eax
	jmp	.L1344
	.p2align 4,,10
	.p2align 3
.L1866:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %r10
	mulq	%rdi
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movq	%rsi, %r9
	imulq	%r10, %rax
	subq	%rax, %rdi
	movq	%rcx, %rax
	movq	%rdi, %rdx
	movq	192(%rsp), %rdi
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r10
# 0 "" 2
#NO_APP
	movq	%rdx, 96(%rsp)
	movq	144(%rsp), %rdx
	movq	%rax, %r8
	movq	%rdi, %rax
	imulq	%r8, %rax
	movq	%rdx, %rsi
	imulq	%r9, %rdi
	imulq	%rbx, %rsi
	addq	%rax, %rsi
	movq	%rdx, %rax
	mulq	%r8
	addq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%r9, %rax
	mulq	144(%rsp)
	movq	%rsi, %rbx
	movq	%rdi, %rsi
	xorl	%edi, %edi
	addq	%rdx, %rsi
	addq	%rbx, %rax
	movq	%rsi, %rdx
	adcq	%rdi, %rdx
	xorl	%edi, %edi
	movq	%rdx, %rsi
	orq	%rdi, %rsi
	jne	.L1304
	movq	24(%rsp), %r9
	movq	$-1, %rsi
	movq	%rax, %rdx
	movabsq	$9223372036854775807, %rdi
	movq	%r9, %r8
	xorl	%r9d, %r9d
	shrq	$63, %r8
	addq	%rsi, %r8
	adcq	%rdi, %r9
	xorl	%eax, %eax
	movq	%r8, %rsi
	xorl	%ebx, %ebx
	orq	%rax, %rcx
	movq	%r9, %rdi
	orq	%rdx, %rbx
	movq	%r8, 80(%rsp)
	cmpq	%rcx, %rsi
	movq	%rdi, %rax
	movq	%r9, 88(%rsp)
	movq	%rcx, %r8
	sbbq	%rbx, %rax
	movq	%rbx, %r9
	jc	.L1304
	movq	96(%rsp), %rdi
	testq	%rdi, %rdi
	je	.L1326
	movq	192(%rsp), %rax
	movq	%rdi, %rcx
	imulq	%rax, %rcx
	movq	144(%rsp), %rax
	mulq	%rdi
	addq	%rdx, %rcx
	movq	%rax, 96(%rsp)
	xorl	%edx, %edx
	movq	96(%rsp), %rbx
	movq	%rcx, 104(%rsp)
	movq	104(%rsp), %rcx
	movq	%rbx, %rdi
	movq	%rcx, %rax
	orq	%rdx, %rax
	jne	.L1877
	movabsq	$2535301200456458803, %rax
	mulq	%rbx
	xorl	%ebx, %ebx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	imulq	%r10, %rax
	subq	%rax, %rdi
	movq	%rdi, %rsi
	xorl	%edi, %edi
.L1316:
	movq	80(%rsp), %rax
	movq	88(%rsp), %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rcx, %rax
	sbbq	%rbx, %rdx
	jc	.L1304
	movq	80(%rsp), %rax
	movq	88(%rsp), %rdx
	addq	%r8, %rcx
	adcq	%r9, %rbx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L1304
	movq	%rsi, %rax
	orq	%rdi, %rax
	je	.L1312
	movq	%rsi, %rax
	xorl	%edx, %edx
	movl	$244140625, %edi
	addq	%rsi, %rax
	adcq	%rdx, %rdx
	movq	%rax, %rsi
	salq	$12, %rdi
	xorl	%eax, %eax
	cmpq	%rsi, %rdi
	sbbq	%rdx, %rax
	jc	.L1314
	testq	%rdx, %rdx
	je	.L1878
.L1626:
	xorl	%eax, %eax
	jmp	.L1310
	.p2align 4,,10
	.p2align 3
.L1845:
	movq	144(%rsp), %r8
	movq	152(%rsp), %r9
	movq	%r8, %rax
	movq	%r9, %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1519
	addq	%rsi, %rcx
	movq	%r9, %rax
	adcq	%rdi, %rbx
	cmpq	%rcx, %r8
	sbbq	%rbx, %rax
	jc	.L1519
	jmp	.L1508
.L1840:
	movq	144(%rsp), %r8
	movq	152(%rsp), %r9
	movq	%r8, %rax
	movq	%r9, %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1599
	addq	%rsi, %rcx
	movq	%r9, %rax
	adcq	%rdi, %rbx
	cmpq	%rcx, %r8
	sbbq	%rbx, %rax
	jc	.L1599
	jmp	.L1588
.L1847:
	movq	144(%rsp), %r8
	movq	152(%rsp), %r9
	movq	%r8, %rax
	movq	%r9, %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%r13, %rax
	jc	.L1459
	addq	%rsi, %rcx
	movq	%r9, %rax
	adcq	%r13, %rbx
	cmpq	%rcx, %r8
	sbbq	%rbx, %rax
	jc	.L1459
	jmp	.L1460
.L1843:
	movq	144(%rsp), %r8
	movq	152(%rsp), %r9
	movq	%r8, %rax
	movq	%r9, %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1559
	addq	%rsi, %rcx
	movq	%r9, %rax
	adcq	%rdi, %rbx
	cmpq	%rcx, %r8
	sbbq	%rbx, %rax
	jc	.L1559
	jmp	.L1548
.L1869:
	movq	32(%rsp), %r9
	movq	(%rsp), %r12
	movq	%r11, 96(%rsp)
	jmp	.L1290
.L1785:
	movq	32(%rsp), %r9
	movq	(%rsp), %r12
	movq	%r13, 96(%rsp)
	jmp	.L1290
.L1849:
	addq	$1, %r13
	movq	(%rsp), %r12
	movl	$1, %r9d
	movq	%r13, 96(%rsp)
	jmp	.L1290
.L1870:
	addq	$1, %r11
	movq	(%rsp), %r12
	movl	$1, %r9d
	movq	%r11, 96(%rsp)
	jmp	.L1290
.L1885:
	movq	80(%rsp), %rax
	cmpq	%rax, %rcx
	jne	.L1629
.L1372:
	movb	$0, 712(%rsp)
	movb	$0, 728(%rsp)
	jmp	.L1367
.L1883:
	movq	80(%rsp), %rax
	cmpq	%rax, %rcx
	jne	.L1627
.L1338:
	movb	$0, 736(%rsp)
	movb	$0, 752(%rsp)
	jmp	.L1333
.L1886:
	movq	80(%rsp), %rax
	cmpq	%rax, %rcx
	jne	.L1625
.L1304:
	movb	$0, 760(%rsp)
	movb	$0, 776(%rsp)
	jmp	.L1299
.L1850:
	addq	$2, %r13
	movq	(%rsp), %r12
	movl	$2, %r9d
	movq	%r13, 96(%rsp)
	jmp	.L1290
.L1871:
	addq	$2, %r11
	movq	(%rsp), %r12
	movl	$2, %r9d
	movq	%r11, 96(%rsp)
	jmp	.L1290
.L1851:
	addq	$3, %r13
	movq	(%rsp), %r12
	movl	$3, %r9d
	movq	%r13, 96(%rsp)
	jmp	.L1290
.L1872:
	addq	$3, %r11
	movq	(%rsp), %r12
	movl	$3, %r9d
	movq	%r11, 96(%rsp)
	jmp	.L1290
.L1890:
	cmpq	%rax, 144(%rsp)
	jne	.L1623
.L1248:
	movb	$0, 760(%rsp)
	movb	$0, 776(%rsp)
	jmp	.L1263
.L1889:
	cmpq	%rax, 144(%rsp)
	jne	.L1620
.L1205:
	movb	$0, 736(%rsp)
	movb	$0, 752(%rsp)
	jmp	.L1220
.L1887:
	cmpq	%rax, 144(%rsp)
	jne	.L1617
.L1162:
	movb	$0, 712(%rsp)
	movb	$0, 728(%rsp)
	jmp	.L1177
.L1204:
	movq	112(%rsp), %r10
	movq	120(%rsp), %r11
	movq	%rcx, %rax
	xorl	%ebp, %ebp
	sarq	$63, %rax
	movq	%r10, %rdx
	movq	%r11, %rsi
	sarq	$63, %rdx
	cmpq	%r11, %rdx
	jne	.L1223
	cmpq	%r8, %rax
	jne	.L1224
	movq	%r10, %rax
	imulq	%rcx
.L1221:
	testq	%rbp, %rbp
	je	.L1231
	movq	96(%rsp), %rax
	sarq	$63, %r8
	movaps	%xmm0, 512(%rsp)
	leaq	640(%rsp), %rdi
	movq	%r8, %xmm0
	subq	$64, %rsp
	.cfi_def_cfa_offset 928
	movl	$12, %ecx
	movaps	%xmm1, 608(%rsp)
	sarq	$63, %rax
	punpcklqdq	%xmm0, %xmm0
	movl	$12, %edx
	movl	$128, %r9d
	movq	%rax, %xmm2
	movl	$3, %r8d
	movl	$12, %esi
	movaps	%xmm0, 592(%rsp)
	punpcklqdq	%xmm2, %xmm2
	movaps	%xmm2, 624(%rsp)
	movdqa	624(%rsp), %xmm0
	movups	%xmm1, 32(%rsp)
	movups	%xmm0, 48(%rsp)
	movdqa	576(%rsp), %xmm0
	movaps	%xmm1, 80(%rsp)
	movups	%xmm0, (%rsp)
	movdqa	592(%rsp), %xmm0
	movups	%xmm0, 16(%rsp)
	call	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	addq	$64, %rsp
	.cfi_def_cfa_offset 864
	movdqa	16(%rsp), %xmm1
	cmpb	$0, 672(%rsp)
	jne	.L1232
	movzbl	640(%rsp), %eax
	movb	$0, 752(%rsp)
	movb	%al, 736(%rsp)
	jmp	.L1220
.L1247:
	movq	112(%rsp), %r10
	movq	120(%rsp), %r11
	movq	%rcx, %rax
	xorl	%ebp, %ebp
	sarq	$63, %rax
	movq	%r10, %rdx
	movq	%r11, %rsi
	sarq	$63, %rdx
	cmpq	%r11, %rdx
	jne	.L1266
	cmpq	%r8, %rax
	jne	.L1267
	movq	%r10, %rax
	imulq	%rcx
.L1264:
	testq	%rbp, %rbp
	je	.L1274
	movq	96(%rsp), %rax
	sarq	$63, %r8
	movaps	%xmm0, 576(%rsp)
	leaq	640(%rsp), %rdi
	movq	%r8, %xmm0
	subq	$64, %rsp
	.cfi_def_cfa_offset 928
	movl	$12, %ecx
	movaps	%xmm1, 672(%rsp)
	sarq	$63, %rax
	punpcklqdq	%xmm0, %xmm0
	movl	$12, %edx
	movl	$128, %r9d
	movq	%rax, %xmm2
	movl	$3, %r8d
	movl	$12, %esi
	movaps	%xmm0, 656(%rsp)
	punpcklqdq	%xmm2, %xmm2
	movaps	%xmm2, 688(%rsp)
	movdqa	688(%rsp), %xmm0
	movups	%xmm1, 32(%rsp)
	movups	%xmm0, 48(%rsp)
	movdqa	640(%rsp), %xmm0
	movaps	%xmm1, 80(%rsp)
	movups	%xmm0, (%rsp)
	movdqa	656(%rsp), %xmm0
	movups	%xmm0, 16(%rsp)
	call	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	addq	$64, %rsp
	.cfi_def_cfa_offset 864
	movdqa	16(%rsp), %xmm1
	cmpb	$0, 672(%rsp)
	jne	.L1275
	movzbl	640(%rsp), %eax
	movb	$0, 776(%rsp)
	movb	%al, 760(%rsp)
	jmp	.L1263
.L1161:
	movq	112(%rsp), %r10
	movq	120(%rsp), %r11
	movq	%rcx, %rax
	xorl	%ebp, %ebp
	sarq	$63, %rax
	movq	%r10, %rdx
	movq	%r11, %rsi
	sarq	$63, %rdx
	cmpq	%r11, %rdx
	jne	.L1180
	cmpq	%r8, %rax
	jne	.L1181
	movq	%r10, %rax
	imulq	%rcx
.L1178:
	testq	%rbp, %rbp
	je	.L1188
	movq	96(%rsp), %rax
	sarq	$63, %r8
	movaps	%xmm0, 448(%rsp)
	leaq	640(%rsp), %rdi
	movq	%r8, %xmm0
	subq	$64, %rsp
	.cfi_def_cfa_offset 928
	movl	$12, %ecx
	movaps	%xmm1, 544(%rsp)
	sarq	$63, %rax
	punpcklqdq	%xmm0, %xmm0
	movl	$12, %edx
	movl	$128, %r9d
	movq	%rax, %xmm2
	movl	$3, %r8d
	movl	$12, %esi
	movaps	%xmm0, 528(%rsp)
	punpcklqdq	%xmm2, %xmm2
	movaps	%xmm2, 560(%rsp)
	movdqa	560(%rsp), %xmm0
	movups	%xmm1, 32(%rsp)
	movups	%xmm0, 48(%rsp)
	movdqa	512(%rsp), %xmm0
	movaps	%xmm1, 80(%rsp)
	movups	%xmm0, (%rsp)
	movdqa	528(%rsp), %xmm0
	movups	%xmm0, 16(%rsp)
	call	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	addq	$64, %rsp
	.cfi_def_cfa_offset 864
	movdqa	16(%rsp), %xmm1
	cmpb	$0, 672(%rsp)
	jne	.L1189
	movzbl	640(%rsp), %eax
	movb	$0, 728(%rsp)
	movb	%al, 712(%rsp)
	jmp	.L1177
	.p2align 4,,10
	.p2align 3
.L1116:
	movq	112(%rsp), %r10
	movq	120(%rsp), %r11
	movq	%rcx, %rax
	xorl	%r9d, %r9d
	sarq	$63, %rax
	movq	%r10, %rdx
	movq	%r11, %rsi
	sarq	$63, %rdx
	cmpq	%r11, %rdx
	jne	.L1137
	cmpq	%r8, %rax
	jne	.L1138
	movq	%r10, %rax
	imulq	%rcx
.L1135:
	testq	%r9, %r9
	je	.L1145
	movq	96(%rsp), %rax
	sarq	$63, %r8
	movaps	%xmm0, 384(%rsp)
	leaq	640(%rsp), %rdi
	movq	%r8, %xmm0
	subq	$64, %rsp
	.cfi_def_cfa_offset 928
	movl	$12, %ecx
	movaps	%xmm1, 480(%rsp)
	sarq	$63, %rax
	punpcklqdq	%xmm0, %xmm0
	movl	$12, %edx
	movl	$128, %r9d
	movq	%rax, %xmm2
	movl	$3, %r8d
	movl	$12, %esi
	movaps	%xmm0, 464(%rsp)
	punpcklqdq	%xmm2, %xmm2
	movaps	%xmm2, 496(%rsp)
	movdqa	496(%rsp), %xmm0
	movups	%xmm1, 32(%rsp)
	movups	%xmm0, 48(%rsp)
	movdqa	448(%rsp), %xmm0
	movaps	%xmm1, 80(%rsp)
	movups	%xmm0, (%rsp)
	movdqa	464(%rsp), %xmm0
	movups	%xmm0, 16(%rsp)
	call	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	movzbl	736(%rsp), %r15d
	addq	$64, %rsp
	.cfi_def_cfa_offset 864
	movdqa	16(%rsp), %xmm1
	testb	%r15b, %r15b
	jne	.L1146
	movzbl	640(%rsp), %eax
	movb	$0, 704(%rsp)
	movb	%al, 688(%rsp)
	jmp	.L1134
.L1867:
	movabsq	$2535301200456458803, %rax
	movq	%rdi, %rsi
	movabsq	$1000000000000, %r8
	mulq	%rdi
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movq	%rdi, %r9
	imulq	%r8, %rax
	subq	%rax, %rsi
	movq	%rcx, %rax
	movq	%rsi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r8
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L1329
.L1145:
	movq	%rax, %rsi
	movq	%rdx, %r8
	movq	%rax, %rcx
	movq	%rdx, %rbx
	testq	%rdx, %rdx
	js	.L1148
	movq	%rdx, %rcx
	testq	%rdx, %rdx
	jne	.L1149
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r8
	mulq	%rsi
	movq	%rdx, %rdi
	xorl	%edx, %edx
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r8, %rdi
	subq	%rdi, %rsi
	je	.L1150
	xorl	%edi, %edi
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	cmpq	%rsi, %r8
	sbbq	%rdi, %rcx
	jnc	.L1151
	addq	$1, %rax
	adcq	$0, %rdx
.L1150:
	movb	$1, 240(%rsp)
	movdqa	240(%rsp), %xmm7
	movq	%rax, 224(%rsp)
	movq	%rax, 256(%rsp)
	movl	$1, %eax
	movq	%rdx, 232(%rsp)
	movl	%eax, %r15d
	movq	%rdx, 264(%rsp)
	movaps	%xmm7, 272(%rsp)
	testb	%al, %al
	jne	.L1159
	movzbl	256(%rsp), %eax
	movb	$0, 704(%rsp)
	movb	%al, 688(%rsp)
	jmp	.L1134
.L1231:
	movq	%rax, %rdi
	movq	%rdx, %r8
	testq	%rdx, %rdx
	js	.L1234
	movq	%rdx, %rcx
	testq	%rdx, %rdx
	jne	.L1235
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r8
	mulq	%rdi
	movq	%rdx, %r9
	xorl	%edx, %edx
	shrq	$37, %r9
	movq	%r9, %rax
	imulq	%r8, %r9
	subq	%r9, %rdi
	movq	%rdi, %rsi
	je	.L1236
	xorl	%edi, %edi
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	cmpq	%rsi, %r8
	sbbq	%rdi, %rcx
	jnc	.L1237
	addq	$1, %rax
	adcq	$0, %rdx
.L1236:
	movb	$1, 240(%rsp)
	movdqa	240(%rsp), %xmm7
	movq	%rax, 224(%rsp)
	movq	%rax, 256(%rsp)
	movl	$1, %eax
	movq	%rdx, 232(%rsp)
	movq	%rdx, 264(%rsp)
	movaps	%xmm7, 272(%rsp)
	testb	%al, %al
	jne	.L1245
	movzbl	256(%rsp), %eax
	movb	$0, 752(%rsp)
	movb	%al, 736(%rsp)
	jmp	.L1220
.L1861:
	movabsq	$2535301200456458803, %rax
	movq	%rdi, %rsi
	movabsq	$1000000000000, %r8
	mulq	%rdi
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movq	%rdi, %r9
	imulq	%r8, %rax
	subq	%rax, %rsi
	movq	%rcx, %rax
	movq	%rsi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r8
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L1397
.L1864:
	movabsq	$2535301200456458803, %rax
	movq	%rdi, %rsi
	movabsq	$1000000000000, %r8
	mulq	%rdi
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movq	%rdi, %r9
	imulq	%r8, %rax
	subq	%rax, %rsi
	movq	%rcx, %rax
	movq	%rsi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r8
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L1363
.L1188:
	movq	%rax, %rdi
	movq	%rdx, %r8
	testq	%rdx, %rdx
	js	.L1191
	movq	%rdx, %rcx
	testq	%rdx, %rdx
	jne	.L1192
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r8
	mulq	%rdi
	movq	%rdx, %r9
	xorl	%edx, %edx
	shrq	$37, %r9
	movq	%r9, %rax
	imulq	%r8, %r9
	subq	%r9, %rdi
	movq	%rdi, %rsi
	je	.L1193
	xorl	%edi, %edi
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	cmpq	%rsi, %r8
	sbbq	%rdi, %rcx
	jnc	.L1194
	addq	$1, %rax
	adcq	$0, %rdx
.L1193:
	movb	$1, 240(%rsp)
	movdqa	240(%rsp), %xmm7
	movq	%rax, 224(%rsp)
	movq	%rax, 256(%rsp)
	movl	$1, %eax
	movq	%rdx, 232(%rsp)
	movq	%rdx, 264(%rsp)
	movaps	%xmm7, 272(%rsp)
	testb	%al, %al
	jne	.L1202
	movzbl	256(%rsp), %eax
	movb	$0, 728(%rsp)
	movb	%al, 712(%rsp)
	jmp	.L1177
.L1274:
	movq	%rax, %rdi
	movq	%rdx, %r8
	testq	%rdx, %rdx
	js	.L1277
	movq	%rdx, %rcx
	testq	%rdx, %rdx
	jne	.L1278
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r8
	mulq	%rdi
	movq	%rdx, %r9
	xorl	%edx, %edx
	shrq	$37, %r9
	movq	%r9, %rax
	imulq	%r8, %r9
	subq	%r9, %rdi
	movq	%rdi, %rsi
	je	.L1279
	xorl	%edi, %edi
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	cmpq	%rsi, %r8
	sbbq	%rdi, %rcx
	jnc	.L1280
	addq	$1, %rax
	adcq	$0, %rdx
.L1279:
	movb	$1, 240(%rsp)
	movdqa	240(%rsp), %xmm7
	movq	%rax, 224(%rsp)
	movq	%rax, 256(%rsp)
	movl	$1, %eax
	movq	%rdx, 232(%rsp)
	movq	%rdx, 264(%rsp)
	movaps	%xmm7, 272(%rsp)
	testb	%al, %al
	jne	.L1288
	movzbl	256(%rsp), %eax
	movb	$0, 776(%rsp)
	movb	%al, 760(%rsp)
	jmp	.L1263
.L1403:
	testq	%r10, %r10
	jns	.L1422
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
.L1422:
	movq	%rsi, 688(%rsp)
	movq	%rdi, 696(%rsp)
	movb	$1, 704(%rsp)
	jmp	.L1421
.L1245:
	movq	256(%rsp), %rax
	movb	$1, 752(%rsp)
	movq	%rax, 736(%rsp)
	movq	264(%rsp), %rax
	movq	%rax, 744(%rsp)
	jmp	.L1220
.L1275:
	movdqa	640(%rsp), %xmm0
	movb	$1, 776(%rsp)
	movups	%xmm0, 760(%rsp)
	jmp	.L1263
.L1202:
	movq	256(%rsp), %rax
	movb	$1, 728(%rsp)
	movq	%rax, 712(%rsp)
	movq	264(%rsp), %rax
	movq	%rax, 720(%rsp)
	jmp	.L1177
.L1189:
	movdqa	640(%rsp), %xmm0
	movb	$1, 728(%rsp)
	movups	%xmm0, 712(%rsp)
	jmp	.L1177
.L1146:
	movdqa	640(%rsp), %xmm0
	movb	$1, 704(%rsp)
	movaps	%xmm0, 688(%rsp)
	jmp	.L1134
.L1232:
	movdqa	640(%rsp), %xmm0
	movb	$1, 752(%rsp)
	movaps	%xmm0, 736(%rsp)
	jmp	.L1220
.L1159:
	movq	256(%rsp), %rax
	movb	$1, 704(%rsp)
	movq	%rax, 688(%rsp)
	movq	264(%rsp), %rax
	movq	%rax, 696(%rsp)
	jmp	.L1134
.L1288:
	movq	256(%rsp), %rax
	movb	$1, 776(%rsp)
	movq	%rax, 760(%rsp)
	movq	264(%rsp), %rax
	movq	%rax, 768(%rsp)
	jmp	.L1263
.L1234:
	negq	%rax
	adcq	$0, %rdx
	movq	%rax, %rbx
	negq	%rdx
	movq	%rdx, %rcx
	jne	.L1239
	movabsq	$2535301200456458803, %rax
	movq	%rbx, %rsi
	movabsq	$1000000000000, %r8
	mulq	%rbx
	movq	%rdx, %rdi
	xorl	%edx, %edx
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r8, %rdi
	subq	%rdi, %rsi
	je	.L1240
	xorl	%edi, %edi
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	cmpq	%rsi, %r8
	sbbq	%rdi, %rcx
	jnc	.L1241
	addq	$1, %rax
	adcq	$0, %rdx
.L1240:
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
	jmp	.L1236
.L1148:
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
	movq	%rbx, %rsi
	jne	.L1153
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %rdi
	mulq	%rcx
	movq	%rdx, %r8
	xorl	%edx, %edx
	shrq	$37, %r8
	movq	%r8, %rax
	imulq	%rdi, %r8
	subq	%r8, %rcx
	je	.L1154
	xorl	%ebx, %ebx
	addq	%rcx, %rcx
	adcq	%rbx, %rbx
	cmpq	%rcx, %rdi
	sbbq	%rbx, %rsi
	jnc	.L1155
	addq	$1, %rax
	adcq	$0, %rdx
.L1154:
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
	jmp	.L1150
.L1206:
	movq	128(%rsp), %rax
	imulq	%rax, %rsi
	mulq	%rdi
	addq	%rdx, %rsi
	movq	%rax, %r8
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rax
	movabsq	$2535301200456458803, %rax
	jne	.L1209
	mulq	%r8
	xorl	%edi, %edi
	xorl	%r9d, %r9d
	movq	%rdx, %rax
	movq	%r8, %rdx
	shrq	$37, %rax
	movq	%rax, %rsi
	imulq	%rbp, %rax
	subq	%rax, %rdx
	movq	%rdx, %r8
.L1210:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1205
	movq	%rcx, %rax
	movq	%rbx, %rdx
	movq	152(%rsp), %rbp
	addq	%rsi, %rax
	adcq	%rdi, %rdx
	movq	144(%rsp), %rdi
	cmpq	%rax, %rdi
	movq	%rbp, %rdi
	sbbq	%rdx, %rdi
	jc	.L1205
	movq	%r8, %rcx
	orq	%r9, %rcx
	je	.L1213
	movq	%r8, %rcx
	xorl	%ebx, %ebx
	addq	%r8, %rcx
	movabsq	$1000000000000, %r8
	movq	%rcx, %rdi
	adcq	%rbx, %rbx
	xorl	%ecx, %ecx
	cmpq	%rdi, %r8
	sbbq	%rbx, %rcx
	jc	.L1214
	testq	%rbx, %rbx
	je	.L1879
.L1619:
	xorl	%ecx, %ecx
	jmp	.L1215
	.p2align 4,,10
	.p2align 3
.L1118:
	movq	128(%rsp), %rax
	imulq	%rax, %rsi
	mulq	%rdi
	addq	%rdx, %rsi
	movq	%rax, %r8
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rax
	jne	.L1121
	movq	%r8, %rax
	xorl	%edi, %edi
	xorl	%r9d, %r9d
	mulq	%r15
	movq	%rdx, %rax
	movq	%r8, %rdx
	shrq	$37, %rax
	movq	%rax, %rsi
	imulq	%rbp, %rax
	subq	%rax, %rdx
	movq	%rdx, %r8
.L1122:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1788
	movq	%rcx, %rax
	movq	%rbx, %rdx
	movq	152(%rsp), %rbp
	addq	%rsi, %rax
	adcq	%rdi, %rdx
	movq	144(%rsp), %rdi
	cmpq	%rax, %rdi
	movq	%rbp, %rdi
	sbbq	%rdx, %rdi
	jc	.L1788
	movq	%r8, %rcx
	orq	%r9, %rcx
	je	.L1126
	movq	%r8, %rcx
	xorl	%ebx, %ebx
	addq	%r8, %rcx
	movabsq	$1000000000000, %r8
	adcq	%rbx, %rbx
	xorl	%esi, %esi
	cmpq	%rcx, %r8
	movq	%rcx, %rdi
	sbbq	%rbx, %rsi
	jc	.L1127
	testq	%rbx, %rbx
	je	.L1880
.L1613:
	xorl	%ecx, %ecx
	jmp	.L1128
	.p2align 4,,10
	.p2align 3
.L1163:
	movq	128(%rsp), %rax
	imulq	%rax, %rsi
	mulq	%rdi
	addq	%rdx, %rsi
	movq	%rax, %r8
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rax
	movabsq	$2535301200456458803, %rax
	jne	.L1166
	mulq	%r8
	xorl	%edi, %edi
	xorl	%r9d, %r9d
	movq	%rdx, %rax
	movq	%r8, %rdx
	shrq	$37, %rax
	movq	%rax, %rsi
	imulq	%rbp, %rax
	subq	%rax, %rdx
	movq	%rdx, %r8
.L1167:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1162
	movq	%rcx, %rax
	movq	%rbx, %rdx
	movq	152(%rsp), %rbp
	addq	%rsi, %rax
	adcq	%rdi, %rdx
	movq	144(%rsp), %rdi
	cmpq	%rax, %rdi
	movq	%rbp, %rdi
	sbbq	%rdx, %rdi
	jc	.L1162
	movq	%r8, %rcx
	orq	%r9, %rcx
	je	.L1170
	movq	%r8, %rcx
	xorl	%ebx, %ebx
	addq	%r8, %rcx
	movabsq	$1000000000000, %r8
	movq	%rcx, %rdi
	adcq	%rbx, %rbx
	xorl	%ecx, %ecx
	cmpq	%rdi, %r8
	sbbq	%rbx, %rcx
	jc	.L1171
	testq	%rbx, %rbx
	je	.L1881
.L1616:
	xorl	%ecx, %ecx
.L1172:
	movzbl	%cl, %ecx
	addq	%rcx, %rax
	adcq	$0, %rdx
.L1170:
	cmpq	$0, 24(%rsp)
	jns	.L1176
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
.L1176:
	movq	%rax, 712(%rsp)
	movq	%rdx, 720(%rsp)
	movb	$1, 728(%rsp)
	jmp	.L1177
.L1277:
	negq	%rax
	adcq	$0, %rdx
	movq	%rax, %rbx
	negq	%rdx
	movq	%rdx, %rcx
	jne	.L1282
	movabsq	$2535301200456458803, %rax
	movq	%rbx, %rsi
	movabsq	$1000000000000, %r8
	mulq	%rbx
	movq	%rdx, %rdi
	xorl	%edx, %edx
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r8, %rdi
	subq	%rdi, %rsi
	je	.L1283
	xorl	%edi, %edi
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	cmpq	%rsi, %r8
	sbbq	%rdi, %rcx
	jnc	.L1284
	addq	$1, %rax
	adcq	$0, %rdx
.L1283:
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
	jmp	.L1279
.L1249:
	movq	128(%rsp), %rax
	imulq	%rax, %rsi
	mulq	%rdi
	addq	%rdx, %rsi
	movq	%rax, %r8
	xorl	%edx, %edx
	movq	%rsi, %rax
	orq	%rdx, %rax
	movabsq	$2535301200456458803, %rax
	jne	.L1252
	mulq	%r8
	xorl	%edi, %edi
	xorl	%r9d, %r9d
	movq	%rdx, %rax
	movq	%r8, %rdx
	shrq	$37, %rax
	movq	%rax, %rsi
	imulq	%rbp, %rax
	subq	%rax, %rdx
	movq	%rdx, %r8
.L1253:
	movq	144(%rsp), %rax
	movq	152(%rsp), %rdx
	subq	%rcx, %rax
	sbbq	%rbx, %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1248
	movq	%rcx, %rax
	movq	%rbx, %rdx
	movq	152(%rsp), %rbp
	addq	%rsi, %rax
	adcq	%rdi, %rdx
	movq	144(%rsp), %rdi
	cmpq	%rax, %rdi
	movq	%rbp, %rdi
	sbbq	%rdx, %rdi
	jc	.L1248
	movq	%r8, %rcx
	orq	%r9, %rcx
	je	.L1256
	movq	%r8, %rcx
	xorl	%ebx, %ebx
	addq	%r8, %rcx
	movabsq	$1000000000000, %r8
	movq	%rcx, %rdi
	adcq	%rbx, %rbx
	xorl	%ecx, %ecx
	cmpq	%rdi, %r8
	sbbq	%rbx, %rcx
	jc	.L1257
	testq	%rbx, %rbx
	je	.L1882
.L1622:
	xorl	%ecx, %ecx
.L1258:
	movzbl	%cl, %ecx
	addq	%rcx, %rax
	adcq	$0, %rdx
.L1256:
	cmpq	$0, 24(%rsp)
	jns	.L1262
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
.L1262:
	movq	%rax, 760(%rsp)
	movq	%rdx, 768(%rsp)
	movb	$1, 776(%rsp)
	jmp	.L1263
.L1191:
	negq	%rax
	adcq	$0, %rdx
	movq	%rax, %rbx
	negq	%rdx
	movq	%rdx, %rcx
	jne	.L1196
	movabsq	$2535301200456458803, %rax
	movq	%rbx, %rsi
	movabsq	$1000000000000, %r8
	mulq	%rbx
	movq	%rdx, %rdi
	xorl	%edx, %edx
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r8, %rdi
	subq	%rdi, %rsi
	je	.L1197
	xorl	%edi, %edi
	addq	%rsi, %rsi
	adcq	%rdi, %rdi
	cmpq	%rsi, %r8
	sbbq	%rdi, %rcx
	jnc	.L1198
	addq	$1, %rax
	adcq	$0, %rdx
.L1197:
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
	jmp	.L1193
.L1223:
	cmpq	%r8, %rax
	jne	.L1226
	movdqa	112(%rsp), %xmm7
	movq	%rcx, %rdi
	movaps	%xmm7, 16(%rsp)
.L1225:
	movq	112(%rsp), %rax
	mulq	%rcx
	movq	%rax, 128(%rsp)
	movq	%rdi, %rax
	movq	%rdx, 136(%rsp)
	mulq	%rsi
	movq	%rax, %r10
	movq	%rdx, %r11
	testq	%rsi, %rsi
	jns	.L1227
	xorl	%eax, %eax
	subq	%rax, %r10
	sbbq	%rdi, %r11
.L1227:
	testq	%rdi, %rdi
	jns	.L1228
	subq	16(%rsp), %r10
	sbbq	24(%rsp), %r11
.L1228:
	movq	136(%rsp), %rdx
	movq	%rdx, %rax
	xorl	%edx, %edx
	addq	%r10, %rax
	movq	%rax, %rdi
	adcq	%r11, %rdx
	sarq	$63, %rdi
	cmpq	%rdx, %rdi
	jne	.L1229
	movq	%rax, %rdx
	movq	128(%rsp), %rax
	jmp	.L1221
.L1137:
	cmpq	%r8, %rax
	jne	.L1140
	movdqa	112(%rsp), %xmm7
	movq	%rcx, %rdi
	movaps	%xmm7, 16(%rsp)
.L1139:
	movq	112(%rsp), %rax
	mulq	%rcx
	movq	%rax, 128(%rsp)
	movq	%rdi, %rax
	movq	%rdx, 136(%rsp)
	mulq	%rsi
	movq	%rax, %r10
	movq	%rdx, %r11
	testq	%rsi, %rsi
	jns	.L1141
	xorl	%eax, %eax
	subq	%rax, %r10
	sbbq	%rdi, %r11
.L1141:
	testq	%rdi, %rdi
	jns	.L1142
	subq	16(%rsp), %r10
	sbbq	24(%rsp), %r11
.L1142:
	movq	136(%rsp), %rdx
	movq	%rdx, %rax
	xorl	%edx, %edx
	addq	%r10, %rax
	movq	%rax, %rdi
	adcq	%r11, %rdx
	sarq	$63, %rdi
	cmpq	%rdx, %rdi
	jne	.L1143
	movq	%rax, %rdx
	movq	128(%rsp), %rax
	jmp	.L1135
.L1266:
	cmpq	%r8, %rax
	jne	.L1269
	movdqa	112(%rsp), %xmm7
	movq	%rcx, %rdi
	movaps	%xmm7, 16(%rsp)
.L1268:
	movq	112(%rsp), %rax
	mulq	%rcx
	movq	%rax, 128(%rsp)
	movq	%rdi, %rax
	movq	%rdx, 136(%rsp)
	mulq	%rsi
	movq	%rax, %r10
	movq	%rdx, %r11
	testq	%rsi, %rsi
	jns	.L1270
	xorl	%eax, %eax
	subq	%rax, %r10
	sbbq	%rdi, %r11
.L1270:
	testq	%rdi, %rdi
	jns	.L1271
	subq	16(%rsp), %r10
	sbbq	24(%rsp), %r11
.L1271:
	movq	136(%rsp), %rdx
	movq	%rdx, %rax
	xorl	%edx, %edx
	addq	%r10, %rax
	movq	%rax, %rdi
	adcq	%r11, %rdx
	sarq	$63, %rdi
	cmpq	%rdx, %rdi
	jne	.L1272
	movq	%rax, %rdx
	movq	128(%rsp), %rax
	jmp	.L1264
.L1180:
	cmpq	%r8, %rax
	jne	.L1183
	movdqa	112(%rsp), %xmm7
	movq	%rcx, %rdi
	movaps	%xmm7, 16(%rsp)
.L1182:
	movq	112(%rsp), %rax
	mulq	%rcx
	movq	%rax, 128(%rsp)
	movq	%rdi, %rax
	movq	%rdx, 136(%rsp)
	mulq	%rsi
	movq	%rax, %r10
	movq	%rdx, %r11
	testq	%rsi, %rsi
	jns	.L1184
	xorl	%eax, %eax
	subq	%rax, %r10
	sbbq	%rdi, %r11
.L1184:
	testq	%rdi, %rdi
	jns	.L1185
	subq	16(%rsp), %r10
	sbbq	24(%rsp), %r11
.L1185:
	movq	136(%rsp), %rdx
	movq	%rdx, %rax
	xorl	%edx, %edx
	addq	%r10, %rax
	movq	%rax, %rdi
	adcq	%r11, %rdx
	sarq	$63, %rdi
	cmpq	%rdx, %rdi
	jne	.L1186
	movq	%rax, %rdx
	movq	128(%rsp), %rax
	jmp	.L1178
.L1858:
	movq	$0, 88(%rsp)
	movq	%rcx, %rax
	mulq	%r14
	movq	%rdx, %rbx
	shrq	$37, %rbx
	movq	%rbx, %rax
	imulq	%r8, %rax
	subq	%rax, %rcx
	movq	80(%rsp), %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r8
# 0 "" 2
#NO_APP
	movq	%rdx, 80(%rsp)
	movq	%rax, %rcx
	jmp	.L1419
.L1192:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %r9
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	imulq	%r9, %rax
	subq	%rax, %rcx
	movq	%rdi, %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r9
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	movq	%rsi, %rdx
	movq	%rcx, %rsi
	orq	%rbx, %rsi
	je	.L1193
.L1200:
	movq	%rcx, %rsi
	xorl	%edi, %edi
	movl	$0, %ebx
	addq	%rcx, %rsi
	movl	$244140625, %ecx
	adcq	%rdi, %rdi
	salq	$12, %rcx
	cmpq	%rsi, %rcx
	movl	$0, %ecx
	sbbq	%rdi, %rcx
	movl	$1, %ecx
	jc	.L1201
	movl	$244140625, %ecx
	salq	$12, %rcx
	xorq	%rsi, %rcx
	orq	%rdi, %rcx
	sete	%cl
	xorl	%ebx, %ebx
	movzbl	%cl, %ecx
	andq	%rdx, %rbx
	andq	%rax, %rcx
.L1201:
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	testq	%r8, %r8
	jns	.L1193
	jmp	.L1197
	.p2align 4,,10
	.p2align 3
.L1149:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %r9
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r9, %rax
	subq	%rax, %rcx
	movq	%rsi, %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r9
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	movq	%rdi, %rdx
	movq	%rcx, %rsi
	orq	%rbx, %rsi
	je	.L1150
.L1157:
	movq	%rcx, %rsi
	xorl	%edi, %edi
	movl	$0, %ebx
	addq	%rcx, %rsi
	movl	$244140625, %ecx
	adcq	%rdi, %rdi
	salq	$12, %rcx
	cmpq	%rsi, %rcx
	movl	$0, %ecx
	sbbq	%rdi, %rcx
	movl	$1, %ecx
	jc	.L1158
	movl	$244140625, %ecx
	salq	$12, %rcx
	xorq	%rsi, %rcx
	orq	%rdi, %rcx
	sete	%cl
	xorl	%ebx, %ebx
	movzbl	%cl, %ecx
	andq	%rdx, %rbx
	andq	%rax, %rcx
.L1158:
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	testq	%r8, %r8
	jns	.L1150
	jmp	.L1154
	.p2align 4,,10
	.p2align 3
.L1267:
	movq	112(%rsp), %rdi
	movq	%xmm0, 16(%rsp)
	movq	%r8, %rsi
	movq	%xmm6, 24(%rsp)
	jmp	.L1268
.L1181:
	movq	112(%rsp), %rdi
	movq	%xmm0, 16(%rsp)
	movq	%r8, %rsi
	movq	%xmm4, 24(%rsp)
	jmp	.L1182
.L1278:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %r9
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	imulq	%r9, %rax
	subq	%rax, %rcx
	movq	%rdi, %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r9
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	movq	%rsi, %rdx
	movq	%rcx, %rsi
	orq	%rbx, %rsi
	je	.L1279
.L1286:
	movq	%rcx, %rsi
	xorl	%edi, %edi
	movl	$0, %ebx
	addq	%rcx, %rsi
	movl	$244140625, %ecx
	adcq	%rdi, %rdi
	salq	$12, %rcx
	cmpq	%rsi, %rcx
	movl	$0, %ecx
	sbbq	%rdi, %rcx
	movl	$1, %ecx
	jc	.L1287
	movl	$244140625, %ecx
	salq	$12, %rcx
	xorq	%rsi, %rcx
	orq	%rdi, %rcx
	sete	%cl
	xorl	%ebx, %ebx
	movzbl	%cl, %ecx
	andq	%rdx, %rbx
	andq	%rax, %rcx
.L1287:
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	testq	%r8, %r8
	jns	.L1279
	jmp	.L1283
	.p2align 4,,10
	.p2align 3
.L1875:
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	mulq	%rcx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movq	%rsi, %rbx
	imulq	%r10, %rax
	subq	%rax, %rcx
	movq	96(%rsp), %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r10
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rsi
	jmp	.L1350
.L1877:
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	mulq	%rcx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movq	%rsi, %rbx
	imulq	%r10, %rax
	subq	%rax, %rcx
	movq	96(%rsp), %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r10
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rsi
	jmp	.L1316
.L1224:
	movq	112(%rsp), %rdi
	movq	%xmm0, 16(%rsp)
	movq	%r8, %rsi
	movq	%xmm5, 24(%rsp)
	jmp	.L1225
.L1138:
	movq	112(%rsp), %rdi
	movq	%xmm0, 16(%rsp)
	movq	%r8, %rsi
	movq	%xmm3, 24(%rsp)
	jmp	.L1139
.L1235:
	movabsq	$2535301200456458803, %rax
	xorl	%ebx, %ebx
	movabsq	$1000000000000, %r9
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	imulq	%r9, %rax
	subq	%rax, %rcx
	movq	%rdi, %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r9
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	movq	%rsi, %rdx
	movq	%rcx, %rsi
	orq	%rbx, %rsi
	je	.L1236
.L1243:
	movq	%rcx, %rsi
	xorl	%edi, %edi
	movl	$0, %ebx
	addq	%rcx, %rsi
	movl	$244140625, %ecx
	adcq	%rdi, %rdi
	salq	$12, %rcx
	cmpq	%rsi, %rcx
	movl	$0, %ecx
	sbbq	%rdi, %rcx
	movl	$1, %ecx
	jc	.L1244
	movl	$244140625, %ecx
	salq	$12, %rcx
	xorq	%rsi, %rcx
	orq	%rdi, %rcx
	sete	%cl
	xorl	%ebx, %ebx
	movzbl	%cl, %ecx
	andq	%rdx, %rbx
	andq	%rax, %rcx
.L1244:
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	testq	%r8, %r8
	jns	.L1236
	jmp	.L1240
	.p2align 4,,10
	.p2align 3
.L1873:
	movabsq	$2535301200456458803, %rax
	xorl	%edi, %edi
	mulq	%rcx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movq	%rsi, %rbx
	imulq	%r10, %rax
	subq	%rax, %rcx
	movq	96(%rsp), %rax
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r10
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rsi
	jmp	.L1384
.L1252:
	mulq	%rsi
	xorl	%r9d, %r9d
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%rbp, %rax
	subq	%rax, %rsi
	movq	%r8, %rax
	movq	%rsi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %r8
	jmp	.L1253
.L1121:
	movq	%rsi, %rax
	xorl	%r9d, %r9d
	mulq	%r15
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%rbp, %rax
	subq	%rax, %rsi
	movq	%r8, %rax
	movq	%rsi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %r8
	jmp	.L1122
.L1166:
	mulq	%rsi
	xorl	%r9d, %r9d
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%rbp, %rax
	subq	%rax, %rsi
	movq	%r8, %rax
	movq	%rsi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %r8
	jmp	.L1167
.L1209:
	mulq	%rsi
	xorl	%r9d, %r9d
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%rbp, %rax
	subq	%rax, %rsi
	movq	%r8, %rax
	movq	%rsi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %rbp
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %r8
	jmp	.L1210
.L1153:
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r9
	mulq	%rbx
	xorl	%ebx, %ebx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r9, %rax
	subq	%rax, %rsi
	movq	%rcx, %rax
	movq	%rsi, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r9
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	movq	%rdi, %rdx
	movq	%rcx, %rsi
	orq	%rbx, %rsi
	je	.L1154
	jmp	.L1157
.L1196:
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r9
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r9, %rax
	subq	%rax, %rcx
	movq	%rbx, %rax
	xorl	%ebx, %ebx
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r9
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	movq	%rdi, %rdx
	movq	%rcx, %rsi
	orq	%rbx, %rsi
	je	.L1197
	jmp	.L1200
.L1282:
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r9
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r9, %rax
	subq	%rax, %rcx
	movq	%rbx, %rax
	xorl	%ebx, %ebx
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r9
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	movq	%rdi, %rdx
	movq	%rcx, %rsi
	orq	%rbx, %rsi
	je	.L1283
	jmp	.L1286
.L1239:
	movabsq	$2535301200456458803, %rax
	movabsq	$1000000000000, %r9
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	imulq	%r9, %rax
	subq	%rax, %rcx
	movq	%rbx, %rax
	xorl	%ebx, %ebx
	movq	%rcx, %rdx
#APP
# 103 "include/fixedwide/detail/mixed_native.hpp" 1
	divq %r9
# 0 "" 2
#NO_APP
	movq	%rdx, %rcx
	movq	%rdi, %rdx
	movq	%rcx, %rsi
	orq	%rbx, %rsi
	je	.L1240
	jmp	.L1243
.L1876:
	cmpq	%rdi, %rsi
	jne	.L1628
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1348
	testq	%rax, %rax
	je	.L1344
.L1348:
	movq	88(%rsp), %rax
	cmpq	%rax, %rbx
	je	.L1883
.L1627:
	movl	$1, %eax
.L1344:
	movzbl	%al, %eax
	addq	%rax, %rcx
	adcq	$0, %rbx
.L1346:
	cmpq	$0, 24(%rsp)
	jns	.L1342
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1342:
	movq	%rcx, 736(%rsp)
	movq	%rbx, 744(%rsp)
	movb	$1, 752(%rsp)
	jmp	.L1333
.L1859:
	cmpq	%rdi, %rsi
	jne	.L1632
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1417
	testq	%rax, %rax
	je	.L1413
.L1417:
	cmpq	%rbx, 24(%rsp)
	je	.L1884
.L1631:
	movl	$1, %eax
.L1413:
	movzbl	%al, %eax
	addq	%rax, %rcx
	adcq	$0, %rbx
.L1415:
	testq	%r10, %r10
	jns	.L1411
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1411:
	movq	%rcx, 688(%rsp)
	movq	%rbx, 696(%rsp)
	movb	$1, 704(%rsp)
	jmp	.L1421
.L1874:
	cmpq	%rdi, %rsi
	jne	.L1630
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1382
	testq	%rax, %rax
	je	.L1378
.L1382:
	movq	88(%rsp), %rax
	cmpq	%rax, %rbx
	je	.L1885
.L1629:
	movl	$1, %eax
.L1378:
	movzbl	%al, %eax
	addq	%rax, %rcx
	adcq	$0, %rbx
.L1380:
	cmpq	$0, 24(%rsp)
	jns	.L1376
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1376:
	movq	%rcx, 712(%rsp)
	movq	%rbx, 720(%rsp)
	movb	$1, 728(%rsp)
	jmp	.L1367
.L1878:
	cmpq	%rdi, %rsi
	jne	.L1626
	movq	%rcx, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1314
	testq	%rax, %rax
	je	.L1310
.L1314:
	movq	88(%rsp), %rax
	cmpq	%rax, %rbx
	je	.L1886
.L1625:
	movl	$1, %eax
.L1310:
	movzbl	%al, %eax
	addq	%rax, %rcx
	adcq	$0, %rbx
.L1312:
	cmpq	$0, 24(%rsp)
	jns	.L1308
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L1308:
	movq	%rcx, 760(%rsp)
	movq	%rbx, 768(%rsp)
	movb	$1, 776(%rsp)
	jmp	.L1299
.L1183:
	movq	%r11, %r9
	movq	%r10, %rax
	imulq	%rcx, %r9
	imulq	%r8, %rax
	addq	%rax, %r9
	movq	%r10, %rax
	mulq	%rcx
	movq	%rax, %rcx
	leaq	(%r9,%rdx), %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	leaq	1(%r11), %rcx
	cmpq	$1, %rcx
	ja	.L1179
	leaq	1(%r8), %rcx
	cmpq	$1, %rcx
	ja	.L1179
	cmpq	%r8, %r11
	jne	.L1187
	cmpq	%rax, %rbp
	movq	%rbp, %rdi
	sbbq	%rdx, %rdi
	jl	.L1178
.L1179:
	movl	$1, %ebp
	jmp	.L1178
.L1187:
	testq	%rdx, %rdx
	js	.L1178
	jmp	.L1179
.L1392:
	addq	$1, %r8
	adcq	$0, %r9
	jmp	.L1394
.L1140:
	movq	112(%rsp), %rax
	movq	%r11, %r10
	imulq	%rcx, %r10
	imulq	%r8, %rax
	addq	%rax, %r10
	movq	112(%rsp), %rax
	mulq	%rcx
	movq	%rax, %rcx
	leaq	(%r10,%rdx), %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	leaq	1(%r11), %rcx
	cmpq	$1, %rcx
	ja	.L1136
	leaq	1(%r8), %rcx
	cmpq	$1, %rcx
	ja	.L1136
	cmpq	%r8, %r11
	jne	.L1144
	cmpq	%rax, %r9
	movq	%r9, %rdi
	sbbq	%rdx, %rdi
	jl	.L1135
.L1136:
	movl	$1, %r9d
	jmp	.L1135
.L1144:
	testq	%rdx, %rdx
	js	.L1135
	jmp	.L1136
	.p2align 4,,10
	.p2align 3
.L1226:
	movq	%r11, %r9
	movq	%r10, %rax
	imulq	%rcx, %r9
	imulq	%r8, %rax
	addq	%rax, %r9
	movq	%r10, %rax
	mulq	%rcx
	movq	%rax, %rcx
	leaq	(%r9,%rdx), %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	leaq	1(%r11), %rcx
	cmpq	$1, %rcx
	ja	.L1222
	leaq	1(%r8), %rcx
	cmpq	$1, %rcx
	ja	.L1222
	cmpq	%r8, %r11
	jne	.L1230
	cmpq	%rax, %rbp
	movq	%rbp, %rdi
	sbbq	%rdx, %rdi
	jl	.L1221
.L1222:
	movl	$1, %ebp
	jmp	.L1221
.L1230:
	testq	%rdx, %rdx
	js	.L1221
	jmp	.L1222
	.p2align 4,,10
	.p2align 3
.L1269:
	movq	%r11, %r9
	movq	%r10, %rax
	imulq	%rcx, %r9
	imulq	%r8, %rax
	addq	%rax, %r9
	movq	%r10, %rax
	mulq	%rcx
	movq	%rax, %rcx
	leaq	(%r9,%rdx), %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	leaq	1(%r11), %rcx
	cmpq	$1, %rcx
	ja	.L1265
	leaq	1(%r8), %rcx
	cmpq	$1, %rcx
	ja	.L1265
	cmpq	%r8, %r11
	jne	.L1273
	cmpq	%rax, %rbp
	movq	%rbp, %rdi
	sbbq	%rdx, %rdi
	jl	.L1264
.L1265:
	movl	$1, %ebp
	jmp	.L1264
.L1273:
	testq	%rdx, %rdx
	js	.L1264
	jmp	.L1265
.L1879:
	cmpq	%r8, %rdi
	jne	.L1619
	movq	%rax, %rcx
	xorl	%ebx, %ebx
	andl	$1, %ecx
	testq	%rbx, %rbx
	jne	.L1214
	testq	%rcx, %rcx
	jne	.L1214
.L1215:
	movzbl	%cl, %ecx
	addq	%rcx, %rax
	adcq	$0, %rdx
.L1213:
	cmpq	$0, 24(%rsp)
	jns	.L1219
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
.L1219:
	movq	%rax, 736(%rsp)
	movq	%rdx, 744(%rsp)
	movb	$1, 752(%rsp)
	jmp	.L1220
.L1324:
	addq	$1, %r8
	adcq	$0, %r9
	jmp	.L1326
.L1880:
	cmpq	%r8, %rdi
	jne	.L1613
	movq	%rax, %rcx
	xorl	%ebx, %ebx
	andl	$1, %ecx
	testq	%rbx, %rbx
	jne	.L1127
	testq	%rcx, %rcx
	jne	.L1127
.L1128:
	movzbl	%cl, %ecx
	addq	%rcx, %rax
	adcq	$0, %rdx
.L1126:
	cmpq	$0, 24(%rsp)
	jns	.L1132
	negq	%rax
	adcq	$0, %rdx
	negq	%rdx
.L1132:
	movq	%rax, 688(%rsp)
	movq	%rdx, 696(%rsp)
	movb	$1, 704(%rsp)
	jmp	.L1133
.L1428:
	addq	$1, 16(%rsp)
	adcq	$0, 24(%rsp)
	jmp	.L1430
.L1358:
	addq	$1, %r8
	adcq	$0, %r9
	jmp	.L1360
.L1865:
	cmpq	%rsi, %rcx
	jne	.L1360
	addq	$1, %r8
	adcq	$0, %r9
	andq	$-2, %r8
	jmp	.L1360
.L1881:
	cmpq	%r8, %rdi
	jne	.L1616
	movq	%rax, %rcx
	xorl	%ebx, %ebx
	andl	$1, %ecx
	testq	%rbx, %rbx
	jne	.L1171
	testq	%rcx, %rcx
	je	.L1172
.L1171:
	cmpq	%rdx, 152(%rsp)
	je	.L1887
.L1617:
	movl	$1, %ecx
	jmp	.L1172
.L1853:
	cmpq	%rsi, %rcx
	jne	.L1430
	movq	16(%rsp), %rax
	movq	24(%rsp), %rdx
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	movq	%rax, 16(%rsp)
	movq	%rdx, 24(%rsp)
	jmp	.L1430
.L1194:
	testq	%rdi, %rdi
	jne	.L1193
	cmpq	%r8, %rsi
	jne	.L1193
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L1193
.L1151:
	testq	%rdi, %rdi
	jne	.L1150
	cmpq	%r8, %rsi
	jne	.L1150
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L1150
.L1868:
	cmpq	%rsi, %rcx
	jne	.L1326
	addq	$1, %r8
	adcq	$0, %r9
	andq	$-2, %r8
	jmp	.L1326
.L1862:
	cmpq	%rsi, %rcx
	jne	.L1394
	addq	$1, %r8
	adcq	$0, %r9
	andq	$-2, %r8
	jmp	.L1394
.L1198:
	testq	%rdi, %rdi
	jne	.L1197
	cmpq	%r8, %rsi
	jne	.L1197
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L1197
.L1127:
	cmpq	%rdx, 152(%rsp)
	je	.L1888
.L1614:
	movl	$1, %ecx
	jmp	.L1128
.L1214:
	cmpq	%rdx, 152(%rsp)
	je	.L1889
.L1620:
	movl	$1, %ecx
	jmp	.L1215
.L1839:
	call	__stack_chk_fail@PLT
.L1241:
	testq	%rdi, %rdi
	jne	.L1240
	cmpq	%r8, %rsi
	jne	.L1240
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L1240
.L1229:
	movq	112(%rsp), %rdi
	movq	120(%rsp), %rsi
	movq	%rdi, %rax
	imulq	%rcx, %rsi
	imulq	%rbx, %rax
	addq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%rcx
	movq	%rax, %rcx
	leaq	(%rsi,%rdx), %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	jmp	.L1222
.L1882:
	cmpq	%r8, %rdi
	jne	.L1622
	movq	%rax, %rcx
	xorl	%ebx, %ebx
	andl	$1, %ecx
	testq	%rbx, %rbx
	jne	.L1257
	testq	%rcx, %rcx
	je	.L1258
.L1257:
	cmpq	%rdx, 152(%rsp)
	je	.L1890
.L1623:
	movl	$1, %ecx
	jmp	.L1258
.L1237:
	testq	%rdi, %rdi
	jne	.L1236
	cmpq	%r8, %rsi
	jne	.L1236
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L1236
.L1186:
	movq	112(%rsp), %rdi
	movq	120(%rsp), %rsi
	movq	%rdi, %rax
	imulq	%rcx, %rsi
	imulq	%rbx, %rax
	addq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%rcx
	movq	%rax, %rcx
	leaq	(%rsi,%rdx), %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	jmp	.L1179
.L1284:
	testq	%rdi, %rdi
	jne	.L1283
	cmpq	%r8, %rsi
	jne	.L1283
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L1283
.L1155:
	testq	%rbx, %rbx
	jne	.L1154
	cmpq	%rdi, %rcx
	jne	.L1154
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L1154
.L1143:
	movq	112(%rsp), %rdi
	movq	120(%rsp), %rsi
	movq	%rdi, %rax
	imulq	%rcx, %rsi
	imulq	%rbx, %rax
	addq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%rcx
	movq	%rax, %rcx
	leaq	(%rsi,%rdx), %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	jmp	.L1136
.L1272:
	movq	112(%rsp), %rdi
	movq	120(%rsp), %rsi
	movq	%rdi, %rax
	imulq	%rcx, %rsi
	imulq	%rbx, %rax
	addq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%rcx
	movq	%rax, %rcx
	leaq	(%rsi,%rdx), %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	jmp	.L1265
.L1280:
	testq	%rdi, %rdi
	jne	.L1279
	cmpq	%r8, %rsi
	jne	.L1279
	addq	$1, %rax
	adcq	$0, %rdx
	andq	$-2, %rax
	jmp	.L1279
	.cfi_endproc
.LFE1694:
	.size	_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_, .-_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.section	.text._ZN9fixedwide6detail5batch14multiply_splitINS_11basic_fixedILm128ELj12EEENS1_16prepared_divisorEEESt8expectedIT_NS_15ArithmeticErrorEEombNS_8RoundingERKT0_,"axG",@progbits,_ZN9fixedwide6detail5batch14multiply_splitINS_11basic_fixedILm128ELj12EEENS1_16prepared_divisorEEESt8expectedIT_NS_15ArithmeticErrorEEombNS_8RoundingERKT0_,comdat
	.p2align 4
	.weak	_ZN9fixedwide6detail5batch14multiply_splitINS_11basic_fixedILm128ELj12EEENS1_16prepared_divisorEEESt8expectedIT_NS_15ArithmeticErrorEEombNS_8RoundingERKT0_
	.type	_ZN9fixedwide6detail5batch14multiply_splitINS_11basic_fixedILm128ELj12EEENS1_16prepared_divisorEEESt8expectedIT_NS_15ArithmeticErrorEEombNS_8RoundingERKT0_, @function
_ZN9fixedwide6detail5batch14multiply_splitINS_11basic_fixedILm128ELj12EEENS1_16prepared_divisorEEESt8expectedIT_NS_15ArithmeticErrorEEombNS_8RoundingERKT0_:
.LFB1961:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movq	%rcx, %r11
	movq	$-1, %rax
	movzbl	%r8b, %ecx
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	addq	%rcx, %rax
	movq	%rdi, %r10
	movq	%rdx, %rdi
	movabsq	$9223372036854775807, %rdx
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	movq	%rdi, %r14
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	adcq	$0, %rdx
	movq	%rsi, %r12
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset 3, -48
	movq	48(%rsp), %rbx
	movq	(%rbx), %rcx
	movq	%rax, -32(%rsp)
	movq	%rdx, -24(%rsp)
	cmpb	$5, %r9b
	je	.L1892
	xorl	%eax, %eax
	movq	%rdi, %rdx
	orq	%rax, %rdx
	jne	.L1893
	imulq	%r11, %rdi
	movq	%rsi, %rax
	mulq	%r11
	movq	%rdi, %r12
	addq	%rdx, %r12
	movq	%rax, %rsi
	movq	%r12, %rdi
	cmpq	$1, %rcx
	je	.L1987
	movq	%rax, %r11
	testq	%r12, %r12
	jne	.L1897
	movq	8(%rbx), %rax
	xorl	%edi, %edi
	mulq	%rsi
	movq	%r11, %rax
	movq	%rdx, %rsi
	imulq	%rcx, %rdx
	subq	%rdx, %rax
	xorl	%edx, %edx
	cmpq	%rcx, %rax
	setnb	%dl
	addq	%rdx, %rsi
	movq	%rax, %rdx
	adcq	$0, %rdi
	subq	%rcx, %rdx
	cmpq	%rcx, %rax
	cmovnb	%rdx, %rax
	xorl	%edx, %edx
.L1899:
	movq	%rax, %r11
	orq	%rdx, %r11
	je	.L1929
	cmpb	$3, %r9b
	je	.L1903
	ja	.L1904
.L1986:
	movl	%r8d, %ecx
	cmpb	$1, %r9b
	je	.L1940
	xorl	$1, %ecx
	cmpb	$2, %r9b
	jne	.L1929
.L1940:
	movq	-32(%rsp), %r14
	movq	-24(%rsp), %r15
	movq	%r14, %rax
	movq	%r15, %rdx
	xorq	%rsi, %rax
	xorq	%rdi, %rdx
	orq	%rdx, %rax
	jne	.L1959
	testb	%cl, %cl
	jne	.L1947
.L1959:
	movzbl	%cl, %ecx
	addq	%rcx, %rsi
	adcq	$0, %rdi
	.p2align 4
	.p2align 3
.L1929:
	testb	%r8b, %r8b
	jne	.L1988
.L1949:
	movq	%rsi, (%r10)
	movq	%rdi, 8(%r10)
	movb	$1, 16(%r10)
.L1891:
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	movq	%r10, %rax
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L1893:
	.cfi_restore_state
	movq	%rdi, %r15
	cmpq	$1, %rcx
	je	.L1914
.L1915:
	xorl	%esi, %esi
	xorl	%edi, %edi
	cmpq	%rcx, %r15
	jnb	.L1989
.L1920:
	movq	%r12, %rax
	xorl	%r13d, %r13d
	movq	%r15, %rdx
	movq	$0, -8(%rsp)
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rcx
# 0 "" 2
#NO_APP
	movq	%rax, %r12
	movq	%rdx, -16(%rsp)
	orq	%rdi, %r13
	orq	%rsi, %r12
	movq	%r13, %r14
	movq	%r12, %rsi
.L1917:
	movq	%rsi, %rax
	xorl	%edi, %edi
	mulq	%r11
	movq	%rax, %r12
	movq	%r14, %rax
	movq	%rdx, %r13
	mulq	%r11
	movq	%r13, %rsi
	addq	%rax, %rsi
	adcq	%rdx, %rdi
	xorl	%edx, %edx
	movq	%rdi, %rax
	orq	%rdx, %rax
	jne	.L1947
.L1919:
	movq	-32(%rsp), %rax
	movq	%rsi, %rdi
	movq	-24(%rsp), %rdx
	xorl	%esi, %esi
	xorl	%r13d, %r13d
	orq	%r12, %rsi
	orq	%r13, %rdi
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1947
	movq	-16(%rsp), %r14
	movq	-8(%rsp), %r15
	movq	%r14, %rax
	orq	%r15, %rax
	je	.L1929
	movq	-16(%rsp), %rax
	mulq	%r11
	movq	%rax, %r12
	movq	%rdx, %r13
	cmpq	$1, %rcx
	je	.L1990
	movq	%rdx, %r11
	movq	%rax, %r14
	testq	%rdx, %rdx
	je	.L1991
	xorl	%r14d, %r14d
	xorl	%r15d, %r15d
	cmpq	%rcx, %rdx
	jb	.L1933
	movq	8(%rbx), %rax
	mulq	%rdx
	movq	%rcx, %rax
	imulq	%rdx, %rax
	subq	%rax, %r11
	xorl	%eax, %eax
	cmpq	%rcx, %r11
	movq	%r11, %rbx
	setnb	%al
	addq	%rdx, %rax
	subq	%rcx, %rbx
	cmpq	%rcx, %r11
	movq	%rax, %rdx
	cmovnb	%rbx, %r11
	xorl	%eax, %eax
	movq	%rax, %r14
	movq	%rdx, %r15
.L1933:
	movq	%r11, %rdx
	movq	%r12, %rax
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rcx
# 0 "" 2
#NO_APP
	movq	%rdx, %r11
	xorl	%edx, %edx
	orq	%r14, %rax
	orq	%r15, %rdx
	movq	%r11, %r14
	xorl	%r15d, %r15d
.L1932:
	movq	-32(%rsp), %r12
	movq	-24(%rsp), %r13
	subq	%rsi, %r12
	sbbq	%rdi, %r13
	cmpq	%rax, %r12
	movq	%r13, %rbx
	sbbq	%rdx, %rbx
	jc	.L1947
	addq	%rax, %rsi
	movq	-32(%rsp), %rax
	adcq	%rdx, %rdi
	movq	-24(%rsp), %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%rdi, %rax
	jc	.L1947
	movq	%r14, %rax
	orq	%r15, %rax
	je	.L1929
	cmpb	$5, %r9b
	je	.L1992
	cmpb	$3, %r9b
	je	.L1938
	jbe	.L1986
	movq	%r14, %rax
	cmpb	$4, %r9b
	jne	.L1929
	.p2align 4
	.p2align 3
.L1982:
	xorl	%edx, %edx
	addq	%rax, %rax
	adcq	%rdx, %rdx
	xorl	%r13d, %r13d
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%r13, %rax
	setnc	%cl
	jmp	.L1940
	.p2align 4,,10
	.p2align 3
.L1987:
	movq	-32(%rsp), %rax
	movq	-24(%rsp), %rdx
	cmpq	%rsi, %rax
	movq	%rdx, %rax
	sbbq	%r12, %rax
	jnc	.L1929
.L1947:
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	movq	%r10, %rax
	popq	%r12
	.cfi_def_cfa_offset 32
	movb	$0, (%r10)
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	movb	$0, 16(%r10)
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L1892:
	.cfi_restore_state
	cmpq	$1, %rcx
	je	.L1914
	movq	%rdi, %r15
	testq	%rdi, %rdi
	jne	.L1915
	movq	8(%rbx), %rax
	imulq	8(%rbx), %rdi
	movq	$0, -8(%rsp)
	mulq	%rsi
	addq	%rdx, %rdi
	movq	%rdi, %rax
	movq	%rdi, %r12
	imulq	%rcx, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%rcx, %rsi
	setnb	%al
	addq	%rax, %r12
	movq	%rsi, %rax
	subq	%rcx, %rax
	cmpq	%rcx, %rsi
	cmovnb	%rax, %rsi
	movq	%r12, %rax
	mulq	%r11
	movq	%rsi, -16(%rsp)
	movq	%rax, %r12
	movq	%rdx, %rsi
	jmp	.L1919
	.p2align 4,,10
	.p2align 3
.L1914:
	movq	$0, -16(%rsp)
	movq	%r12, %rsi
	movl	$1, %ecx
	movq	$0, -8(%rsp)
	jmp	.L1917
	.p2align 4,,10
	.p2align 3
.L1988:
	negq	%rsi
	adcq	$0, %rdi
	negq	%rdi
	jmp	.L1949
	.p2align 4,,10
	.p2align 3
.L1989:
	movq	8(%rbx), %rax
	xorl	%esi, %esi
	mulq	%r14
	movq	%rcx, %rax
	imulq	%rdx, %rax
	subq	%rax, %r15
	cmpq	%rcx, %r15
	movq	%r15, %rax
	setnb	%sil
	addq	%rdx, %rsi
	subq	%rcx, %rax
	cmpq	%rcx, %r15
	cmovnb	%rax, %r15
	movq	%rsi, %rdi
	xorl	%esi, %esi
	jmp	.L1920
	.p2align 4,,10
	.p2align 3
.L1897:
	cmpq	%rcx, %r12
	jnb	.L1993
	movq	%r12, %rdx
	xorl	%edi, %edi
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rcx
# 0 "" 2
#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L1899
	.p2align 4,,10
	.p2align 3
.L1993:
	movq	8(%rbx), %rax
	xorl	%esi, %esi
	movq	-32(%rsp), %r14
	movq	-24(%rsp), %r15
	mulq	%r12
	movq	%rcx, %rax
	imulq	%rdx, %rax
	subq	%rax, %r12
	cmpq	%rcx, %r12
	movq	%r12, %rax
	setnb	%sil
	addq	%rdx, %rsi
	subq	%rcx, %rax
	cmpq	%rcx, %r12
	cmovnb	%rax, %r12
	movq	%rsi, %rdi
	movq	%r11, %rax
	movq	%r12, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %rcx
# 0 "" 2
#NO_APP
	cmpq	%rax, %r14
	movq	%rax, %rsi
	movq	%r15, %rax
	sbbq	%rdi, %rax
	jc	.L1947
	movq	%rdx, %rax
	xorl	%edx, %edx
	jmp	.L1899
	.p2align 4,,10
	.p2align 3
.L1991:
	movq	8(%rbx), %rax
	xorl	%r13d, %r13d
	xorl	%ebx, %ebx
	mulq	%r12
	movq	%rdx, %rax
	imulq	%rcx, %rax
	subq	%rax, %r14
	movq	%rdx, %rax
	movq	%r13, %rdx
	cmpq	%rcx, %r14
	movq	%r14, %r11
	setnb	%bl
	addq	%rbx, %rax
	movq	%r14, %rbx
	adcq	$0, %rdx
	subq	%rcx, %rbx
	cmpq	%rcx, %r14
	cmovnb	%rbx, %r11
	xorl	%r15d, %r15d
	movq	%r11, %r14
	jmp	.L1932
	.p2align 4,,10
	.p2align 3
.L1903:
	xorl	%edx, %edx
	addq	%rax, %rax
	adcq	%rdx, %rdx
	xorl	%r13d, %r13d
	movq	%rax, %r14
	cmpq	%rax, %rcx
	movq	%r13, %rax
	sbbq	%rdx, %rax
	jc	.L1957
	movq	%r14, %rax
	xorq	%r13, %rdx
	xorq	%rcx, %rax
	orq	%rdx, %rax
	jne	.L1929
	movq	%rsi, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	orq	%rdx, %rax
	je	.L1929
.L1957:
	movl	$1, %ecx
	jmp	.L1940
	.p2align 4,,10
	.p2align 3
.L1990:
	movq	-32(%rsp), %rcx
	movq	-24(%rsp), %rbx
	movq	%rcx, %rax
	movq	%rbx, %rdx
	subq	%rsi, %rax
	sbbq	%rdi, %rdx
	cmpq	%r12, %rax
	movq	%rdx, %rax
	sbbq	%r13, %rax
	jc	.L1947
	addq	%r12, %rsi
	movq	%rbx, %rax
	adcq	%r13, %rdi
	cmpq	%rsi, %rcx
	sbbq	%rdi, %rax
	jc	.L1947
	jmp	.L1929
	.p2align 4,,10
	.p2align 3
.L1904:
	cmpb	$4, %r9b
	je	.L1982
	jmp	.L1929
.L1992:
	movb	$2, (%r10)
	movb	$0, 16(%r10)
	jmp	.L1891
.L1938:
	movq	%r14, %rax
	xorl	%edx, %edx
	addq	%r14, %rax
	adcq	%rdx, %rdx
	xorl	%r13d, %r13d
	cmpq	%rax, %rcx
	movq	%r13, %rbx
	sbbq	%rdx, %rbx
	jc	.L1957
	testq	%rdx, %rdx
	jne	.L1929
	cmpq	%rcx, %rax
	jne	.L1929
	movq	%rsi, %rax
	xorl	%edx, %edx
	andl	$1, %eax
	testq	%rdx, %rdx
	jne	.L1957
	testq	%rax, %rax
	jne	.L1957
	jmp	.L1929
	.cfi_endproc
.LFE1961:
	.size	_ZN9fixedwide6detail5batch14multiply_splitINS_11basic_fixedILm128ELj12EEENS1_16prepared_divisorEEESt8expectedIT_NS_15ArithmeticErrorEEombNS_8RoundingERKT0_, .-_ZN9fixedwide6detail5batch14multiply_splitINS_11basic_fixedILm128ELj12EEENS1_16prepared_divisorEEESt8expectedIT_NS_15ArithmeticErrorEEombNS_8RoundingERKT0_
	.text
	.p2align 4
	.globl	_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.type	_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE, @function
_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE:
.LFB1704:
	.cfi_startproc
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	movq	%rdi, %r11
	movq	%rsi, %r10
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$472, %rsp
	.cfi_def_cfa_offset 528
	movq	%rdx, 56(%rsp)
	movq	%rcx, 16(%rsp)
	movq	%r8, 24(%rsp)
	movq	%r9, 32(%rsp)
	movq	%fs:40, %rbp
	movq	%rbp, 456(%rsp)
	movq	528(%rsp), %rbp
	movb	$5, 312(%rsp)
	movq	%rbp, 304(%rsp)
	movq	%rbp, %rax
	testq	%rbp, %rbp
	jns	.L1995
	negq	%rax
.L1995:
	movq	%rax, %r9
	movabsq	$-2601111570856684097, %rax
	mulq	%r9
	movq	%r9, %rcx
	movabsq	$10000000000, %rax
	shrq	$33, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	movq	%rax, %rdx
	movq	%rcx, %rax
	.p2align 4
	.p2align 4
	.p2align 3
.L1996:
	movq	%rdx, %rcx
	xorl	%edx, %edx
	divq	%rcx
	movq	%rcx, %rax
	testq	%rdx, %rdx
	jne	.L1996
	movq	$0, 8(%rsp)
	movq	%r9, %rax
	movq	%rdx, %rbx
	xorl	%edx, %edx
	divq	%rcx
	xorl	%edx, %edx
	movq	%rax, (%rsp)
	movq	%rax, %r9
	movq	%rax, 336(%rsp)
	movabsq	$10000000000, %rax
	divq	%rcx
	movq	%rax, %r15
	cmpq	$1, %rax
	je	.L2253
	xorl	%ecx, %ecx
	xorl	%edi, %edi
	movq	%rax, %rdx
	movl	$1, %esi
	shrq	$63, %rbp
	movq	%r9, 88(%rsp)
	movq	%r10, 64(%rsp)
	movq	%r11, 80(%rsp)
	call	__udivti3@PLT
	cmpq	$3, 32(%rsp)
	movq	%r15, 320(%rsp)
	movq	%rax, 40(%rsp)
	movq	80(%rsp), %r11
	movq	%rdx, 48(%rsp)
	movq	64(%rsp), %r10
	movq	%rax, 328(%rsp)
	movq	88(%rsp), %r9
	movb	%bpl, 204(%rsp)
	movb	%bpl, 344(%rsp)
	jbe	.L1999
	movq	%r11, 88(%rsp)
	movq	56(%rsp), %rbp
	movq	%r10, %r12
	xorl	%r13d, %r13d
	movq	%rbx, 160(%rsp)
	movq	%r10, 168(%rsp)
	movq	%r9, 192(%rsp)
.L2136:
	movq	(%r12), %rsi
	movq	8(%r12), %rdi
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	negq	%rcx
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rdi, %rdx
	cmovs	%rsi, %rcx
	movq	%rdx, %r10
	testq	%rdx, %rdx
	je	.L2000
	cmpq	%r15, %rdx
	jb	.L2001
	movq	40(%rsp), %rax
	xorl	%r8d, %r8d
	mulq	%rdx
	movq	%r15, %rax
	imulq	%rdx, %rax
	subq	%rax, %r10
	cmpq	%r15, %r10
	movq	%r10, %rax
	setnb	%r8b
	addq	%rdx, %r8
	subq	%r15, %rax
	cmpq	%r15, %r10
	cmovnb	%rax, %r10
	movq	%rcx, %rax
	xorl	%ebx, %ebx
	movq	%r10, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	(%rsp), %rax
	movq	%rdx, 64(%rsp)
	mulq	%rcx
	movq	%rdx, %r11
	movq	%rax, %r10
	movq	%r8, %rax
	mulq	(%rsp)
	movq	%r11, %rcx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	xorl	%edx, %edx
	movq	%rbx, %rax
	orq	%rdx, %rax
	je	.L2254
.L2012:
	movb	$0, 352(%rsp)
	movb	$0, 368(%rsp)
.L2008:
	movb	$0, 80(%rsp)
	jmp	.L2016
	.p2align 4,,10
	.p2align 3
.L2253:
	shrq	$63, %rbp
	xorl	%r13d, %r13d
	cmpq	$3, 32(%rsp)
	movq	$1, 320(%rsp)
	movb	%bpl, 204(%rsp)
	movq	56(%rsp), %r12
	movb	%bpl, 344(%rsp)
	movq	%r10, %rbp
	movq	$0, 328(%rsp)
	jbe	.L1999
	movq	%rbx, 48(%rsp)
	movq	%r11, 40(%rsp)
	movq	%r9, 64(%rsp)
	movq	%r10, 80(%rsp)
	movq	(%rsp), %r10
.L2181:
	movq	0(%rbp), %rsi
	movq	8(%rbp), %rdi
	movq	%r10, %rax
	movq	%rsi, %rcx
	movq	%rdi, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rsi, %rcx
	cmovs	%rdi, %r8
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rax, %rcx
	movq	%r8, %rax
	movq	%rdx, %rbx
	mulq	%r10
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	jne	.L2137
	movq	%rdi, %rax
	movq	%r8, %r9
	xorl	%ebx, %ebx
	xorl	%r8d, %r8d
	orq	%r8, %rcx
	orq	%r9, %rbx
	xorl	%edx, %edx
	shrq	$63, %rax
	cmpb	204(%rsp), %al
	movabsq	$-9223372036854775808, %rdi
	sete	%dl
	xorl	%esi, %esi
	subq	%rdx, %rsi
	sbbq	$0, %rdi
	cmpq	%rcx, %rsi
	sbbq	%rbx, %rdi
	jnc	.L2138
.L2137:
	movb	$0, 352(%rsp)
	xorl	%r15d, %r15d
	movb	$0, 368(%rsp)
.L2139:
	movq	16(%rbp), %rsi
	movq	24(%rbp), %rdi
	leaq	1(%r13), %rax
	movq	%rax, (%rsp)
	movq	%r10, %rax
	movq	%rsi, %rcx
	movq	%rdi, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rsi, %rcx
	cmovs	%rdi, %r8
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rax, %rcx
	movq	%r8, %rax
	movq	%rdx, %rbx
	mulq	%r10
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	jne	.L2141
	movq	%rdi, %rax
	movq	%r8, %r9
	xorl	%ebx, %ebx
	xorl	%r8d, %r8d
	orq	%r8, %rcx
	orq	%r9, %rbx
	xorl	%edx, %edx
	shrq	$63, %rax
	cmpb	204(%rsp), %al
	movabsq	$-9223372036854775808, %rdi
	sete	%dl
	xorl	%esi, %esi
	subq	%rdx, %rsi
	sbbq	$0, %rdi
	cmpq	%rcx, %rsi
	sbbq	%rbx, %rdi
	jnc	.L2142
.L2141:
	movb	$0, 376(%rsp)
	movb	$0, 392(%rsp)
.L2143:
	movq	32(%rbp), %rsi
	movq	40(%rbp), %rdi
	movq	%r10, %rax
	movq	%rsi, %rcx
	movq	%rdi, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rsi, %rcx
	cmovs	%rdi, %r8
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rax, %rcx
	movq	%r8, %rax
	movq	%rdx, %rbx
	mulq	%r10
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	jne	.L2145
	movq	%rdi, %rax
	movq	%r8, %r9
	xorl	%ebx, %ebx
	xorl	%r8d, %r8d
	orq	%r8, %rcx
	orq	%r9, %rbx
	xorl	%edx, %edx
	shrq	$63, %rax
	cmpb	204(%rsp), %al
	movabsq	$-9223372036854775808, %rdi
	sete	%dl
	xorl	%esi, %esi
	subq	%rdx, %rsi
	sbbq	$0, %rdi
	cmpq	%rcx, %rsi
	sbbq	%rbx, %rdi
	jnc	.L2146
.L2145:
	movb	$0, 400(%rsp)
	movb	$0, 416(%rsp)
.L2147:
	movq	48(%rbp), %rsi
	movq	56(%rbp), %rdi
	movq	%r10, %rax
	movq	%rsi, %rcx
	movq	%rdi, %r8
	negq	%rcx
	adcq	$0, %r8
	negq	%r8
	cmovs	%rsi, %rcx
	cmovs	%rdi, %r8
	xorl	%r9d, %r9d
	mulq	%rcx
	movq	%rax, %rcx
	movq	%r8, %rax
	movq	%rdx, %rbx
	mulq	%r10
	movq	%rbx, %r8
	addq	%rax, %r8
	adcq	%rdx, %r9
	xorl	%edx, %edx
	movq	%r9, %rax
	orq	%rdx, %rax
	jne	.L2149
	movq	%rdi, %rax
	movq	%r8, %r9
	xorl	%ebx, %ebx
	xorl	%r8d, %r8d
	orq	%r8, %rcx
	orq	%r9, %rbx
	xorl	%edx, %edx
	shrq	$63, %rax
	cmpb	204(%rsp), %al
	movabsq	$-9223372036854775808, %rdi
	sete	%dl
	xorl	%esi, %esi
	subq	%rdx, %rsi
	sbbq	$0, %rdi
	cmpq	%rcx, %rsi
	sbbq	%rbx, %rdi
	jnc	.L2150
.L2149:
	movb	$0, 424(%rsp)
	movb	$0, 440(%rsp)
.L2151:
	testb	%r15b, %r15b
	je	.L2241
	movq	352(%rsp), %rcx
	movq	(%r12), %rax
	movq	360(%rsp), %rbx
	movq	8(%r12), %rdx
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	jo	.L2155
	movq	%rax, 240(%rsp)
	movq	24(%rsp), %rax
	movq	%rdx, 248(%rsp)
	movl	(%rax,%r13,4), %esi
	movq	16(%rsp), %rax
	salq	$4, %rsi
	addq	%rax, %rsi
	movq	(%rsi), %rax
	movq	8(%rsi), %rdx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jno	.L2158
.L2155:
	xorl	%eax, %eax
.L2159:
	movl	%r14d, %ebx
	movb	%al, %bh
	movl	%ebx, %eax
	movl	%ebx, %r14d
	shrw	$8, %ax
	je	.L2242
	cmpb	$0, 392(%rsp)
	je	.L2207
	movq	376(%rsp), %rcx
	movq	16(%r12), %rax
	movq	384(%rsp), %rbx
	movq	24(%r12), %rdx
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	jo	.L2162
	movq	%rax, 240(%rsp)
	movq	24(%rsp), %rax
	movq	%rdx, 248(%rsp)
	movl	4(%rax,%r13,4), %esi
	movq	16(%rsp), %rax
	salq	$4, %rsi
	addq	%rax, %rsi
	movq	(%rsi), %rax
	movq	8(%rsi), %rdx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jno	.L2165
.L2162:
	xorl	%eax, %eax
.L2166:
	movl	%r14d, %ebx
	movb	%al, %bh
	movl	%ebx, %eax
	movl	%ebx, %r14d
	shrw	$8, %ax
	je	.L2208
	cmpb	$0, 416(%rsp)
	leaq	2(%r13), %rsi
	je	.L2209
	movq	400(%rsp), %rax
	movq	32(%r12), %rcx
	movq	408(%rsp), %rdx
	movq	40(%r12), %rbx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jo	.L2169
	movq	%rbx, 248(%rsp)
	movq	24(%rsp), %rbx
	movq	%rcx, 240(%rsp)
	movl	8(%rbx,%r13,4), %edi
	movq	16(%rsp), %rbx
	salq	$4, %rdi
	addq	%rbx, %rdi
	movq	(%rdi), %rcx
	movq	8(%rdi), %rbx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jno	.L2172
.L2169:
	xorl	%eax, %eax
.L2173:
	movl	%r14d, %ebx
	movb	%al, %bh
	movl	%ebx, %eax
	movl	%ebx, %r14d
	shrw	$8, %ax
	je	.L2212
	cmpb	$0, 440(%rsp)
	leaq	3(%r13), %rsi
	je	.L2211
	movq	424(%rsp), %rax
	movq	48(%r12), %rcx
	movq	432(%rsp), %rdx
	movq	56(%r12), %rbx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jo	.L2176
	movq	%rbx, 248(%rsp)
	movq	24(%rsp), %rbx
	movq	%rcx, 240(%rsp)
	movl	12(%rbx,%r13,4), %edi
	movq	16(%rsp), %rbx
	salq	$4, %rdi
	addq	%rbx, %rdi
	movq	(%rdi), %rcx
	movq	8(%rdi), %rbx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jo	.L2176
	movdqa	240(%rsp), %xmm0
	movq	%rcx, 272(%rsp)
	movl	$1, %eax
	movq	%rbx, 280(%rsp)
	movups	%xmm0, 48(%r12)
	movdqa	272(%rsp), %xmm0
	movups	%xmm0, (%rdi)
.L2180:
	movl	%r14d, %ebx
	movb	%al, %bh
	movl	%ebx, %eax
	movl	%ebx, %r14d
	shrw	$8, %ax
	je	.L2212
	movq	32(%rsp), %rax
	addq	$4, %r13
	addq	$64, %rbp
	addq	$64, %r12
	subq	%r13, %rax
	cmpq	$3, %rax
	ja	.L2181
	movq	40(%rsp), %r11
	movq	80(%rsp), %r10
	movq	%r13, %rbx
	movq	64(%rsp), %r9
.L1999:
	movq	32(%rsp), %rax
	cmpq	%rax, %rbx
	jnb	.L2184
	movq	56(%rsp), %r13
	movq	%rbx, %rax
	movq	%r11, 40(%rsp)
	leaq	320(%rsp), %rbp
	salq	$4, %rax
	leaq	208(%rsp), %r12
	leaq	(%r10,%rax), %r14
	addq	%rax, %r13
	movzwl	206(%rsp), %eax
	movq	%r14, %r15
	movq	%rbp, %r14
	movq	%r9, %rbp
.L2195:
	movq	8(%r15), %r11
	movq	(%r15), %r10
	movl	%eax, (%rsp)
	movl	$5, %r9d
	movq	%r12, %rdi
	movq	%r11, %r8
	movq	%r10, %rcx
	movq	%r11, %rdx
	shrq	$63, %r8
	xorb	204(%rsp), %r8b
	negq	%rcx
	adcq	$0, %rdx
	movq	%rcx, %rsi
	movq	%rbp, %rcx
	negq	%rdx
	cmovs	%r11, %rdx
	cmovs	%r10, %rsi
	subq	$8, %rsp
	.cfi_def_cfa_offset 536
	andl	$1, %r8d
	pushq	%r14
	.cfi_def_cfa_offset 544
	call	_ZN9fixedwide6detail5batch14multiply_splitINS_11basic_fixedILm128ELj12EEENS1_16prepared_divisorEEESt8expectedIT_NS_15ArithmeticErrorEEombNS_8RoundingERKT0_
	popq	%rax
	.cfi_def_cfa_offset 536
	popq	%rdx
	.cfi_def_cfa_offset 528
	cmpb	$0, 224(%rsp)
	movl	(%rsp), %eax
	je	.L2255
	movq	208(%rsp), %rsi
	movq	0(%r13), %r8
	movq	216(%rsp), %rdi
	movq	8(%r13), %r9
	addq	%rsi, %r8
	adcq	%rdi, %r9
	jo	.L2193
	movq	24(%rsp), %rdx
	movq	16(%rsp), %rcx
	movq	%r8, 240(%rsp)
	movq	%r9, 248(%rsp)
	movl	(%rdx,%rbx,4), %edx
	salq	$4, %rdx
	addq	%rcx, %rdx
	movq	(%rdx), %r8
	movq	8(%rdx), %r9
	addq	%r8, %rsi
	adcq	%r9, %rdi
	jo	.L2193
	movdqa	240(%rsp), %xmm0
	movq	%rsi, 272(%rsp)
	movq	%rdi, 280(%rsp)
	movups	%xmm0, 0(%r13)
	movdqa	272(%rsp), %xmm0
	movups	%xmm0, (%rdx)
	movl	$1, %edx
.L2190:
	movb	%dl, %ah
	movl	%eax, %edi
	shrw	$8, %di
	je	.L2256
	addq	$1, %rbx
	addq	$16, %r15
	addq	$16, %r13
	cmpq	%rbx, 32(%rsp)
	jne	.L2195
	movq	40(%rsp), %r11
.L2184:
	movb	$1, 16(%r11)
	jmp	.L1994
	.p2align 4,,10
	.p2align 3
.L2000:
	movq	48(%rsp), %r9
	movq	40(%rsp), %rax
	xorl	%r10d, %r10d
	mulq	%rcx
	imulq	%rcx, %r9
	addq	%rdx, %r9
	movq	%r9, %rax
	imulq	%r15, %r9
	subq	%r9, %rcx
	cmpq	%r15, %rcx
	setnb	%r10b
	addq	%r10, %rax
	movq	%rax, %r8
	cmpq	%r15, %rcx
	jnb	.L2006
.L2247:
	movq	(%rsp), %rax
	movq	%rdi, %r10
	xorl	%ebx, %ebx
	movabsq	$9223372036854775807, %r9
	movq	$0, 144(%rsp)
	shrq	$63, %r10
	mulq	%r8
	movb	%r10b, 80(%rsp)
	xorb	204(%rsp), %r10b
	movq	$-1, %r8
	movzbl	%r10b, %r10d
	addq	%r10, %r8
	adcq	$0, %r9
	movq	%rdx, 152(%rsp)
	xorl	%edx, %edx
	orq	144(%rsp), %rax
	orq	152(%rsp), %rdx
	movq	%rax, %r10
	movq	%rdx, %r11
.L2199:
	movq	%rcx, %rax
	orq	%rbx, %rax
	jne	.L2257
.L2252:
	movzbl	204(%rsp), %ebx
	cmpb	%bl, 80(%rsp)
	je	.L2017
	negq	%r10
	adcq	$0, %r11
	negq	%r11
.L2017:
	movq	%r10, 352(%rsp)
	movq	%r11, 360(%rsp)
	movb	$1, 368(%rsp)
	movb	$1, 80(%rsp)
.L2016:
	movq	16(%r12), %rsi
	movq	24(%r12), %rdi
	leaq	1(%r13), %rax
	movq	%rax, 64(%rsp)
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	negq	%rcx
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rdi, %rdx
	cmovs	%rsi, %rcx
	movq	%rdx, %r10
	testq	%rdx, %rdx
	je	.L2028
	cmpq	%r15, %rdx
	jb	.L2029
	movq	40(%rsp), %rax
	xorl	%r8d, %r8d
	mulq	%rdx
	movq	%r15, %rax
	imulq	%rdx, %rax
	subq	%rax, %r10
	cmpq	%r15, %r10
	movq	%r10, %rax
	setnb	%r8b
	addq	%rdx, %r8
	subq	%r15, %rax
	cmpq	%r15, %r10
	cmovnb	%rax, %r10
	movq	%rcx, %rax
	movq	%r8, %r11
	xorl	%ebx, %ebx
	movq	%r10, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	(%rsp), %rax
	movq	%rdx, 176(%rsp)
	mulq	%rcx
	movq	%rdx, %r9
	movq	%rax, %r8
	movq	%r11, %rax
	mulq	(%rsp)
	movq	%r9, %rcx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	xorl	%edx, %edx
	movq	%rbx, %rax
	orq	%rdx, %rax
	je	.L2258
.L2040:
	movb	$0, 376(%rsp)
	movb	$0, 392(%rsp)
.L2036:
	movq	32(%r12), %rsi
	movq	40(%r12), %rdi
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	negq	%rcx
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rdi, %rdx
	cmovs	%rsi, %rcx
	movq	%rdx, %r10
	testq	%rdx, %rdx
	je	.L2054
	cmpq	%r15, %rdx
	jb	.L2055
	movq	40(%rsp), %rax
	xorl	%r8d, %r8d
	mulq	%rdx
	movq	%r15, %rax
	imulq	%rdx, %rax
	subq	%rax, %r10
	cmpq	%r15, %r10
	movq	%r10, %rax
	setnb	%r8b
	addq	%rdx, %r8
	subq	%r15, %rax
	cmpq	%r15, %r10
	cmovnb	%rax, %r10
	movq	%rcx, %rax
	movq	%r8, %r11
	xorl	%ebx, %ebx
	movq	%r10, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	(%rsp), %rax
	movq	%rdx, 176(%rsp)
	mulq	%rcx
	movq	%rdx, %r9
	movq	%rax, %r8
	movq	%r11, %rax
	mulq	(%rsp)
	movq	%r9, %rcx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	xorl	%edx, %edx
	movq	%rbx, %rax
	orq	%rdx, %rax
	je	.L2259
.L2066:
	movb	$0, 400(%rsp)
	movb	$0, 416(%rsp)
.L2062:
	movq	48(%r12), %rsi
	movq	56(%r12), %rdi
	movq	%rsi, %rcx
	movq	%rdi, %rdx
	negq	%rcx
	adcq	$0, %rdx
	negq	%rdx
	cmovs	%rdi, %rdx
	cmovs	%rsi, %rcx
	movq	%rdx, %r10
	testq	%rdx, %rdx
	je	.L2080
	cmpq	%r15, %rdx
	jb	.L2081
	movq	40(%rsp), %rax
	xorl	%r8d, %r8d
	mulq	%rdx
	movq	%r15, %rax
	imulq	%rdx, %rax
	subq	%rax, %r10
	cmpq	%r15, %r10
	movq	%r10, %rax
	setnb	%r8b
	addq	%rdx, %r8
	subq	%r15, %rax
	cmpq	%r15, %r10
	cmovnb	%rax, %r10
	movq	%rcx, %rax
	movq	%r8, %r11
	xorl	%ebx, %ebx
	movq	%r10, %rdx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	movq	%rax, %rcx
	movq	(%rsp), %rax
	movq	%rdx, 176(%rsp)
	mulq	%rcx
	movq	%rdx, %r9
	movq	%rax, %r8
	movq	%r11, %rax
	mulq	(%rsp)
	movq	%r9, %rcx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	xorl	%edx, %edx
	movq	%rbx, %rax
	orq	%rdx, %rax
	je	.L2260
.L2092:
	movb	$0, 424(%rsp)
	movb	$0, 440(%rsp)
.L2088:
	cmpb	$0, 80(%rsp)
	je	.L2239
.L2267:
	movq	352(%rsp), %rcx
	movq	0(%rbp), %rax
	movq	360(%rsp), %rbx
	movq	8(%rbp), %rdx
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	jo	.L2113
	movq	%rax, 240(%rsp)
	movq	24(%rsp), %rax
	movq	%rdx, 248(%rsp)
	movl	(%rax,%r13,4), %esi
	movq	16(%rsp), %rax
	salq	$4, %rsi
	addq	%rax, %rsi
	movq	(%rsi), %rax
	movq	8(%rsi), %rdx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jo	.L2113
	movq	%rbx, 280(%rsp)
	movl	$1, %eax
	movl	%r14d, %ebx
	movdqa	240(%rsp), %xmm0
	movb	%al, %bh
	movq	%rcx, 272(%rsp)
	movups	%xmm0, 0(%rbp)
	movl	%ebx, %eax
	movl	%ebx, %r14d
	movdqa	272(%rsp), %xmm0
	shrw	$8, %ax
	movups	%xmm0, (%rsi)
	je	.L2240
.L2261:
	cmpb	$0, 392(%rsp)
	je	.L2201
	movq	376(%rsp), %rcx
	movq	16(%rbp), %rax
	movq	384(%rsp), %rbx
	movq	24(%rbp), %rdx
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	jo	.L2121
	movq	%rax, 240(%rsp)
	movq	24(%rsp), %rax
	movq	%rdx, 248(%rsp)
	movl	4(%rax,%r13,4), %esi
	movq	16(%rsp), %rax
	salq	$4, %rsi
	addq	%rax, %rsi
	movq	(%rsi), %rax
	movq	8(%rsi), %rdx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jo	.L2121
	movq	%rbx, 280(%rsp)
	movl	$1, %eax
	movl	%r14d, %ebx
	movdqa	240(%rsp), %xmm0
	movb	%al, %bh
	movq	%rcx, 272(%rsp)
	movups	%xmm0, 16(%rbp)
	movl	%ebx, %eax
	movl	%ebx, %r14d
	movdqa	272(%rsp), %xmm0
	shrw	$8, %ax
	movups	%xmm0, (%rsi)
	je	.L2202
.L2262:
	cmpb	$0, 416(%rsp)
	leaq	2(%r13), %rsi
	je	.L2203
	movq	400(%rsp), %rcx
	movq	32(%rbp), %rax
	movq	408(%rsp), %rbx
	movq	40(%rbp), %rdx
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	jo	.L2128
	movq	%rax, 240(%rsp)
	movq	24(%rsp), %rax
	movq	%rdx, 248(%rsp)
	movl	8(%rax,%r13,4), %edi
	movq	16(%rsp), %rax
	salq	$4, %rdi
	addq	%rax, %rdi
	movq	(%rdi), %rax
	movq	8(%rdi), %rdx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jo	.L2128
	movdqa	240(%rsp), %xmm0
	movq	%rcx, 272(%rsp)
	movl	$1, %eax
	movq	%rbx, 280(%rsp)
	movups	%xmm0, 32(%rbp)
	movdqa	272(%rsp), %xmm0
	movups	%xmm0, (%rdi)
.L2125:
	movl	%r14d, %ebx
	movb	%al, %bh
	movl	%ebx, %eax
	movl	%ebx, %r14d
	shrw	$8, %ax
	je	.L2206
	cmpb	$0, 440(%rsp)
	leaq	3(%r13), %rsi
	je	.L2205
	movq	424(%rsp), %rcx
	movq	48(%rbp), %rax
	movq	432(%rsp), %rbx
	movq	56(%rbp), %rdx
	addq	%rcx, %rax
	adcq	%rbx, %rdx
	jo	.L2135
	movq	%rax, 240(%rsp)
	movq	24(%rsp), %rax
	movq	%rdx, 248(%rsp)
	movl	12(%rax,%r13,4), %edi
	movq	16(%rsp), %rax
	salq	$4, %rdi
	addq	%rax, %rdi
	movq	(%rdi), %rax
	movq	8(%rdi), %rdx
	addq	%rax, %rcx
	adcq	%rdx, %rbx
	jo	.L2135
	movdqa	240(%rsp), %xmm0
	movq	%rcx, 272(%rsp)
	movl	$1, %eax
	movq	%rbx, 280(%rsp)
	movups	%xmm0, 48(%rbp)
	movdqa	272(%rsp), %xmm0
	movups	%xmm0, (%rdi)
.L2132:
	movl	%r14d, %ebx
	movb	%al, %bh
	movl	%ebx, %eax
	movl	%ebx, %r14d
	shrw	$8, %ax
	je	.L2206
	movq	32(%rsp), %rax
	addq	$4, %r13
	addq	$64, %r12
	addq	$64, %rbp
	subq	%r13, %rax
	cmpq	$3, %rax
	ja	.L2136
	movq	88(%rsp), %r11
	movq	168(%rsp), %r10
	movq	%r13, %rbx
	movq	192(%rsp), %r9
	jmp	.L1999
	.p2align 4,,10
	.p2align 3
.L2176:
	xorl	%eax, %eax
	jmp	.L2180
	.p2align 4,,10
	.p2align 3
.L2158:
	movdqa	240(%rsp), %xmm0
	movq	%rcx, 272(%rsp)
	movl	$1, %eax
	movq	%rbx, 280(%rsp)
	movups	%xmm0, (%r12)
	movdqa	272(%rsp), %xmm0
	movups	%xmm0, (%rsi)
	jmp	.L2159
	.p2align 4,,10
	.p2align 3
.L2150:
	cmpb	204(%rsp), %al
	je	.L2152
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L2152:
	movq	%rcx, 424(%rsp)
	movq	%rbx, 432(%rsp)
	movb	$1, 440(%rsp)
	jmp	.L2151
	.p2align 4,,10
	.p2align 3
.L2146:
	cmpb	204(%rsp), %al
	je	.L2148
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L2148:
	movq	%rcx, 400(%rsp)
	movq	%rbx, 408(%rsp)
	movb	$1, 416(%rsp)
	jmp	.L2147
	.p2align 4,,10
	.p2align 3
.L2142:
	cmpb	204(%rsp), %al
	je	.L2144
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L2144:
	movq	%rcx, 376(%rsp)
	movq	%rbx, 384(%rsp)
	movb	$1, 392(%rsp)
	jmp	.L2143
	.p2align 4,,10
	.p2align 3
.L2138:
	cmpb	204(%rsp), %al
	je	.L2140
	negq	%rcx
	adcq	$0, %rbx
	negq	%rbx
.L2140:
	movq	%rcx, 352(%rsp)
	movl	$1, %r15d
	movq	%rbx, 360(%rsp)
	movb	$1, 368(%rsp)
	jmp	.L2139
	.p2align 4,,10
	.p2align 3
.L2113:
	xorl	%eax, %eax
	movl	%r14d, %ebx
	movb	%al, %bh
	movl	%ebx, %eax
	movl	%ebx, %r14d
	shrw	$8, %ax
	jne	.L2261
.L2240:
	movq	88(%rsp), %r11
	jmp	.L2114
	.p2align 4,,10
	.p2align 3
.L2121:
	xorl	%eax, %eax
	movl	%r14d, %ebx
	movb	%al, %bh
	movl	%ebx, %eax
	movl	%ebx, %r14d
	shrw	$8, %ax
	jne	.L2262
.L2202:
	movq	88(%rsp), %r11
	movq	64(%rsp), %r13
	jmp	.L2114
	.p2align 4,,10
	.p2align 3
.L2128:
	xorl	%eax, %eax
	jmp	.L2125
	.p2align 4,,10
	.p2align 3
.L2135:
	xorl	%eax, %eax
	jmp	.L2132
	.p2align 4,,10
	.p2align 3
.L2001:
	movq	%rcx, %rax
	movq	%rdi, %rsi
	movq	$-1, %r8
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	movq	%rax, %r10
	movq	(%rsp), %rax
	movq	%rdx, %rcx
	shrq	$63, %rsi
	movabsq	$9223372036854775807, %r9
	movb	%sil, 80(%rsp)
	xorb	204(%rsp), %sil
	mulq	%r10
	movzbl	%sil, %esi
	addq	%rsi, %r8
	adcq	$0, %r9
	movq	%rax, %r10
	movq	%rcx, %rax
	movq	%rdx, %r11
	orq	%rbx, %rax
	je	.L2252
.L2257:
	movq	(%rsp), %rax
	mulq	%rcx
	movq	%rax, %rcx
	movq	%rdx, %rdi
	movq	%rax, %rsi
	testq	%rdx, %rdx
	jne	.L2263
	movq	48(%rsp), %rdi
	xorl	%ebx, %ebx
	imulq	%rax, %rdi
	movq	40(%rsp), %rax
	mulq	%rcx
	addq	%rdx, %rdi
	movq	%rdi, %rax
	movq	%rdi, %rcx
	imulq	%r15, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r15, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%r15, %rax
	cmpq	%r15, %rsi
	cmovnb	%rax, %rsi
	xorl	%edi, %edi
.L2019:
	movq	%r8, %rax
	movq	%r9, %rdx
	subq	%r10, %rax
	sbbq	%r11, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L2012
	addq	%rcx, %r10
	movq	%r9, %rax
	adcq	%rbx, %r11
	cmpq	%r10, %r8
	sbbq	%r11, %rax
	jc	.L2012
	movq	%rsi, %rcx
	orq	%rdi, %rcx
	je	.L2252
	movb	$2, 352(%rsp)
	movb	$0, 368(%rsp)
	jmp	.L2008
	.p2align 4,,10
	.p2align 3
.L2006:
	subq	%r15, %rcx
	jmp	.L2247
	.p2align 4,,10
	.p2align 3
.L2165:
	movdqa	240(%rsp), %xmm0
	movq	%rcx, 272(%rsp)
	movl	$1, %eax
	movq	%rbx, 280(%rsp)
	movups	%xmm0, 16(%r12)
	movdqa	272(%rsp), %xmm0
	movups	%xmm0, (%rsi)
	jmp	.L2166
	.p2align 4,,10
	.p2align 3
.L2172:
	movdqa	240(%rsp), %xmm0
	movq	%rcx, 272(%rsp)
	movl	$1, %eax
	movq	%rbx, 280(%rsp)
	movups	%xmm0, 32(%r12)
	movdqa	272(%rsp), %xmm0
	movups	%xmm0, (%rdi)
	jmp	.L2173
	.p2align 4,,10
	.p2align 3
.L2028:
	movq	48(%rsp), %r9
	movq	40(%rsp), %rax
	xorl	%r10d, %r10d
	mulq	%rcx
	imulq	%rcx, %r9
	addq	%rdx, %r9
	movq	%r9, %rax
	imulq	%r15, %r9
	subq	%r9, %rcx
	cmpq	%r15, %rcx
	setnb	%r10b
	addq	%r10, %rax
	movq	%rax, %r8
	cmpq	%r15, %rcx
	jb	.L2246
	subq	%r15, %rcx
.L2246:
	movq	(%rsp), %rax
	movq	%rdi, %r9
	movq	$-1, %r10
	xorl	%ebx, %ebx
	movabsq	$9223372036854775807, %r11
	shrq	$63, %r9
	movq	$0, 128(%rsp)
	mulq	%r8
	movb	%r9b, 205(%rsp)
	xorb	204(%rsp), %r9b
	movzbl	%r9b, %r9d
	addq	%r9, %r10
	movq	%rdx, %r8
	adcq	$0, %r11
	xorl	%edx, %edx
	orq	128(%rsp), %rax
	movq	%r8, 136(%rsp)
	orq	136(%rsp), %rdx
	movq	%rax, %r8
	movq	%rdx, %r9
.L2198:
	movq	%rcx, %rax
	orq	%rbx, %rax
	je	.L2037
	movq	(%rsp), %rax
	mulq	%rcx
	movq	%rax, %rcx
	movq	%rdx, %rdi
	movq	%rax, %rsi
	testq	%rdx, %rdx
	jne	.L2264
	movq	48(%rsp), %rdi
	xorl	%ebx, %ebx
	imulq	%rax, %rdi
	movq	40(%rsp), %rax
	mulq	%rcx
	addq	%rdx, %rdi
	movq	%rdi, %rax
	movq	%rdi, %rcx
	imulq	%r15, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r15, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%r15, %rax
	cmpq	%r15, %rsi
	cmovnb	%rax, %rsi
	xorl	%edi, %edi
.L2046:
	movq	%r10, %rax
	movq	%r11, %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L2040
	addq	%rcx, %r8
	movq	%r11, %rax
	adcq	%rbx, %r9
	cmpq	%r8, %r10
	sbbq	%r9, %rax
	jc	.L2040
	movq	%rsi, %rcx
	orq	%rdi, %rcx
	je	.L2037
	movb	$2, 376(%rsp)
	movb	$0, 392(%rsp)
	jmp	.L2036
	.p2align 4,,10
	.p2align 3
.L2054:
	movq	48(%rsp), %r9
	movq	40(%rsp), %rax
	xorl	%r10d, %r10d
	mulq	%rcx
	imulq	%rcx, %r9
	addq	%rdx, %r9
	movq	%r9, %rax
	imulq	%r15, %r9
	subq	%r9, %rcx
	cmpq	%r15, %rcx
	setnb	%r10b
	addq	%r10, %rax
	movq	%rax, %r8
	cmpq	%r15, %rcx
	jb	.L2245
	subq	%r15, %rcx
.L2245:
	movq	(%rsp), %rax
	movq	%rdi, %r9
	movq	$-1, %r10
	xorl	%ebx, %ebx
	movabsq	$9223372036854775807, %r11
	shrq	$63, %r9
	movq	$0, 112(%rsp)
	mulq	%r8
	movb	%r9b, 205(%rsp)
	xorb	204(%rsp), %r9b
	movzbl	%r9b, %r9d
	addq	%r9, %r10
	adcq	$0, %r11
	movq	%rdx, 120(%rsp)
	orq	112(%rsp), %rax
	xorl	%edx, %edx
	orq	120(%rsp), %rdx
	movq	%rax, %r8
	movq	%rdx, %r9
.L2197:
	movq	%rcx, %rax
	orq	%rbx, %rax
	je	.L2063
	movq	(%rsp), %rax
	mulq	%rcx
	movq	%rax, %rcx
	movq	%rdx, %rdi
	movq	%rax, %rsi
	testq	%rdx, %rdx
	jne	.L2265
	movq	48(%rsp), %rdi
	xorl	%ebx, %ebx
	imulq	%rax, %rdi
	movq	40(%rsp), %rax
	mulq	%rcx
	addq	%rdx, %rdi
	movq	%rdi, %rax
	movq	%rdi, %rcx
	imulq	%r15, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r15, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%r15, %rax
	cmpq	%r15, %rsi
	cmovnb	%rax, %rsi
	xorl	%edi, %edi
.L2072:
	movq	%r10, %rax
	movq	%r11, %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L2066
	addq	%rcx, %r8
	movq	%r11, %rax
	adcq	%rbx, %r9
	cmpq	%r8, %r10
	sbbq	%r9, %rax
	jc	.L2066
	movq	%rsi, %rcx
	orq	%rdi, %rcx
	je	.L2063
	movb	$2, 400(%rsp)
	movb	$0, 416(%rsp)
	jmp	.L2062
	.p2align 4,,10
	.p2align 3
.L2080:
	movq	48(%rsp), %r9
	movq	40(%rsp), %rax
	xorl	%r10d, %r10d
	mulq	%rcx
	imulq	%rcx, %r9
	addq	%rdx, %r9
	movq	%r9, %rax
	imulq	%r15, %r9
	subq	%r9, %rcx
	cmpq	%r15, %rcx
	setnb	%r10b
	addq	%r10, %rax
	movq	%rax, %r8
	cmpq	%r15, %rcx
	jb	.L2244
	subq	%r15, %rcx
.L2244:
	movq	(%rsp), %rax
	movq	%rdi, %r9
	movq	$-1, %r10
	xorl	%ebx, %ebx
	movabsq	$9223372036854775807, %r11
	shrq	$63, %r9
	movq	$0, 96(%rsp)
	mulq	%r8
	movb	%r9b, 205(%rsp)
	xorb	204(%rsp), %r9b
	movzbl	%r9b, %r9d
	addq	%r9, %r10
	adcq	$0, %r11
	movq	%rdx, 104(%rsp)
	orq	96(%rsp), %rax
	xorl	%edx, %edx
	orq	104(%rsp), %rdx
	movq	%rax, %r8
	movq	%rdx, %r9
.L2196:
	movq	%rcx, %rax
	orq	%rbx, %rax
	je	.L2089
	movq	(%rsp), %rax
	mulq	%rcx
	movq	%rax, %rcx
	movq	%rdx, %rdi
	movq	%rax, %rsi
	testq	%rdx, %rdx
	jne	.L2266
	movq	48(%rsp), %rdi
	xorl	%ebx, %ebx
	imulq	%rax, %rdi
	movq	40(%rsp), %rax
	mulq	%rcx
	addq	%rdx, %rdi
	movq	%rdi, %rax
	movq	%rdi, %rcx
	imulq	%r15, %rax
	subq	%rax, %rsi
	xorl	%eax, %eax
	cmpq	%r15, %rsi
	setnb	%al
	addq	%rax, %rcx
	movq	%rsi, %rax
	adcq	$0, %rbx
	subq	%r15, %rax
	cmpq	%r15, %rsi
	cmovnb	%rax, %rsi
	xorl	%edi, %edi
.L2098:
	movq	%r10, %rax
	movq	%r11, %rdx
	subq	%r8, %rax
	sbbq	%r9, %rdx
	cmpq	%rcx, %rax
	movq	%rdx, %rax
	sbbq	%rbx, %rax
	jc	.L2092
	addq	%rcx, %r8
	movq	%r11, %rax
	adcq	%rbx, %r9
	cmpq	%r8, %r10
	sbbq	%r9, %rax
	jc	.L2092
	movq	%rsi, %rcx
	orq	%rdi, %rcx
	je	.L2089
	movb	$2, 424(%rsp)
	movb	$0, 440(%rsp)
	jmp	.L2088
	.p2align 4,,10
	.p2align 3
.L2037:
	movzbl	204(%rsp), %ebx
	cmpb	%bl, 205(%rsp)
	je	.L2053
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L2053:
	movq	%r8, 376(%rsp)
	movq	%r9, 384(%rsp)
	movb	$1, 392(%rsp)
	jmp	.L2036
	.p2align 4,,10
	.p2align 3
.L2063:
	movzbl	204(%rsp), %ebx
	cmpb	%bl, 205(%rsp)
	je	.L2079
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L2079:
	movq	%r8, 400(%rsp)
	movq	%r9, 408(%rsp)
	movb	$1, 416(%rsp)
	jmp	.L2062
	.p2align 4,,10
	.p2align 3
.L2089:
	movzbl	205(%rsp), %ebx
	cmpb	%bl, 204(%rsp)
	je	.L2105
	negq	%r8
	adcq	$0, %r9
	negq	%r9
.L2105:
	cmpb	$0, 80(%rsp)
	movq	%r8, 424(%rsp)
	movq	%r9, 432(%rsp)
	movb	$1, 440(%rsp)
	jne	.L2267
	.p2align 4
	.p2align 3
.L2239:
	movq	160(%rsp), %rbx
	movq	88(%rsp), %r11
.L2106:
	leaq	(%rbx,%rbx,2), %rax
	movq	%r13, (%r11)
	movzbl	352(%rsp,%rax,8), %eax
	movb	$0, 16(%r11)
	movb	%al, 8(%r11)
	jmp	.L1994
.L2260:
	movq	%rdi, %rsi
	movq	$-1, %r10
	movq	%rcx, %rbx
	movabsq	$9223372036854775807, %r11
	shrq	$63, %rsi
	movb	%sil, 205(%rsp)
	xorb	204(%rsp), %sil
	movzbl	%sil, %esi
	addq	%rsi, %r10
	adcq	$0, %r11
	xorl	%ecx, %ecx
	xorl	%r9d, %r9d
	orq	%rcx, %r8
	orq	%rbx, %r9
	movq	%r11, %rax
	cmpq	%r8, %r10
	sbbq	%r9, %rax
	jc	.L2092
	movq	176(%rsp), %rcx
	xorl	%ebx, %ebx
	jmp	.L2196
	.p2align 4,,10
	.p2align 3
.L2259:
	movq	%rdi, %rsi
	movq	$-1, %r10
	movq	%rcx, %rbx
	movabsq	$9223372036854775807, %r11
	shrq	$63, %rsi
	movb	%sil, 205(%rsp)
	xorb	204(%rsp), %sil
	movzbl	%sil, %esi
	addq	%rsi, %r10
	adcq	$0, %r11
	xorl	%ecx, %ecx
	xorl	%r9d, %r9d
	orq	%rcx, %r8
	orq	%rbx, %r9
	movq	%r11, %rax
	cmpq	%r8, %r10
	sbbq	%r9, %rax
	jc	.L2066
	movq	176(%rsp), %rcx
	xorl	%ebx, %ebx
	jmp	.L2197
	.p2align 4,,10
	.p2align 3
.L2055:
	movq	%rcx, %rax
	movq	%rdi, %rsi
	movq	$-1, %r10
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	(%rsp), %rax
	movq	%rdx, %rcx
	shrq	$63, %rsi
	movb	%sil, 205(%rsp)
	xorb	204(%rsp), %sil
	movabsq	$9223372036854775807, %r11
	mulq	%r8
	movzbl	%sil, %esi
	addq	%rsi, %r10
	adcq	$0, %r11
	movq	%rax, %r8
	movq	%rdx, %r9
	jmp	.L2197
.L2254:
	movq	%rdi, %rsi
	movq	$-1, %r8
	movq	%rcx, %rbx
	movabsq	$9223372036854775807, %r9
	shrq	$63, %rsi
	movb	%sil, 80(%rsp)
	xorb	204(%rsp), %sil
	movzbl	%sil, %esi
	addq	%rsi, %r8
	adcq	$0, %r9
	xorl	%ecx, %ecx
	xorl	%r11d, %r11d
	orq	%rcx, %r10
	orq	%rbx, %r11
	movq	%r9, %rax
	cmpq	%r10, %r8
	sbbq	%r11, %rax
	jc	.L2012
	movq	64(%rsp), %rcx
	xorl	%ebx, %ebx
	jmp	.L2199
	.p2align 4,,10
	.p2align 3
.L2029:
	movq	%rdi, %rsi
	movq	%rcx, %rax
	movq	$-1, %r10
	xorl	%ebx, %ebx
	movabsq	$9223372036854775807, %r11
	shrq	$63, %rsi
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	movb	%sil, 205(%rsp)
	movq	%rdx, %rcx
	xorb	204(%rsp), %sil
	mulq	(%rsp)
	movzbl	%sil, %esi
	addq	%rsi, %r10
	adcq	$0, %r11
	movq	%rax, %r8
	movq	%rdx, %r9
	jmp	.L2198
.L2258:
	movq	%rdi, %rsi
	movq	$-1, %r10
	movq	%rcx, %rbx
	movabsq	$9223372036854775807, %r11
	shrq	$63, %rsi
	movb	%sil, 205(%rsp)
	xorb	204(%rsp), %sil
	movzbl	%sil, %esi
	addq	%rsi, %r10
	adcq	$0, %r11
	xorl	%ecx, %ecx
	xorl	%r9d, %r9d
	orq	%rcx, %r8
	orq	%rbx, %r9
	movq	%r11, %rax
	cmpq	%r8, %r10
	sbbq	%r9, %rax
	jc	.L2040
	movq	176(%rsp), %rcx
	xorl	%ebx, %ebx
	jmp	.L2198
	.p2align 4,,10
	.p2align 3
.L2081:
	movq	%rcx, %rax
	movq	%rdi, %rsi
	movq	$-1, %r10
	xorl	%ebx, %ebx
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	movq	%rax, %r8
	movq	(%rsp), %rax
	movq	%rdx, %rcx
	shrq	$63, %rsi
	movb	%sil, 205(%rsp)
	xorb	204(%rsp), %sil
	movabsq	$9223372036854775807, %r11
	mulq	%r8
	movzbl	%sil, %esi
	addq	%rsi, %r10
	adcq	$0, %r11
	movq	%rax, %r8
	movq	%rdx, %r9
	jmp	.L2196
	.p2align 4,,10
	.p2align 3
.L2206:
	movq	88(%rsp), %r11
	movq	%rsi, %r13
.L2114:
	movq	%r13, (%r11)
	movb	$0, 8(%r11)
	movb	$0, 16(%r11)
.L1994:
	movq	456(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L2268
	addq	$472, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%r11, %rax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L2201:
	.cfi_restore_state
	movq	88(%rsp), %r11
	movq	64(%rsp), %r13
	movl	$1, %ebx
	jmp	.L2106
	.p2align 4,,10
	.p2align 3
.L2203:
	movq	88(%rsp), %r11
	movq	%rsi, %r13
	movl	$2, %ebx
	jmp	.L2106
	.p2align 4,,10
	.p2align 3
.L2193:
	xorl	%edx, %edx
	jmp	.L2190
.L2212:
	movq	40(%rsp), %r11
	movq	%rsi, %r13
	jmp	.L2114
.L2205:
	movq	88(%rsp), %r11
	movq	%rsi, %r13
	movl	$3, %ebx
	jmp	.L2106
.L2241:
	movq	48(%rsp), %rbx
	movq	40(%rsp), %r11
	jmp	.L2106
.L2242:
	movq	40(%rsp), %r11
	jmp	.L2114
.L2207:
	movq	(%rsp), %r15
	movq	40(%rsp), %r11
	movl	$1, %ebx
	movq	%r15, %r13
	jmp	.L2106
.L2208:
	movq	(%rsp), %r15
	movq	40(%rsp), %r11
	movq	%r15, %r13
	jmp	.L2114
.L2264:
	cmpq	%r15, %rdx
	jnb	.L2045
	movq	$0, 176(%rsp)
	movq	$0, 184(%rsp)
.L2048:
	movq	%rcx, %rax
	movq	%rdi, %rdx
	xorl	%ebx, %ebx
	xorl	%edi, %edi
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	orq	184(%rsp), %rbx
	movq	%rax, %rcx
	movq	%rdx, %rsi
	orq	176(%rsp), %rcx
	jmp	.L2046
.L2263:
	cmpq	%r15, %rdx
	jnb	.L2018
	movq	$0, 64(%rsp)
	movq	$0, 72(%rsp)
.L2021:
	movq	%rcx, %rax
	movq	%rdi, %rdx
	xorl	%ebx, %ebx
	xorl	%edi, %edi
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	orq	72(%rsp), %rbx
	movq	%rax, %rcx
	movq	%rdx, %rsi
	orq	64(%rsp), %rcx
	jmp	.L2019
.L2265:
	cmpq	%r15, %rdx
	jnb	.L2071
	movq	$0, 176(%rsp)
	movq	$0, 184(%rsp)
.L2074:
	movq	%rcx, %rax
	movq	%rdi, %rdx
	xorl	%ebx, %ebx
	xorl	%edi, %edi
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	orq	184(%rsp), %rbx
	movq	%rax, %rcx
	movq	%rdx, %rsi
	orq	176(%rsp), %rcx
	jmp	.L2072
.L2266:
	cmpq	%r15, %rdx
	jnb	.L2097
	movq	$0, 176(%rsp)
	movq	$0, 184(%rsp)
.L2100:
	movq	%rcx, %rax
	movq	%rdi, %rdx
	xorl	%ebx, %ebx
	xorl	%edi, %edi
#APP
# 69 "include/fixedwide/detail/batch_multiply.hpp" 1
	divq %r15
# 0 "" 2
#NO_APP
	orq	184(%rsp), %rbx
	movq	%rax, %rcx
	movq	%rdx, %rsi
	orq	176(%rsp), %rcx
	jmp	.L2098
.L2097:
	movq	40(%rsp), %rax
	mulq	%rdx
	movq	%r15, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	xorl	%eax, %eax
	cmpq	%r15, %rdi
	movq	%rdi, %rsi
	setnb	%al
	addq	%rdx, %rax
	subq	%r15, %rsi
	cmpq	%r15, %rdi
	movq	%rax, %rdx
	cmovnb	%rsi, %rdi
	xorl	%eax, %eax
	movq	%rax, 176(%rsp)
	movq	%rdx, 184(%rsp)
	jmp	.L2100
.L2071:
	movq	40(%rsp), %rax
	mulq	%rdx
	movq	%r15, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	xorl	%eax, %eax
	cmpq	%r15, %rdi
	movq	%rdi, %rsi
	setnb	%al
	addq	%rdx, %rax
	subq	%r15, %rsi
	cmpq	%r15, %rdi
	movq	%rax, %rdx
	cmovnb	%rsi, %rdi
	xorl	%eax, %eax
	movq	%rax, 176(%rsp)
	movq	%rdx, 184(%rsp)
	jmp	.L2074
.L2018:
	movq	40(%rsp), %rax
	mulq	%rdx
	movq	%r15, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	xorl	%eax, %eax
	cmpq	%r15, %rdi
	movq	%rdi, %rsi
	setnb	%al
	addq	%rdx, %rax
	subq	%r15, %rsi
	cmpq	%r15, %rdi
	movq	%rax, %rdx
	cmovnb	%rsi, %rdi
	xorl	%eax, %eax
	movq	%rax, 64(%rsp)
	movq	%rdx, 72(%rsp)
	jmp	.L2021
.L2045:
	movq	40(%rsp), %rax
	mulq	%rdx
	movq	%r15, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	xorl	%eax, %eax
	cmpq	%r15, %rdi
	movq	%rdi, %rsi
	setnb	%al
	addq	%rdx, %rax
	subq	%r15, %rsi
	cmpq	%r15, %rdi
	movq	%rax, %rdx
	cmovnb	%rsi, %rdi
	xorl	%eax, %eax
	movq	%rax, 176(%rsp)
	movq	%rdx, 184(%rsp)
	jmp	.L2048
.L2209:
	movq	40(%rsp), %r11
	movq	%rsi, %r13
	movl	$2, %ebx
	jmp	.L2106
.L2211:
	movq	40(%rsp), %r11
	movq	%rsi, %r13
	movl	$3, %ebx
	jmp	.L2106
.L2255:
	movq	40(%rsp), %r11
	movzbl	208(%rsp), %eax
	movq	%rbx, (%r11)
	movb	%al, 8(%r11)
	movb	$0, 16(%r11)
	jmp	.L1994
.L2256:
	movq	40(%rsp), %r11
	movq	%rbx, (%r11)
	movb	$0, 8(%r11)
	movb	$0, 16(%r11)
	jmp	.L1994
.L2268:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE1704:
	.size	_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE, .-_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.globl	__udivti3
	.ident	"GCC: (Ubuntu 14.2.0-4ubuntu2~24.04.1) 14.2.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
