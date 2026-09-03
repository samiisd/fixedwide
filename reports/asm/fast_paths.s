	.file	"asm_sample.cpp"
                                        # Start of file scope inline assembly
	.globl	_ZSt21ios_base_library_initv

                                        # End of file scope inline assembly
	.text
	.globl	_Z22asm_mul64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_ # -- Begin function _Z22asm_mul64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_
	.p2align	4
	.type	_Z22asm_mul64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_,@function
_Z22asm_mul64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_: # @_Z22asm_mul64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_
# %bb.0:
	movabsq	$500000000000, %r8              # imm = 0x746A528800
	movq	%rdi, %rax
	#APP
	imulq	%rsi
	#NO_APP
	movq	%rdx, %rcx
	addq	%r8, %rdx
	shrq	$12, %rdx
	cmpq	$244140624, %rdx                # imm = 0xE8D4A50
	ja	.LBB0_4
# %bb.1:
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	movq	%rcx, %rdx
	#APP
	idivq	%rsi
	#NO_APP
	testq	%rdx, %rdx
	je	.LBB0_3
# %bb.2:
	movq	%rdx, %rsi
	negq	%rsi
	cmovsq	%rdx, %rsi
	movl	%eax, %edx
	andl	$1, %edx
	subq	%rdx, %r8
	sarq	$63, %rcx
	orq	$1, %rcx
	xorl	%edx, %edx
	cmpq	%r8, %rsi
	cmovaq	%rcx, %rdx
	addq	%rdx, %rax
.LBB0_3:
	movq	%rax, %rcx
	andq	$-256, %rcx
	movb	$1, %dl
	movzbl	%al, %eax
	orq	%rcx, %rax
	retq
.LBB0_4:
	pushq	%rax
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	movl	$3, %ecx
	callq	_ZN9fixedwide6detail10mul64_implElllNS_8RoundingE@PLT
	movq	%rax, %rsi
	andq	$-256, %rsi
	xorl	%ecx, %ecx
	andb	$1, %dl
	cmovneq	%rsi, %rcx
	addq	$8, %rsp
	movzbl	%al, %eax
	orq	%rcx, %rax
	retq
.Lfunc_end0:
	.size	_Z22asm_mul64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_, .Lfunc_end0-_Z22asm_mul64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_
                                        # -- End function
	.globl	_Z22asm_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_ # -- Begin function _Z22asm_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_
	.p2align	4
	.type	_Z22asm_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_,@function
_Z22asm_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_: # @_Z22asm_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_
# %bb.0:
	testq	%rsi, %rsi
	je	.LBB1_7
# %bb.1:
	pushq	%rbx
	movabsq	$1000000000000, %rcx            # imm = 0xE8D4A51000
	movq	%rdi, %rax
	#APP
	imulq	%rcx
	#NO_APP
	movq	%rdx, %rcx
	testq	%rsi, %rsi
	sets	%dl
	movq	%rsi, %r9
	negq	%r9
	cmovsq	%rsi, %r9
	movq	%r9, %r8
	shrq	%r8
	leaq	(%rcx,%r8), %r11
	movq	%r9, %rbx
	andq	$-2, %rbx
	testq	%r11, %r11
	sete	%r10b
	cmpq	%rbx, %r11
	jae	.LBB1_8
# %bb.2:
	andb	%r10b, %dl
	jne	.LBB1_8
# %bb.3:
	movq	%rcx, %rdx
	#APP
	idivq	%rsi
	#NO_APP
	testq	%rdx, %rdx
	je	.LBB1_5
# %bb.4:
	xorq	%rsi, %rcx
	movq	%rdx, %rsi
	negq	%rsi
	cmovsq	%rdx, %rsi
	notl	%r9d
	movl	%eax, %edx
	andl	%r9d, %edx
	andl	$1, %edx
	subq	%rdx, %r8
	sarq	$63, %rcx
	orq	$1, %rcx
	xorl	%edx, %edx
	cmpq	%r8, %rsi
	cmovaq	%rcx, %rdx
	addq	%rdx, %rax
.LBB1_5:
	movq	%rax, %rcx
	andq	$-256, %rcx
	movb	$1, %dl
	popq	%rbx
	movzbl	%al, %eax
	orq	%rcx, %rax
                                        # kill: def $dl killed $dl killed $rdx
	retq
.LBB1_7:
	movl	$1, %eax
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	movzbl	%al, %eax
	orq	%rcx, %rax
                                        # kill: def $dl killed $dl killed $rdx
	retq
.LBB1_8:
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	movl	$3, %ecx
	callq	_ZN9fixedwide6detail10div64_implElllNS_8RoundingE@PLT
                                        # kill: def $dl killed $dl def $rdx
	movq	%rax, %rsi
	andq	$-256, %rsi
	xorl	%ecx, %ecx
	andb	$1, %dl
	cmovneq	%rsi, %rcx
	popq	%rbx
	movzbl	%al, %eax
	orq	%rcx, %rax
                                        # kill: def $dl killed $dl killed $rdx
	retq
.Lfunc_end1:
	.size	_Z22asm_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_, .Lfunc_end1-_Z22asm_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_
                                        # -- End function
	.globl	_Z26asm_mul_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_S1_ # -- Begin function _Z26asm_mul_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_S1_
	.p2align	4
	.type	_Z26asm_mul_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_S1_,@function
_Z26asm_mul_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_S1_: # @_Z26asm_mul_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_S1_
# %bb.0:
	testq	%rdx, %rdx
	je	.LBB2_1
# %bb.3:
	pushq	%r14
	pushq	%rbx
	pushq	%rax
	movq	%rdx, %r8
	movq	%rdi, %rax
	#APP
	imulq	%rsi
	#NO_APP
	movq	%rdx, %rcx
	testq	%r8, %r8
	sets	%dl
	movq	%r8, %r10
	negq	%r10
	cmovsq	%r8, %r10
	movq	%r10, %r9
	shrq	%r9
	leaq	(%rcx,%r9), %rbx
	movq	%r10, %r14
	andq	$-2, %r14
	testq	%rbx, %rbx
	sete	%r11b
	cmpq	%r14, %rbx
	jae	.LBB2_8
# %bb.4:
	andb	%r11b, %dl
	jne	.LBB2_8
# %bb.5:
	movq	%rcx, %rdx
	#APP
	idivq	%r8
	#NO_APP
	testq	%rdx, %rdx
	je	.LBB2_7
# %bb.6:
	xorq	%r8, %rcx
	movq	%rdx, %rsi
	negq	%rsi
	cmovsq	%rdx, %rsi
	notl	%r10d
	movl	%eax, %edx
	andl	%r10d, %edx
	andl	$1, %edx
	subq	%rdx, %r9
	sarq	$63, %rcx
	orq	$1, %rcx
	xorl	%edx, %edx
	cmpq	%r9, %rsi
	cmovaq	%rcx, %rdx
	addq	%rdx, %rax
.LBB2_7:
	movq	%rax, %rcx
	andq	$-256, %rcx
	movb	$1, %dl
	jmp	.LBB2_9
.LBB2_1:
	movl	$1, %eax
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	movzbl	%al, %eax
	orq	%rcx, %rax
                                        # kill: def $dl killed $dl killed $rdx
	retq
.LBB2_8:
	movq	%r8, %rdx
	movl	$3, %ecx
	callq	_ZN9fixedwide6detail14mul_div64_implElllNS_8RoundingE@PLT
                                        # kill: def $dl killed $dl def $rdx
	movq	%rax, %rsi
	andq	$-256, %rsi
	xorl	%ecx, %ecx
	andb	$1, %dl
	cmovneq	%rsi, %rcx
.LBB2_9:
	addq	$8, %rsp
	popq	%rbx
	popq	%r14
	movzbl	%al, %eax
	orq	%rcx, %rax
                                        # kill: def $dl killed $dl killed $rdx
	retq
.Lfunc_end2:
	.size	_Z26asm_mul_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_S1_, .Lfunc_end2-_Z26asm_mul_div64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEES1_S1_
                                        # -- End function
	.globl	_Z27asm_quantize64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEE # -- Begin function _Z27asm_quantize64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEE
	.p2align	4
	.type	_Z27asm_quantize64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEE,@function
_Z27asm_quantize64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEE: # @_Z27asm_quantize64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEE
# %bb.0:
	pushq	%rax
	movl	$12, %esi
	movl	$4, %edx
	movl	$3, %ecx
	callq	_ZN9fixedwide6detail15quantize64_implEljjNS_8RoundingE@PLT
	movq	%rax, %rcx
	andq	$-256, %rcx
	xorl	%esi, %esi
	andb	$1, %dl
	cmovneq	%rcx, %rsi
	movzbl	%al, %eax
	orq	%rsi, %rax
	popq	%rcx
	retq
.Lfunc_end3:
	.size	_Z27asm_quantize64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEE, .Lfunc_end3-_Z27asm_quantize64_nearest_evenN9fixedwide11basic_fixedILm64ELj12EEE
                                        # -- End function
	.globl	_Z23asm_mul128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_ # -- Begin function _Z23asm_mul128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_
	.p2align	4
	.type	_Z23asm_mul128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_,@function
_Z23asm_mul128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_: # @_Z23asm_mul128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_
# %bb.0:
	pushq	%rbx
	subq	$48, %rsp
	movq	%rdi, %rbx
	movq	%fs:40, %rax
	movq	%rax, 40(%rsp)
	cmpq	$-1, %rdx
	je	.LBB4_4
# %bb.1:
	testq	%rdx, %rdx
	jne	.LBB4_2
# %bb.3:
	testq	%rsi, %rsi
	setns	%al
	cmpq	$-1, %r8
	jne	.LBB4_6
	jmp	.LBB4_10
.LBB4_4:
	movq	%rsi, %rax
	shrq	$63, %rax
	cmpq	$-1, %r8
	je	.LBB4_10
.LBB4_6:
	testq	%r8, %r8
	jne	.LBB4_8
# %bb.7:
	movb	$1, %dil
	testq	%rcx, %rcx
	js	.LBB4_8
# %bb.11:
	andb	%dil, %al
	cmpb	$1, %al
	je	.LBB4_12
	jmp	.LBB4_8
.LBB4_2:
	xorl	%eax, %eax
	cmpq	$-1, %r8
	jne	.LBB4_6
.LBB4_10:
	movq	%rcx, %rdi
	shrq	$63, %rdi
	andb	%dil, %al
	cmpb	$1, %al
	jne	.LBB4_8
.LBB4_12:
	movq	%rsi, %rax
	movq	%rdx, %r9
	#APP
	imulq	%rcx
	#NO_APP
	movq	%rdx, %rdi
	movq	%r9, %rdx
	movabsq	$-500000000000, %r9             # imm = 0xFFFFFF8B95AD7800
	addq	%rdi, %r9
	movabsq	$-1000000000000, %r10           # imm = 0xFFFFFF172B5AF000
	cmpq	%r10, %r9
	jae	.LBB4_13
.LBB4_8:
	movl	$3, (%rsp)
	leaq	16(%rsp), %rdi
	movl	$12, %r9d
	callq	_ZN9fixedwide6detail11mul128_implENS_4wide6int128ES2_jNS_8RoundingE@PLT
	cmpb	$0, 32(%rsp)
	je	.LBB4_9
# %bb.18:
	movaps	16(%rsp), %xmm0
	movups	%xmm0, (%rbx)
	movb	$1, %al
	jmp	.LBB4_19
.LBB4_9:
	movzbl	16(%rsp), %eax
	movb	%al, (%rbx)
	xorl	%eax, %eax
.LBB4_19:
	movb	%al, 16(%rbx)
	movq	%fs:40, %rax
	cmpq	40(%rsp), %rax
	jne	.LBB4_22
.LBB4_21:
	movq	%rbx, %rax
	addq	$48, %rsp
	popq	%rbx
	retq
.LBB4_13:
	movabsq	$1000000000000, %rcx            # imm = 0xE8D4A51000
	movq	%rdi, %rdx
	#APP
	idivq	%rcx
	#NO_APP
	testq	%rdx, %rdx
	je	.LBB4_17
# %bb.14:
	movq	%rdx, %rsi
	negq	%rsi
	cmovsq	%rdx, %rsi
	addq	%rsi, %rsi
	cmpq	%rcx, %rsi
	ja	.LBB4_16
# %bb.15:
	sete	%cl
	andb	%al, %cl
	cmpb	$1, %cl
	jne	.LBB4_17
.LBB4_16:
	sarq	$63, %rdi
	orq	$1, %rdi
	addq	%rdi, %rax
.LBB4_17:
	movq	%rax, (%rbx)
	sarq	$63, %rax
	movq	%rax, 8(%rbx)
	movb	$1, 16(%rbx)
	movq	%fs:40, %rax
	cmpq	40(%rsp), %rax
	je	.LBB4_21
.LBB4_22:
	callq	__stack_chk_fail@PLT
.Lfunc_end4:
	.size	_Z23asm_mul128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_, .Lfunc_end4-_Z23asm_mul128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_
                                        # -- End function
	.globl	_Z23asm_div128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_ # -- Begin function _Z23asm_div128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_
	.p2align	4
	.type	_Z23asm_div128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_,@function
_Z23asm_div128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_: # @_Z23asm_div128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_
# %bb.0:
	movl	$3, %r9d
	jmp	_ZN9fixedwide3divILm128ELj12EEESt8expectedINS_11basic_fixedIXT_EXT0_EEENS_15ArithmeticErrorEES3_S3_NS_8RoundingE # TAILCALL
.Lfunc_end5:
	.size	_Z23asm_div128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_, .Lfunc_end5-_Z23asm_div128_nearest_evenN9fixedwide11basic_fixedILm128ELj12EEES1_
                                        # -- End function
	.section	.text._ZN9fixedwide3divILm128ELj12EEESt8expectedINS_11basic_fixedIXT_EXT0_EEENS_15ArithmeticErrorEES3_S3_NS_8RoundingE,"axG",@progbits,_ZN9fixedwide3divILm128ELj12EEESt8expectedINS_11basic_fixedIXT_EXT0_EEENS_15ArithmeticErrorEES3_S3_NS_8RoundingE,comdat
	.weak	_ZN9fixedwide3divILm128ELj12EEESt8expectedINS_11basic_fixedIXT_EXT0_EEENS_15ArithmeticErrorEES3_S3_NS_8RoundingE # -- Begin function _ZN9fixedwide3divILm128ELj12EEESt8expectedINS_11basic_fixedIXT_EXT0_EEENS_15ArithmeticErrorEES3_S3_NS_8RoundingE
	.p2align	4
	.type	_ZN9fixedwide3divILm128ELj12EEESt8expectedINS_11basic_fixedIXT_EXT0_EEENS_15ArithmeticErrorEES3_S3_NS_8RoundingE,@function
_ZN9fixedwide3divILm128ELj12EEESt8expectedINS_11basic_fixedIXT_EXT0_EEENS_15ArithmeticErrorEES3_S3_NS_8RoundingE: # @_ZN9fixedwide3divILm128ELj12EEESt8expectedINS_11basic_fixedIXT_EXT0_EEENS_15ArithmeticErrorEES3_S3_NS_8RoundingE
# %bb.0:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	subq	$48, %rsp
	movq	%rdi, %rbx
	movq	%fs:40, %rax
	movq	%rax, 40(%rsp)
	movq	%rcx, %rax
	orq	%r8, %rax
	jne	.LBB6_2
# %bb.1:
	movb	$1, (%rbx)
	movb	$0, 16(%rbx)
	movq	%fs:40, %rax
	cmpq	40(%rsp), %rax
	je	.LBB6_24
	jmp	.LBB6_25
.LBB6_2:
	movzbl	%r9b, %r10d
	cmpb	$3, %r10b
	je	.LBB6_4
# %bb.3:
	testl	%r10d, %r10d
	jne	.LBB6_11
.LBB6_4:
	cmpq	$-1, %rdx
	je	.LBB6_8
# %bb.5:
	testq	%rdx, %rdx
	jne	.LBB6_6
# %bb.7:
	testq	%rsi, %rsi
	setns	%al
	testq	%r8, %r8
	jne	.LBB6_13
	jmp	.LBB6_10
.LBB6_8:
	movq	%rsi, %rax
	shrq	$63, %rax
	testq	%r8, %r8
	je	.LBB6_10
.LBB6_13:
	cmpq	$-1, %r8
	jne	.LBB6_11
# %bb.14:
	movq	%rcx, %rdi
	shrq	$63, %rdi
	jmp	.LBB6_15
.LBB6_6:
	xorl	%eax, %eax
	testq	%r8, %r8
	jne	.LBB6_13
.LBB6_10:
	movb	$1, %dil
	testq	%rcx, %rcx
	js	.LBB6_11
.LBB6_15:
	andb	%dil, %al
	cmpb	$1, %al
	jne	.LBB6_11
# %bb.16:
	movabsq	$1000000000000, %rdi            # imm = 0xE8D4A51000
	movq	%rsi, %rax
	movq	%rdx, %r11
	#APP
	imulq	%rdi
	#NO_APP
	movq	%rdx, %rdi
	movq	%r11, %rdx
	movq	%rcx, %r14
	negq	%r14
	cmovsq	%rcx, %r14
	movq	%r14, %r11
	shrq	%r11
	leaq	(%rdi,%r11), %r15
	movq	%r14, %r12
	andq	$-2, %r12
	cmpq	%r12, %r15
	jae	.LBB6_11
# %bb.17:
	testq	%rcx, %rcx
	sets	%bpl
	testq	%r15, %r15
	sete	%r15b
	testb	%r15b, %bpl
	je	.LBB6_18
.LBB6_11:
	movl	%r10d, (%rsp)
	leaq	16(%rsp), %rdi
	movl	$12, %r9d
	callq	_ZN9fixedwide6detail11div128_implENS_4wide6int128ES2_jNS_8RoundingE@PLT
	cmpb	$0, 32(%rsp)
	je	.LBB6_12
# %bb.21:
	movaps	16(%rsp), %xmm0
	movups	%xmm0, (%rbx)
	movb	$1, %al
	jmp	.LBB6_22
.LBB6_12:
	movzbl	16(%rsp), %eax
	movb	%al, (%rbx)
	xorl	%eax, %eax
.LBB6_22:
	movb	%al, 16(%rbx)
	movq	%fs:40, %rax
	cmpq	40(%rsp), %rax
	jne	.LBB6_25
.LBB6_24:
	movq	%rbx, %rax
	addq	$48, %rsp
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
.LBB6_18:
	cmpb	$3, %r9b
	sete	%sil
	movq	%rdi, %rdx
	#APP
	idivq	%rcx
	#NO_APP
	testq	%rdx, %rdx
	setne	%r8b
	andb	%sil, %r8b
	cmpb	$1, %r8b
	jne	.LBB6_20
# %bb.19:
	xorq	%rcx, %rdi
	movq	%rdx, %rcx
	negq	%rcx
	cmovsq	%rdx, %rcx
	notl	%r14d
	movl	%eax, %edx
	andl	%r14d, %edx
	andl	$1, %edx
	subq	%rdx, %r11
	sarq	$63, %rdi
	orq	$1, %rdi
	xorl	%edx, %edx
	cmpq	%r11, %rcx
	cmovaq	%rdi, %rdx
	addq	%rdx, %rax
.LBB6_20:
	movq	%rax, (%rbx)
	sarq	$63, %rax
	movq	%rax, 8(%rbx)
	movb	$1, 16(%rbx)
	movq	%fs:40, %rax
	cmpq	40(%rsp), %rax
	je	.LBB6_24
.LBB6_25:
	callq	__stack_chk_fail@PLT
.Lfunc_end6:
	.size	_ZN9fixedwide3divILm128ELj12EEESt8expectedINS_11basic_fixedIXT_EXT0_EEENS_15ArithmeticErrorEES3_S3_NS_8RoundingE, .Lfunc_end6-_ZN9fixedwide3divILm128ELj12EEESt8expectedINS_11basic_fixedIXT_EXT0_EEENS_15ArithmeticErrorEES3_S3_NS_8RoundingE
                                        # -- End function
	.ident	"clang version 22.1.8"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym __stack_chk_fail
