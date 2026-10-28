	.file	"batch_codegen.cpp"
	.text
	.globl	_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE # -- Begin function _ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE,@function
_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE: # @_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$40, %rsp
	.cfi_def_cfa_offset 96
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%rsi, 32(%rsp)                  # 8-byte Spill
	movq	%rdi, (%rsp)                    # 8-byte Spill
	testq	%rcx, %rcx
	je	.LBB0_14
# %bb.1:
	movq	%r8, %rbx
	movq	%rcx, %r15
	movq	%rdx, %r12
	movq	%r8, %r14
	negq	%r14
	cmovsq	%r8, %r14
	xorl	%ebp, %ebp
	xorl	%r13d, %r13d
	.p2align	4
.LBB0_2:                                # =>This Inner Loop Header: Depth=1
	movq	(%r12,%rbp), %rsi
	movq	8(%r12,%rbp), %rcx
	movq	%rsi, %rdi
	negq	%rdi
	movl	$0, %eax
	sbbq	%rcx, %rax
	testq	%rcx, %rcx
	cmovnsq	%rsi, %rdi
	cmovnsq	%rcx, %rax
	mulq	%r14
	testq	%rdx, %rdx
	jne	.LBB0_12
# %bb.3:                                #   in Loop: Header=BB0_2 Depth=1
	movq	%rdx, %r8
	movq	%rax, %r9
	movq	%rdi, %rax
	mulq	%r14
	addq	%r9, %rdx
	adcq	$0, %r8
	jne	.LBB0_12
# %bb.4:                                #   in Loop: Header=BB0_2 Depth=1
	movq	%rcx, %rdi
	xorq	%rbx, %rdi
	movq	%rdi, %r8
	shrq	$63, %r8
	addq	$-1, %r8
	movabsq	$9223372036854775807, %r9       # imm = 0x7FFFFFFFFFFFFFFF
	adcq	$0, %r9
	cmpq	%rax, %r8
	sbbq	%rdx, %r9
	jae	.LBB0_5
	.p2align	4
.LBB0_12:                               #   in Loop: Header=BB0_2 Depth=1
	leaq	8(%rsp), %rdi
	movq	%rcx, %rdx
	movq	%rbx, %rcx
	movl	$5, %r8d
	callq	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	testb	$1, 24(%rsp)
	je	.LBB0_10
.LBB0_13:                               #   in Loop: Header=BB0_2 Depth=1
	movups	8(%rsp), %xmm0
	movq	32(%rsp), %rax                  # 8-byte Reload
	movups	%xmm0, (%rax,%rbp)
	incq	%r13
	addq	$16, %rbp
	cmpq	%r13, %r15
	jne	.LBB0_2
	jmp	.LBB0_14
.LBB0_5:                                #   in Loop: Header=BB0_2 Depth=1
	movq	%rax, %rcx
	negq	%rcx
	movl	$0, %r8d
	sbbq	%rdx, %r8
	testq	%rdi, %rdi
	cmovnsq	%rdx, %r8
	testq	%rdx, %rdx
	je	.LBB0_6
# %bb.7:                                #   in Loop: Header=BB0_2 Depth=1
	movq	%rax, %rsi
	movq	%rdx, %rax
	movq	%rdx, %rdi
	movabsq	$-2601111570856684097, %rcx     # imm = 0xDBE6FECEBDEDD5BF
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$33, %rcx
	movq	%rcx, %rax
	movabsq	$10000000000, %r9               # imm = 0x2540BE400
	imulq	%r9, %rax
	subq	%rax, %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	#APP
	divq	%r9
	#NO_APP
	testq	%rdx, %rdx
	je	.LBB0_11
	jmp	.LBB0_9
.LBB0_6:                                #   in Loop: Header=BB0_2 Depth=1
	movq	%rax, %rcx
	movabsq	$-2601111570856684097, %rdx     # imm = 0xDBE6FECEBDEDD5BF
	mulq	%rdx
	movq	%rdx, %rax
	movq	%rcx, %rdx
	shrq	$33, %rax
	movq	%rax, %rcx
	movabsq	$10000000000, %rsi              # imm = 0x2540BE400
	imulq	%rsi, %rcx
	subq	%rcx, %rdx
	xorl	%ecx, %ecx
	testq	%rdx, %rdx
	jne	.LBB0_9
.LBB0_11:                               #   in Loop: Header=BB0_2 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %esi
	sbbq	%rcx, %rsi
	testq	%r8, %r8
	cmovnsq	%rcx, %rsi
	cmovnsq	%rax, %rdx
	movq	%rdx, 8(%rsp)
	movq	%rsi, 16(%rsp)
	jmp	.LBB0_13
.LBB0_14:
	movq	(%rsp), %rax                    # 8-byte Reload
	movb	$1, 16(%rax)
	jmp	.LBB0_15
.LBB0_9:
	movb	$2, 8(%rsp)
.LBB0_10:
	movzbl	8(%rsp), %ecx
	movq	(%rsp), %rax                    # 8-byte Reload
	movq	%r13, (%rax)
	movb	%cl, 8(%rax)
	movb	$0, 16(%rax)
.LBB0_15:
	addq	$40, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE, .Lfunc_end0-_ZN23fixedwide_batch_codegen15scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.cfi_endproc
                                        # -- End function
	.globl	_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE # -- Begin function _ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE,@function
_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE: # @_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$40, %rsp
	.cfi_def_cfa_offset 96
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%rcx, 16(%rsp)                  # 8-byte Spill
	movq	%rdx, 24(%rsp)                  # 8-byte Spill
	movq	%rsi, %r13
	movq	%rdi, 8(%rsp)                   # 8-byte Spill
	movq	%r8, %rcx
	negq	%rcx
	movq	%r8, 32(%rsp)                   # 8-byte Spill
	cmovsq	%r8, %rcx
	movabsq	$-2601111570856684097, %rdx     # imm = 0xDBE6FECEBDEDD5BF
	movq	%rcx, %rax
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$33, %rsi
	movabsq	$10000000000, %rdx              # imm = 0x2540BE400
	imulq	%rdx, %rsi
	movq	%rcx, %rax
	subq	%rsi, %rax
	jmp	.LBB1_2
	.p2align	4
.LBB1_3:                                #   in Loop: Header=BB1_2 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $edx killed $edx def $rdx
	movq	%rsi, %rax
	testq	%rdx, %rdx
	je	.LBB1_4
.LBB1_2:                                # =>This Inner Loop Header: Depth=1
	movq	%rdx, %rsi
	movq	%rax, %rdx
	orq	%rsi, %rdx
	shrq	$32, %rdx
	je	.LBB1_3
# %bb.1:                                #   in Loop: Header=BB1_2 Depth=1
	xorl	%edx, %edx
	divq	%rsi
	movq	%rsi, %rax
	testq	%rdx, %rdx
	jne	.LBB1_2
.LBB1_4:
	movq	%rcx, %rax
	orq	%rsi, %rax
	shrq	$32, %rax
	je	.LBB1_6
# %bb.5:
	movq	%rcx, %rax
	xorl	%edx, %edx
	divq	%rsi
	movq	%rax, %rbp
	jmp	.LBB1_7
.LBB1_6:
	movl	%ecx, %eax
	xorl	%edx, %edx
	divl	%esi
	movl	%eax, %ebp
.LBB1_7:
	movq	$0, (%rsp)                      # 8-byte Folded Spill
	movabsq	$10000000000, %rax              # imm = 0x2540BE400
	xorl	%edx, %edx
	divq	%rsi
	movq	%rax, %rbx
	cmpq	$1, %rax
	je	.LBB1_9
# %bb.8:
	movl	$1, %esi
	xorl	%edi, %edi
	movq	%rbx, %rdx
	xorl	%ecx, %ecx
	callq	__udivti3@PLT
	movq	%rax, (%rsp)                    # 8-byte Spill
.LBB1_9:
	movb	$1, %cl
	cmpq	$0, 16(%rsp)                    # 8-byte Folded Reload
	je	.LBB1_36
# %bb.10:
	xorl	%r11d, %r11d
	xorl	%r10d, %r10d
	jmp	.LBB1_13
	.p2align	4
.LBB1_11:                               #   in Loop: Header=BB1_13 Depth=1
	movq	%rsi, %rax
	movq	%rdi, %r8
.LBB1_12:                               #   in Loop: Header=BB1_13 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %esi
	sbbq	%r8, %rsi
	testq	%rcx, %rcx
	cmovnsq	%r8, %rsi
	cmovnsq	%rax, %rdx
	movq	%rdx, %rax
	shrq	$8, %rax
	movb	%dl, (%r13,%r11)
	movq	%rdx, %rcx
	shrq	$56, %rcx
	movb	%cl, 7(%r13,%r11)
	shrq	$40, %rdx
	movw	%dx, 5(%r13,%r11)
	movl	%eax, 1(%r13,%r11)
	movq	%rsi, 8(%r13,%r11)
	incq	%r10
	addq	$16, %r11
	cmpq	%r10, 16(%rsp)                  # 8-byte Folded Reload
	je	.LBB1_35
.LBB1_13:                               # =>This Inner Loop Header: Depth=1
	movq	24(%rsp), %rax                  # 8-byte Reload
	movq	(%rax,%r11), %rdx
	movq	8(%rax,%r11), %rcx
	movq	%rdx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rcx, %rdi
	testq	%rcx, %rcx
	cmovnsq	%rdx, %rax
	cmovnsq	%rcx, %rdi
	movl	$0, %r8d
	cmpq	$1, %rbx
	je	.LBB1_20
# %bb.14:                               #   in Loop: Header=BB1_13 Depth=1
	testq	%rdi, %rdi
	je	.LBB1_17
# %bb.15:                               #   in Loop: Header=BB1_13 Depth=1
	cmpq	%rdi, %rbx
	jbe	.LBB1_18
# %bb.16:                               #   in Loop: Header=BB1_13 Depth=1
	xorl	%esi, %esi
	jmp	.LBB1_19
.LBB1_17:                               #   in Loop: Header=BB1_13 Depth=1
	movq	%rax, %r8
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rbx, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %rdx
	cmpq	%rbx, %r8
	movl	$0, %eax
	cmovaeq	%rbx, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB1_20
.LBB1_18:                               #   in Loop: Header=BB1_13 Depth=1
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbx, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbx, %rdi
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %rdi
.LBB1_19:                               #   in Loop: Header=BB1_13 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%rbx
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB1_20:                               #   in Loop: Header=BB1_13 Depth=1
	xorl	%r15d, %r15d
	xorq	32(%rsp), %rcx                  # 8-byte Folded Reload
	setns	%r15b
	negq	%r15
	movabsq	$-9223372036854775808, %r12     # imm = 0x8000000000000000
	sbbq	$0, %r12
	mulq	%rbp
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%rbp
	movq	%rax, %rdi
	addq	%r9, %rdi
	adcq	$0, %rdx
	jne	.LBB1_41
# %bb.21:                               #   in Loop: Header=BB1_13 Depth=1
	cmpq	%rsi, %r15
	movq	%r12, %rax
	sbbq	%rdi, %rax
	jb	.LBB1_41
# %bb.22:                               #   in Loop: Header=BB1_13 Depth=1
	testq	%r8, %r8
	je	.LBB1_11
# %bb.23:                               #   in Loop: Header=BB1_13 Depth=1
	movq	%r8, %rax
	mulq	%rbp
	movq	%rdx, %r8
	cmpq	$1, %rbx
	jne	.LBB1_25
# %bb.24:                               #   in Loop: Header=BB1_13 Depth=1
	xorl	%edx, %edx
	jmp	.LBB1_31
	.p2align	4
.LBB1_25:                               #   in Loop: Header=BB1_13 Depth=1
	testq	%r8, %r8
	je	.LBB1_28
# %bb.26:                               #   in Loop: Header=BB1_13 Depth=1
	cmpq	%r8, %rbx
	jbe	.LBB1_29
# %bb.27:                               #   in Loop: Header=BB1_13 Depth=1
	movq	%r8, %rdx
	xorl	%r8d, %r8d
	jmp	.LBB1_30
.LBB1_28:                               #   in Loop: Header=BB1_13 Depth=1
	movq	%rax, %r8
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbx, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %rax
	cmpq	%rbx, %r8
	movl	$0, %r9d
	cmovaeq	%rbx, %r9
	movq	%r8, %rdx
	subq	%r9, %rdx
	xorl	%r8d, %r8d
	jmp	.LBB1_31
.LBB1_29:                               #   in Loop: Header=BB1_13 Depth=1
	movq	%rax, %r9
	movq	%r8, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r9, %rax
	movq	%rdx, %r9
	movq	%rbx, %rdx
	imulq	%r9, %rdx
	subq	%rdx, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %r9
	cmpq	%rbx, %r8
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %r8
	movq	%r8, %rdx
	movq	%r9, %r8
.LBB1_30:                               #   in Loop: Header=BB1_13 Depth=1
	#APP
	divq	%rbx
	#NO_APP
.LBB1_31:                               #   in Loop: Header=BB1_13 Depth=1
	movq	%r15, %r9
	subq	%rsi, %r9
	movq	%r12, %r14
	sbbq	%rdi, %r14
	cmpq	%rax, %r9
	sbbq	%r8, %r14
	jb	.LBB1_41
# %bb.32:                               #   in Loop: Header=BB1_13 Depth=1
	addq	%rsi, %rax
	adcq	%rdi, %r8
	cmpq	%rax, %r15
	sbbq	%r8, %r12
	jb	.LBB1_41
# %bb.33:                               #   in Loop: Header=BB1_13 Depth=1
	testq	%rdx, %rdx
	je	.LBB1_12
# %bb.34:
	movb	$2, %cl
	jmp	.LBB1_42
.LBB1_35:
	movq	8(%rsp), %rax                   # 8-byte Reload
	movb	$1, %cl
	jmp	.LBB1_37
.LBB1_36:
	movq	8(%rsp), %rax                   # 8-byte Reload
.LBB1_37:
	movb	%cl, 16(%rax)
	addq	$40, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.LBB1_41:
	.cfi_def_cfa_offset 96
	xorl	%ecx, %ecx
.LBB1_42:
	movq	8(%rsp), %rax                   # 8-byte Reload
	movq	%r10, (%rax)
	movb	%cl, 8(%rax)
	xorl	%ecx, %ecx
	jmp	.LBB1_37
.Lfunc_end1:
	.size	_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE, .Lfunc_end1-_ZN23fixedwide_batch_codegen24prepared_scalar_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.cfi_endproc
                                        # -- End function
	.globl	_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE # -- Begin function _ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE,@function
_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE: # @_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$200, %rsp
	.cfi_def_cfa_offset 256
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%r8, %rbp
	movq	%rcx, 128(%rsp)                 # 8-byte Spill
	movq	%rdx, %r14
	movq	%rsi, %r13
	movq	%rdi, 56(%rsp)                  # 8-byte Spill
	movq	%r8, %rcx
	negq	%rcx
	cmovsq	%r8, %rcx
	movabsq	$-2601111570856684097, %rdx     # imm = 0xDBE6FECEBDEDD5BF
	movq	%rcx, %rax
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$33, %rsi
	movabsq	$10000000000, %rdx              # imm = 0x2540BE400
	imulq	%rdx, %rsi
	movq	%rcx, %rax
	subq	%rsi, %rax
	jmp	.LBB2_1
	.p2align	4
.LBB2_3:                                #   in Loop: Header=BB2_1 Depth=1
	xorl	%edx, %edx
	divq	%rsi
	movq	%rsi, %rax
	testq	%rdx, %rdx
	je	.LBB2_5
.LBB2_1:                                # =>This Inner Loop Header: Depth=1
	movq	%rdx, %rsi
	movq	%rax, %rdx
	orq	%rsi, %rdx
	shrq	$32, %rdx
	jne	.LBB2_3
# %bb.2:                                #   in Loop: Header=BB2_1 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $edx killed $edx def $rdx
	movq	%rsi, %rax
	testq	%rdx, %rdx
	jne	.LBB2_1
.LBB2_5:
	movq	%rcx, %rax
	orq	%rsi, %rax
	shrq	$32, %rax
	je	.LBB2_6
# %bb.7:
	movq	%rcx, %rax
	xorl	%edx, %edx
	divq	%rsi
	jmp	.LBB2_8
.LBB2_6:
	movl	%ecx, %eax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $eax killed $eax def $rax
.LBB2_8:
	movq	%rax, 48(%rsp)                  # 8-byte Spill
	movq	$0, 16(%rsp)                    # 8-byte Folded Spill
	movabsq	$10000000000, %rax              # imm = 0x2540BE400
	xorl	%edx, %edx
	divq	%rsi
	movq	%rax, %rbx
	cmpq	$1, %rax
	je	.LBB2_10
# %bb.9:
	movl	$1, %esi
	xorl	%edi, %edi
	movq	%rbx, %rdx
	xorl	%ecx, %ecx
	callq	__udivti3@PLT
	movq	%rax, 16(%rsp)                  # 8-byte Spill
.LBB2_10:
	shrq	$63, %rbp
	cmpq	$4, 128(%rsp)                   # 8-byte Folded Reload
	movq	%rbp, 120(%rsp)                 # 8-byte Spill
	movq	%r14, 64(%rsp)                  # 8-byte Spill
	jb	.LBB2_11
# %bb.18:
	movl	$56, %r15d
	xorl	%r9d, %r9d
                                        # implicit-def: $al
	movq	%rax, 112(%rsp)                 # 8-byte Spill
                                        # implicit-def: $al
	movq	%rax, 104(%rsp)                 # 8-byte Spill
                                        # implicit-def: $al
	movq	%rax, 96(%rsp)                  # 8-byte Spill
                                        # implicit-def: $al
	movq	%rax, 88(%rsp)                  # 8-byte Spill
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
	.p2align	4
.LBB2_19:                               # =>This Inner Loop Header: Depth=1
	movq	-56(%r14,%r15), %rdx
	movq	-48(%r14,%r15), %rsi
	movq	%rsi, %rcx
	shrq	$63, %rcx
	movq	%rdx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rsi, %rdi
	testq	%rsi, %rsi
	cmovnsq	%rdx, %rax
	cmovnsq	%rsi, %rdi
	movl	$0, %r8d
	cmpq	$1, %rbx
	movq	%r9, 32(%rsp)                   # 8-byte Spill
	je	.LBB2_26
# %bb.20:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB2_21
# %bb.22:                               #   in Loop: Header=BB2_19 Depth=1
	cmpq	%rdi, %rbx
	jbe	.LBB2_24
# %bb.23:                               #   in Loop: Header=BB2_19 Depth=1
	xorl	%esi, %esi
	jmp	.LBB2_25
.LBB2_21:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %r8
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rbx, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %rdx
	cmpq	%rbx, %r8
	movl	$0, %eax
	cmovaeq	%rbx, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB2_26
.LBB2_24:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbx, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbx, %rdi
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %rdi
.LBB2_25:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%rbx
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB2_26:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 40(%rsp)                    # 8-byte Folded Spill
	cmpq	%rbp, %rcx
	movl	$0, %r10d
	movq	$-1, %rdx
	cmoveq	%rdx, %r10
	cmpl	%ebp, %ecx
	movabsq	$-9223372036854775808, %r11     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rdx      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rdx, %r11
	movq	48(%rsp), %r12                  # 8-byte Reload
	mulq	%r12
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r12
	addq	%r9, %rax
	adcq	$0, %rdx
	jne	.LBB2_27
# %bb.28:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 40(%rsp)                    # 8-byte Folded Spill
	cmpq	%rsi, %r10
	movq	%r11, %rdx
	sbbq	%rax, %rdx
	jb	.LBB2_27
# %bb.29:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%r8, %r8
	je	.LBB2_30
# %bb.31:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %r9
	movq	%r8, %rax
	mulq	48(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rdi
	cmpq	$1, %rbx
	jne	.LBB2_33
# %bb.32:                               #   in Loop: Header=BB2_19 Depth=1
	xorl	%edx, %edx
	jmp	.LBB2_39
.LBB2_30:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rsi, %rdi
	negq	%rdi
	movl	$0, %edx
	sbbq	%rax, %rdx
	cmpl	%ebp, %ecx
	cmoveq	%rax, %rdx
	movq	%rdx, 192(%rsp)                 # 8-byte Spill
	cmpq	%rbp, %rcx
	cmoveq	%rsi, %rdi
	movq	%rdi, %rax
	movq	%rdi, 40(%rsp)                  # 8-byte Spill
.LBB2_45:                               #   in Loop: Header=BB2_19 Depth=1
	shrq	$8, %rax
	movq	%rax, 184(%rsp)                 # 8-byte Spill
	movb	$1, %al
	movl	%eax, 12(%rsp)                  # 4-byte Spill
	jmp	.LBB2_46
.LBB2_33:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB2_34
# %bb.35:                               #   in Loop: Header=BB2_19 Depth=1
	cmpq	%rdi, %rbx
	jbe	.LBB2_37
# %bb.36:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rdi, %rdx
	xorl	%edi, %edi
	jmp	.LBB2_38
.LBB2_34:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rdi
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbx, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %rax
	cmpq	%rbx, %rdi
	movl	$0, %r8d
	cmovaeq	%rbx, %r8
	movq	%rdi, %rdx
	subq	%r8, %rdx
	xorl	%edi, %edi
	jmp	.LBB2_39
.LBB2_37:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %r8
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%r8, %rax
	movq	%rdx, %r8
	movq	%rbx, %rdx
	imulq	%r8, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %r8
	cmpq	%rbx, %rdi
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %rdi
	movq	%rdi, %rdx
	movq	%r8, %rdi
.LBB2_38:                               #   in Loop: Header=BB2_19 Depth=1
	#APP
	divq	%rbx
	#NO_APP
.LBB2_39:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%r10, %r8
	subq	%rsi, %r8
	movq	%r11, %r12
	sbbq	%r9, %r12
	movq	$0, 40(%rsp)                    # 8-byte Folded Spill
	cmpq	%rax, %r8
	sbbq	%rdi, %r12
	jb	.LBB2_27
# %bb.40:                               #   in Loop: Header=BB2_19 Depth=1
	addq	%rsi, %rax
	adcq	%r9, %rdi
	cmpq	%rax, %r10
	sbbq	%rdi, %r11
	jae	.LBB2_42
# %bb.41:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 40(%rsp)                    # 8-byte Folded Spill
	.p2align	4
.LBB2_27:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 112(%rsp)                   # 8-byte Folded Spill
	movl	$0, 12(%rsp)                    # 4-byte Folded Spill
.LBB2_46:                               #   in Loop: Header=BB2_19 Depth=1
	movq	-40(%r14,%r15), %rdx
	movq	-32(%r14,%r15), %rsi
	movq	%rsi, %rcx
	shrq	$63, %rcx
	movq	%rdx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rsi, %rdi
	testq	%rsi, %rsi
	cmovnsq	%rdx, %rax
	cmovnsq	%rsi, %rdi
	movl	$0, %r8d
	cmpq	$1, %rbx
	je	.LBB2_53
# %bb.47:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB2_48
# %bb.49:                               #   in Loop: Header=BB2_19 Depth=1
	cmpq	%rdi, %rbx
	jbe	.LBB2_51
# %bb.50:                               #   in Loop: Header=BB2_19 Depth=1
	xorl	%esi, %esi
	jmp	.LBB2_52
.LBB2_48:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %r8
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rbx, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %rdx
	cmpq	%rbx, %r8
	movl	$0, %eax
	cmovaeq	%rbx, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB2_53
.LBB2_51:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbx, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbx, %rdi
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %rdi
.LBB2_52:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%rbx
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB2_53:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 80(%rsp)                    # 8-byte Folded Spill
	cmpq	%rbp, %rcx
	movl	$0, %r10d
	movq	$-1, %rdx
	cmoveq	%rdx, %r10
	cmpl	%ebp, %ecx
	movabsq	$-9223372036854775808, %r11     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rdx      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rdx, %r11
	movq	48(%rsp), %r12                  # 8-byte Reload
	mulq	%r12
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r12
	addq	%r9, %rax
	adcq	$0, %rdx
	jne	.LBB2_54
# %bb.55:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 80(%rsp)                    # 8-byte Folded Spill
	cmpq	%rsi, %r10
	movq	%r11, %rdx
	sbbq	%rax, %rdx
	jb	.LBB2_54
# %bb.56:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%r8, %r8
	je	.LBB2_57
# %bb.58:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %r9
	movq	%r8, %rax
	mulq	48(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rdi
	cmpq	$1, %rbx
	jne	.LBB2_60
# %bb.59:                               #   in Loop: Header=BB2_19 Depth=1
	xorl	%edx, %edx
	jmp	.LBB2_66
.LBB2_57:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rsi, %rdi
	negq	%rdi
	movl	$0, %edx
	sbbq	%rax, %rdx
	cmpl	%ebp, %ecx
	cmoveq	%rax, %rdx
	movq	%rdx, 176(%rsp)                 # 8-byte Spill
	cmpq	%rbp, %rcx
	cmoveq	%rsi, %rdi
	movq	%rdi, %rax
	movq	%rdi, 80(%rsp)                  # 8-byte Spill
.LBB2_72:                               #   in Loop: Header=BB2_19 Depth=1
	shrq	$8, %rax
	movq	%rax, 168(%rsp)                 # 8-byte Spill
	movb	$1, %al
	movl	%eax, 28(%rsp)                  # 4-byte Spill
	jmp	.LBB2_73
.LBB2_60:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB2_61
# %bb.62:                               #   in Loop: Header=BB2_19 Depth=1
	cmpq	%rdi, %rbx
	jbe	.LBB2_64
# %bb.63:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rdi, %rdx
	xorl	%edi, %edi
	jmp	.LBB2_65
.LBB2_42:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB2_44
# %bb.43:                               #   in Loop: Header=BB2_19 Depth=1
	movb	$2, %al
	movq	%rax, 112(%rsp)                 # 8-byte Spill
	movl	$0, 12(%rsp)                    # 4-byte Folded Spill
	movb	$2, %al
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	jmp	.LBB2_46
.LBB2_61:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rdi
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbx, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %rax
	cmpq	%rbx, %rdi
	movl	$0, %r8d
	cmovaeq	%rbx, %r8
	movq	%rdi, %rdx
	subq	%r8, %rdx
	xorl	%edi, %edi
	jmp	.LBB2_66
.LBB2_64:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %r8
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%r8, %rax
	movq	%rdx, %r8
	movq	%rbx, %rdx
	imulq	%r8, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %r8
	cmpq	%rbx, %rdi
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %rdi
	movq	%rdi, %rdx
	movq	%r8, %rdi
.LBB2_65:                               #   in Loop: Header=BB2_19 Depth=1
	#APP
	divq	%rbx
	#NO_APP
.LBB2_66:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%r10, %r8
	subq	%rsi, %r8
	movq	%r11, %r12
	sbbq	%r9, %r12
	movq	$0, 80(%rsp)                    # 8-byte Folded Spill
	cmpq	%rax, %r8
	sbbq	%rdi, %r12
	jb	.LBB2_54
# %bb.67:                               #   in Loop: Header=BB2_19 Depth=1
	addq	%rsi, %rax
	adcq	%r9, %rdi
	cmpq	%rax, %r10
	sbbq	%rdi, %r11
	jae	.LBB2_69
# %bb.68:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 80(%rsp)                    # 8-byte Folded Spill
	.p2align	4
.LBB2_54:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 104(%rsp)                   # 8-byte Folded Spill
	movl	$0, 28(%rsp)                    # 4-byte Folded Spill
.LBB2_73:                               #   in Loop: Header=BB2_19 Depth=1
	movq	-24(%r14,%r15), %rdx
	movq	-16(%r14,%r15), %rsi
	movq	%rsi, %rcx
	shrq	$63, %rcx
	movq	%rdx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rsi, %rdi
	testq	%rsi, %rsi
	cmovnsq	%rdx, %rax
	cmovnsq	%rsi, %rdi
	movl	$0, %r8d
	cmpq	$1, %rbx
	jne	.LBB2_75
# %bb.74:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%r14, %r11
	jmp	.LBB2_81
	.p2align	4
.LBB2_75:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB2_76
# %bb.77:                               #   in Loop: Header=BB2_19 Depth=1
	cmpq	%rdi, %rbx
	jbe	.LBB2_79
# %bb.78:                               #   in Loop: Header=BB2_19 Depth=1
	xorl	%esi, %esi
	jmp	.LBB2_80
.LBB2_76:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%r14, %r11
	movq	%rax, %r8
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rbx, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %rdx
	cmpq	%rbx, %r8
	movl	$0, %eax
	cmovaeq	%rbx, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB2_81
.LBB2_79:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbx, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbx, %rdi
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %rdi
.LBB2_80:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%r14, %r11
	movq	%rdi, %rdx
	#APP
	divq	%rbx
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB2_81:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 72(%rsp)                    # 8-byte Folded Spill
	cmpq	%rbp, %rcx
	movl	$0, %r10d
	movq	$-1, %rdx
	cmoveq	%rdx, %r10
	cmpl	%ebp, %ecx
	movabsq	$-9223372036854775808, %r12     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rdx      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rdx, %r12
	movq	48(%rsp), %r14                  # 8-byte Reload
	mulq	%r14
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r14
	addq	%r9, %rax
	adcq	$0, %rdx
	jne	.LBB2_82
# %bb.83:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 72(%rsp)                    # 8-byte Folded Spill
	cmpq	%rsi, %r10
	movq	%r12, %rdx
	sbbq	%rax, %rdx
	jae	.LBB2_84
.LBB2_82:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 96(%rsp)                    # 8-byte Folded Spill
	movl	$0, 24(%rsp)                    # 4-byte Folded Spill
.LBB2_102:                              #   in Loop: Header=BB2_19 Depth=1
	movq	-8(%r11,%r15), %rcx
	movq	(%r11,%r15), %rdx
	movq	%rdx, %r10
	shrq	$63, %r10
	movq	%rcx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rdx, %rdi
	testq	%rdx, %rdx
	cmovnsq	%rcx, %rax
	cmovnsq	%rdx, %rdi
	movl	$0, %r8d
	cmpq	$1, %rbx
	je	.LBB2_109
# %bb.103:                              #   in Loop: Header=BB2_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB2_104
# %bb.105:                              #   in Loop: Header=BB2_19 Depth=1
	cmpq	%rdi, %rbx
	jbe	.LBB2_107
# %bb.106:                              #   in Loop: Header=BB2_19 Depth=1
	xorl	%esi, %esi
	jmp	.LBB2_108
	.p2align	4
.LBB2_84:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%r8, %r8
	je	.LBB2_85
# %bb.86:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %r9
	movq	%r8, %rax
	mulq	48(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rdi
	cmpq	$1, %rbx
	jne	.LBB2_88
# %bb.87:                               #   in Loop: Header=BB2_19 Depth=1
	xorl	%edx, %edx
	jmp	.LBB2_94
.LBB2_104:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %r8
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rbx, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %rdx
	cmpq	%rbx, %r8
	movl	$0, %eax
	cmovaeq	%rbx, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB2_109
.LBB2_107:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rcx
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rcx, %rax
	movq	%rdx, %rsi
	movq	%rbx, %rcx
	imulq	%rdx, %rcx
	subq	%rcx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbx, %rdi
	movl	$0, %ecx
	cmovaeq	%rbx, %rcx
	subq	%rcx, %rdi
.LBB2_108:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%rbx
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB2_109:                              #   in Loop: Header=BB2_19 Depth=1
	xorl	%esi, %esi
	cmpq	%rbp, %r10
	movl	$0, %r12d
	movq	$-1, %rcx
	cmoveq	%rcx, %r12
	cmpl	%ebp, %r10d
	movabsq	$-9223372036854775808, %r14     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rcx      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rcx, %r14
	movq	48(%rsp), %r11                  # 8-byte Reload
	mulq	%r11
	movq	%rdx, %r9
	movq	%rax, %rcx
	movq	%rdi, %rax
	mulq	%r11
	addq	%r9, %rax
	adcq	$0, %rdx
	jne	.LBB2_110
# %bb.112:                              #   in Loop: Header=BB2_19 Depth=1
	xorl	%esi, %esi
	cmpq	%rcx, %r12
	movq	%r14, %rdx
	sbbq	%rax, %rdx
	jae	.LBB2_113
.LBB2_110:                              #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 88(%rsp)                    # 8-byte Folded Spill
	xorl	%eax, %eax
.LBB2_111:                              #   in Loop: Header=BB2_19 Depth=1
	movq	64(%rsp), %r14                  # 8-byte Reload
	movq	32(%rsp), %r9                   # 8-byte Reload
	movq	40(%rsp), %rdx                  # 8-byte Reload
	movl	12(%rsp), %r8d                  # 4-byte Reload
.LBB2_129:                              #   in Loop: Header=BB2_19 Depth=1
	testb	%r8b, %r8b
	je	.LBB2_130
# %bb.131:                              #   in Loop: Header=BB2_19 Depth=1
	movb	%dl, -56(%r13,%r15)
	movq	184(%rsp), %rdx                 # 8-byte Reload
	movl	%edx, -55(%r13,%r15)
	movq	%rdx, %rcx
	shrq	$48, %rcx
	movb	%cl, -49(%r13,%r15)
	movq	%rdx, %rcx
	shrq	$32, %rcx
	movw	%cx, -51(%r13,%r15)
	movq	192(%rsp), %rcx                 # 8-byte Reload
	movq	%rcx, -48(%r13,%r15)
	cmpb	$0, 28(%rsp)                    # 1-byte Folded Reload
	je	.LBB2_132
# %bb.133:                              #   in Loop: Header=BB2_19 Depth=1
	movq	80(%rsp), %rcx                  # 8-byte Reload
	movb	%cl, -40(%r13,%r15)
	movq	168(%rsp), %rdx                 # 8-byte Reload
	movl	%edx, -39(%r13,%r15)
	movq	%rdx, %rcx
	shrq	$48, %rcx
	movb	%cl, -33(%r13,%r15)
	movq	%rdx, %rcx
	shrq	$32, %rcx
	movw	%cx, -35(%r13,%r15)
	movq	176(%rsp), %rcx                 # 8-byte Reload
	movq	%rcx, -32(%r13,%r15)
	cmpb	$0, 24(%rsp)                    # 1-byte Folded Reload
	je	.LBB2_134
# %bb.135:                              #   in Loop: Header=BB2_19 Depth=1
	movq	72(%rsp), %rcx                  # 8-byte Reload
	movb	%cl, -24(%r13,%r15)
	movq	152(%rsp), %rdx                 # 8-byte Reload
	movl	%edx, -23(%r13,%r15)
	movq	%rdx, %rcx
	shrq	$48, %rcx
	movb	%cl, -17(%r13,%r15)
	movq	%rdx, %rcx
	shrq	$32, %rcx
	movw	%cx, -19(%r13,%r15)
	movq	160(%rsp), %rcx                 # 8-byte Reload
	movq	%rcx, -16(%r13,%r15)
	testb	%al, %al
	je	.LBB2_136
# %bb.12:                               #   in Loop: Header=BB2_19 Depth=1
	movb	%sil, -8(%r13,%r15)
	movq	136(%rsp), %rcx                 # 8-byte Reload
	movl	%ecx, -7(%r13,%r15)
	movq	%rcx, %rax
	shrq	$48, %rax
	movb	%al, -1(%r13,%r15)
	movq	%rcx, %rax
	shrq	$32, %rax
	movw	%ax, -3(%r13,%r15)
	movq	144(%rsp), %rax                 # 8-byte Reload
	movq	%rax, (%r13,%r15)
	addq	$64, %r15
	movq	128(%rsp), %rax                 # 8-byte Reload
	addq	%r9, %rax
	addq	$-4, %rax
	addq	$-4, %r9
	cmpq	$3, %rax
	ja	.LBB2_19
	jmp	.LBB2_13
	.p2align	4
.LBB2_113:                              #   in Loop: Header=BB2_19 Depth=1
	testq	%r8, %r8
	je	.LBB2_114
# %bb.115:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rsi
	movq	%r8, %rax
	mulq	48(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rdi
	cmpq	$1, %rbx
	jne	.LBB2_117
# %bb.116:                              #   in Loop: Header=BB2_19 Depth=1
	xorl	%edx, %edx
	jmp	.LBB2_123
.LBB2_85:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rsi, %rdi
	negq	%rdi
	movl	$0, %edx
	sbbq	%rax, %rdx
	cmpl	%ebp, %ecx
	cmoveq	%rax, %rdx
	movq	%rdx, 160(%rsp)                 # 8-byte Spill
	cmpq	%rbp, %rcx
	cmoveq	%rsi, %rdi
	movq	%rdi, %rax
	movq	%rdi, 72(%rsp)                  # 8-byte Spill
.LBB2_101:                              #   in Loop: Header=BB2_19 Depth=1
	shrq	$8, %rax
	movq	%rax, 152(%rsp)                 # 8-byte Spill
	movb	$1, %al
	movl	%eax, 24(%rsp)                  # 4-byte Spill
	jmp	.LBB2_102
.LBB2_114:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%rcx, %rsi
	negq	%rsi
	movl	$0, %edx
	sbbq	%rax, %rdx
	cmpl	%ebp, %r10d
	cmoveq	%rax, %rdx
	movq	%rdx, 144(%rsp)                 # 8-byte Spill
	cmpq	%rbp, %r10
	cmoveq	%rcx, %rsi
	movq	%rsi, %rax
	shrq	$8, %rax
	movq	%rax, 136(%rsp)                 # 8-byte Spill
	movb	$1, %al
	jmp	.LBB2_111
.LBB2_88:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB2_89
# %bb.90:                               #   in Loop: Header=BB2_19 Depth=1
	cmpq	%rdi, %rbx
	jbe	.LBB2_92
# %bb.91:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rdi, %rdx
	xorl	%edi, %edi
	jmp	.LBB2_93
.LBB2_117:                              #   in Loop: Header=BB2_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB2_118
# %bb.119:                              #   in Loop: Header=BB2_19 Depth=1
	cmpq	%rdi, %rbx
	jbe	.LBB2_121
# %bb.120:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%rdi, %rdx
	xorl	%edi, %edi
	jmp	.LBB2_122
.LBB2_69:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB2_71
# %bb.70:                               #   in Loop: Header=BB2_19 Depth=1
	movb	$2, %al
	movq	%rax, 104(%rsp)                 # 8-byte Spill
	movl	$0, 28(%rsp)                    # 4-byte Folded Spill
	movb	$2, %al
	movq	%rax, 80(%rsp)                  # 8-byte Spill
	jmp	.LBB2_73
.LBB2_89:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rdi
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbx, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %rax
	cmpq	%rbx, %rdi
	movl	$0, %r8d
	cmovaeq	%rbx, %r8
	movq	%rdi, %rdx
	subq	%r8, %rdx
	xorl	%edi, %edi
	jmp	.LBB2_94
.LBB2_118:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rdi
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbx, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %rax
	cmpq	%rbx, %rdi
	movl	$0, %r8d
	cmovaeq	%rbx, %r8
	movq	%rdi, %rdx
	subq	%r8, %rdx
	xorl	%edi, %edi
	jmp	.LBB2_123
.LBB2_92:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %r8
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%r8, %rax
	movq	%rdx, %r8
	movq	%rbx, %rdx
	imulq	%r8, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %r8
	cmpq	%rbx, %rdi
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %rdi
	movq	%rdi, %rdx
	movq	%r8, %rdi
.LBB2_93:                               #   in Loop: Header=BB2_19 Depth=1
	#APP
	divq	%rbx
	#NO_APP
.LBB2_94:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%r10, %r8
	subq	%rsi, %r8
	movq	%r12, %rbp
	sbbq	%r9, %rbp
	movq	$0, 72(%rsp)                    # 8-byte Folded Spill
	cmpq	%rax, %r8
	sbbq	%rdi, %rbp
	jae	.LBB2_96
# %bb.95:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 96(%rsp)                    # 8-byte Folded Spill
	movl	$0, 24(%rsp)                    # 4-byte Folded Spill
	movq	120(%rsp), %rbp                 # 8-byte Reload
	jmp	.LBB2_102
.LBB2_96:                               #   in Loop: Header=BB2_19 Depth=1
	addq	%rsi, %rax
	adcq	%r9, %rdi
	cmpq	%rax, %r10
	sbbq	%rdi, %r12
	movq	120(%rsp), %rbp                 # 8-byte Reload
	jae	.LBB2_98
# %bb.97:                               #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 72(%rsp)                    # 8-byte Folded Spill
	jmp	.LBB2_82
.LBB2_98:                               #   in Loop: Header=BB2_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB2_100
# %bb.99:                               #   in Loop: Header=BB2_19 Depth=1
	movb	$2, %al
	movq	%rax, 96(%rsp)                  # 8-byte Spill
	movl	$0, 24(%rsp)                    # 4-byte Folded Spill
	movb	$2, %al
	movq	%rax, 72(%rsp)                  # 8-byte Spill
	jmp	.LBB2_102
.LBB2_121:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %r8
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%r8, %rax
	movq	%rdx, %r8
	movq	%rbx, %rdx
	imulq	%r8, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %r8
	cmpq	%rbx, %rdi
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %rdi
	movq	%rdi, %rdx
	movq	%r8, %rdi
.LBB2_122:                              #   in Loop: Header=BB2_19 Depth=1
	#APP
	divq	%rbx
	#NO_APP
.LBB2_123:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%r12, %r9
	subq	%rcx, %r9
	movq	%r14, %rbp
	movq	%rsi, %r8
	sbbq	%rsi, %rbp
	xorl	%esi, %esi
	cmpq	%rax, %r9
	sbbq	%rdi, %rbp
	jae	.LBB2_125
# %bb.124:                              #   in Loop: Header=BB2_19 Depth=1
	movq	$0, 88(%rsp)                    # 8-byte Folded Spill
	xorl	%eax, %eax
	movq	120(%rsp), %rbp                 # 8-byte Reload
	jmp	.LBB2_111
.LBB2_125:                              #   in Loop: Header=BB2_19 Depth=1
	addq	%rcx, %rax
	adcq	%r8, %rdi
	xorl	%esi, %esi
	cmpq	%rax, %r12
	sbbq	%rdi, %r14
	movq	120(%rsp), %rbp                 # 8-byte Reload
	jb	.LBB2_110
# %bb.126:                              #   in Loop: Header=BB2_19 Depth=1
	testq	%rdx, %rdx
	movq	64(%rsp), %r14                  # 8-byte Reload
	movq	32(%rsp), %r9                   # 8-byte Reload
	movq	40(%rsp), %rdx                  # 8-byte Reload
	movl	12(%rsp), %r8d                  # 4-byte Reload
	je	.LBB2_128
# %bb.127:                              #   in Loop: Header=BB2_19 Depth=1
	movb	$2, %al
	movq	%rax, 88(%rsp)                  # 8-byte Spill
	xorl	%eax, %eax
	movb	$2, %sil
	jmp	.LBB2_129
.LBB2_44:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rsi
	negq	%rsi
	movl	$0, %edx
	sbbq	%rdi, %rdx
	cmpl	%ebp, %ecx
	cmoveq	%rdi, %rdx
	movq	%rdx, 192(%rsp)                 # 8-byte Spill
	cmpq	%rbp, %rcx
	cmoveq	%rax, %rsi
	movq	%rsi, %rax
	movq	%rsi, 40(%rsp)                  # 8-byte Spill
	jmp	.LBB2_45
.LBB2_71:                               #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rsi
	negq	%rsi
	movl	$0, %edx
	sbbq	%rdi, %rdx
	cmpl	%ebp, %ecx
	cmoveq	%rdi, %rdx
	movq	%rdx, 176(%rsp)                 # 8-byte Spill
	cmpq	%rbp, %rcx
	cmoveq	%rax, %rsi
	movq	%rsi, %rax
	movq	%rsi, 80(%rsp)                  # 8-byte Spill
	jmp	.LBB2_72
.LBB2_100:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rsi
	negq	%rsi
	movl	$0, %edx
	sbbq	%rdi, %rdx
	cmpl	%ebp, %ecx
	cmoveq	%rdi, %rdx
	movq	%rdx, 160(%rsp)                 # 8-byte Spill
	cmpq	%rbp, %rcx
	cmoveq	%rax, %rsi
	movq	%rsi, %rax
	movq	%rsi, 72(%rsp)                  # 8-byte Spill
	jmp	.LBB2_101
.LBB2_128:                              #   in Loop: Header=BB2_19 Depth=1
	movq	%rax, %rsi
	negq	%rsi
	movl	$0, %ecx
	sbbq	%rdi, %rcx
	cmpl	%ebp, %r10d
	cmoveq	%rdi, %rcx
	movq	%rcx, 144(%rsp)                 # 8-byte Spill
	cmpq	%rbp, %r10
	cmoveq	%rax, %rsi
	movq	%rsi, %rax
	shrq	$8, %rax
	movq	%rax, 136(%rsp)                 # 8-byte Spill
	movb	$1, %al
	jmp	.LBB2_129
.LBB2_11:
	xorl	%r9d, %r9d
	cmpq	128(%rsp), %r9                  # 8-byte Folded Reload
	jb	.LBB2_15
	jmp	.LBB2_152
.LBB2_13:
	negq	%r9
	cmpq	128(%rsp), %r9                  # 8-byte Folded Reload
	jae	.LBB2_152
.LBB2_15:
	movq	%r9, %r10
	shlq	$4, %r10
	jmp	.LBB2_16
	.p2align	4
.LBB2_150:                              #   in Loop: Header=BB2_16 Depth=1
	movq	%rsi, %rax
	movq	%rdi, %r8
	movq	32(%rsp), %r9                   # 8-byte Reload
	movq	%rcx, %r14
.LBB2_151:                              #   in Loop: Header=BB2_16 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %esi
	sbbq	%r8, %rsi
	movq	120(%rsp), %rbp                 # 8-byte Reload
	cmpl	%ebp, %r11d
	cmoveq	%r8, %rsi
	cmpq	%rbp, %r11
	cmoveq	%rax, %rdx
	movq	%rdx, %rax
	shrq	$8, %rax
	movb	%dl, (%r13,%r10)
	movq	%rdx, %rdi
	shrq	$56, %rdi
	movb	%dil, 7(%r13,%r10)
	shrq	$40, %rdx
	movw	%dx, 5(%r13,%r10)
	movl	%eax, 1(%r13,%r10)
	movq	%rsi, 8(%r13,%r10)
	incq	%r9
	addq	$16, %r10
	cmpq	%r9, 128(%rsp)                  # 8-byte Folded Reload
	je	.LBB2_152
.LBB2_16:                               # =>This Inner Loop Header: Depth=1
	movq	(%r14,%r10), %rdx
	movq	8(%r14,%r10), %rsi
	movq	%rsi, %r11
	shrq	$63, %r11
	movq	%rdx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rsi, %rdi
	testq	%rsi, %rsi
	cmovnsq	%rdx, %rax
	cmovnsq	%rsi, %rdi
	movl	$0, %r8d
	cmpq	$1, %rbx
	movq	%r9, 32(%rsp)                   # 8-byte Spill
	jne	.LBB2_139
# %bb.17:                               #   in Loop: Header=BB2_16 Depth=1
	movq	%r14, %rcx
	jmp	.LBB2_145
	.p2align	4
.LBB2_139:                              #   in Loop: Header=BB2_16 Depth=1
	testq	%rdi, %rdi
	je	.LBB2_140
# %bb.141:                              #   in Loop: Header=BB2_16 Depth=1
	cmpq	%rdi, %rbx
	jbe	.LBB2_143
# %bb.142:                              #   in Loop: Header=BB2_16 Depth=1
	xorl	%esi, %esi
	jmp	.LBB2_144
.LBB2_140:                              #   in Loop: Header=BB2_16 Depth=1
	movq	%r14, %rcx
	movq	%rax, %r8
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rbx, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %rdx
	cmpq	%rbx, %r8
	movl	$0, %eax
	cmovaeq	%rbx, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB2_145
.LBB2_143:                              #   in Loop: Header=BB2_16 Depth=1
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbx, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	cmpq	%rbx, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbx, %rdi
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %rdi
.LBB2_144:                              #   in Loop: Header=BB2_16 Depth=1
	movq	%r14, %rcx
	movq	%rdi, %rdx
	#APP
	divq	%rbx
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB2_145:                              #   in Loop: Header=BB2_16 Depth=1
	xorl	%r15d, %r15d
	cmpq	%rbp, %r11
	movl	$0, %r12d
	movq	$-1, %rdx
	cmoveq	%rdx, %r12
	cmpl	%ebp, %r11d
	movabsq	$-9223372036854775808, %rbp     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rdx      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rdx, %rbp
	movq	48(%rsp), %r14                  # 8-byte Reload
	mulq	%r14
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r14
	movq	%rax, %rdi
	addq	%r9, %rdi
	adcq	$0, %rdx
	jne	.LBB2_146
# %bb.147:                              #   in Loop: Header=BB2_16 Depth=1
	cmpq	%rsi, %r12
	movq	%rbp, %rax
	sbbq	%rdi, %rax
	jb	.LBB2_148
# %bb.149:                              #   in Loop: Header=BB2_16 Depth=1
	testq	%r8, %r8
	je	.LBB2_150
# %bb.153:                              #   in Loop: Header=BB2_16 Depth=1
	movq	%r8, %rax
	mulq	48(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %r8
	cmpq	$1, %rbx
	jne	.LBB2_155
# %bb.154:                              #   in Loop: Header=BB2_16 Depth=1
	xorl	%edx, %edx
	jmp	.LBB2_161
	.p2align	4
.LBB2_155:                              #   in Loop: Header=BB2_16 Depth=1
	testq	%r8, %r8
	je	.LBB2_156
# %bb.157:                              #   in Loop: Header=BB2_16 Depth=1
	cmpq	%r8, %rbx
	jbe	.LBB2_159
# %bb.158:                              #   in Loop: Header=BB2_16 Depth=1
	movq	%r8, %rdx
	xorl	%r8d, %r8d
	jmp	.LBB2_160
.LBB2_156:                              #   in Loop: Header=BB2_16 Depth=1
	movq	%rax, %r8
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbx, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %rax
	cmpq	%rbx, %r8
	movl	$0, %r9d
	cmovaeq	%rbx, %r9
	movq	%r8, %rdx
	subq	%r9, %rdx
	xorl	%r8d, %r8d
	jmp	.LBB2_161
.LBB2_159:                              #   in Loop: Header=BB2_16 Depth=1
	movq	%rax, %r9
	movq	%r8, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%r9, %rax
	movq	%rdx, %r9
	movq	%rbx, %rdx
	imulq	%r9, %rdx
	subq	%rdx, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %r9
	cmpq	%rbx, %r8
	movl	$0, %edx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %r8
	movq	%r8, %rdx
	movq	%r9, %r8
.LBB2_160:                              #   in Loop: Header=BB2_16 Depth=1
	#APP
	divq	%rbx
	#NO_APP
.LBB2_161:                              #   in Loop: Header=BB2_16 Depth=1
	movq	%r12, %r9
	subq	%rsi, %r9
	movq	%rbp, %r14
	sbbq	%rdi, %r14
	cmpq	%rax, %r9
	sbbq	%r8, %r14
	jb	.LBB2_148
# %bb.162:                              #   in Loop: Header=BB2_16 Depth=1
	addq	%rsi, %rax
	adcq	%rdi, %r8
	cmpq	%rax, %r12
	sbbq	%r8, %rbp
	jb	.LBB2_148
# %bb.163:                              #   in Loop: Header=BB2_16 Depth=1
	testq	%rdx, %rdx
	movq	64(%rsp), %r14                  # 8-byte Reload
	movq	32(%rsp), %r9                   # 8-byte Reload
	je	.LBB2_151
# %bb.164:
	movb	$2, %r15b
	movq	56(%rsp), %rax                  # 8-byte Reload
	jmp	.LBB2_165
.LBB2_152:
	movq	56(%rsp), %rax                  # 8-byte Reload
	movb	$1, 16(%rax)
	jmp	.LBB2_167
.LBB2_148:
	xorl	%r15d, %r15d
.LBB2_146:
	movq	56(%rsp), %rax                  # 8-byte Reload
	movq	32(%rsp), %r9                   # 8-byte Reload
.LBB2_165:
	movq	%r9, (%rax)
	movb	%r15b, 8(%rax)
.LBB2_166:
	movb	$0, 16(%rax)
.LBB2_167:
	addq	$200, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.LBB2_130:
	.cfi_def_cfa_offset 256
	xorl	%ecx, %ecx
	movq	112(%rsp), %rdx                 # 8-byte Reload
	jmp	.LBB2_138
.LBB2_132:
	movl	$1, %ecx
	movq	104(%rsp), %rdx                 # 8-byte Reload
	jmp	.LBB2_137
.LBB2_134:
	movl	$2, %ecx
	movq	96(%rsp), %rdx                  # 8-byte Reload
	jmp	.LBB2_137
.LBB2_136:
	movl	$3, %ecx
	movq	88(%rsp), %rdx                  # 8-byte Reload
.LBB2_137:
                                        # kill: def $dl killed $dl killed $rdx def $rdx
.LBB2_138:
	subq	%r9, %rcx
	movq	56(%rsp), %rax                  # 8-byte Reload
	movq	%rcx, (%rax)
	movb	%dl, 8(%rax)
	jmp	.LBB2_166
.Lfunc_end2:
	.size	_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE, .Lfunc_end2-_ZN23fixedwide_batch_codegen16blocked_multiplyEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mNS1_ILm64ELj10EEE
	.cfi_endproc
                                        # -- End function
	.globl	_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_ # -- Begin function _ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_,@function
_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_: # @_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$56, %rsp
	.cfi_def_cfa_offset 112
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%r8, 16(%rsp)                   # 8-byte Spill
	movq	%rsi, 24(%rsp)                  # 8-byte Spill
	movq	%rdi, (%rsp)                    # 8-byte Spill
	testq	%rcx, %rcx
	je	.LBB3_22
# %bb.1:
	movq	%r9, %r14
	movq	%rcx, %r12
	movq	%rdx, %r13
	xorl	%ebx, %ebx
	movq	16(%rsp), %rax                  # 8-byte Reload
	movq	%rax, %rcx
	negq	%rcx
	movl	$0, %ebp
	sbbq	%r9, %rbp
	testq	%r9, %r9
	cmovnsq	%r9, %rbp
	cmovnsq	%rax, %rcx
	movq	%rcx, 8(%rsp)                   # 8-byte Spill
	xorl	%r15d, %r15d
	.p2align	4
.LBB3_2:                                # =>This Inner Loop Header: Depth=1
	movq	(%r13,%rbx), %rsi
	movq	8(%r13,%rbx), %rcx
	movq	%rsi, %rdi
	negq	%rdi
	movl	$0, %eax
	sbbq	%rcx, %rax
	testq	%rcx, %rcx
	cmovnsq	%rsi, %rdi
	cmovnsq	%rcx, %rax
	testq	%rbp, %rbp
	je	.LBB3_4
# %bb.3:                                #   in Loop: Header=BB3_2 Depth=1
	testq	%rax, %rax
	jne	.LBB3_20
.LBB3_4:                                #   in Loop: Header=BB3_2 Depth=1
	movq	%rax, %rdx
	orq	%rbp, %rdx
	je	.LBB3_5
# %bb.6:                                #   in Loop: Header=BB3_2 Depth=1
	testq	%rax, %rax
	je	.LBB3_8
# %bb.7:                                #   in Loop: Header=BB3_2 Depth=1
	mulq	8(%rsp)                         # 8-byte Folded Reload
	movq	%rdx, %r8
	movq	%rax, %r9
	testq	%rdx, %rdx
	je	.LBB3_10
	jmp	.LBB3_20
.LBB3_5:                                #   in Loop: Header=BB3_2 Depth=1
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	jmp	.LBB3_10
.LBB3_8:                                #   in Loop: Header=BB3_2 Depth=1
	movq	%rdi, %rax
	mulq	%rbp
	movq	%rdx, %r8
	movq	%rax, %r9
	testq	%rdx, %rdx
	jne	.LBB3_20
	.p2align	4
.LBB3_10:                               #   in Loop: Header=BB3_2 Depth=1
	movq	%rdi, %rax
	mulq	8(%rsp)                         # 8-byte Folded Reload
	addq	%r9, %rdx
	adcq	$0, %r8
	jne	.LBB3_20
# %bb.11:                               #   in Loop: Header=BB3_2 Depth=1
	movq	%rcx, %rdi
	xorq	%r14, %rdi
	movq	%rdi, %r8
	shrq	$63, %r8
	addq	$-1, %r8
	movabsq	$9223372036854775807, %r9       # imm = 0x7FFFFFFFFFFFFFFF
	adcq	$0, %r9
	cmpq	%rax, %r8
	sbbq	%rdx, %r9
	jae	.LBB3_12
	.p2align	4
.LBB3_20:                               #   in Loop: Header=BB3_2 Depth=1
	leaq	32(%rsp), %rdi
	movq	%rcx, %rdx
	movq	16(%rsp), %rcx                  # 8-byte Reload
	movq	%r14, %r8
	movl	$3, %r9d
	callq	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	testb	$1, 48(%rsp)
	je	.LBB3_24
.LBB3_21:                               #   in Loop: Header=BB3_2 Depth=1
	movups	32(%rsp), %xmm0
	movq	24(%rsp), %rax                  # 8-byte Reload
	movups	%xmm0, (%rax,%rbx)
	incq	%r15
	addq	$16, %rbx
	cmpq	%r15, %r12
	jne	.LBB3_2
	jmp	.LBB3_22
.LBB3_12:                               #   in Loop: Header=BB3_2 Depth=1
	movq	%rax, %rcx
	negq	%rcx
	movl	$0, %r8d
	sbbq	%rdx, %r8
	testq	%rdi, %rdi
	cmovnsq	%rdx, %r8
	testq	%rdx, %rdx
	je	.LBB3_13
# %bb.14:                               #   in Loop: Header=BB3_2 Depth=1
	movq	%rax, %rsi
	movq	%rdx, %rax
	movq	%rdx, %rdi
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r9             # imm = 0xE8D4A51000
	imulq	%r9, %rax
	subq	%rax, %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	#APP
	divq	%r9
	#NO_APP
	testq	%rdx, %rdx
	jne	.LBB3_16
	jmp	.LBB3_19
.LBB3_13:                               #   in Loop: Header=BB3_2 Depth=1
	movq	%rax, %rcx
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	movq	%rcx, %rdx
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rcx
	subq	%rcx, %rdx
	xorl	%ecx, %ecx
	testq	%rdx, %rdx
	je	.LBB3_19
.LBB3_16:                               #   in Loop: Header=BB3_2 Depth=1
	movb	$1, %sil
	movabsq	$500000000000, %rdi             # imm = 0x746A528800
	cmpq	%rdi, %rdx
	ja	.LBB3_18
# %bb.17:                               #   in Loop: Header=BB3_2 Depth=1
	sete	%sil
	andb	%al, %sil
.LBB3_18:                               #   in Loop: Header=BB3_2 Depth=1
	movzbl	%sil, %edx
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB3_19:                               #   in Loop: Header=BB3_2 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %esi
	sbbq	%rcx, %rsi
	testq	%r8, %r8
	cmovnsq	%rcx, %rsi
	cmovnsq	%rax, %rdx
	movq	%rdx, 32(%rsp)
	movq	%rsi, 40(%rsp)
	jmp	.LBB3_21
.LBB3_22:
	movq	(%rsp), %rax                    # 8-byte Reload
	movb	$1, 16(%rax)
	jmp	.LBB3_23
.LBB3_24:
	movzbl	32(%rsp), %ecx
	movq	(%rsp), %rax                    # 8-byte Reload
	movq	%r15, (%rax)
	movb	%cl, 8(%rax)
	movb	$0, 16(%rax)
.LBB3_23:
	addq	$56, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end3:
	.size	_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_, .Lfunc_end3-_ZN23fixedwide_batch_codegen9scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.cfi_endproc
                                        # -- End function
	.globl	_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_ # -- Begin function _ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_,@function
_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_: # @_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$104, %rsp
	.cfi_def_cfa_offset 160
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%r9, %r13
	movq	%rcx, 48(%rsp)                  # 8-byte Spill
	movq	%rdx, 96(%rsp)                  # 8-byte Spill
	movq	%rsi, 88(%rsp)                  # 8-byte Spill
	xorl	%r14d, %r14d
	movq	%r8, %rbp
	negq	%rbp
	sbbq	%r9, %r14
	movq	%rdi, (%rsp)                    # 8-byte Spill
	testq	%r9, %r9
	movq	%r8, 24(%rsp)                   # 8-byte Spill
	cmovnsq	%r8, %rbp
	cmovnsq	%r9, %r14
	movabsq	$2535301200456458803, %r15      # imm = 0x232F33025BD42233
	testq	%r14, %r14
	je	.LBB4_2
# %bb.1:
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	movq	%r14, %rax
	mulq	%r15
	shrq	$37, %rdx
	imulq	%rsi, %rdx
	movq	%r14, %rcx
	subq	%rdx, %rcx
	movq	%rbp, %rax
	movq	%rcx, %rdx
	#APP
	divq	%rsi
	#NO_APP
	movq	%rdx, %rax
	movq	%rsi, %rdx
	jmp	.LBB4_4
.LBB4_2:
	movq	%rbp, %rax
	mulq	%r15
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	movq	%rbp, %rax
	subq	%rcx, %rax
	jmp	.LBB4_4
	.p2align	4
.LBB4_5:                                #   in Loop: Header=BB4_4 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ecx
                                        # kill: def $edx killed $edx def $rdx
	movq	%rcx, %rax
	testq	%rdx, %rdx
	je	.LBB4_6
.LBB4_4:                                # =>This Inner Loop Header: Depth=1
	movq	%rdx, %rcx
	movq	%rax, %rdx
	orq	%rcx, %rdx
	shrq	$32, %rdx
	je	.LBB4_5
# %bb.3:                                #   in Loop: Header=BB4_4 Depth=1
	xorl	%edx, %edx
	divq	%rcx
	movq	%rcx, %rax
	testq	%rdx, %rdx
	jne	.LBB4_4
.LBB4_6:
	testq	%r14, %r14
	je	.LBB4_9
# %bb.7:
	movq	%r14, %rax
	orq	%rcx, %rax
	shrq	$32, %rax
	je	.LBB4_11
# %bb.8:
	movq	%r14, %rax
	xorl	%edx, %edx
	divq	%rcx
	jmp	.LBB4_12
.LBB4_9:
	movq	%rbp, %rax
	orq	%rcx, %rax
	shrq	$32, %rax
	je	.LBB4_13
# %bb.10:
	movq	%rbp, %rax
	xorl	%edx, %edx
	divq	%rcx
	jmp	.LBB4_14
.LBB4_11:
	movl	%r14d, %eax
	xorl	%edx, %edx
	divl	%ecx
                                        # kill: def $edx killed $edx def $rdx
                                        # kill: def $eax killed $eax def $rax
.LBB4_12:
	movq	%rax, 8(%rsp)                   # 8-byte Spill
	movq	%rbp, %rax
	#APP
	divq	%rcx
	#NO_APP
	movq	%rax, 32(%rsp)                  # 8-byte Spill
	jmp	.LBB4_15
.LBB4_13:
	movl	%ebp, %eax
	xorl	%edx, %edx
	divl	%ecx
                                        # kill: def $eax killed $eax def $rax
.LBB4_14:
	movq	%rax, 32(%rsp)                  # 8-byte Spill
	movq	$0, 8(%rsp)                     # 8-byte Folded Spill
.LBB4_15:
	movq	$0, 16(%rsp)                    # 8-byte Folded Spill
	movabsq	$1000000000000, %rax            # imm = 0xE8D4A51000
	xorl	%edx, %edx
	divq	%rcx
	movq	%rax, %rbx
	cmpq	$1, %rax
	je	.LBB4_17
# %bb.16:
	movl	$1, %esi
	xorl	%edi, %edi
	movq	%rbx, %rdx
	xorl	%ecx, %ecx
	callq	__udivti3@PLT
	movq	%rax, 16(%rsp)                  # 8-byte Spill
.LBB4_17:
	cmpq	$0, 48(%rsp)                    # 8-byte Folded Reload
	je	.LBB4_114
# %bb.18:
	movq	%r14, %rax
	mulq	%r15
	cmpq	$0, 8(%rsp)                     # 8-byte Folded Reload
	movq	%r13, 40(%rsp)                  # 8-byte Spill
	jne	.LBB4_66
# %bb.19:
	xorl	%r14d, %r14d
	xorl	%ebp, %ebp
	xorl	%r12d, %r12d
	movq	%rbx, 24(%rsp)                  # 8-byte Spill
	jmp	.LBB4_23
	.p2align	4
.LBB4_20:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%rdi, %rax
	movq	%r8, %rsi
	movq	40(%rsp), %r13                  # 8-byte Reload
	movq	%r15, %r10
.LBB4_21:                               #   in Loop: Header=BB4_23 Depth=1
	movq	24(%rsp), %rbx                  # 8-byte Reload
.LBB4_22:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%rax, %rcx
	negq	%rcx
	movq	%r14, %rdx
	sbbq	%rsi, %rdx
	testq	%r10, %r10
	cmovnsq	%rsi, %rdx
	cmovnsq	%rax, %rcx
	movq	%rcx, 56(%rsp)
	movq	%rdx, 64(%rsp)
	movups	56(%rsp), %xmm0
	movq	88(%rsp), %rax                  # 8-byte Reload
	movups	%xmm0, (%rax,%rbp)
	incq	%r12
	addq	$16, %rbp
	cmpq	%r12, 48(%rsp)                  # 8-byte Folded Reload
	je	.LBB4_114
.LBB4_23:                               # =>This Inner Loop Header: Depth=1
	movq	96(%rsp), %rcx                  # 8-byte Reload
	movq	(%rcx,%rbp), %rax
	movq	8(%rcx,%rbp), %rcx
	movq	%rcx, %r10
	xorq	%r13, %r10
	movq	%rax, %rsi
	negq	%rsi
	movq	%r14, %r8
	sbbq	%rcx, %r8
	testq	%rcx, %rcx
	cmovnsq	%rax, %rsi
	cmovnsq	%rcx, %r8
	testq	%r8, %r8
	je	.LBB4_26
# %bb.24:                               #   in Loop: Header=BB4_23 Depth=1
	cmpq	$1, %rbx
	jne	.LBB4_28
# %bb.25:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r14, %r11
	xorl	%r9d, %r9d
	jmp	.LBB4_35
	.p2align	4
.LBB4_26:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%rsi, %rax
	mulq	32(%rsp)                        # 8-byte Folded Reload
	imulq	8(%rsp), %rsi                   # 8-byte Folded Reload
	addq	%rdx, %rsi
	cmpq	$1, %rbx
	jne	.LBB4_30
# %bb.27:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r14, %rdx
	jmp	.LBB4_47
	.p2align	4
.LBB4_28:                               #   in Loop: Header=BB4_23 Depth=1
	cmpq	%r8, %rbx
	jbe	.LBB4_33
# %bb.29:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r14, %rdi
	jmp	.LBB4_34
.LBB4_30:                               #   in Loop: Header=BB4_23 Depth=1
	testq	%rsi, %rsi
	je	.LBB4_43
# %bb.31:                               #   in Loop: Header=BB4_23 Depth=1
	cmpq	%rsi, %rbx
	jbe	.LBB4_45
# %bb.32:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r14, %rdi
	jmp	.LBB4_46
.LBB4_33:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r8, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rdi
	movq	%rbx, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%rbx, %r8
	sbbq	$-1, %rdi
	cmpq	%rbx, %r8
	movq	%r14, %rax
	cmovaeq	%rbx, %rax
	subq	%rax, %r8
.LBB4_34:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r14, %r11
	movq	%rsi, %rax
	movq	%r8, %rdx
	#APP
	divq	%rbx
	#NO_APP
	movq	%rdx, %r9
	movq	%rdi, %r8
	movq	%rax, %rsi
.LBB4_35:                               #   in Loop: Header=BB4_23 Depth=1
	xorl	%ebx, %ebx
	movq	%r10, %r15
	testq	%r10, %r10
	movabsq	$-9223372036854775808, %rcx     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rax      # imm = 0x7FFFFFFFFFFFFFFF
	cmovnsq	%rax, %rcx
	movl	$0, %r13d
	movq	$-1, %rax
	cmovnsq	%rax, %r13
	movq	32(%rsp), %r14                  # 8-byte Reload
	movq	%r14, %rax
	mulq	%rsi
	movq	%rax, %rdi
	movq	8(%rsp), %r10                   # 8-byte Reload
	imulq	%r10, %rsi
	addq	%rdx, %rsi
	imulq	%r8, %r10
	movq	%r14, %rax
	mulq	%r8
	movq	%rax, %r8
	addq	%rsi, %r8
	adcq	%r10, %rdx
	jne	.LBB4_116
# %bb.36:                               #   in Loop: Header=BB4_23 Depth=1
	cmpq	%rdi, %r13
	movq	%rcx, %rax
	sbbq	%r8, %rax
	jb	.LBB4_115
# %bb.37:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r9, %rax
	movq	%r11, %r14
	orq	%r11, %rax
	je	.LBB4_20
# %bb.38:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r9, %rax
	movq	32(%rsp), %rsi                  # 8-byte Reload
	mulq	%rsi
	imulq	8(%rsp), %r9                    # 8-byte Folded Reload
	addq	%rdx, %r9
	movq	%r14, %r10
	imulq	%rsi, %r10
	addq	%r9, %r10
	movq	24(%rsp), %rbx                  # 8-byte Reload
	cmpq	$1, %rbx
	jne	.LBB4_40
# %bb.39:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r10, %rsi
	movq	%r14, %rdx
	jmp	.LBB4_57
.LBB4_40:                               #   in Loop: Header=BB4_23 Depth=1
	testq	%r10, %r10
	je	.LBB4_44
# %bb.41:                               #   in Loop: Header=BB4_23 Depth=1
	cmpq	%r10, %rbx
	jbe	.LBB4_55
# %bb.42:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r14, %rsi
	jmp	.LBB4_56
.LBB4_43:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%rax, %rsi
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbx, %rcx
	imulq	%rdx, %rcx
	subq	%rcx, %rsi
	cmpq	%rbx, %rsi
	sbbq	$-1, %rax
	cmpq	%rbx, %rsi
	movq	%r14, %rcx
	cmovaeq	%rbx, %rcx
	movq	%rsi, %rdx
	subq	%rcx, %rdx
	movq	%r14, %rsi
	jmp	.LBB4_47
.LBB4_44:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%rax, %rsi
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbx, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rsi
	cmpq	%rbx, %rsi
	sbbq	$-1, %rax
	cmpq	%rbx, %rsi
	movq	%r14, %r9
	cmovaeq	%rbx, %r9
	movq	%rsi, %rdx
	subq	%r9, %rdx
	movq	%r14, %rsi
	jmp	.LBB4_57
.LBB4_45:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%rax, %rcx
	movq	%rsi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rcx, %rax
	movq	%rdx, %rdi
	movq	%rbx, %rcx
	imulq	%rdx, %rcx
	subq	%rcx, %rsi
	cmpq	%rbx, %rsi
	sbbq	$-1, %rdi
	cmpq	%rbx, %rsi
	movq	%r14, %rcx
	cmovaeq	%rbx, %rcx
	subq	%rcx, %rsi
.LBB4_46:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%rsi, %rdx
	#APP
	divq	%rbx
	#NO_APP
	movq	%rdi, %rsi
.LBB4_47:                               #   in Loop: Header=BB4_23 Depth=1
	xorl	%ebx, %ebx
	testq	%r10, %r10
	movabsq	$-9223372036854775808, %rcx     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rdi      # imm = 0x7FFFFFFFFFFFFFFF
	cmovnsq	%rdi, %rcx
	movl	$0, %edi
	movq	$-1, %r8
	cmovnsq	%r8, %rdi
	cmpq	%rax, %rdi
	movq	%rcx, %r8
	sbbq	%rsi, %r8
	jb	.LBB4_116
# %bb.48:                               #   in Loop: Header=BB4_23 Depth=1
	testq	%rdx, %rdx
	je	.LBB4_21
# %bb.49:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r10, %r11
	movq	%rdx, %r9
	shrq	$63, %r9
	addq	%rdx, %rdx
	movb	$1, %r8b
	movq	24(%rsp), %rbx                  # 8-byte Reload
	cmpq	%rdx, %rbx
	movq	%r14, %r10
	sbbq	%r9, %r10
	jb	.LBB4_51
# %bb.50:                               #   in Loop: Header=BB4_23 Depth=1
	xorq	%rbx, %rdx
	orq	%r9, %rdx
	sete	%r8b
	andb	%al, %r8b
.LBB4_51:                               #   in Loop: Header=BB4_23 Depth=1
	xorq	%rax, %rdi
	xorq	%rsi, %rcx
	orq	%rdi, %rcx
	movq	%r11, %r10
	jne	.LBB4_53
# %bb.52:                               #   in Loop: Header=BB4_23 Depth=1
	testb	%r8b, %r8b
	jne	.LBB4_115
.LBB4_53:                               #   in Loop: Header=BB4_23 Depth=1
	movzbl	%r8b, %ecx
	addq	%rcx, %rax
	adcq	$0, %rsi
	jmp	.LBB4_22
.LBB4_55:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%rax, %rsi
	movq	%r10, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbx, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %r10
	cmpq	%rbx, %r10
	sbbq	$-1, %rsi
	cmpq	%rbx, %r10
	movq	%r14, %rdx
	cmovaeq	%rbx, %rdx
	subq	%rdx, %r10
.LBB4_56:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r10, %rdx
	#APP
	divq	%rbx
	#NO_APP
.LBB4_57:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%r13, %r9
	subq	%rdi, %r9
	movq	%rcx, %r10
	sbbq	%r8, %r10
	cmpq	%rax, %r9
	sbbq	%rsi, %r10
	jb	.LBB4_115
# %bb.58:                               #   in Loop: Header=BB4_23 Depth=1
	addq	%rdi, %rax
	adcq	%r8, %rsi
	cmpq	%rax, %r13
	movq	%rcx, %rdi
	sbbq	%rsi, %rdi
	jb	.LBB4_115
# %bb.59:                               #   in Loop: Header=BB4_23 Depth=1
	testq	%rdx, %rdx
	je	.LBB4_65
# %bb.60:                               #   in Loop: Header=BB4_23 Depth=1
	movq	%rdx, %r8
	shrq	$63, %r8
	addq	%rdx, %rdx
	movb	$1, %dil
	cmpq	%rdx, %rbx
	movq	%r14, %r9
	sbbq	%r8, %r9
	movq	%r15, %r10
	jb	.LBB4_62
# %bb.61:                               #   in Loop: Header=BB4_23 Depth=1
	xorq	%rbx, %rdx
	orq	%r8, %rdx
	sete	%dil
	andb	%al, %dil
.LBB4_62:                               #   in Loop: Header=BB4_23 Depth=1
	testb	%dil, %dil
	je	.LBB4_64
# %bb.63:                               #   in Loop: Header=BB4_23 Depth=1
	xorq	%rax, %r13
	xorq	%rsi, %rcx
	orq	%r13, %rcx
	je	.LBB4_115
.LBB4_64:                               #   in Loop: Header=BB4_23 Depth=1
	movzbl	%dil, %ecx
	addq	%rcx, %rax
	adcq	$0, %rsi
	movq	40(%rsp), %r13                  # 8-byte Reload
	jmp	.LBB4_22
.LBB4_65:                               #   in Loop: Header=BB4_23 Depth=1
	movq	40(%rsp), %r13                  # 8-byte Reload
	movq	%r15, %r10
	jmp	.LBB4_22
.LBB4_66:
	testq	%r14, %r14
	je	.LBB4_89
# %bb.67:
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, 8(%rsp)                   # 8-byte Spill
	movabsq	$1000000000000, %rcx            # imm = 0xE8D4A51000
	imulq	%rcx, %rax
	subq	%rax, %r14
	xorl	%r15d, %r15d
	xorl	%r12d, %r12d
	movq	%rbp, 80(%rsp)                  # 8-byte Spill
	jmp	.LBB4_70
	.p2align	4
.LBB4_68:                               #   in Loop: Header=BB4_70 Depth=1
	leaq	56(%rsp), %rdi
	movq	%rcx, %rdx
	movq	24(%rsp), %rcx                  # 8-byte Reload
	movq	%r13, %r8
	movl	$3, %r9d
	callq	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	testb	$1, 72(%rsp)
	je	.LBB4_88
.LBB4_69:                               #   in Loop: Header=BB4_70 Depth=1
	movups	56(%rsp), %xmm0
	movq	88(%rsp), %rax                  # 8-byte Reload
	movups	%xmm0, (%rax,%r15)
	incq	%r12
	addq	$16, %r15
	cmpq	%r12, 48(%rsp)                  # 8-byte Folded Reload
	je	.LBB4_114
.LBB4_70:                               # =>This Inner Loop Header: Depth=1
	movq	96(%rsp), %rax                  # 8-byte Reload
	movq	(%rax,%r15), %rsi
	movq	8(%rax,%r15), %rcx
	movq	%rsi, %rdi
	negq	%rdi
	movl	$0, %r9d
	sbbq	%rcx, %r9
	testq	%rcx, %rcx
	cmovnsq	%rsi, %rdi
	cmovnsq	%rcx, %r9
	testq	%r9, %r9
	jne	.LBB4_68
# %bb.71:                               #   in Loop: Header=BB4_70 Depth=1
	movq	%rbp, %rax
	movq	%r14, %rdx
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	#APP
	divq	%rsi
	#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %r11
	xorq	%r13, %rcx
	testq	%rcx, %rcx
	movabsq	$9223372036854775807, %r13      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r13
	movq	$-1, %rbx
	movl	$0, %eax
	cmovsq	%rax, %rbx
	movq	%r9, %r8
	imulq	%rsi, %r8
	movq	%rdi, %rax
	mulq	%rsi
	movq	%rax, %rsi
	movq	%rdx, %r10
	addq	%r8, %r10
	movq	%r9, %rbp
	movq	8(%rsp), %rdx                   # 8-byte Reload
	imulq	%rdx, %rbp
	movq	%rdi, %rax
	mulq	%rdx
	movq	%rax, %r8
	addq	%r10, %r8
	adcq	%rbp, %rdx
	jne	.LBB4_87
# %bb.72:                               #   in Loop: Header=BB4_70 Depth=1
	cmpq	%rsi, %rbx
	movq	%r13, %rax
	sbbq	%r8, %rax
	jb	.LBB4_87
# %bb.73:                               #   in Loop: Header=BB4_70 Depth=1
	testq	%r11, %r11
	je	.LBB4_76
# %bb.74:                               #   in Loop: Header=BB4_70 Depth=1
	movq	%rdi, %rax
	mulq	%r11
	movq	%rax, %r10
	imulq	%r11, %r9
	addq	%rdx, %r9
	je	.LBB4_77
# %bb.75:                               #   in Loop: Header=BB4_70 Depth=1
	movq	%r9, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movabsq	$1000000000000, %r11            # imm = 0xE8D4A51000
	imulq	%r11, %rax
	subq	%rax, %r9
	movq	%r10, %rax
	movq	%r9, %rdx
	#APP
	divq	%r11
	#NO_APP
	movq	%rdx, %r10
	jmp	.LBB4_78
.LBB4_76:                               #   in Loop: Header=BB4_70 Depth=1
	movq	%rsi, %rax
	movq	%r8, %rdi
	jmp	.LBB4_86
.LBB4_77:                               #   in Loop: Header=BB4_70 Depth=1
	movq	%r10, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rdx
	movabsq	$1000000000000, %rdi            # imm = 0xE8D4A51000
	imulq	%rdi, %rdx
	subq	%rdx, %r10
	xorl	%edi, %edi
.LBB4_78:                               #   in Loop: Header=BB4_70 Depth=1
	movq	%rbx, %rdx
	subq	%rsi, %rdx
	movq	%r13, %r9
	sbbq	%r8, %r9
	cmpq	%rax, %rdx
	sbbq	%rdi, %r9
	jb	.LBB4_87
# %bb.79:                               #   in Loop: Header=BB4_70 Depth=1
	addq	%rsi, %rax
	adcq	%r8, %rdi
	cmpq	%rax, %rbx
	movq	%r13, %rdx
	sbbq	%rdi, %rdx
	jb	.LBB4_87
# %bb.80:                               #   in Loop: Header=BB4_70 Depth=1
	testq	%r10, %r10
	je	.LBB4_86
# %bb.81:                               #   in Loop: Header=BB4_70 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rsi             # imm = 0x746A528800
	cmpq	%rsi, %r10
	ja	.LBB4_83
# %bb.82:                               #   in Loop: Header=BB4_70 Depth=1
	sete	%sil
	movl	%eax, %edx
	andb	%sil, %dl
.LBB4_83:                               #   in Loop: Header=BB4_70 Depth=1
	testb	%dl, %dl
	je	.LBB4_85
# %bb.84:                               #   in Loop: Header=BB4_70 Depth=1
	xorq	%rax, %rbx
	xorq	%rdi, %r13
	orq	%rbx, %r13
	je	.LBB4_87
.LBB4_85:                               #   in Loop: Header=BB4_70 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rdi
.LBB4_86:                               #   in Loop: Header=BB4_70 Depth=1
	movq	80(%rsp), %rbp                  # 8-byte Reload
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %esi
	sbbq	%rdi, %rsi
	testq	%rcx, %rcx
	cmovnsq	%rdi, %rsi
	cmovnsq	%rax, %rdx
	movq	%rdx, 56(%rsp)
	movq	%rsi, 64(%rsp)
	movq	40(%rsp), %r13                  # 8-byte Reload
	jmp	.LBB4_69
.LBB4_87:
	movb	$0, 56(%rsp)
.LBB4_88:
	movzbl	56(%rsp), %ebx
	jmp	.LBB4_116
.LBB4_89:
	xorl	%r10d, %r10d
	xorl	%r12d, %r12d
	jmp	.LBB4_92
	.p2align	4
.LBB4_90:                               #   in Loop: Header=BB4_92 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB4_91:                               #   in Loop: Header=BB4_92 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %esi
	sbbq	%rcx, %rsi
	testq	%r11, %r11
	cmovnsq	%rcx, %rsi
	cmovnsq	%rax, %rdx
	movq	%rdx, 56(%rsp)
	movq	%rsi, 64(%rsp)
	movups	56(%rsp), %xmm0
	movq	88(%rsp), %rax                  # 8-byte Reload
	movups	%xmm0, (%rax,%r10)
	incq	%r12
	addq	$16, %r10
	cmpq	%r12, 48(%rsp)                  # 8-byte Folded Reload
	je	.LBB4_114
.LBB4_92:                               # =>This Inner Loop Header: Depth=1
	movq	96(%rsp), %rcx                  # 8-byte Reload
	movq	(%rcx,%r10), %rax
	movq	8(%rcx,%r10), %r11
	movq	%rax, %rcx
	negq	%rcx
	movl	$0, %esi
	sbbq	%r11, %rsi
	testq	%r11, %r11
	cmovnsq	%rax, %rcx
	cmovnsq	%r11, %rsi
	xorq	%r13, %r11
	testq	%rsi, %rsi
	je	.LBB4_98
# %bb.93:                               #   in Loop: Header=BB4_92 Depth=1
	movq	%rsi, %rax
	mulq	%r15
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movabsq	$1000000000000, %r8             # imm = 0xE8D4A51000
	imulq	%r8, %rax
	subq	%rax, %rsi
	movq	%rcx, %rax
	movq	%rsi, %rdx
	#APP
	divq	%r8
	#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %rcx
	xorl	%ebx, %ebx
	testq	%r11, %r11
	movabsq	$9223372036854775807, %r14      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r14
	movq	$-1, %r15
	cmovsq	%rbx, %r15
	movq	%rbp, %rax
	mulq	%rsi
	movq	%rdx, %r8
	movq	%rax, %rsi
	movq	%rbp, %rax
	mulq	%rdi
	movq	%rax, %rdi
	addq	%r8, %rdi
	adcq	$0, %rdx
	jne	.LBB4_116
# %bb.94:                               #   in Loop: Header=BB4_92 Depth=1
	cmpq	%rsi, %r15
	movq	%r14, %rax
	sbbq	%rdi, %rax
	jb	.LBB4_115
# %bb.95:                               #   in Loop: Header=BB4_92 Depth=1
	testq	%rcx, %rcx
	je	.LBB4_100
# %bb.96:                               #   in Loop: Header=BB4_92 Depth=1
	movq	%rbp, %rax
	mulq	%rcx
	movq	%rax, %r8
	testq	%rdx, %rdx
	movabsq	$2535301200456458803, %rbx      # imm = 0x232F33025BD42233
	je	.LBB4_104
# %bb.97:                               #   in Loop: Header=BB4_92 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r9
	mulq	%rbx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %rbx            # imm = 0xE8D4A51000
	imulq	%rbx, %rax
	subq	%rax, %r9
	movq	%r8, %rax
	movq	%r9, %rdx
	#APP
	divq	%rbx
	#NO_APP
	movq	%rdx, %r8
	jmp	.LBB4_105
	.p2align	4
.LBB4_98:                               #   in Loop: Header=BB4_92 Depth=1
	movq	%rcx, %rax
	mulq	%rbp
	movq	%rax, %rsi
	testq	%rdx, %rdx
	je	.LBB4_101
# %bb.99:                               #   in Loop: Header=BB4_92 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %rdi
	mulq	%r15
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r8             # imm = 0xE8D4A51000
	imulq	%r8, %rax
	subq	%rax, %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	#APP
	divq	%r8
	#NO_APP
	movq	%rdx, %rsi
	testq	%rsi, %rsi
	jne	.LBB4_102
	jmp	.LBB4_91
	.p2align	4
.LBB4_100:                              #   in Loop: Header=BB4_92 Depth=1
	movq	%rsi, %rax
	movq	%rdi, %rcx
	movabsq	$2535301200456458803, %r15      # imm = 0x232F33025BD42233
	jmp	.LBB4_91
.LBB4_101:                              #   in Loop: Header=BB4_92 Depth=1
	movq	%rsi, %rax
	mulq	%r15
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %rsi
	xorl	%ecx, %ecx
	testq	%rsi, %rsi
	je	.LBB4_91
.LBB4_102:                              #   in Loop: Header=BB4_92 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rdi             # imm = 0x746A528800
	cmpq	%rdi, %rsi
	ja	.LBB4_90
# %bb.103:                              #   in Loop: Header=BB4_92 Depth=1
	sete	%sil
	movl	%eax, %edx
	andb	%sil, %dl
	jmp	.LBB4_90
.LBB4_104:                              #   in Loop: Header=BB4_92 Depth=1
	movq	%r8, %rax
	mulq	%rbx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %r8
	xorl	%ecx, %ecx
.LBB4_105:                              #   in Loop: Header=BB4_92 Depth=1
	movq	%r15, %rdx
	subq	%rsi, %rdx
	movq	%r14, %r9
	sbbq	%rdi, %r9
	cmpq	%rax, %rdx
	sbbq	%rcx, %r9
	jb	.LBB4_115
# %bb.106:                              #   in Loop: Header=BB4_92 Depth=1
	addq	%rsi, %rax
	adcq	%rdi, %rcx
	cmpq	%rax, %r15
	movq	%r14, %rdx
	sbbq	%rcx, %rdx
	jb	.LBB4_115
# %bb.107:                              #   in Loop: Header=BB4_92 Depth=1
	testq	%r8, %r8
	je	.LBB4_113
# %bb.108:                              #   in Loop: Header=BB4_92 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rsi             # imm = 0x746A528800
	cmpq	%rsi, %r8
	ja	.LBB4_110
# %bb.109:                              #   in Loop: Header=BB4_92 Depth=1
	sete	%sil
	movl	%eax, %edx
	andb	%sil, %dl
.LBB4_110:                              #   in Loop: Header=BB4_92 Depth=1
	testb	%dl, %dl
	je	.LBB4_112
# %bb.111:                              #   in Loop: Header=BB4_92 Depth=1
	xorq	%rax, %r15
	xorq	%rcx, %r14
	orq	%r15, %r14
	je	.LBB4_115
.LBB4_112:                              #   in Loop: Header=BB4_92 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB4_113:                              #   in Loop: Header=BB4_92 Depth=1
	movabsq	$2535301200456458803, %r15      # imm = 0x232F33025BD42233
	jmp	.LBB4_91
.LBB4_114:
	movq	(%rsp), %rax                    # 8-byte Reload
	movb	$1, 16(%rax)
	jmp	.LBB4_117
.LBB4_115:
	xorl	%ebx, %ebx
.LBB4_116:
	movq	(%rsp), %rax                    # 8-byte Reload
	movq	%r12, (%rax)
	movb	%bl, 8(%rax)
	movb	$0, 16(%rax)
.LBB4_117:
	addq	$104, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end4:
	.size	_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_, .Lfunc_end4-_ZN23fixedwide_batch_codegen18prepared_scalar_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.cfi_endproc
                                        # -- End function
	.globl	_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_ # -- Begin function _ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_,@function
_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_: # @_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$232, %rsp
	.cfi_def_cfa_offset 288
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%rcx, 192(%rsp)                 # 8-byte Spill
	movq	%rdx, %r15
	xorl	%r14d, %r14d
	movq	%r8, %r10
	negq	%r10
	sbbq	%r9, %r14
	movq	%rdi, %rbx
	testq	%r9, %r9
	movq	%r8, 200(%rsp)                  # 8-byte Spill
	cmovnsq	%r8, %r10
	movq	%r9, %r12
	cmovnsq	%r9, %r14
	movabsq	$2535301200456458803, %r13      # imm = 0x232F33025BD42233
	testq	%r14, %r14
	je	.LBB5_2
# %bb.1:
	movabsq	$1000000000000, %rdi            # imm = 0xE8D4A51000
	movq	%r14, %rax
	mulq	%r13
	shrq	$37, %rdx
	imulq	%rdi, %rdx
	movq	%r14, %rcx
	subq	%rdx, %rcx
	movq	%r10, %rax
	movq	%rcx, %rdx
	#APP
	divq	%rdi
	#NO_APP
	movq	%rdx, %rax
	movq	%rdi, %rdx
	jmp	.LBB5_4
.LBB5_2:
	movq	%r10, %rax
	mulq	%r13
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	movq	%r10, %rax
	subq	%rcx, %rax
	jmp	.LBB5_4
	.p2align	4
.LBB5_5:                                #   in Loop: Header=BB5_4 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%ecx
                                        # kill: def $edx killed $edx def $rdx
	movq	%rcx, %rax
	testq	%rdx, %rdx
	je	.LBB5_6
.LBB5_4:                                # =>This Inner Loop Header: Depth=1
	movq	%rdx, %rcx
	movq	%rax, %rdx
	orq	%rcx, %rdx
	shrq	$32, %rdx
	je	.LBB5_5
# %bb.3:                                #   in Loop: Header=BB5_4 Depth=1
	xorl	%edx, %edx
	divq	%rcx
	movq	%rcx, %rax
	testq	%rdx, %rdx
	jne	.LBB5_4
.LBB5_6:
	testq	%r14, %r14
	movq	%r10, 48(%rsp)                  # 8-byte Spill
	je	.LBB5_9
# %bb.7:
	movq	%r14, %rax
	orq	%rcx, %rax
	shrq	$32, %rax
	je	.LBB5_11
# %bb.8:
	movq	%r14, %rax
	xorl	%edx, %edx
	divq	%rcx
	jmp	.LBB5_12
.LBB5_9:
	movq	%r10, %rax
	orq	%rcx, %rax
	shrq	$32, %rax
	je	.LBB5_13
# %bb.10:
	movq	%r10, %rax
	xorl	%edx, %edx
	divq	%rcx
	jmp	.LBB5_14
.LBB5_11:
	movl	%r14d, %eax
	xorl	%edx, %edx
	divl	%ecx
                                        # kill: def $edx killed $edx def $rdx
                                        # kill: def $eax killed $eax def $rax
.LBB5_12:
	movq	%rax, 32(%rsp)                  # 8-byte Spill
	movq	%r10, %rax
	#APP
	divq	%rcx
	#NO_APP
	movq	%rax, 56(%rsp)                  # 8-byte Spill
	jmp	.LBB5_15
.LBB5_13:
	movl	%r10d, %eax
	xorl	%edx, %edx
	divl	%ecx
                                        # kill: def $eax killed $eax def $rax
.LBB5_14:
	movq	%rax, 56(%rsp)                  # 8-byte Spill
	movq	$0, 32(%rsp)                    # 8-byte Folded Spill
.LBB5_15:
	movq	$0, 16(%rsp)                    # 8-byte Folded Spill
	movabsq	$1000000000000, %rax            # imm = 0xE8D4A51000
	xorl	%edx, %edx
	divq	%rcx
	movq	%rax, %rbp
	cmpq	$1, %rax
	movq	%rsi, 176(%rsp)                 # 8-byte Spill
	je	.LBB5_17
# %bb.16:
	movl	$1, %esi
	xorl	%edi, %edi
	movq	%rbp, %rdx
	xorl	%ecx, %ecx
	callq	__udivti3@PLT
	movq	176(%rsp), %rsi                 # 8-byte Reload
	movq	%rax, 16(%rsp)                  # 8-byte Spill
.LBB5_17:
	movq	%r14, 160(%rsp)                 # 8-byte Spill
	movq	%rbx, 224(%rsp)                 # 8-byte Spill
	movq	%r12, %rax
	shrq	$63, %rax
	movq	%rax, 24(%rsp)                  # 8-byte Spill
	cmpq	$4, 192(%rsp)                   # 8-byte Folded Reload
	movq	%r12, 8(%rsp)                   # 8-byte Spill
	movq	%r15, 184(%rsp)                 # 8-byte Spill
	jb	.LBB5_487
# %bb.18:
	xorl	%r14d, %r14d
	movq	160(%rsp), %rsi                 # 8-byte Reload
	movq	%rsi, %rax
	mulq	%r13
	shrq	$37, %rdx
	movq	%rdx, 216(%rsp)                 # 8-byte Spill
	movq	%rdx, %rax
	movabsq	$1000000000000, %rcx            # imm = 0xE8D4A51000
	imulq	%rcx, %rax
	subq	%rax, %rsi
	movq	%rsi, 208(%rsp)                 # 8-byte Spill
	movl	$48, %ebx
	xorl	%r13d, %r13d
	movq	%rbp, 168(%rsp)                 # 8-byte Spill
	.p2align	4
.LBB5_19:                               # =>This Inner Loop Header: Depth=1
	movq	-48(%r15,%rbx), %rsi
	movq	-40(%r15,%rbx), %rdx
	cmpq	$0, 32(%rsp)                    # 8-byte Folded Reload
	jne	.LBB5_23
# %bb.20:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %r9
	shrq	$63, %r9
	movq	%rsi, %rcx
	negq	%rcx
	movq	%r14, %rdi
	sbbq	%rdx, %rdi
	testq	%rdx, %rdx
	cmovnsq	%rsi, %rcx
	cmovnsq	%rdx, %rdi
	testq	%rdi, %rdi
	je	.LBB5_26
# %bb.21:                               #   in Loop: Header=BB5_19 Depth=1
	cmpq	$1, %rbp
	jne	.LBB5_32
# %bb.22:                               #   in Loop: Header=BB5_19 Depth=1
	xorl	%r8d, %r8d
	jmp	.LBB5_42
	.p2align	4
.LBB5_23:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rsi, %r8
	negq	%r8
	movq	%r14, %rcx
	sbbq	%rdx, %rcx
	testq	%rdx, %rdx
	cmovnsq	%rsi, %r8
	cmovnsq	%rdx, %rcx
	movq	%rdx, %r9
	xorq	%r12, %r9
	cmpq	$0, 160(%rsp)                   # 8-byte Folded Reload
	je	.LBB5_28
# %bb.24:                               #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_34
# %bb.25:                               #   in Loop: Header=BB5_19 Depth=1
	leaq	64(%rsp), %rdi
	movq	200(%rsp), %rcx                 # 8-byte Reload
	movq	%r12, %r8
	movl	$3, %r9d
	callq	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	jmp	.LBB5_247
.LBB5_26:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rax
	mulq	56(%rsp)                        # 8-byte Folded Reload
	imulq	32(%rsp), %rcx                  # 8-byte Folded Reload
	addq	%rdx, %rcx
	cmpq	$1, %rbp
	jne	.LBB5_37
# %bb.27:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rdx
	jmp	.LBB5_59
.LBB5_28:                               #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_48
# %bb.29:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r12, %rbp
	movq	%rcx, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rax
	subq	%rax, %rcx
	movq	%r8, %rax
	movq	%rcx, %rdx
	#APP
	divq	%rsi
	#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rsi
	testq	%r9, %r9
	movabsq	$9223372036854775807, %r10      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r10
	movq	$-1, %r11
	cmovsq	%r14, %r11
	movq	48(%rsp), %r12                  # 8-byte Reload
	movq	%r12, %rax
	mulq	%rcx
	movq	%rdx, %r8
	movq	%rax, %rcx
	movq	%r12, %rax
	mulq	%rdi
	addq	%r8, %rax
	adcq	$0, %rdx
	jne	.LBB5_31
# %bb.30:                               #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %r11
	movq	%r10, %rdx
	sbbq	%rax, %rdx
	jae	.LBB5_51
.LBB5_31:                               #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 64(%rsp)
	movb	$0, 80(%rsp)
	movq	%rbp, %r12
	movq	168(%rsp), %rbp                 # 8-byte Reload
	jmp	.LBB5_247
.LBB5_32:                               #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rdi, %rbp
	jbe	.LBB5_40
# %bb.33:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rsi
	jmp	.LBB5_41
.LBB5_34:                               #   in Loop: Header=BB5_19 Depth=1
	movq	48(%rsp), %rax                  # 8-byte Reload
	movq	208(%rsp), %rdx                 # 8-byte Reload
	movabsq	$1000000000000, %rcx            # imm = 0xE8D4A51000
	#APP
	divq	%rcx
	#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rdi
	testq	%r9, %r9
	movabsq	$9223372036854775807, %r10      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r10
	movq	$-1, %r11
	cmovsq	%r14, %r11
	movq	%r8, %rax
	mulq	%rcx
	movq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%r8, %rax
	mulq	216(%rsp)                       # 8-byte Folded Reload
	addq	%rsi, %rax
	adcq	$0, %rdx
	jne	.LBB5_36
# %bb.35:                               #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %r11
	movq	%r10, %rdx
	sbbq	%rax, %rdx
	jae	.LBB5_54
.LBB5_36:                               #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 64(%rsp)
	movb	$0, 80(%rsp)
	jmp	.LBB5_247
.LBB5_37:                               #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_50
# %bb.38:                               #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %rbp
	jbe	.LBB5_57
# %bb.39:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rsi
	jmp	.LBB5_58
.LBB5_40:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rsi
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	cmpq	%rbp, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbp, %rdi
	movq	%r14, %rax
	cmovaeq	%rbp, %rax
	subq	%rax, %rdi
.LBB5_41:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rax
	movq	%rdi, %rdx
	#APP
	divq	%rbp
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
	movq	%rax, %rcx
.LBB5_42:                               #   in Loop: Header=BB5_19 Depth=1
	movq	24(%rsp), %rax                  # 8-byte Reload
	cmpq	%rax, %r9
	movq	%r14, %r10
	movq	$-1, %rdx
	cmoveq	%rdx, %r10
	cmpl	%eax, %r9d
	movabsq	$-9223372036854775808, %r11     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rax      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rax, %r11
	movq	56(%rsp), %r15                  # 8-byte Reload
	movq	%r15, %rax
	mulq	%rcx
	movq	%rax, %rsi
	movq	32(%rsp), %r12                  # 8-byte Reload
	imulq	%r12, %rcx
	addq	%rdx, %rcx
	imulq	%rdi, %r12
	movq	%r15, %rax
	mulq	%rdi
	addq	%rcx, %rax
	adcq	%r12, %rdx
	jne	.LBB5_44
# %bb.43:                               #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rsi, %r10
	movq	%r11, %rcx
	sbbq	%rax, %rcx
	jae	.LBB5_45
.LBB5_44:                               #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 64(%rsp)
	xorl	%eax, %eax
	movq	8(%rsp), %r12                   # 8-byte Reload
	movq	184(%rsp), %r15                 # 8-byte Reload
	jmp	.LBB5_88
.LBB5_45:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r8, %rcx
	orq	%r14, %rcx
	movq	184(%rsp), %r15                 # 8-byte Reload
	je	.LBB5_69
# %bb.46:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %r12
	movq	%r8, %rax
	movq	56(%rsp), %rcx                  # 8-byte Reload
	mulq	%rcx
	imulq	32(%rsp), %r8                   # 8-byte Folded Reload
	addq	%rdx, %r8
	movq	%r14, %rdi
	imulq	%rcx, %rdi
	addq	%r8, %rdi
	cmpq	$1, %rbp
	jne	.LBB5_76
# %bb.47:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rcx
	movq	%r14, %rdx
	jmp	.LBB5_84
.LBB5_48:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r8, %rax
	mulq	48(%rsp)                        # 8-byte Folded Reload
	movq	%rax, %rsi
	testq	%rdx, %rdx
	je	.LBB5_70
# %bb.49:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %rdi
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r8             # imm = 0xE8D4A51000
	imulq	%r8, %rax
	subq	%rax, %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	#APP
	divq	%r8
	#NO_APP
	movq	%rdx, %rsi
	testq	%rsi, %rsi
	jne	.LBB5_71
	jmp	.LBB5_74
.LBB5_50:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbp, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rax
	cmpq	%rbp, %rcx
	movq	%r14, %rsi
	cmovaeq	%rbp, %rsi
	movq	%rcx, %rdx
	subq	%rsi, %rdx
	movq	%r14, %rcx
	jmp	.LBB5_59
.LBB5_51:                               #   in Loop: Header=BB5_19 Depth=1
	testq	%rsi, %rsi
	je	.LBB5_79
# %bb.52:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	48(%rsp), %rax                  # 8-byte Reload
	mulq	%rsi
	movq	%rax, %rdi
	testq	%rdx, %rdx
	movq	168(%rsp), %rbp                 # 8-byte Reload
	je	.LBB5_236
# %bb.53:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r8
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movabsq	$1000000000000, %r12            # imm = 0xE8D4A51000
	imulq	%r12, %rax
	subq	%rax, %r8
	movq	%rdi, %rax
	movq	%r8, %rdx
	#APP
	divq	%r12
	#NO_APP
	movq	%rdx, %rdi
	jmp	.LBB5_237
.LBB5_54:                               #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_80
# %bb.55:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	%r8, %rax
	mulq	%rdi
	movq	%rax, %rdi
	testq	%rdx, %rdx
	je	.LBB5_243
# %bb.56:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r8
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movabsq	$1000000000000, %r12            # imm = 0xE8D4A51000
	imulq	%r12, %rax
	subq	%rax, %r8
	movq	%rdi, %rax
	movq	%r8, %rdx
	#APP
	divq	%r12
	#NO_APP
	movq	%rdx, %rdi
	jmp	.LBB5_244
.LBB5_57:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rsi
	movq	%rcx, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbp, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rsi
	cmpq	%rbp, %rcx
	movq	%r14, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rcx
.LBB5_58:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rdx
	#APP
	divq	%rbp
	#NO_APP
	movq	%rsi, %rcx
.LBB5_59:                               #   in Loop: Header=BB5_19 Depth=1
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	movabsq	$-9223372036854775808, %rsi     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %r8       # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%r8, %rsi
	cmpq	%rdi, %r9
	movq	%r14, %rdi
	movq	$-1, %r8
	cmoveq	%r8, %rdi
	cmpq	%rax, %rdi
	movq	%rsi, %r8
	sbbq	%rcx, %r8
	jb	.LBB5_65
# %bb.60:                               #   in Loop: Header=BB5_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB5_68
# %bb.61:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %r10
	shrq	$63, %r10
	addq	%rdx, %rdx
	movb	$1, %r8b
	cmpq	%rdx, %rbp
	movq	%r14, %r11
	sbbq	%r10, %r11
	jb	.LBB5_63
# %bb.62:                               #   in Loop: Header=BB5_19 Depth=1
	xorq	%rbp, %rdx
	orq	%r10, %rdx
	sete	%r8b
	andb	%al, %r8b
.LBB5_63:                               #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %rdi
	xorq	%rcx, %rsi
	orq	%rdi, %rsi
	jne	.LBB5_66
# %bb.64:                               #   in Loop: Header=BB5_19 Depth=1
	testb	%r8b, %r8b
	je	.LBB5_66
.LBB5_65:                               #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 64(%rsp)
	xorl	%eax, %eax
	jmp	.LBB5_88
.LBB5_66:                               #   in Loop: Header=BB5_19 Depth=1
	movzbl	%r8b, %edx
.LBB5_67:                               #   in Loop: Header=BB5_19 Depth=1
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB5_68:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rcx, %rsi
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	cmoveq	%rcx, %rsi
	cmpq	%rdi, %r9
	cmoveq	%rax, %rdx
	movq	%rdx, 64(%rsp)
	movq	%rsi, 72(%rsp)
	movb	$1, %al
	jmp	.LBB5_88
.LBB5_69:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rsi, %rcx
	negq	%rcx
	movq	%r14, %rdx
	sbbq	%rax, %rdx
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	cmoveq	%rax, %rdx
	cmpq	%rdi, %r9
	cmoveq	%rsi, %rcx
	movq	%rcx, 64(%rsp)
	movq	%rdx, 72(%rsp)
	movb	$1, %al
	jmp	.LBB5_87
.LBB5_70:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rsi, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %rsi
	movq	%r14, %rcx
	testq	%rsi, %rsi
	je	.LBB5_74
.LBB5_71:                               #   in Loop: Header=BB5_19 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rdi             # imm = 0x746A528800
	cmpq	%rdi, %rsi
	ja	.LBB5_73
# %bb.72:                               #   in Loop: Header=BB5_19 Depth=1
	sete	%sil
	movl	%eax, %edx
	andb	%sil, %dl
.LBB5_73:                               #   in Loop: Header=BB5_19 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB5_74:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rcx, %rsi
	testq	%r9, %r9
	cmovnsq	%rcx, %rsi
	cmovnsq	%rax, %rdx
.LBB5_75:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, 64(%rsp)
	movq	%rsi, 72(%rsp)
	movb	$1, 80(%rsp)
	jmp	.LBB5_247
.LBB5_76:                               #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_81
# %bb.77:                               #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rdi, %rbp
	jbe	.LBB5_82
# %bb.78:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rcx
	jmp	.LBB5_83
.LBB5_79:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rbp, %r12
	movq	%rcx, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rax, %rsi
	testq	%r9, %r9
	cmovnsq	%rax, %rsi
	cmovnsq	%rcx, %rdx
	movq	%rdx, 64(%rsp)
	movq	%rsi, 72(%rsp)
	movb	$1, 80(%rsp)
	movq	168(%rsp), %rbp                 # 8-byte Reload
	jmp	.LBB5_247
.LBB5_80:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rax, %rsi
	testq	%r9, %r9
	cmovnsq	%rax, %rsi
	cmovnsq	%rcx, %rdx
	jmp	.LBB5_75
.LBB5_81:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbp, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rax
	cmpq	%rbp, %rcx
	movq	%r14, %rdi
	cmovaeq	%rbp, %rdi
	movq	%rcx, %rdx
	subq	%rdi, %rdx
	movq	%r14, %rcx
	jmp	.LBB5_84
.LBB5_82:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rcx, %rax
	movq	%rdx, %rcx
	movq	%rbp, %rdx
	imulq	%rcx, %rdx
	subq	%rdx, %rdi
	cmpq	%rbp, %rdi
	sbbq	$-1, %rcx
	cmpq	%rbp, %rdi
	movq	%r14, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rdi
.LBB5_83:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%rbp
	#NO_APP
.LBB5_84:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r10, %rdi
	subq	%rsi, %rdi
	movq	%r11, %r8
	sbbq	%r12, %r8
	cmpq	%rax, %rdi
	sbbq	%rcx, %r8
	jb	.LBB5_86
# %bb.85:                               #   in Loop: Header=BB5_19 Depth=1
	addq	%rsi, %rax
	adcq	%r12, %rcx
	cmpq	%rax, %r10
	movq	%r11, %rsi
	sbbq	%rcx, %rsi
	jae	.LBB5_123
.LBB5_86:                               #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 64(%rsp)
	xorl	%eax, %eax
.LBB5_87:                               #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
	.p2align	4
.LBB5_88:                               #   in Loop: Header=BB5_19 Depth=1
	movb	%al, 80(%rsp)
	movq	-32(%r15,%rbx), %rax
	movq	-24(%r15,%rbx), %rdx
	movq	%rdx, %r9
	shrq	$63, %r9
	movq	%rax, %rcx
	negq	%rcx
	movq	%r14, %rdi
	sbbq	%rdx, %rdi
	testq	%rdx, %rdx
	cmovnsq	%rax, %rcx
	cmovnsq	%rdx, %rdi
	testq	%rdi, %rdi
	je	.LBB5_91
# %bb.89:                               #   in Loop: Header=BB5_19 Depth=1
	cmpq	$1, %rbp
	jne	.LBB5_93
# %bb.90:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %r8
	jmp	.LBB5_100
.LBB5_91:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rax
	mulq	56(%rsp)                        # 8-byte Folded Reload
	imulq	32(%rsp), %rcx                  # 8-byte Folded Reload
	addq	%rdx, %rcx
	cmpq	$1, %rbp
	jne	.LBB5_95
# %bb.92:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rdx
	jmp	.LBB5_109
.LBB5_93:                               #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rdi, %rbp
	jbe	.LBB5_98
# %bb.94:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rsi
	jmp	.LBB5_99
.LBB5_95:                               #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_106
# %bb.96:                               #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %rbp
	jbe	.LBB5_107
# %bb.97:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rsi
	jmp	.LBB5_108
.LBB5_98:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rsi
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	cmpq	%rbp, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbp, %rdi
	movq	%r14, %rax
	cmovaeq	%rbp, %rax
	subq	%rax, %rdi
.LBB5_99:                               #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rax
	movq	%rdi, %rdx
	#APP
	divq	%rbp
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
	movq	%rax, %rcx
.LBB5_100:                              #   in Loop: Header=BB5_19 Depth=1
	movq	24(%rsp), %rax                  # 8-byte Reload
	cmpq	%rax, %r9
	movq	%r14, %r10
	movq	$-1, %rdx
	cmoveq	%rdx, %r10
	cmpl	%eax, %r9d
	movabsq	$-9223372036854775808, %r11     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rax      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rax, %r11
	movq	56(%rsp), %r15                  # 8-byte Reload
	movq	%r15, %rax
	mulq	%rcx
	movq	%rax, %rsi
	movq	32(%rsp), %r12                  # 8-byte Reload
	imulq	%r12, %rcx
	addq	%rdx, %rcx
	imulq	%rdi, %r12
	movq	%r15, %rax
	mulq	%rdi
	addq	%rcx, %rax
	adcq	%r12, %rdx
	jne	.LBB5_102
# %bb.101:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rsi, %r10
	movq	%r11, %rcx
	sbbq	%rax, %rcx
	jae	.LBB5_103
.LBB5_102:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 88(%rsp)
	xorl	%eax, %eax
	movq	8(%rsp), %r12                   # 8-byte Reload
	movq	184(%rsp), %r15                 # 8-byte Reload
	jmp	.LBB5_136
.LBB5_103:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r8, %rcx
	orq	%r14, %rcx
	movq	184(%rsp), %r15                 # 8-byte Reload
	je	.LBB5_119
# %bb.104:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %r12
	movq	%r8, %rax
	movq	56(%rsp), %rcx                  # 8-byte Reload
	mulq	%rcx
	imulq	32(%rsp), %r8                   # 8-byte Folded Reload
	addq	%rdx, %r8
	movq	%r14, %rdi
	imulq	%rcx, %rdi
	addq	%r8, %rdi
	cmpq	$1, %rbp
	jne	.LBB5_120
# %bb.105:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rcx
	movq	%r14, %rdx
	jmp	.LBB5_132
.LBB5_106:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbp, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rax
	cmpq	%rbp, %rcx
	movq	%r14, %rsi
	cmovaeq	%rbp, %rsi
	movq	%rcx, %rdx
	subq	%rsi, %rdx
	movq	%r14, %rcx
	jmp	.LBB5_109
.LBB5_107:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rsi
	movq	%rcx, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbp, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rsi
	cmpq	%rbp, %rcx
	movq	%r14, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rcx
.LBB5_108:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rdx
	#APP
	divq	%rbp
	#NO_APP
	movq	%rsi, %rcx
.LBB5_109:                              #   in Loop: Header=BB5_19 Depth=1
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	movabsq	$-9223372036854775808, %rsi     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %r8       # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%r8, %rsi
	cmpq	%rdi, %r9
	movq	%r14, %rdi
	movq	$-1, %r8
	cmoveq	%r8, %rdi
	cmpq	%rax, %rdi
	movq	%rsi, %r8
	sbbq	%rcx, %r8
	jb	.LBB5_115
# %bb.110:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB5_118
# %bb.111:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %r10
	shrq	$63, %r10
	addq	%rdx, %rdx
	movb	$1, %r8b
	cmpq	%rdx, %rbp
	movq	%r14, %r11
	sbbq	%r10, %r11
	jb	.LBB5_113
# %bb.112:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rbp, %rdx
	orq	%r10, %rdx
	sete	%r8b
	andb	%al, %r8b
.LBB5_113:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %rdi
	xorq	%rcx, %rsi
	orq	%rdi, %rsi
	jne	.LBB5_116
# %bb.114:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%r8b, %r8b
	je	.LBB5_116
.LBB5_115:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 88(%rsp)
	xorl	%eax, %eax
	jmp	.LBB5_136
.LBB5_116:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%r8b, %edx
.LBB5_117:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB5_118:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rcx, %rsi
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	cmoveq	%rcx, %rsi
	cmpq	%rdi, %r9
	cmoveq	%rax, %rdx
	movq	%rdx, 88(%rsp)
	movq	%rsi, 96(%rsp)
	movb	$1, %al
	jmp	.LBB5_136
.LBB5_119:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rsi, %rcx
	negq	%rcx
	movq	%r14, %rdx
	sbbq	%rax, %rdx
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	cmoveq	%rax, %rdx
	cmpq	%rdi, %r9
	cmoveq	%rsi, %rcx
	movq	%rcx, 88(%rsp)
	movq	%rdx, 96(%rsp)
	movb	$1, %al
	jmp	.LBB5_135
.LBB5_120:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_129
# %bb.121:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rdi, %rbp
	jbe	.LBB5_130
# %bb.122:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rcx
	jmp	.LBB5_131
.LBB5_123:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB5_360
# %bb.124:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rdi
	shrq	$63, %rdi
	addq	%rdx, %rdx
	movb	$1, %sil
	cmpq	%rdx, %rbp
	movq	%r14, %r8
	sbbq	%rdi, %r8
	jb	.LBB5_126
# %bb.125:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rbp, %rdx
	orq	%rdi, %rdx
	sete	%sil
	andb	%al, %sil
.LBB5_126:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%sil, %sil
	movq	8(%rsp), %r12                   # 8-byte Reload
	je	.LBB5_128
# %bb.127:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %r10
	xorq	%rcx, %r11
	orq	%r10, %r11
	je	.LBB5_65
.LBB5_128:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%sil, %edx
	jmp	.LBB5_67
.LBB5_129:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbp, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rax
	cmpq	%rbp, %rcx
	movq	%r14, %rdi
	cmovaeq	%rbp, %rdi
	movq	%rcx, %rdx
	subq	%rdi, %rdx
	movq	%r14, %rcx
	jmp	.LBB5_132
.LBB5_130:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rcx, %rax
	movq	%rdx, %rcx
	movq	%rbp, %rdx
	imulq	%rcx, %rdx
	subq	%rdx, %rdi
	cmpq	%rbp, %rdi
	sbbq	$-1, %rcx
	cmpq	%rbp, %rdi
	movq	%r14, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rdi
.LBB5_131:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%rbp
	#NO_APP
.LBB5_132:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r10, %rdi
	subq	%rsi, %rdi
	movq	%r11, %r8
	sbbq	%r12, %r8
	cmpq	%rax, %rdi
	sbbq	%rcx, %r8
	jb	.LBB5_134
# %bb.133:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rsi, %rax
	adcq	%r12, %rcx
	cmpq	%rax, %r10
	movq	%r11, %rsi
	sbbq	%rcx, %rsi
	jae	.LBB5_171
.LBB5_134:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 88(%rsp)
	xorl	%eax, %eax
.LBB5_135:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
	.p2align	4
.LBB5_136:                              #   in Loop: Header=BB5_19 Depth=1
	movb	%al, 104(%rsp)
	movq	-16(%r15,%rbx), %rax
	movq	-8(%r15,%rbx), %rdx
	movq	%rdx, %r9
	shrq	$63, %r9
	movq	%rax, %rcx
	negq	%rcx
	movq	%r14, %rdi
	sbbq	%rdx, %rdi
	testq	%rdx, %rdx
	cmovnsq	%rax, %rcx
	cmovnsq	%rdx, %rdi
	testq	%rdi, %rdi
	je	.LBB5_139
# %bb.137:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	$1, %rbp
	jne	.LBB5_141
# %bb.138:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %r8
	jmp	.LBB5_148
.LBB5_139:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rax
	mulq	56(%rsp)                        # 8-byte Folded Reload
	imulq	32(%rsp), %rcx                  # 8-byte Folded Reload
	addq	%rdx, %rcx
	cmpq	$1, %rbp
	jne	.LBB5_143
# %bb.140:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rdx
	jmp	.LBB5_157
.LBB5_141:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rdi, %rbp
	jbe	.LBB5_146
# %bb.142:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rsi
	jmp	.LBB5_147
.LBB5_143:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_154
# %bb.144:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %rbp
	jbe	.LBB5_155
# %bb.145:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rsi
	jmp	.LBB5_156
.LBB5_146:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rsi
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	cmpq	%rbp, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbp, %rdi
	movq	%r14, %rax
	cmovaeq	%rbp, %rax
	subq	%rax, %rdi
.LBB5_147:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rax
	movq	%rdi, %rdx
	#APP
	divq	%rbp
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
	movq	%rax, %rcx
.LBB5_148:                              #   in Loop: Header=BB5_19 Depth=1
	movq	24(%rsp), %rax                  # 8-byte Reload
	cmpq	%rax, %r9
	movq	%r14, %r10
	movq	$-1, %rdx
	cmoveq	%rdx, %r10
	cmpl	%eax, %r9d
	movabsq	$-9223372036854775808, %r11     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rax      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rax, %r11
	movq	56(%rsp), %r15                  # 8-byte Reload
	movq	%r15, %rax
	mulq	%rcx
	movq	%rax, %rsi
	movq	32(%rsp), %r12                  # 8-byte Reload
	imulq	%r12, %rcx
	addq	%rdx, %rcx
	imulq	%rdi, %r12
	movq	%r15, %rax
	mulq	%rdi
	addq	%rcx, %rax
	adcq	%r12, %rdx
	jne	.LBB5_150
# %bb.149:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rsi, %r10
	movq	%r11, %rcx
	sbbq	%rax, %rcx
	jae	.LBB5_151
.LBB5_150:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 112(%rsp)
	xorl	%eax, %eax
	movq	8(%rsp), %r12                   # 8-byte Reload
	movq	184(%rsp), %r15                 # 8-byte Reload
	jmp	.LBB5_184
.LBB5_151:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r8, %rcx
	orq	%r14, %rcx
	movq	184(%rsp), %r15                 # 8-byte Reload
	je	.LBB5_167
# %bb.152:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %r12
	movq	%r8, %rax
	movq	56(%rsp), %rcx                  # 8-byte Reload
	mulq	%rcx
	imulq	32(%rsp), %r8                   # 8-byte Folded Reload
	addq	%rdx, %r8
	movq	%r14, %rdi
	imulq	%rcx, %rdi
	addq	%r8, %rdi
	cmpq	$1, %rbp
	jne	.LBB5_168
# %bb.153:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rcx
	movq	%r14, %rdx
	jmp	.LBB5_180
.LBB5_154:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbp, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rax
	cmpq	%rbp, %rcx
	movq	%r14, %rsi
	cmovaeq	%rbp, %rsi
	movq	%rcx, %rdx
	subq	%rsi, %rdx
	movq	%r14, %rcx
	jmp	.LBB5_157
.LBB5_155:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rsi
	movq	%rcx, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbp, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rsi
	cmpq	%rbp, %rcx
	movq	%r14, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rcx
.LBB5_156:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rdx
	#APP
	divq	%rbp
	#NO_APP
	movq	%rsi, %rcx
.LBB5_157:                              #   in Loop: Header=BB5_19 Depth=1
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	movabsq	$-9223372036854775808, %rsi     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %r8       # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%r8, %rsi
	cmpq	%rdi, %r9
	movq	%r14, %rdi
	movq	$-1, %r8
	cmoveq	%r8, %rdi
	cmpq	%rax, %rdi
	movq	%rsi, %r8
	sbbq	%rcx, %r8
	jb	.LBB5_163
# %bb.158:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB5_166
# %bb.159:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %r10
	shrq	$63, %r10
	addq	%rdx, %rdx
	movb	$1, %r8b
	cmpq	%rdx, %rbp
	movq	%r14, %r11
	sbbq	%r10, %r11
	jb	.LBB5_161
# %bb.160:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rbp, %rdx
	orq	%r10, %rdx
	sete	%r8b
	andb	%al, %r8b
.LBB5_161:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %rdi
	xorq	%rcx, %rsi
	orq	%rdi, %rsi
	jne	.LBB5_164
# %bb.162:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%r8b, %r8b
	je	.LBB5_164
.LBB5_163:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 112(%rsp)
	xorl	%eax, %eax
	jmp	.LBB5_184
.LBB5_164:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%r8b, %edx
.LBB5_165:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB5_166:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rcx, %rsi
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	cmoveq	%rcx, %rsi
	cmpq	%rdi, %r9
	cmoveq	%rax, %rdx
	movq	%rdx, 112(%rsp)
	movq	%rsi, 120(%rsp)
	movb	$1, %al
	jmp	.LBB5_184
.LBB5_167:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rsi, %rcx
	negq	%rcx
	movq	%r14, %rdx
	sbbq	%rax, %rdx
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	cmoveq	%rax, %rdx
	cmpq	%rdi, %r9
	cmoveq	%rsi, %rcx
	movq	%rcx, 112(%rsp)
	movq	%rdx, 120(%rsp)
	movb	$1, %al
	jmp	.LBB5_183
.LBB5_168:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_177
# %bb.169:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rdi, %rbp
	jbe	.LBB5_178
# %bb.170:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rcx
	jmp	.LBB5_179
.LBB5_171:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB5_361
# %bb.172:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rdi
	shrq	$63, %rdi
	addq	%rdx, %rdx
	movb	$1, %sil
	cmpq	%rdx, %rbp
	movq	%r14, %r8
	sbbq	%rdi, %r8
	jb	.LBB5_174
# %bb.173:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rbp, %rdx
	orq	%rdi, %rdx
	sete	%sil
	andb	%al, %sil
.LBB5_174:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %r10
	xorq	%rcx, %r11
	orq	%r10, %r11
	movq	8(%rsp), %r12                   # 8-byte Reload
	jne	.LBB5_176
# %bb.175:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%sil, %sil
	jne	.LBB5_115
.LBB5_176:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%sil, %edx
	jmp	.LBB5_117
.LBB5_177:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbp, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rax
	cmpq	%rbp, %rcx
	movq	%r14, %rdi
	cmovaeq	%rbp, %rdi
	movq	%rcx, %rdx
	subq	%rdi, %rdx
	movq	%r14, %rcx
	jmp	.LBB5_180
.LBB5_178:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rcx, %rax
	movq	%rdx, %rcx
	movq	%rbp, %rdx
	imulq	%rcx, %rdx
	subq	%rdx, %rdi
	cmpq	%rbp, %rdi
	sbbq	$-1, %rcx
	cmpq	%rbp, %rdi
	movq	%r14, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rdi
.LBB5_179:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%rbp
	#NO_APP
.LBB5_180:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r10, %rdi
	subq	%rsi, %rdi
	movq	%r11, %r8
	sbbq	%r12, %r8
	cmpq	%rax, %rdi
	sbbq	%rcx, %r8
	jb	.LBB5_182
# %bb.181:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rsi, %rax
	adcq	%r12, %rcx
	cmpq	%rax, %r10
	movq	%r11, %rsi
	sbbq	%rcx, %rsi
	jae	.LBB5_219
.LBB5_182:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 112(%rsp)
	xorl	%eax, %eax
.LBB5_183:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
	.p2align	4
.LBB5_184:                              #   in Loop: Header=BB5_19 Depth=1
	movb	%al, 128(%rsp)
	movq	(%r15,%rbx), %rax
	movq	8(%r15,%rbx), %rdx
	movq	%rdx, %r9
	shrq	$63, %r9
	movq	%rax, %rcx
	negq	%rcx
	movq	%r14, %rdi
	sbbq	%rdx, %rdi
	testq	%rdx, %rdx
	cmovnsq	%rax, %rcx
	cmovnsq	%rdx, %rdi
	testq	%rdi, %rdi
	je	.LBB5_187
# %bb.185:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	$1, %rbp
	jne	.LBB5_189
# %bb.186:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %r8
	jmp	.LBB5_196
.LBB5_187:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rax
	mulq	56(%rsp)                        # 8-byte Folded Reload
	imulq	32(%rsp), %rcx                  # 8-byte Folded Reload
	addq	%rdx, %rcx
	cmpq	$1, %rbp
	jne	.LBB5_191
# %bb.188:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rdx
	jmp	.LBB5_206
.LBB5_189:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rdi, %rbp
	jbe	.LBB5_194
# %bb.190:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rsi
	jmp	.LBB5_195
.LBB5_191:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_203
# %bb.192:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %rbp
	jbe	.LBB5_204
# %bb.193:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rsi
	jmp	.LBB5_205
.LBB5_194:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rsi
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	cmpq	%rbp, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbp, %rdi
	movq	%r14, %rax
	cmovaeq	%rbp, %rax
	subq	%rax, %rdi
.LBB5_195:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rax
	movq	%rdi, %rdx
	#APP
	divq	%rbp
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
	movq	%rax, %rcx
.LBB5_196:                              #   in Loop: Header=BB5_19 Depth=1
	movq	24(%rsp), %rax                  # 8-byte Reload
	cmpq	%rax, %r9
	movq	%r14, %r10
	movq	$-1, %rdx
	cmoveq	%rdx, %r10
	cmpl	%eax, %r9d
	movabsq	$-9223372036854775808, %r11     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rax      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rax, %r11
	movq	56(%rsp), %r15                  # 8-byte Reload
	movq	%r15, %rax
	mulq	%rcx
	movq	%rax, %rsi
	movq	32(%rsp), %r12                  # 8-byte Reload
	imulq	%r12, %rcx
	addq	%rdx, %rcx
	imulq	%rdi, %r12
	movq	%r15, %rax
	mulq	%rdi
	addq	%rcx, %rax
	adcq	%r12, %rdx
	jne	.LBB5_198
# %bb.197:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rsi, %r10
	movq	%r11, %rcx
	sbbq	%rax, %rcx
	jae	.LBB5_200
.LBB5_198:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 136(%rsp)
	movb	$0, 152(%rsp)
.LBB5_199:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
	movq	184(%rsp), %r15                 # 8-byte Reload
	cmpb	$1, 80(%rsp)
	movq	176(%rsp), %rsi                 # 8-byte Reload
	je	.LBB5_353
	jmp	.LBB5_320
.LBB5_200:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r8, %rcx
	orq	%r14, %rcx
	je	.LBB5_215
# %bb.201:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %r12
	movq	%r8, %rax
	movq	56(%rsp), %rcx                  # 8-byte Reload
	mulq	%rcx
	imulq	32(%rsp), %r8                   # 8-byte Folded Reload
	addq	%rdx, %r8
	movq	%r14, %rdi
	imulq	%rcx, %rdi
	addq	%r8, %rdi
	cmpq	$1, %rbp
	movq	184(%rsp), %r15                 # 8-byte Reload
	jne	.LBB5_216
# %bb.202:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rcx
	movq	%r14, %rdx
	jmp	.LBB5_228
.LBB5_203:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbp, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rax
	cmpq	%rbp, %rcx
	movq	%r14, %rsi
	cmovaeq	%rbp, %rsi
	movq	%rcx, %rdx
	subq	%rsi, %rdx
	movq	%r14, %rcx
	jmp	.LBB5_206
.LBB5_204:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rsi
	movq	%rcx, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbp, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rsi
	cmpq	%rbp, %rcx
	movq	%r14, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rcx
.LBB5_205:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rdx
	#APP
	divq	%rbp
	#NO_APP
	movq	%rsi, %rcx
.LBB5_206:                              #   in Loop: Header=BB5_19 Depth=1
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	movabsq	$-9223372036854775808, %rsi     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %r8       # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%r8, %rsi
	cmpq	%rdi, %r9
	movq	%r14, %rdi
	movq	$-1, %r8
	cmoveq	%r8, %rdi
	cmpq	%rax, %rdi
	movq	%rsi, %r8
	sbbq	%rcx, %r8
	jb	.LBB5_327
# %bb.207:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB5_214
# %bb.208:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %r10
	shrq	$63, %r10
	addq	%rdx, %rdx
	movb	$1, %r8b
	cmpq	%rdx, %rbp
	movq	%r14, %r11
	sbbq	%r10, %r11
	jb	.LBB5_210
# %bb.209:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rbp, %rdx
	orq	%r10, %rdx
	sete	%r8b
	andb	%al, %r8b
.LBB5_210:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %rdi
	xorq	%rcx, %rsi
	orq	%rdi, %rsi
	jne	.LBB5_212
# %bb.211:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%r8b, %r8b
	jne	.LBB5_327
.LBB5_212:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%r8b, %edx
.LBB5_213:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB5_214:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rcx, %rsi
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	cmoveq	%rcx, %rsi
	cmpq	%rdi, %r9
	cmoveq	%rax, %rdx
	jmp	.LBB5_341
.LBB5_215:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rsi, %rcx
	negq	%rcx
	movq	%r14, %rdx
	sbbq	%rax, %rdx
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r9d
	cmoveq	%rax, %rdx
	cmpq	%rdi, %r9
	cmoveq	%rsi, %rcx
	movq	%rcx, 136(%rsp)
	movq	%rdx, 144(%rsp)
	movb	$1, 152(%rsp)
	jmp	.LBB5_199
.LBB5_216:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_225
# %bb.217:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rdi, %rbp
	jbe	.LBB5_226
# %bb.218:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r14, %rcx
	jmp	.LBB5_227
.LBB5_219:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB5_362
# %bb.220:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rdi
	shrq	$63, %rdi
	addq	%rdx, %rdx
	movb	$1, %sil
	cmpq	%rdx, %rbp
	movq	%r14, %r8
	sbbq	%rdi, %r8
	jb	.LBB5_222
# %bb.221:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rbp, %rdx
	orq	%rdi, %rdx
	sete	%sil
	andb	%al, %sil
.LBB5_222:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %r10
	xorq	%rcx, %r11
	orq	%r10, %r11
	movq	8(%rsp), %r12                   # 8-byte Reload
	jne	.LBB5_224
# %bb.223:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%sil, %sil
	jne	.LBB5_163
.LBB5_224:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%sil, %edx
	jmp	.LBB5_165
.LBB5_225:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbp, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rax
	cmpq	%rbp, %rcx
	movq	%r14, %rdi
	cmovaeq	%rbp, %rdi
	movq	%rcx, %rdx
	subq	%rdi, %rdx
	movq	%r14, %rcx
	jmp	.LBB5_228
.LBB5_226:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rcx, %rax
	movq	%rdx, %rcx
	movq	%rbp, %rdx
	imulq	%rcx, %rdx
	subq	%rdx, %rdi
	cmpq	%rbp, %rdi
	sbbq	$-1, %rcx
	cmpq	%rbp, %rdi
	movq	%r14, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rdi
.LBB5_227:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%rbp
	#NO_APP
.LBB5_228:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r10, %rdi
	subq	%rsi, %rdi
	movq	%r11, %r8
	sbbq	%r12, %r8
	cmpq	%rax, %rdi
	sbbq	%rcx, %r8
	jb	.LBB5_352
# %bb.229:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rsi, %rax
	adcq	%r12, %rcx
	cmpq	%rax, %r10
	movq	%r11, %rsi
	sbbq	%rcx, %rsi
	jb	.LBB5_352
# %bb.230:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdx, %rdx
	je	.LBB5_363
# %bb.231:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rdi
	shrq	$63, %rdi
	addq	%rdx, %rdx
	movb	$1, %sil
	cmpq	%rdx, %rbp
	movq	%r14, %r8
	sbbq	%rdi, %r8
	jb	.LBB5_233
# %bb.232:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rbp, %rdx
	orq	%rdi, %rdx
	sete	%sil
	andb	%al, %sil
.LBB5_233:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %r10
	xorq	%rcx, %r11
	orq	%r10, %r11
	movq	8(%rsp), %r12                   # 8-byte Reload
	jne	.LBB5_235
# %bb.234:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%sil, %sil
	jne	.LBB5_327
.LBB5_235:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%sil, %edx
	jmp	.LBB5_213
.LBB5_236:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rdx
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	movq	%r14, %rsi
.LBB5_237:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r11, %rdx
	subq	%rcx, %rdx
	movq	%r10, %r8
	sbbq	40(%rsp), %r8                   # 8-byte Folded Reload
	cmpq	%rax, %rdx
	sbbq	%rsi, %r8
	jb	.LBB5_246
# %bb.238:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rcx, %rax
	adcq	40(%rsp), %rsi                  # 8-byte Folded Reload
	cmpq	%rax, %r11
	movq	%r10, %rcx
	sbbq	%rsi, %rcx
	jb	.LBB5_246
# %bb.239:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_391
# %bb.240:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$1, %cl
	movabsq	$500000000000, %rdx             # imm = 0x746A528800
	cmpq	%rdx, %rdi
	ja	.LBB5_242
# %bb.241:                              #   in Loop: Header=BB5_19 Depth=1
	sete	%dl
	movl	%eax, %ecx
	andb	%dl, %cl
.LBB5_242:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%cl, %cl
	movq	8(%rsp), %r12                   # 8-byte Reload
	jne	.LBB5_368
	jmp	.LBB5_369
.LBB5_243:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rdx
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	movq	%r14, %rsi
.LBB5_244:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r11, %rdx
	subq	%rcx, %rdx
	movq	%r10, %r8
	movq	40(%rsp), %r12                  # 8-byte Reload
	sbbq	%r12, %r8
	cmpq	%rax, %rdx
	sbbq	%rsi, %r8
	jb	.LBB5_246
# %bb.245:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rcx, %rax
	adcq	%r12, %rsi
	cmpq	%rax, %r11
	movq	%r10, %rcx
	sbbq	%rsi, %rcx
	jae	.LBB5_364
.LBB5_246:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 64(%rsp)
	movb	$0, 80(%rsp)
	movq	8(%rsp), %r12                   # 8-byte Reload
	.p2align	4
.LBB5_247:                              #   in Loop: Header=BB5_19 Depth=1
	movq	-32(%r15,%rbx), %rsi
	movq	-24(%r15,%rbx), %rdx
	movq	%rsi, %r8
	negq	%r8
	movq	%r14, %rcx
	sbbq	%rdx, %rcx
	testq	%rdx, %rdx
	cmovnsq	%rsi, %r8
	cmovnsq	%rdx, %rcx
	movq	%rdx, %r9
	xorq	%r12, %r9
	cmpq	$0, 160(%rsp)                   # 8-byte Folded Reload
	je	.LBB5_250
# %bb.248:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_254
# %bb.249:                              #   in Loop: Header=BB5_19 Depth=1
	leaq	88(%rsp), %rdi
	movq	200(%rsp), %rcx                 # 8-byte Reload
	movq	%r12, %r8
	movl	$3, %r9d
	callq	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	jmp	.LBB5_282
.LBB5_250:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_257
# %bb.251:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r12, %rbp
	movq	%rcx, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rax
	subq	%rax, %rcx
	movq	%r8, %rax
	movq	%rcx, %rdx
	#APP
	divq	%rsi
	#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rsi
	testq	%r9, %r9
	movabsq	$9223372036854775807, %r10      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r10
	movq	$-1, %r11
	cmovsq	%r14, %r11
	movq	48(%rsp), %r12                  # 8-byte Reload
	movq	%r12, %rax
	mulq	%rcx
	movq	%rdx, %r8
	movq	%rax, %rcx
	movq	%r12, %rax
	mulq	%rdi
	addq	%r8, %rax
	adcq	$0, %rdx
	jne	.LBB5_253
# %bb.252:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %r11
	movq	%r10, %rdx
	sbbq	%rax, %rdx
	jae	.LBB5_259
.LBB5_253:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 88(%rsp)
	movb	$0, 104(%rsp)
	movq	%rbp, %r12
	movq	168(%rsp), %rbp                 # 8-byte Reload
	jmp	.LBB5_282
.LBB5_254:                              #   in Loop: Header=BB5_19 Depth=1
	movq	48(%rsp), %rax                  # 8-byte Reload
	movq	208(%rsp), %rdx                 # 8-byte Reload
	movabsq	$1000000000000, %rcx            # imm = 0xE8D4A51000
	#APP
	divq	%rcx
	#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rdi
	testq	%r9, %r9
	movabsq	$9223372036854775807, %r10      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r10
	movq	$-1, %r11
	cmovsq	%r14, %r11
	movq	%r8, %rax
	mulq	%rcx
	movq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%r8, %rax
	mulq	216(%rsp)                       # 8-byte Folded Reload
	addq	%rsi, %rax
	adcq	$0, %rdx
	jne	.LBB5_256
# %bb.255:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %r11
	movq	%r10, %rdx
	sbbq	%rax, %rdx
	jae	.LBB5_262
.LBB5_256:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 88(%rsp)
	movb	$0, 104(%rsp)
	jmp	.LBB5_282
.LBB5_257:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r8, %rax
	mulq	48(%rsp)                        # 8-byte Folded Reload
	movq	%rax, %rsi
	testq	%rdx, %rdx
	je	.LBB5_265
# %bb.258:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %rdi
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r8             # imm = 0xE8D4A51000
	imulq	%r8, %rax
	subq	%rax, %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	#APP
	divq	%r8
	#NO_APP
	movq	%rdx, %rsi
	testq	%rsi, %rsi
	jne	.LBB5_266
	jmp	.LBB5_269
.LBB5_259:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rsi, %rsi
	je	.LBB5_271
# %bb.260:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	48(%rsp), %rax                  # 8-byte Reload
	mulq	%rsi
	movq	%rax, %rdi
	testq	%rdx, %rdx
	movq	168(%rsp), %rbp                 # 8-byte Reload
	je	.LBB5_273
# %bb.261:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r8
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movabsq	$1000000000000, %r12            # imm = 0xE8D4A51000
	imulq	%r12, %rax
	subq	%rax, %r8
	movq	%rdi, %rax
	movq	%r8, %rdx
	#APP
	divq	%r12
	#NO_APP
	movq	%rdx, %rdi
	jmp	.LBB5_274
.LBB5_262:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_272
# %bb.263:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	%r8, %rax
	mulq	%rdi
	movq	%rax, %rdi
	testq	%rdx, %rdx
	je	.LBB5_278
# %bb.264:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r8
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movabsq	$1000000000000, %r12            # imm = 0xE8D4A51000
	imulq	%r12, %rax
	subq	%rax, %r8
	movq	%rdi, %rax
	movq	%r8, %rdx
	#APP
	divq	%r12
	#NO_APP
	movq	%rdx, %rdi
	jmp	.LBB5_279
.LBB5_265:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rsi, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %rsi
	movq	%r14, %rcx
	testq	%rsi, %rsi
	je	.LBB5_269
.LBB5_266:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rdi             # imm = 0x746A528800
	cmpq	%rdi, %rsi
	ja	.LBB5_268
# %bb.267:                              #   in Loop: Header=BB5_19 Depth=1
	sete	%sil
	movl	%eax, %edx
	andb	%sil, %dl
.LBB5_268:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB5_269:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rcx, %rsi
	testq	%r9, %r9
	cmovnsq	%rcx, %rsi
	cmovnsq	%rax, %rdx
.LBB5_270:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, 88(%rsp)
	movq	%rsi, 96(%rsp)
	movb	$1, 104(%rsp)
	jmp	.LBB5_282
.LBB5_271:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rbp, %r12
	movq	%rcx, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rax, %rsi
	testq	%r9, %r9
	cmovnsq	%rax, %rsi
	cmovnsq	%rcx, %rdx
	movq	%rdx, 88(%rsp)
	movq	%rsi, 96(%rsp)
	movb	$1, 104(%rsp)
	movq	168(%rsp), %rbp                 # 8-byte Reload
	jmp	.LBB5_282
.LBB5_272:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rax, %rsi
	testq	%r9, %r9
	cmovnsq	%rax, %rsi
	cmovnsq	%rcx, %rdx
	jmp	.LBB5_270
.LBB5_273:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rdx
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	movq	%r14, %rsi
.LBB5_274:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r11, %rdx
	subq	%rcx, %rdx
	movq	%r10, %r8
	sbbq	40(%rsp), %r8                   # 8-byte Folded Reload
	cmpq	%rax, %rdx
	sbbq	%rsi, %r8
	jb	.LBB5_281
# %bb.275:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rcx, %rax
	adcq	40(%rsp), %rsi                  # 8-byte Folded Reload
	cmpq	%rax, %r11
	movq	%r10, %rcx
	sbbq	%rsi, %rcx
	jb	.LBB5_281
	jmp	.LBB5_276
.LBB5_278:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rdx
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	movq	%r14, %rsi
.LBB5_279:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r11, %rdx
	subq	%rcx, %rdx
	movq	%r10, %r8
	movq	40(%rsp), %r12                  # 8-byte Reload
	sbbq	%r12, %r8
	cmpq	%rax, %rdx
	sbbq	%rsi, %r8
	jb	.LBB5_281
# %bb.280:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rcx, %rax
	adcq	%r12, %rsi
	cmpq	%rax, %r11
	movq	%r10, %rcx
	sbbq	%rsi, %rcx
	jae	.LBB5_276
.LBB5_281:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 88(%rsp)
	movb	$0, 104(%rsp)
	movq	8(%rsp), %r12                   # 8-byte Reload
	.p2align	4
.LBB5_282:                              #   in Loop: Header=BB5_19 Depth=1
	movq	-16(%r15,%rbx), %rsi
	movq	-8(%r15,%rbx), %rdx
	movq	%rsi, %r8
	negq	%r8
	movq	%r14, %rcx
	sbbq	%rdx, %rcx
	testq	%rdx, %rdx
	cmovnsq	%rsi, %r8
	cmovnsq	%rdx, %rcx
	movq	%rdx, %r9
	xorq	%r12, %r9
	cmpq	$0, 160(%rsp)                   # 8-byte Folded Reload
	je	.LBB5_285
# %bb.283:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_289
# %bb.284:                              #   in Loop: Header=BB5_19 Depth=1
	leaq	112(%rsp), %rdi
	movq	200(%rsp), %rcx                 # 8-byte Reload
	movq	%r12, %r8
	movl	$3, %r9d
	callq	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	jmp	.LBB5_317
.LBB5_285:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_292
# %bb.286:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r12, %rbp
	movq	%rcx, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rax
	subq	%rax, %rcx
	movq	%r8, %rax
	movq	%rcx, %rdx
	#APP
	divq	%rsi
	#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rsi
	testq	%r9, %r9
	movabsq	$9223372036854775807, %r10      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r10
	movq	$-1, %r11
	cmovsq	%r14, %r11
	movq	48(%rsp), %r12                  # 8-byte Reload
	movq	%r12, %rax
	mulq	%rcx
	movq	%rdx, %r8
	movq	%rax, %rcx
	movq	%r12, %rax
	mulq	%rdi
	addq	%r8, %rax
	adcq	$0, %rdx
	jne	.LBB5_288
# %bb.287:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %r11
	movq	%r10, %rdx
	sbbq	%rax, %rdx
	jae	.LBB5_294
.LBB5_288:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 112(%rsp)
	movb	$0, 128(%rsp)
	movq	%rbp, %r12
	movq	168(%rsp), %rbp                 # 8-byte Reload
	jmp	.LBB5_317
.LBB5_289:                              #   in Loop: Header=BB5_19 Depth=1
	movq	48(%rsp), %rax                  # 8-byte Reload
	movq	208(%rsp), %rdx                 # 8-byte Reload
	movabsq	$1000000000000, %rcx            # imm = 0xE8D4A51000
	#APP
	divq	%rcx
	#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rdi
	testq	%r9, %r9
	movabsq	$9223372036854775807, %r10      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r10
	movq	$-1, %r11
	cmovsq	%r14, %r11
	movq	%r8, %rax
	mulq	%rcx
	movq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%r8, %rax
	mulq	216(%rsp)                       # 8-byte Folded Reload
	addq	%rsi, %rax
	adcq	$0, %rdx
	jne	.LBB5_291
# %bb.290:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %r11
	movq	%r10, %rdx
	sbbq	%rax, %rdx
	jae	.LBB5_297
.LBB5_291:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 112(%rsp)
	movb	$0, 128(%rsp)
	jmp	.LBB5_317
.LBB5_292:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r8, %rax
	mulq	48(%rsp)                        # 8-byte Folded Reload
	movq	%rax, %rsi
	testq	%rdx, %rdx
	je	.LBB5_300
# %bb.293:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %rdi
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r8             # imm = 0xE8D4A51000
	imulq	%r8, %rax
	subq	%rax, %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	#APP
	divq	%r8
	#NO_APP
	movq	%rdx, %rsi
	testq	%rsi, %rsi
	jne	.LBB5_301
	jmp	.LBB5_304
.LBB5_294:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rsi, %rsi
	je	.LBB5_306
# %bb.295:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	48(%rsp), %rax                  # 8-byte Reload
	mulq	%rsi
	movq	%rax, %rdi
	testq	%rdx, %rdx
	movq	168(%rsp), %rbp                 # 8-byte Reload
	je	.LBB5_308
# %bb.296:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r8
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movabsq	$1000000000000, %r12            # imm = 0xE8D4A51000
	imulq	%r12, %rax
	subq	%rax, %r8
	movq	%rdi, %rax
	movq	%r8, %rdx
	#APP
	divq	%r12
	#NO_APP
	movq	%rdx, %rdi
	jmp	.LBB5_309
.LBB5_297:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_307
# %bb.298:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	%r8, %rax
	mulq	%rdi
	movq	%rax, %rdi
	testq	%rdx, %rdx
	je	.LBB5_313
# %bb.299:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r8
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movabsq	$1000000000000, %r12            # imm = 0xE8D4A51000
	imulq	%r12, %rax
	subq	%rax, %r8
	movq	%rdi, %rax
	movq	%r8, %rdx
	#APP
	divq	%r12
	#NO_APP
	movq	%rdx, %rdi
	jmp	.LBB5_314
.LBB5_300:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rsi, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %rsi
	movq	%r14, %rcx
	testq	%rsi, %rsi
	je	.LBB5_304
.LBB5_301:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rdi             # imm = 0x746A528800
	cmpq	%rdi, %rsi
	ja	.LBB5_303
# %bb.302:                              #   in Loop: Header=BB5_19 Depth=1
	sete	%sil
	movl	%eax, %edx
	andb	%sil, %dl
.LBB5_303:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB5_304:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rcx, %rsi
	testq	%r9, %r9
	cmovnsq	%rcx, %rsi
	cmovnsq	%rax, %rdx
.LBB5_305:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, 112(%rsp)
	movq	%rsi, 120(%rsp)
	movb	$1, 128(%rsp)
	jmp	.LBB5_317
.LBB5_306:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rbp, %r12
	movq	%rcx, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rax, %rsi
	testq	%r9, %r9
	cmovnsq	%rax, %rsi
	cmovnsq	%rcx, %rdx
	movq	%rdx, 112(%rsp)
	movq	%rsi, 120(%rsp)
	movb	$1, 128(%rsp)
	movq	168(%rsp), %rbp                 # 8-byte Reload
	jmp	.LBB5_317
.LBB5_307:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rax, %rsi
	testq	%r9, %r9
	cmovnsq	%rax, %rsi
	cmovnsq	%rcx, %rdx
	jmp	.LBB5_305
.LBB5_276:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_277
# %bb.371:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$1, %cl
	movabsq	$500000000000, %rdx             # imm = 0x746A528800
	cmpq	%rdx, %rdi
	ja	.LBB5_373
# %bb.372:                              #   in Loop: Header=BB5_19 Depth=1
	sete	%dl
	movl	%eax, %ecx
	andb	%dl, %cl
.LBB5_373:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%cl, %cl
	movq	8(%rsp), %r12                   # 8-byte Reload
	je	.LBB5_375
# %bb.374:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %r11
	xorq	%rsi, %r10
	orq	%r11, %r10
	je	.LBB5_256
.LBB5_375:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%cl, %ecx
	addq	%rcx, %rax
	adcq	$0, %rsi
	jmp	.LBB5_376
.LBB5_308:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rdx
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	movq	%r14, %rsi
.LBB5_309:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r11, %rdx
	subq	%rcx, %rdx
	movq	%r10, %r8
	sbbq	40(%rsp), %r8                   # 8-byte Folded Reload
	cmpq	%rax, %rdx
	sbbq	%rsi, %r8
	jb	.LBB5_316
# %bb.310:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rcx, %rax
	adcq	40(%rsp), %rsi                  # 8-byte Folded Reload
	cmpq	%rax, %r11
	movq	%r10, %rcx
	sbbq	%rsi, %rcx
	jb	.LBB5_316
	jmp	.LBB5_311
.LBB5_313:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rdx
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	movq	%r14, %rsi
.LBB5_314:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r11, %rdx
	subq	%rcx, %rdx
	movq	%r10, %r8
	movq	40(%rsp), %r12                  # 8-byte Reload
	sbbq	%r12, %r8
	cmpq	%rax, %rdx
	sbbq	%rsi, %r8
	jb	.LBB5_316
# %bb.315:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rcx, %rax
	adcq	%r12, %rsi
	cmpq	%rax, %r11
	movq	%r10, %rcx
	sbbq	%rsi, %rcx
	jae	.LBB5_311
.LBB5_316:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 112(%rsp)
	movb	$0, 128(%rsp)
	movq	8(%rsp), %r12                   # 8-byte Reload
	.p2align	4
.LBB5_317:                              #   in Loop: Header=BB5_19 Depth=1
	movq	(%r15,%rbx), %rsi
	movq	8(%r15,%rbx), %rdx
	movq	%rsi, %r8
	negq	%r8
	movq	%r14, %rcx
	sbbq	%rdx, %rcx
	testq	%rdx, %rdx
	cmovnsq	%rsi, %r8
	cmovnsq	%rdx, %rcx
	movq	%rdx, %r9
	xorq	%r12, %r9
	cmpq	$0, 160(%rsp)                   # 8-byte Folded Reload
	je	.LBB5_321
# %bb.318:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_325
# %bb.319:                              #   in Loop: Header=BB5_19 Depth=1
	leaq	136(%rsp), %rdi
	movq	200(%rsp), %rcx                 # 8-byte Reload
	movq	%r12, %r8
	movl	$3, %r9d
	callq	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	cmpb	$1, 80(%rsp)
	movq	176(%rsp), %rsi                 # 8-byte Reload
	je	.LBB5_353
	jmp	.LBB5_320
.LBB5_321:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_328
# %bb.322:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r12, %rbp
	movq	%rcx, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rax
	subq	%rax, %rcx
	movq	%r8, %rax
	movq	%rcx, %rdx
	#APP
	divq	%rsi
	#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rsi
	testq	%r9, %r9
	movabsq	$9223372036854775807, %r10      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r10
	movq	$-1, %r11
	cmovsq	%r14, %r11
	movq	48(%rsp), %r12                  # 8-byte Reload
	movq	%r12, %rax
	mulq	%rcx
	movq	%rdx, %r8
	movq	%rax, %rcx
	movq	%r12, %rax
	mulq	%rdi
	addq	%r8, %rax
	adcq	$0, %rdx
	jne	.LBB5_324
# %bb.323:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %r11
	movq	%r10, %rdx
	sbbq	%rax, %rdx
	jae	.LBB5_330
.LBB5_324:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 136(%rsp)
	movb	$0, 152(%rsp)
	movq	%rbp, %r12
	movq	168(%rsp), %rbp                 # 8-byte Reload
	cmpb	$1, 80(%rsp)
	movq	176(%rsp), %rsi                 # 8-byte Reload
	je	.LBB5_353
	jmp	.LBB5_320
.LBB5_325:                              #   in Loop: Header=BB5_19 Depth=1
	movq	48(%rsp), %rax                  # 8-byte Reload
	movq	208(%rsp), %rdx                 # 8-byte Reload
	movabsq	$1000000000000, %rcx            # imm = 0xE8D4A51000
	#APP
	divq	%rcx
	#NO_APP
	movq	%rax, %rcx
	movq	%rdx, %rdi
	testq	%r9, %r9
	movabsq	$9223372036854775807, %r10      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r10
	movq	$-1, %r11
	cmovsq	%r14, %r11
	movq	%r8, %rax
	mulq	%rcx
	movq	%rdx, %rsi
	movq	%rax, %rcx
	movq	%r8, %rax
	mulq	216(%rsp)                       # 8-byte Folded Reload
	addq	%rsi, %rax
	adcq	$0, %rdx
	jne	.LBB5_327
# %bb.326:                              #   in Loop: Header=BB5_19 Depth=1
	cmpq	%rcx, %r11
	movq	%r10, %rdx
	sbbq	%rax, %rdx
	jae	.LBB5_333
	.p2align	4
.LBB5_327:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 136(%rsp)
	movb	$0, 152(%rsp)
	cmpb	$1, 80(%rsp)
	movq	176(%rsp), %rsi                 # 8-byte Reload
	je	.LBB5_353
	jmp	.LBB5_320
.LBB5_328:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r8, %rax
	mulq	48(%rsp)                        # 8-byte Folded Reload
	movq	%rax, %rsi
	testq	%rdx, %rdx
	je	.LBB5_336
# %bb.329:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %rdi
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r8             # imm = 0xE8D4A51000
	imulq	%r8, %rax
	subq	%rax, %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	#APP
	divq	%r8
	#NO_APP
	movq	%rdx, %rsi
	testq	%rsi, %rsi
	jne	.LBB5_337
	jmp	.LBB5_340
.LBB5_330:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rsi, %rsi
	je	.LBB5_342
# %bb.331:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	48(%rsp), %rax                  # 8-byte Reload
	mulq	%rsi
	movq	%rax, %rdi
	testq	%rdx, %rdx
	movq	168(%rsp), %rbp                 # 8-byte Reload
	je	.LBB5_344
# %bb.332:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r8
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movabsq	$1000000000000, %r12            # imm = 0xE8D4A51000
	imulq	%r12, %rax
	subq	%rax, %r8
	movq	%rdi, %rax
	movq	%r8, %rdx
	#APP
	divq	%r12
	#NO_APP
	movq	%rdx, %rdi
	jmp	.LBB5_345
.LBB5_333:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_343
# %bb.334:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	%r8, %rax
	mulq	%rdi
	movq	%rax, %rdi
	testq	%rdx, %rdx
	je	.LBB5_349
# %bb.335:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r8
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movabsq	$1000000000000, %r12            # imm = 0xE8D4A51000
	imulq	%r12, %rax
	subq	%rax, %r8
	movq	%rdi, %rax
	movq	%r8, %rdx
	#APP
	divq	%r12
	#NO_APP
	movq	%rdx, %rdi
	jmp	.LBB5_350
.LBB5_336:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rsi, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %rsi
	movq	%r14, %rcx
	testq	%rsi, %rsi
	je	.LBB5_340
.LBB5_337:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rdi             # imm = 0x746A528800
	cmpq	%rdi, %rsi
	ja	.LBB5_339
# %bb.338:                              #   in Loop: Header=BB5_19 Depth=1
	sete	%sil
	movl	%eax, %edx
	andb	%sil, %dl
.LBB5_339:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB5_340:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rcx, %rsi
	testq	%r9, %r9
	cmovnsq	%rcx, %rsi
	cmovnsq	%rax, %rdx
.LBB5_341:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdx, 136(%rsp)
	movq	%rsi, 144(%rsp)
	movb	$1, 152(%rsp)
	cmpb	$1, 80(%rsp)
	movq	176(%rsp), %rsi                 # 8-byte Reload
	je	.LBB5_353
	jmp	.LBB5_320
.LBB5_342:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rbp, %r12
	movq	%rcx, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rax, %rsi
	testq	%r9, %r9
	cmovnsq	%rax, %rsi
	cmovnsq	%rcx, %rdx
	movq	%rdx, 136(%rsp)
	movq	%rsi, 144(%rsp)
	movb	$1, 152(%rsp)
	movq	168(%rsp), %rbp                 # 8-byte Reload
	cmpb	$1, 80(%rsp)
	movq	176(%rsp), %rsi                 # 8-byte Reload
	je	.LBB5_353
	jmp	.LBB5_320
.LBB5_343:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rcx, %rdx
	negq	%rdx
	movq	%r14, %rsi
	sbbq	%rax, %rsi
	testq	%r9, %r9
	cmovnsq	%rax, %rsi
	cmovnsq	%rcx, %rdx
	jmp	.LBB5_341
.LBB5_311:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_312
# %bb.378:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$1, %cl
	movabsq	$500000000000, %rdx             # imm = 0x746A528800
	cmpq	%rdx, %rdi
	ja	.LBB5_380
# %bb.379:                              #   in Loop: Header=BB5_19 Depth=1
	sete	%dl
	movl	%eax, %ecx
	andb	%dl, %cl
.LBB5_380:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%cl, %cl
	movq	8(%rsp), %r12                   # 8-byte Reload
	je	.LBB5_382
# %bb.381:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %r11
	xorq	%rsi, %r10
	orq	%r11, %r10
	je	.LBB5_291
.LBB5_382:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%cl, %ecx
	addq	%rcx, %rax
	adcq	$0, %rsi
	jmp	.LBB5_383
.LBB5_344:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rdx
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	movq	%r14, %rsi
.LBB5_345:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r11, %rdx
	subq	%rcx, %rdx
	movq	%r10, %r8
	sbbq	40(%rsp), %r8                   # 8-byte Folded Reload
	cmpq	%rax, %rdx
	sbbq	%rsi, %r8
	jb	.LBB5_352
# %bb.346:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rcx, %rax
	adcq	40(%rsp), %rsi                  # 8-byte Folded Reload
	cmpq	%rax, %r11
	movq	%r10, %rcx
	sbbq	%rsi, %rcx
	jb	.LBB5_352
	jmp	.LBB5_347
.LBB5_349:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rdi, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rdx
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	movq	%r14, %rsi
.LBB5_350:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%r11, %rdx
	subq	%rcx, %rdx
	movq	%r10, %r8
	movq	40(%rsp), %r12                  # 8-byte Reload
	sbbq	%r12, %r8
	cmpq	%rax, %rdx
	sbbq	%rsi, %r8
	jb	.LBB5_352
# %bb.351:                              #   in Loop: Header=BB5_19 Depth=1
	addq	%rcx, %rax
	adcq	%r12, %rsi
	cmpq	%rax, %r11
	movq	%r10, %rcx
	sbbq	%rsi, %rcx
	jae	.LBB5_347
.LBB5_352:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$0, 136(%rsp)
	movb	$0, 152(%rsp)
	movq	8(%rsp), %r12                   # 8-byte Reload
	cmpb	$1, 80(%rsp)
	movq	176(%rsp), %rsi                 # 8-byte Reload
	jne	.LBB5_320
	.p2align	4
.LBB5_353:                              #   in Loop: Header=BB5_19 Depth=1
	movups	64(%rsp), %xmm0
	movups	%xmm0, -48(%rsi,%rbx)
	cmpb	$1, 104(%rsp)
	jne	.LBB5_491
# %bb.354:                              #   in Loop: Header=BB5_19 Depth=1
	leaq	88(%rsp), %rax
	movups	(%rax), %xmm0
	movups	%xmm0, -32(%rsi,%rbx)
	cmpb	$1, 128(%rsp)
	jne	.LBB5_492
# %bb.355:                              #   in Loop: Header=BB5_19 Depth=1
	leaq	112(%rsp), %rax
	movups	(%rax), %xmm0
	movups	%xmm0, -16(%rsi,%rbx)
	cmpb	$1, 152(%rsp)
	jne	.LBB5_493
# %bb.356:                              #   in Loop: Header=BB5_19 Depth=1
	leaq	136(%rsp), %rax
	movups	(%rax), %xmm0
	movups	%xmm0, (%rsi,%rbx)
	addq	$64, %rbx
	movq	192(%rsp), %rax                 # 8-byte Reload
	addq	%r13, %rax
	addq	$-4, %rax
	addq	$-4, %r13
	cmpq	$3, %rax
	ja	.LBB5_19
	jmp	.LBB5_357
.LBB5_347:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	je	.LBB5_348
# %bb.385:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$1, %cl
	movabsq	$500000000000, %rdx             # imm = 0x746A528800
	cmpq	%rdx, %rdi
	ja	.LBB5_387
# %bb.386:                              #   in Loop: Header=BB5_19 Depth=1
	sete	%dl
	movl	%eax, %ecx
	andb	%dl, %cl
.LBB5_387:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%cl, %cl
	movq	8(%rsp), %r12                   # 8-byte Reload
	je	.LBB5_389
# %bb.388:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %r11
	xorq	%rsi, %r10
	orq	%r11, %r10
	je	.LBB5_327
.LBB5_389:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%cl, %ecx
	addq	%rcx, %rax
	adcq	$0, %rsi
	jmp	.LBB5_390
.LBB5_360:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
	jmp	.LBB5_68
.LBB5_361:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
	jmp	.LBB5_118
.LBB5_362:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
	jmp	.LBB5_166
.LBB5_363:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
	jmp	.LBB5_214
.LBB5_364:                              #   in Loop: Header=BB5_19 Depth=1
	testq	%rdi, %rdi
	movq	8(%rsp), %r12                   # 8-byte Reload
	je	.LBB5_392
# %bb.365:                              #   in Loop: Header=BB5_19 Depth=1
	movb	$1, %cl
	movabsq	$500000000000, %rdx             # imm = 0x746A528800
	cmpq	%rdx, %rdi
	ja	.LBB5_367
# %bb.366:                              #   in Loop: Header=BB5_19 Depth=1
	sete	%dl
	movl	%eax, %ecx
	andb	%dl, %cl
.LBB5_367:                              #   in Loop: Header=BB5_19 Depth=1
	testb	%cl, %cl
	je	.LBB5_369
.LBB5_368:                              #   in Loop: Header=BB5_19 Depth=1
	xorq	%rax, %r11
	xorq	%rsi, %r10
	orq	%r11, %r10
	je	.LBB5_36
.LBB5_369:                              #   in Loop: Header=BB5_19 Depth=1
	movzbl	%cl, %ecx
	addq	%rcx, %rax
	adcq	$0, %rsi
	jmp	.LBB5_392
.LBB5_277:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
.LBB5_376:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	negq	%rcx
	movq	%r14, %rdx
	sbbq	%rsi, %rdx
	testq	%r9, %r9
	cmovnsq	%rsi, %rdx
	cmovnsq	%rax, %rcx
	movq	%rcx, 88(%rsp)
	movq	%rdx, 96(%rsp)
	movb	$1, 104(%rsp)
	jmp	.LBB5_282
.LBB5_312:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
.LBB5_383:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	negq	%rcx
	movq	%r14, %rdx
	sbbq	%rsi, %rdx
	testq	%r9, %r9
	cmovnsq	%rsi, %rdx
	cmovnsq	%rax, %rcx
	movq	%rcx, 112(%rsp)
	movq	%rdx, 120(%rsp)
	movb	$1, 128(%rsp)
	jmp	.LBB5_317
.LBB5_348:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
.LBB5_390:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	negq	%rcx
	movq	%r14, %rdx
	sbbq	%rsi, %rdx
	testq	%r9, %r9
	cmovnsq	%rsi, %rdx
	cmovnsq	%rax, %rcx
	movq	%rcx, 136(%rsp)
	movq	%rdx, 144(%rsp)
	movb	$1, 152(%rsp)
	cmpb	$1, 80(%rsp)
	movq	176(%rsp), %rsi                 # 8-byte Reload
	je	.LBB5_353
	jmp	.LBB5_320
.LBB5_391:                              #   in Loop: Header=BB5_19 Depth=1
	movq	8(%rsp), %r12                   # 8-byte Reload
.LBB5_392:                              #   in Loop: Header=BB5_19 Depth=1
	movq	%rax, %rcx
	negq	%rcx
	movq	%r14, %rdx
	sbbq	%rsi, %rdx
	testq	%r9, %r9
	cmovnsq	%rsi, %rdx
	cmovnsq	%rax, %rcx
	movq	%rcx, 64(%rsp)
	movq	%rdx, 72(%rsp)
	movb	$1, 80(%rsp)
	jmp	.LBB5_247
.LBB5_487:
	xorl	%r13d, %r13d
	cmpq	192(%rsp), %r13                 # 8-byte Folded Reload
	jb	.LBB5_358
	jmp	.LBB5_488
.LBB5_357:
	negq	%r13
	cmpq	192(%rsp), %r13                 # 8-byte Folded Reload
	jae	.LBB5_488
.LBB5_358:
	movq	160(%rsp), %rdi                 # 8-byte Reload
	movq	%rdi, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	cmpq	$0, 32(%rsp)                    # 8-byte Folded Reload
	jne	.LBB5_438
# %bb.359:
	movq	%r13, %r11
	shlq	$4, %r11
	xorl	%ebx, %ebx
	jmp	.LBB5_395
	.p2align	4
.LBB5_437:                              #   in Loop: Header=BB5_395 Depth=1
	movq	176(%rsp), %r9                  # 8-byte Reload
.LBB5_394:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movq	%rbx, %rsi
	sbbq	%rcx, %rsi
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r14d
	cmoveq	%rcx, %rsi
	cmpq	%rdi, %r14
	cmoveq	%rax, %rdx
	movq	%rdx, 64(%rsp)
	movq	%rsi, 72(%rsp)
	movups	64(%rsp), %xmm0
	movups	%xmm0, (%r9,%r11)
	incq	%r13
	addq	$16, %r11
	cmpq	%r13, 192(%rsp)                 # 8-byte Folded Reload
	je	.LBB5_488
.LBB5_395:                              # =>This Inner Loop Header: Depth=1
	movq	(%r15,%r11), %rax
	movq	8(%r15,%r11), %rdx
	movq	%rdx, %r14
	shrq	$63, %r14
	movq	%rax, %rcx
	negq	%rcx
	movq	%rbx, %rdi
	sbbq	%rdx, %rdi
	testq	%rdx, %rdx
	cmovnsq	%rax, %rcx
	cmovnsq	%rdx, %rdi
	testq	%rdi, %rdi
	je	.LBB5_398
# %bb.396:                              #   in Loop: Header=BB5_395 Depth=1
	cmpq	$1, %rbp
	movq	%r13, 160(%rsp)                 # 8-byte Spill
	jne	.LBB5_400
# %bb.397:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rbx, %r13
	xorl	%r8d, %r8d
	jmp	.LBB5_407
	.p2align	4
.LBB5_398:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rcx, %rax
	mulq	56(%rsp)                        # 8-byte Folded Reload
	imulq	32(%rsp), %rcx                  # 8-byte Folded Reload
	addq	%rdx, %rcx
	cmpq	$1, %rbp
	jne	.LBB5_402
# %bb.399:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rbx, %rdx
	jmp	.LBB5_419
	.p2align	4
.LBB5_400:                              #   in Loop: Header=BB5_395 Depth=1
	cmpq	%rdi, %rbp
	jbe	.LBB5_405
# %bb.401:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rbx, %rsi
	jmp	.LBB5_406
.LBB5_402:                              #   in Loop: Header=BB5_395 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_415
# %bb.403:                              #   in Loop: Header=BB5_395 Depth=1
	cmpq	%rcx, %rbp
	jbe	.LBB5_417
# %bb.404:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rbx, %rsi
	jmp	.LBB5_418
.LBB5_405:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rdi, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rsi
	movq	%rbp, %rax
	imulq	%rdx, %rax
	subq	%rax, %rdi
	cmpq	%rbp, %rdi
	sbbq	$-1, %rsi
	cmpq	%rbp, %rdi
	movq	%rbx, %rax
	cmovaeq	%rbp, %rax
	subq	%rax, %rdi
.LBB5_406:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rbx, %r13
	movq	%rcx, %rax
	movq	%rdi, %rdx
	#APP
	divq	%rbp
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
	movq	%rax, %rcx
.LBB5_407:                              #   in Loop: Header=BB5_395 Depth=1
	xorl	%r9d, %r9d
	movq	24(%rsp), %rax                  # 8-byte Reload
	cmpq	%rax, %r14
	movl	$0, %r10d
	movq	$-1, %rdx
	cmoveq	%rdx, %r10
	cmpl	%eax, %r14d
	movabsq	$-9223372036854775808, %r12     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rax      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rax, %r12
	movq	56(%rsp), %r15                  # 8-byte Reload
	movq	%r15, %rax
	mulq	%rcx
	movq	%rax, %rsi
	movq	32(%rsp), %rbx                  # 8-byte Reload
	imulq	%rbx, %rcx
	addq	%rdx, %rcx
	imulq	%rdi, %rbx
	movq	%r15, %rax
	mulq	%rdi
	movq	%rax, %rdi
	addq	%rcx, %rdi
	adcq	%rbx, %rdx
	jne	.LBB5_489
# %bb.408:                              #   in Loop: Header=BB5_395 Depth=1
	cmpq	%rsi, %r10
	movq	%r12, %rax
	sbbq	%rdi, %rax
	jb	.LBB5_490
# %bb.409:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%r8, %rax
	movq	%r13, %rbx
	orq	%r13, %rax
	je	.LBB5_393
# %bb.410:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%r8, %rax
	movq	56(%rsp), %rcx                  # 8-byte Reload
	mulq	%rcx
	imulq	32(%rsp), %r8                   # 8-byte Folded Reload
	addq	%rdx, %r8
	movq	%rbx, %r9
	imulq	%rcx, %r9
	addq	%r8, %r9
	cmpq	$1, %rbp
	movq	184(%rsp), %r15                 # 8-byte Reload
	movq	160(%rsp), %r13                 # 8-byte Reload
	jne	.LBB5_412
# %bb.411:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rbx, %rdx
	movq	%r9, %rcx
	jmp	.LBB5_428
	.p2align	4
.LBB5_393:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rsi, %rax
	movq	%rdi, %rcx
	movq	176(%rsp), %r9                  # 8-byte Reload
	movq	184(%rsp), %r15                 # 8-byte Reload
	movq	160(%rsp), %r13                 # 8-byte Reload
	jmp	.LBB5_394
.LBB5_412:                              #   in Loop: Header=BB5_395 Depth=1
	testq	%r9, %r9
	je	.LBB5_416
# %bb.413:                              #   in Loop: Header=BB5_395 Depth=1
	cmpq	%r9, %rbp
	jbe	.LBB5_426
# %bb.414:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rbx, %rcx
	jmp	.LBB5_427
.LBB5_415:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rax, %rcx
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbp, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rax
	cmpq	%rbp, %rcx
	movq	%rbx, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rcx
	movq	%rcx, %rdx
	movq	%rbx, %rcx
	jmp	.LBB5_419
.LBB5_416:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rax, %rcx
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%rbp, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rax
	cmpq	%rbp, %rcx
	movq	%rbx, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rcx
	movq	%rcx, %rdx
	movq	%rbx, %rcx
	jmp	.LBB5_428
.LBB5_417:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rax, %rsi
	movq	%rcx, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%rbp, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rcx
	cmpq	%rbp, %rcx
	sbbq	$-1, %rsi
	cmpq	%rbp, %rcx
	movq	%rbx, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %rcx
.LBB5_418:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rcx, %rdx
	#APP
	divq	%rbp
	#NO_APP
	movq	%rsi, %rcx
.LBB5_419:                              #   in Loop: Header=BB5_395 Depth=1
	movq	24(%rsp), %rdi                  # 8-byte Reload
	cmpl	%edi, %r14d
	movabsq	$-9223372036854775808, %rsi     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %r8       # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%r8, %rsi
	xorl	%r9d, %r9d
	cmpq	%rdi, %r14
	movl	$0, %edi
	movq	$-1, %r8
	cmoveq	%r8, %rdi
	cmpq	%rax, %rdi
	movq	%rsi, %r8
	sbbq	%rcx, %r8
	jb	.LBB5_461
# %bb.420:                              #   in Loop: Header=BB5_395 Depth=1
	testq	%rdx, %rdx
	je	.LBB5_437
# %bb.421:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rdx, %r9
	shrq	$63, %r9
	addq	%rdx, %rdx
	movb	$1, %r8b
	cmpq	%rdx, %rbp
	movq	%rbx, %r10
	sbbq	%r9, %r10
	jb	.LBB5_423
# %bb.422:                              #   in Loop: Header=BB5_395 Depth=1
	xorq	%rbp, %rdx
	orq	%r9, %rdx
	sete	%dl
	movl	%eax, %r8d
	andb	%dl, %r8b
.LBB5_423:                              #   in Loop: Header=BB5_395 Depth=1
	xorq	%rax, %rdi
	xorq	%rcx, %rsi
	orq	%rdi, %rsi
	movq	176(%rsp), %r9                  # 8-byte Reload
	jne	.LBB5_425
# %bb.424:                              #   in Loop: Header=BB5_395 Depth=1
	testb	%r8b, %r8b
	jne	.LBB5_485
.LBB5_425:                              #   in Loop: Header=BB5_395 Depth=1
	movzbl	%r8b, %edx
	jmp	.LBB5_436
.LBB5_426:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rax, %rcx
	movq	%r9, %rax
	mulq	16(%rsp)                        # 8-byte Folded Reload
	movq	%rcx, %rax
	movq	%rdx, %rcx
	movq	%rbp, %rdx
	imulq	%rcx, %rdx
	subq	%rdx, %r9
	cmpq	%rbp, %r9
	sbbq	$-1, %rcx
	cmpq	%rbp, %r9
	movq	%rbx, %rdx
	cmovaeq	%rbp, %rdx
	subq	%rdx, %r9
.LBB5_427:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%r9, %rdx
	#APP
	divq	%rbp
	#NO_APP
.LBB5_428:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%r10, %r8
	subq	%rsi, %r8
	movq	%r12, %r9
	sbbq	%rdi, %r9
	cmpq	%rax, %r8
	sbbq	%rcx, %r9
	jb	.LBB5_485
# %bb.429:                              #   in Loop: Header=BB5_395 Depth=1
	addq	%rsi, %rax
	adcq	%rdi, %rcx
	cmpq	%rax, %r10
	movq	%r12, %rsi
	sbbq	%rcx, %rsi
	jb	.LBB5_485
# %bb.430:                              #   in Loop: Header=BB5_395 Depth=1
	testq	%rdx, %rdx
	je	.LBB5_437
# %bb.431:                              #   in Loop: Header=BB5_395 Depth=1
	movq	%rdx, %rdi
	shrq	$63, %rdi
	addq	%rdx, %rdx
	movb	$1, %sil
	cmpq	%rdx, %rbp
	movq	%rbx, %r8
	sbbq	%rdi, %r8
	movq	176(%rsp), %r9                  # 8-byte Reload
	jb	.LBB5_433
# %bb.432:                              #   in Loop: Header=BB5_395 Depth=1
	xorq	%rbp, %rdx
	orq	%rdi, %rdx
	sete	%dl
	movl	%eax, %esi
	andb	%dl, %sil
.LBB5_433:                              #   in Loop: Header=BB5_395 Depth=1
	testb	%sil, %sil
	je	.LBB5_435
# %bb.434:                              #   in Loop: Header=BB5_395 Depth=1
	xorq	%rax, %r10
	xorq	%rcx, %r12
	orq	%r10, %r12
	je	.LBB5_485
.LBB5_435:                              #   in Loop: Header=BB5_395 Depth=1
	movzbl	%sil, %edx
.LBB5_436:                              #   in Loop: Header=BB5_395 Depth=1
	addq	%rdx, %rax
	adcq	$0, %rcx
	jmp	.LBB5_394
.LBB5_438:
	testq	%rdi, %rdi
	je	.LBB5_462
# %bb.439:
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, 32(%rsp)                  # 8-byte Spill
	movabsq	$1000000000000, %rcx            # imm = 0xE8D4A51000
	imulq	%rcx, %rax
	subq	%rax, %rdi
	movq	%rdi, 160(%rsp)                 # 8-byte Spill
	movq	%r13, %r14
	shlq	$4, %r14
	jmp	.LBB5_442
	.p2align	4
.LBB5_440:                              #   in Loop: Header=BB5_442 Depth=1
	leaq	64(%rsp), %rdi
	movq	%rcx, %rdx
	movq	200(%rsp), %rcx                 # 8-byte Reload
	movq	%r12, %r8
	movl	$3, %r9d
	callq	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	testb	$1, 80(%rsp)
	movq	176(%rsp), %rax                 # 8-byte Reload
	je	.LBB5_460
.LBB5_441:                              #   in Loop: Header=BB5_442 Depth=1
	movups	64(%rsp), %xmm0
	movups	%xmm0, (%rax,%r14)
	incq	%r13
	addq	$16, %r14
	cmpq	%r13, 192(%rsp)                 # 8-byte Folded Reload
	je	.LBB5_488
.LBB5_442:                              # =>This Inner Loop Header: Depth=1
	movq	(%r15,%r14), %rsi
	movq	8(%r15,%r14), %rcx
	movq	%rsi, %rdi
	negq	%rdi
	movl	$0, %r9d
	sbbq	%rcx, %r9
	testq	%rcx, %rcx
	cmovnsq	%rsi, %rdi
	cmovnsq	%rcx, %r9
	testq	%r9, %r9
	jne	.LBB5_440
# %bb.443:                              #   in Loop: Header=BB5_442 Depth=1
	movq	48(%rsp), %rax                  # 8-byte Reload
	movq	160(%rsp), %rdx                 # 8-byte Reload
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	#APP
	divq	%rsi
	#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %r11
	xorq	%r12, %rcx
	testq	%rcx, %rcx
	movabsq	$9223372036854775807, %r12      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r12
	movq	$-1, %rbp
	movl	$0, %eax
	cmovsq	%rax, %rbp
	movq	%r9, %r8
	imulq	%rsi, %r8
	movq	%rdi, %rax
	mulq	%rsi
	movq	%rax, %rsi
	movq	%rdx, %r10
	addq	%r8, %r10
	movq	%r9, %rbx
	movq	32(%rsp), %rdx                  # 8-byte Reload
	imulq	%rdx, %rbx
	movq	%rdi, %rax
	mulq	%rdx
	movq	%rax, %r8
	addq	%r10, %r8
	adcq	%rbx, %rdx
	jne	.LBB5_459
# %bb.444:                              #   in Loop: Header=BB5_442 Depth=1
	cmpq	%rsi, %rbp
	movq	%r12, %rax
	sbbq	%r8, %rax
	jb	.LBB5_459
# %bb.445:                              #   in Loop: Header=BB5_442 Depth=1
	testq	%r11, %r11
	je	.LBB5_448
# %bb.446:                              #   in Loop: Header=BB5_442 Depth=1
	movq	%rdi, %rax
	mulq	%r11
	movq	%rax, %r10
	imulq	%r11, %r9
	addq	%rdx, %r9
	je	.LBB5_449
# %bb.447:                              #   in Loop: Header=BB5_442 Depth=1
	movq	%r9, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movabsq	$1000000000000, %r11            # imm = 0xE8D4A51000
	imulq	%r11, %rax
	subq	%rax, %r9
	movq	%r10, %rax
	movq	%r9, %rdx
	#APP
	divq	%r11
	#NO_APP
	movq	%rdx, %r10
	jmp	.LBB5_450
.LBB5_448:                              #   in Loop: Header=BB5_442 Depth=1
	movq	%rsi, %rax
	movq	%r8, %rdi
	jmp	.LBB5_458
.LBB5_449:                              #   in Loop: Header=BB5_442 Depth=1
	movq	%r10, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rdx
	movabsq	$1000000000000, %rdi            # imm = 0xE8D4A51000
	imulq	%rdi, %rdx
	subq	%rdx, %r10
	xorl	%edi, %edi
.LBB5_450:                              #   in Loop: Header=BB5_442 Depth=1
	movq	%rbp, %rdx
	subq	%rsi, %rdx
	movq	%r12, %r9
	sbbq	%r8, %r9
	cmpq	%rax, %rdx
	sbbq	%rdi, %r9
	jb	.LBB5_459
# %bb.451:                              #   in Loop: Header=BB5_442 Depth=1
	addq	%rsi, %rax
	adcq	%r8, %rdi
	cmpq	%rax, %rbp
	movq	%r12, %rdx
	sbbq	%rdi, %rdx
	jb	.LBB5_459
# %bb.452:                              #   in Loop: Header=BB5_442 Depth=1
	testq	%r10, %r10
	je	.LBB5_458
# %bb.453:                              #   in Loop: Header=BB5_442 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rsi             # imm = 0x746A528800
	cmpq	%rsi, %r10
	ja	.LBB5_455
# %bb.454:                              #   in Loop: Header=BB5_442 Depth=1
	sete	%sil
	movl	%eax, %edx
	andb	%sil, %dl
.LBB5_455:                              #   in Loop: Header=BB5_442 Depth=1
	testb	%dl, %dl
	je	.LBB5_457
# %bb.456:                              #   in Loop: Header=BB5_442 Depth=1
	xorq	%rax, %rbp
	xorq	%rdi, %r12
	orq	%rbp, %r12
	je	.LBB5_459
.LBB5_457:                              #   in Loop: Header=BB5_442 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rdi
.LBB5_458:                              #   in Loop: Header=BB5_442 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %esi
	sbbq	%rdi, %rsi
	testq	%rcx, %rcx
	cmovnsq	%rdi, %rsi
	cmovnsq	%rax, %rdx
	movq	%rdx, 64(%rsp)
	movq	%rsi, 72(%rsp)
	movq	176(%rsp), %rax                 # 8-byte Reload
	movq	8(%rsp), %r12                   # 8-byte Reload
	jmp	.LBB5_441
.LBB5_459:
	movb	$0, 64(%rsp)
.LBB5_460:
	movzbl	64(%rsp), %r9d
	jmp	.LBB5_461
.LBB5_462:
	movq	%r13, %r10
	shlq	$4, %r10
	movq	48(%rsp), %rbp                  # 8-byte Reload
	jmp	.LBB5_465
	.p2align	4
.LBB5_463:                              #   in Loop: Header=BB5_465 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rcx
.LBB5_464:                              #   in Loop: Header=BB5_465 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %edi
	sbbq	%rcx, %rdi
	testq	%r11, %r11
	cmovnsq	%rcx, %rdi
	cmovnsq	%rax, %rdx
	movq	%rdx, 64(%rsp)
	movq	%rdi, 72(%rsp)
	movups	64(%rsp), %xmm0
	movups	%xmm0, (%rsi,%r10)
	incq	%r13
	addq	$16, %r10
	cmpq	%r13, 192(%rsp)                 # 8-byte Folded Reload
	je	.LBB5_488
.LBB5_465:                              # =>This Inner Loop Header: Depth=1
	movq	(%r15,%r10), %rax
	movq	8(%r15,%r10), %r11
	movq	%rax, %rcx
	negq	%rcx
	movl	$0, %r8d
	sbbq	%r11, %r8
	testq	%r11, %r11
	cmovnsq	%rax, %rcx
	cmovnsq	%r11, %r8
	xorq	%r12, %r11
	testq	%r8, %r8
	je	.LBB5_471
# %bb.466:                              #   in Loop: Header=BB5_465 Depth=1
	movq	%r8, %rax
	movabsq	$2535301200456458803, %rdx      # imm = 0x232F33025BD42233
	mulq	%rdx
	movq	%rdx, %rdi
	shrq	$37, %rdi
	movq	%rdi, %rax
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rax
	subq	%rax, %r8
	movq	%rcx, %rax
	movq	%r8, %rdx
	#APP
	divq	%rsi
	#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %rcx
	xorl	%r9d, %r9d
	testq	%r11, %r11
	movabsq	$9223372036854775807, %rbx      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %rbx
	movq	$-1, %r14
	cmovsq	%r9, %r14
	movq	%rbp, %rax
	mulq	%rsi
	movq	%rdx, %r8
	movq	%rax, %rsi
	movq	%rbp, %rax
	mulq	%rdi
	movq	%rax, %rdi
	addq	%r8, %rdi
	adcq	$0, %rdx
	jne	.LBB5_461
# %bb.467:                              #   in Loop: Header=BB5_465 Depth=1
	cmpq	%rsi, %r14
	movq	%rbx, %rax
	sbbq	%rdi, %rax
	jb	.LBB5_485
# %bb.468:                              #   in Loop: Header=BB5_465 Depth=1
	testq	%rcx, %rcx
	je	.LBB5_473
# %bb.469:                              #   in Loop: Header=BB5_465 Depth=1
	movq	%rbp, %rax
	mulq	%rcx
	movq	%rax, %r8
	testq	%rdx, %rdx
	je	.LBB5_477
# %bb.470:                              #   in Loop: Header=BB5_465 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r9
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r12            # imm = 0xE8D4A51000
	imulq	%r12, %rax
	subq	%rax, %r9
	movq	%r8, %rax
	movq	%r9, %rdx
	#APP
	divq	%r12
	#NO_APP
	movq	%rdx, %r8
	jmp	.LBB5_478
	.p2align	4
.LBB5_471:                              #   in Loop: Header=BB5_465 Depth=1
	movq	%rcx, %rax
	mulq	48(%rsp)                        # 8-byte Folded Reload
	movq	%rax, %r8
	testq	%rdx, %rdx
	je	.LBB5_474
# %bb.472:                              #   in Loop: Header=BB5_465 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %rdi
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r9             # imm = 0xE8D4A51000
	imulq	%r9, %rax
	subq	%rax, %rdi
	movq	%r8, %rax
	movq	%rdi, %rdx
	#APP
	divq	%r9
	#NO_APP
	movq	%rdx, %r8
	testq	%r8, %r8
	jne	.LBB5_475
	jmp	.LBB5_464
	.p2align	4
.LBB5_473:                              #   in Loop: Header=BB5_465 Depth=1
	movq	%rsi, %rax
	movq	%rdi, %rcx
	movq	176(%rsp), %rsi                 # 8-byte Reload
	jmp	.LBB5_464
.LBB5_474:                              #   in Loop: Header=BB5_465 Depth=1
	movq	%r8, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %r8
	xorl	%ecx, %ecx
	testq	%r8, %r8
	je	.LBB5_464
.LBB5_475:                              #   in Loop: Header=BB5_465 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rdi             # imm = 0x746A528800
	cmpq	%rdi, %r8
	ja	.LBB5_463
# %bb.476:                              #   in Loop: Header=BB5_465 Depth=1
	sete	%dil
	movl	%eax, %edx
	andb	%dil, %dl
	jmp	.LBB5_463
.LBB5_477:                              #   in Loop: Header=BB5_465 Depth=1
	movq	%r8, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %r8
	xorl	%ecx, %ecx
.LBB5_478:                              #   in Loop: Header=BB5_465 Depth=1
	movq	%r14, %rdx
	subq	%rsi, %rdx
	movq	%rbx, %r9
	sbbq	%rdi, %r9
	cmpq	%rax, %rdx
	sbbq	%rcx, %r9
	jb	.LBB5_485
# %bb.479:                              #   in Loop: Header=BB5_465 Depth=1
	addq	%rsi, %rax
	adcq	%rdi, %rcx
	cmpq	%rax, %r14
	movq	%rbx, %rdx
	sbbq	%rcx, %rdx
	jb	.LBB5_485
# %bb.480:                              #   in Loop: Header=BB5_465 Depth=1
	testq	%r8, %r8
	je	.LBB5_486
# %bb.481:                              #   in Loop: Header=BB5_465 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rsi             # imm = 0x746A528800
	cmpq	%rsi, %r8
	movq	176(%rsp), %rsi                 # 8-byte Reload
	movq	8(%rsp), %r12                   # 8-byte Reload
	ja	.LBB5_483
# %bb.482:                              #   in Loop: Header=BB5_465 Depth=1
	sete	%dil
	movl	%eax, %edx
	andb	%dil, %dl
.LBB5_483:                              #   in Loop: Header=BB5_465 Depth=1
	testb	%dl, %dl
	je	.LBB5_463
# %bb.484:                              #   in Loop: Header=BB5_465 Depth=1
	xorq	%rax, %r14
	xorq	%rcx, %rbx
	orq	%r14, %rbx
	jne	.LBB5_463
	jmp	.LBB5_485
.LBB5_486:                              #   in Loop: Header=BB5_465 Depth=1
	movq	176(%rsp), %rsi                 # 8-byte Reload
	movq	8(%rsp), %r12                   # 8-byte Reload
	jmp	.LBB5_464
.LBB5_488:
	movq	224(%rsp), %rax                 # 8-byte Reload
	movb	$1, 16(%rax)
	jmp	.LBB5_496
.LBB5_485:
	xorl	%r9d, %r9d
	jmp	.LBB5_461
.LBB5_489:
	movq	160(%rsp), %r13                 # 8-byte Reload
	jmp	.LBB5_461
.LBB5_490:
	xorl	%r9d, %r9d
	movq	160(%rsp), %r13                 # 8-byte Reload
.LBB5_461:
	movq	224(%rsp), %rax                 # 8-byte Reload
	movq	%r13, (%rax)
	movb	%r9b, 8(%rax)
.LBB5_495:
	movb	$0, 16(%rax)
.LBB5_496:
	addq	$232, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.LBB5_320:
	.cfi_def_cfa_offset 288
	leaq	64(%rsp), %rax
	jmp	.LBB5_494
.LBB5_491:
	movl	$1, %r14d
	leaq	88(%rsp), %rax
	jmp	.LBB5_494
.LBB5_492:
	movl	$2, %r14d
	leaq	112(%rsp), %rax
	jmp	.LBB5_494
.LBB5_493:
	movl	$3, %r14d
	leaq	136(%rsp), %rax
.LBB5_494:
	subq	%r13, %r14
	movzbl	(%rax), %ecx
	movq	224(%rsp), %rax                 # 8-byte Reload
	movq	%r14, (%rax)
	movb	%cl, 8(%rax)
	jmp	.LBB5_495
.Lfunc_end5:
	.size	_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_, .Lfunc_end5-_ZN23fixedwide_batch_codegen10blocked_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_mS2_
	.cfi_endproc
                                        # -- End function
	.globl	_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m # -- Begin function _ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m,@function
_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m: # @_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$56, %rsp
	.cfi_def_cfa_offset 112
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%rsi, 48(%rsp)                  # 8-byte Spill
	movq	%rdi, 16(%rsp)                  # 8-byte Spill
	testq	%r8, %r8
	je	.LBB6_44
# %bb.1:
	movq	%r8, %r14
	movq	%rcx, %r15
	movq	%rdx, %r12
	xorl	%ebp, %ebp
	xorl	%ebx, %ebx
	movq	%r8, 8(%rsp)                    # 8-byte Spill
	jmp	.LBB6_2
	.p2align	4
.LBB6_42:                               #   in Loop: Header=BB6_2 Depth=1
	leaq	24(%rsp), %rdi
	movl	$3, %r9d
	callq	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	testb	$1, 40(%rsp)
	je	.LBB6_41
.LBB6_43:                               #   in Loop: Header=BB6_2 Depth=1
	movups	24(%rsp), %xmm0
	movq	48(%rsp), %rax                  # 8-byte Reload
	movups	%xmm0, (%rax,%rbp)
	incq	%rbx
	addq	$16, %rbp
	cmpq	%rbx, %r14
	je	.LBB6_44
.LBB6_2:                                # =>This Inner Loop Header: Depth=1
	movq	(%r12,%rbp), %rsi
	movq	8(%r12,%rbp), %rdx
	movq	(%r15,%rbp), %rcx
	movq	%rsi, %r10
	negq	%r10
	movl	$0, %edi
	sbbq	%rdx, %rdi
	movq	8(%r15,%rbp), %r8
	testq	%rdx, %rdx
	cmovnsq	%rsi, %r10
	cmovnsq	%rdx, %rdi
	movq	%rcx, %r9
	negq	%r9
	movl	$0, %r11d
	sbbq	%r8, %r11
	testq	%r8, %r8
	cmovnsq	%rcx, %r9
	cmovnsq	%r8, %r11
	movq	%r8, %r13
	xorq	%rdx, %r13
	testq	%r11, %r11
	je	.LBB6_3
# %bb.28:                               #   in Loop: Header=BB6_2 Depth=1
	testq	%rdi, %rdi
	jne	.LBB6_42
# %bb.29:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%r11, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %rsi            # imm = 0xE8D4A51000
	imulq	%rsi, %rax
	subq	%rax, %r11
	movq	%r9, %rax
	movq	%r11, %rdx
	#APP
	divq	%rsi
	#NO_APP
	movq	%rax, %rsi
	movq	%rdx, %r8
	testq	%r13, %r13
	movabsq	$9223372036854775807, %r11      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r11
	movq	$-1, %r14
	movl	$0, %eax
	cmovsq	%rax, %r14
	movq	%r10, %rax
	mulq	%rsi
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%r10, %rax
	mulq	%rcx
	movq	%rax, %rdi
	addq	%r9, %rdi
	adcq	$0, %rdx
	jne	.LBB6_40
# %bb.30:                               #   in Loop: Header=BB6_2 Depth=1
	cmpq	%rsi, %r14
	movq	%r11, %rax
	sbbq	%rdi, %rax
	jb	.LBB6_40
# %bb.31:                               #   in Loop: Header=BB6_2 Depth=1
	testq	%r8, %r8
	je	.LBB6_32
# %bb.33:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%r10, %rax
	mulq	%r8
	movq	%rax, %r8
	testq	%rdx, %rdx
	je	.LBB6_34
# %bb.35:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r9
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r10            # imm = 0xE8D4A51000
	imulq	%r10, %rax
	subq	%rax, %r9
	movq	%r8, %rax
	movq	%r9, %rdx
	#APP
	divq	%r10
	#NO_APP
	movq	%rdx, %r8
	jmp	.LBB6_36
	.p2align	4
.LBB6_3:                                #   in Loop: Header=BB6_2 Depth=1
	testq	%rdi, %rdi
	je	.LBB6_4
# %bb.11:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%rdi, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rsi
	shrq	$37, %rsi
	movq	%rsi, %rax
	movabsq	$1000000000000, %rcx            # imm = 0xE8D4A51000
	imulq	%rcx, %rax
	subq	%rax, %rdi
	movq	%r10, %rax
	movq	%rdi, %rdx
	#APP
	divq	%rcx
	#NO_APP
	movq	%rax, %rdi
	movq	%rdx, %rcx
	testq	%r13, %r13
	movabsq	$9223372036854775807, %r11      # imm = 0x7FFFFFFFFFFFFFFF
	movabsq	$-9223372036854775808, %rax     # imm = 0x8000000000000000
	cmovsq	%rax, %r11
	movq	$-1, %r14
	movl	$0, %eax
	cmovsq	%rax, %r14
	movq	%r9, %rax
	mulq	%rdi
	movq	%rdx, %r10
	movq	%rax, %rdi
	movq	%r9, %rax
	mulq	%rsi
	movq	%rax, %r8
	addq	%r10, %r8
	adcq	$0, %rdx
	jne	.LBB6_40
# %bb.12:                               #   in Loop: Header=BB6_2 Depth=1
	cmpq	%rdi, %r14
	movq	%r11, %rax
	sbbq	%r8, %rax
	jb	.LBB6_40
# %bb.13:                               #   in Loop: Header=BB6_2 Depth=1
	testq	%rcx, %rcx
	je	.LBB6_14
# %bb.15:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%r9, %rax
	mulq	%rcx
	movq	%rax, %rsi
	testq	%rdx, %rdx
	je	.LBB6_16
# %bb.17:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %r9
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r10            # imm = 0xE8D4A51000
	imulq	%r10, %rax
	subq	%rax, %r9
	movq	%rsi, %rax
	movq	%r9, %rdx
	#APP
	divq	%r10
	#NO_APP
	movq	%rdx, %rsi
	jmp	.LBB6_18
.LBB6_4:                                #   in Loop: Header=BB6_2 Depth=1
	movq	%r9, %rax
	mulq	%r10
	movq	%rax, %rsi
	testq	%rdx, %rdx
	je	.LBB6_5
# %bb.6:                                #   in Loop: Header=BB6_2 Depth=1
	movq	%rdx, %rax
	movq	%rdx, %rdi
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$37, %rcx
	movq	%rcx, %rax
	movabsq	$1000000000000, %r8             # imm = 0xE8D4A51000
	imulq	%r8, %rax
	subq	%rax, %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	#APP
	divq	%r8
	#NO_APP
	movq	%rdx, %rsi
	testq	%rsi, %rsi
	jne	.LBB6_8
	jmp	.LBB6_27
.LBB6_32:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%rsi, %rax
	movq	%rdi, %rcx
	jmp	.LBB6_26
.LBB6_14:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%rdi, %rax
	movq	%r8, %rcx
	jmp	.LBB6_26
.LBB6_5:                                #   in Loop: Header=BB6_2 Depth=1
	movq	%rsi, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %rsi
	xorl	%ecx, %ecx
	testq	%rsi, %rsi
	je	.LBB6_27
.LBB6_8:                                #   in Loop: Header=BB6_2 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rdi             # imm = 0x746A528800
	cmpq	%rdi, %rsi
	ja	.LBB6_10
# %bb.9:                                #   in Loop: Header=BB6_2 Depth=1
	sete	%sil
	movl	%eax, %edx
	andb	%sil, %dl
.LBB6_10:                               #   in Loop: Header=BB6_2 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rcx
	jmp	.LBB6_27
.LBB6_34:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%r8, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %r8
	xorl	%ecx, %ecx
.LBB6_36:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%r14, %rdx
	subq	%rsi, %rdx
	movq	%r11, %r9
	sbbq	%rdi, %r9
	cmpq	%rax, %rdx
	sbbq	%rcx, %r9
	jb	.LBB6_40
# %bb.37:                               #   in Loop: Header=BB6_2 Depth=1
	addq	%rsi, %rax
	adcq	%rdi, %rcx
	cmpq	%rax, %r14
	movq	%r11, %rdx
	sbbq	%rcx, %rdx
	jb	.LBB6_40
# %bb.38:                               #   in Loop: Header=BB6_2 Depth=1
	testq	%r8, %r8
	je	.LBB6_26
# %bb.39:                               #   in Loop: Header=BB6_2 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rsi             # imm = 0x746A528800
	cmpq	%rsi, %r8
	ja	.LBB6_23
.LBB6_22:                               #   in Loop: Header=BB6_2 Depth=1
	sete	%sil
	movl	%eax, %edx
	andb	%sil, %dl
.LBB6_23:                               #   in Loop: Header=BB6_2 Depth=1
	testb	%dl, %dl
	je	.LBB6_25
# %bb.24:                               #   in Loop: Header=BB6_2 Depth=1
	xorq	%rax, %r14
	xorq	%rcx, %r11
	orq	%r14, %r11
	je	.LBB6_40
.LBB6_25:                               #   in Loop: Header=BB6_2 Depth=1
	movzbl	%dl, %edx
	addq	%rdx, %rax
	adcq	$0, %rcx
	jmp	.LBB6_26
.LBB6_16:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%rsi, %rax
	movabsq	$2535301200456458803, %rcx      # imm = 0x232F33025BD42233
	mulq	%rcx
	movq	%rdx, %rax
	shrq	$37, %rax
	movq	%rax, %rcx
	movabsq	$1000000000000, %rdx            # imm = 0xE8D4A51000
	imulq	%rdx, %rcx
	subq	%rcx, %rsi
	xorl	%ecx, %ecx
.LBB6_18:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%r14, %rdx
	subq	%rdi, %rdx
	movq	%r11, %r9
	sbbq	%r8, %r9
	cmpq	%rax, %rdx
	sbbq	%rcx, %r9
	jb	.LBB6_40
# %bb.19:                               #   in Loop: Header=BB6_2 Depth=1
	addq	%rdi, %rax
	adcq	%r8, %rcx
	cmpq	%rax, %r14
	movq	%r11, %rdx
	sbbq	%rcx, %rdx
	jb	.LBB6_40
# %bb.20:                               #   in Loop: Header=BB6_2 Depth=1
	testq	%rsi, %rsi
	je	.LBB6_26
# %bb.21:                               #   in Loop: Header=BB6_2 Depth=1
	movb	$1, %dl
	movabsq	$500000000000, %rdi             # imm = 0x746A528800
	cmpq	%rdi, %rsi
	jbe	.LBB6_22
	jmp	.LBB6_23
	.p2align	4
.LBB6_26:                               #   in Loop: Header=BB6_2 Depth=1
	movq	8(%rsp), %r14                   # 8-byte Reload
.LBB6_27:                               #   in Loop: Header=BB6_2 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %esi
	sbbq	%rcx, %rsi
	testq	%r13, %r13
	cmovnsq	%rcx, %rsi
	cmovnsq	%rax, %rdx
	movq	%rdx, 24(%rsp)
	movq	%rsi, 32(%rsp)
	jmp	.LBB6_43
.LBB6_44:
	movq	16(%rsp), %rax                  # 8-byte Reload
	movb	$1, 16(%rax)
	jmp	.LBB6_45
.LBB6_40:
	movb	$0, 24(%rsp)
.LBB6_41:
	movzbl	24(%rsp), %ecx
	movq	16(%rsp), %rax                  # 8-byte Reload
	movq	%rbx, (%rax)
	movb	%cl, 8(%rax)
	movb	$0, 16(%rax)
.LBB6_45:
	addq	$56, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end6:
	.size	_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m, .Lfunc_end6-_ZN23fixedwide_batch_codegen10varying_fxEPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m
	.cfi_endproc
                                        # -- End function
	.globl	_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE # -- Begin function _ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE,@function
_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE: # @_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$56, %rsp
	.cfi_def_cfa_offset 112
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%r8, 48(%rsp)                   # 8-byte Spill
	movq	%rdi, 8(%rsp)                   # 8-byte Spill
	movq	%r9, 40(%rsp)                   # 8-byte Spill
	testq	%r9, %r9
	je	.LBB7_13
# %bb.1:
	movq	%rcx, %r12
	movq	%rdx, %r13
	movq	%rsi, %rbp
	movq	112(%rsp), %rax
	movq	%rax, %r15
	negq	%r15
	cmovsq	%rax, %r15
	movl	$8, %r14d
	xorl	%ebx, %ebx
	.p2align	4
.LBB7_2:                                # =>This Inner Loop Header: Depth=1
	movq	-8(%rbp,%r14), %rsi
	movq	(%rbp,%r14), %rcx
	movq	%rsi, %rdi
	negq	%rdi
	movl	$0, %eax
	sbbq	%rcx, %rax
	testq	%rcx, %rcx
	cmovnsq	%rsi, %rdi
	cmovnsq	%rcx, %rax
	mulq	%r15
	testq	%rdx, %rdx
	jne	.LBB7_5
# %bb.3:                                #   in Loop: Header=BB7_2 Depth=1
	movq	%rdx, %r8
	movq	%rax, %r9
	movq	%rdi, %rax
	mulq	%r15
	addq	%r9, %rdx
	adcq	$0, %r8
	jne	.LBB7_5
# %bb.4:                                #   in Loop: Header=BB7_2 Depth=1
	movq	%rcx, %rdi
	xorq	112(%rsp), %rdi
	movq	%rdi, %r8
	shrq	$63, %r8
	addq	$-1, %r8
	movabsq	$9223372036854775807, %r9       # imm = 0x7FFFFFFFFFFFFFFF
	adcq	$0, %r9
	cmpq	%rax, %r8
	sbbq	%rdx, %r9
	jae	.LBB7_9
	.p2align	4
.LBB7_5:                                #   in Loop: Header=BB7_2 Depth=1
	leaq	16(%rsp), %rdi
	movq	%rcx, %rdx
	movq	112(%rsp), %rcx
	movl	$5, %r8d
	callq	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	testb	$1, 32(%rsp)
	je	.LBB7_16
.LBB7_6:                                #   in Loop: Header=BB7_2 Depth=1
	movq	16(%rsp), %rax
	movq	24(%rsp), %rcx
	movq	-8(%r13,%r14), %rdx
	addq	%rax, %rdx
	movq	(%r13,%r14), %rsi
	adcq	%rcx, %rsi
	jo	.LBB7_14
# %bb.7:                                #   in Loop: Header=BB7_2 Depth=1
	movq	48(%rsp), %rdi                  # 8-byte Reload
	movl	(%rdi,%rbx,4), %edi
	shlq	$4, %rdi
	addq	(%r12,%rdi), %rax
	adcq	8(%r12,%rdi), %rcx
	jo	.LBB7_14
# %bb.8:                                #   in Loop: Header=BB7_2 Depth=1
	addq	%r12, %rdi
	movq	%rdx, -8(%r13,%r14)
	movq	%rsi, (%r13,%r14)
	movq	%rax, (%rdi)
	movq	%rcx, 8(%rdi)
	incq	%rbx
	addq	$16, %r14
	cmpq	%rbx, 40(%rsp)                  # 8-byte Folded Reload
	jne	.LBB7_2
	jmp	.LBB7_13
.LBB7_9:                                #   in Loop: Header=BB7_2 Depth=1
	movq	%rax, %rcx
	negq	%rcx
	movl	$0, %r8d
	sbbq	%rdx, %r8
	testq	%rdi, %rdi
	cmovnsq	%rdx, %r8
	testq	%rdx, %rdx
	je	.LBB7_11
# %bb.10:                               #   in Loop: Header=BB7_2 Depth=1
	movq	%rax, %rsi
	movq	%rdx, %rax
	movq	%rdx, %rdi
	movabsq	$-2601111570856684097, %rcx     # imm = 0xDBE6FECEBDEDD5BF
	mulq	%rcx
	movq	%rdx, %rcx
	shrq	$33, %rcx
	movq	%rcx, %rax
	movabsq	$10000000000, %r9               # imm = 0x2540BE400
	imulq	%r9, %rax
	subq	%rax, %rdi
	movq	%rsi, %rax
	movq	%rdi, %rdx
	#APP
	divq	%r9
	#NO_APP
	testq	%rdx, %rdx
	je	.LBB7_12
	jmp	.LBB7_15
.LBB7_11:                               #   in Loop: Header=BB7_2 Depth=1
	movq	%rax, %rcx
	movabsq	$-2601111570856684097, %rdx     # imm = 0xDBE6FECEBDEDD5BF
	mulq	%rdx
	movq	%rdx, %rax
	movq	%rcx, %rdx
	shrq	$33, %rax
	movq	%rax, %rcx
	movabsq	$10000000000, %rsi              # imm = 0x2540BE400
	imulq	%rsi, %rcx
	subq	%rcx, %rdx
	xorl	%ecx, %ecx
	testq	%rdx, %rdx
	jne	.LBB7_15
.LBB7_12:                               #   in Loop: Header=BB7_2 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %esi
	sbbq	%rcx, %rsi
	testq	%r8, %r8
	cmovnsq	%rcx, %rsi
	cmovnsq	%rax, %rdx
	movq	%rdx, 16(%rsp)
	movq	%rsi, 24(%rsp)
	jmp	.LBB7_6
.LBB7_13:
	movq	8(%rsp), %rax                   # 8-byte Reload
	movb	$1, 16(%rax)
	jmp	.LBB7_18
.LBB7_14:
	movq	8(%rsp), %rax                   # 8-byte Reload
	movq	%rbx, (%rax)
	movb	$0, 8(%rax)
	jmp	.LBB7_17
.LBB7_15:
	movb	$2, 16(%rsp)
.LBB7_16:
	movzbl	16(%rsp), %ecx
	movq	8(%rsp), %rax                   # 8-byte Reload
	movq	%rbx, (%rax)
	movb	%cl, 8(%rax)
.LBB7_17:
	movb	$0, 16(%rax)
.LBB7_18:
	addq	$56, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end7:
	.size	_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE, .Lfunc_end7-_ZN23fixedwide_batch_codegen12scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.cfi_endproc
                                        # -- End function
	.globl	_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE # -- Begin function _ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE,@function
_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE: # @_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.cfi_startproc
# %bb.0:
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$16, %rsp
	.cfi_def_cfa_offset 32
	.cfi_offset %rbx, -16
	movq	%rdi, %rbx
	movq	32(%rsp), %rax
	movq	%rax, (%rsp)
	callq	_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	movq	%rbx, %rax
	addq	$16, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end8:
	.size	_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE, .Lfunc_end8-_ZN23fixedwide_batch_codegen21prepared_scalar_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.cfi_endproc
                                        # -- End function
	.section	.text._ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE,"axG",@progbits,_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE,comdat
	.weak	_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE # -- Begin function _ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE,@function
_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE: # @_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$72, %rsp
	.cfi_def_cfa_offset 128
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%r9, 48(%rsp)                   # 8-byte Spill
	movq	%r8, 56(%rsp)                   # 8-byte Spill
	movq	%rcx, 24(%rsp)                  # 8-byte Spill
	movq	%rdx, %r12
	movq	%rsi, 64(%rsp)                  # 8-byte Spill
	movq	%rdi, %rbx
	movq	128(%rsp), %rax
	movq	%rax, %rcx
	negq	%rcx
	cmovsq	%rax, %rcx
	movabsq	$-2601111570856684097, %rdx     # imm = 0xDBE6FECEBDEDD5BF
	movq	%rcx, %rax
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$33, %rsi
	movabsq	$10000000000, %rdx              # imm = 0x2540BE400
	imulq	%rdx, %rsi
	movq	%rcx, %rax
	subq	%rsi, %rax
	jmp	.LBB9_1
	.p2align	4
.LBB9_3:                                #   in Loop: Header=BB9_1 Depth=1
	xorl	%edx, %edx
	divq	%rsi
	movq	%rsi, %rax
	testq	%rdx, %rdx
	je	.LBB9_5
.LBB9_1:                                # =>This Inner Loop Header: Depth=1
	movq	%rdx, %rsi
	movq	%rax, %rdx
	orq	%rsi, %rdx
	shrq	$32, %rdx
	jne	.LBB9_3
# %bb.2:                                #   in Loop: Header=BB9_1 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $edx killed $edx def $rdx
	movq	%rsi, %rax
	testq	%rdx, %rdx
	jne	.LBB9_1
.LBB9_5:
	movq	%rcx, %rax
	orq	%rsi, %rax
	shrq	$32, %rax
	je	.LBB9_6
# %bb.7:
	movq	%rcx, %rax
	xorl	%edx, %edx
	divq	%rsi
	jmp	.LBB9_8
.LBB9_6:
	movl	%ecx, %eax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $eax killed $eax def $rax
.LBB9_8:
	movq	%rax, 32(%rsp)                  # 8-byte Spill
	movq	$0, 8(%rsp)                     # 8-byte Folded Spill
	movabsq	$10000000000, %rax              # imm = 0x2540BE400
	xorl	%edx, %edx
	divq	%rsi
	movq	%rax, %r15
	cmpq	$1, %rax
	je	.LBB9_10
# %bb.9:
	movl	$1, %esi
	xorl	%edi, %edi
	movq	%r15, %rdx
	xorl	%ecx, %ecx
	callq	__udivti3@PLT
	movq	%rax, 8(%rsp)                   # 8-byte Spill
.LBB9_10:
	movb	$1, %al
	cmpq	$0, 48(%rsp)                    # 8-byte Folded Reload
	je	.LBB9_46
# %bb.11:
	movq	%rbx, 40(%rsp)                  # 8-byte Spill
	movl	$8, %r10d
	xorl	%r11d, %r11d
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rcx
	movq	24(%rsp), %rbx                  # 8-byte Reload
	movq	%r12, (%rsp)                    # 8-byte Spill
	.p2align	4
.LBB9_12:                               # =>This Inner Loop Header: Depth=1
	movq	64(%rsp), %rax                  # 8-byte Reload
	movq	-8(%rax,%r10), %rdx
	movq	(%rax,%r10), %rbp
	movq	%rdx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rbp, %rdi
	testq	%rbp, %rbp
	cmovnsq	%rdx, %rax
	cmovnsq	%rbp, %rdi
	movl	$0, %r8d
	cmpq	$1, %r15
	je	.LBB9_19
# %bb.13:                               #   in Loop: Header=BB9_12 Depth=1
	testq	%rdi, %rdi
	je	.LBB9_14
# %bb.15:                               #   in Loop: Header=BB9_12 Depth=1
	cmpq	%rdi, %r15
	jbe	.LBB9_17
# %bb.16:                               #   in Loop: Header=BB9_12 Depth=1
	xorl	%esi, %esi
	jmp	.LBB9_18
.LBB9_14:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%rax, %r8
	mulq	8(%rsp)                         # 8-byte Folded Reload
	movq	%r15, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%r15, %r8
	sbbq	$-1, %rdx
	cmpq	%r15, %r8
	movl	$0, %eax
	cmovaeq	%r15, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB9_19
.LBB9_17:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	8(%rsp)                         # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%r15, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	cmpq	%r15, %rdi
	sbbq	$-1, %rsi
	cmpq	%r15, %rdi
	movl	$0, %edx
	cmovaeq	%r15, %rdx
	subq	%rdx, %rdi
.LBB9_18:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%r15
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB9_19:                               #   in Loop: Header=BB9_12 Depth=1
	xorl	%r12d, %r12d
	xorq	128(%rsp), %rbp
	setns	%r12b
	negq	%r12
	movabsq	$-9223372036854775808, %r13     # imm = 0x8000000000000000
	sbbq	$0, %r13
	movq	32(%rsp), %r14                  # 8-byte Reload
	mulq	%r14
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r14
	movq	%rax, %rdi
	addq	%r9, %rdi
	adcq	$0, %rdx
	jne	.LBB9_20
# %bb.22:                               #   in Loop: Header=BB9_12 Depth=1
	cmpq	%rsi, %r12
	movq	%r13, %rax
	sbbq	%rdi, %rax
	jb	.LBB9_20
# %bb.23:                               #   in Loop: Header=BB9_12 Depth=1
	testq	%r8, %r8
	je	.LBB9_24
# %bb.25:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%r8, %rax
	mulq	32(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %r8
	cmpq	$1, %r15
	jne	.LBB9_27
# %bb.26:                               #   in Loop: Header=BB9_12 Depth=1
	xorl	%edx, %edx
	jmp	.LBB9_33
	.p2align	4
.LBB9_24:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%rsi, %rdx
	negq	%rdx
	movl	$0, %ecx
	sbbq	%rdi, %rcx
	testq	%rbp, %rbp
	cmovnsq	%rdi, %rcx
	cmovnsq	%rsi, %rdx
	movq	%rdx, %r14
	shrq	$8, %r14
	movb	$1, %al
	movq	(%rsp), %r9                     # 8-byte Reload
	jmp	.LBB9_39
	.p2align	4
.LBB9_27:                               #   in Loop: Header=BB9_12 Depth=1
	testq	%r8, %r8
	je	.LBB9_28
# %bb.29:                               #   in Loop: Header=BB9_12 Depth=1
	cmpq	%r8, %r15
	jbe	.LBB9_31
# %bb.30:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%r8, %rdx
	xorl	%r8d, %r8d
	jmp	.LBB9_32
.LBB9_28:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%rax, %r8
	mulq	8(%rsp)                         # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%r15, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %r8
	cmpq	%r15, %r8
	sbbq	$-1, %rax
	cmpq	%r15, %r8
	movl	$0, %r9d
	cmovaeq	%r15, %r9
	movq	%r8, %rdx
	subq	%r9, %rdx
	xorl	%r8d, %r8d
	jmp	.LBB9_33
.LBB9_31:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%rax, %r9
	movq	%r8, %rax
	mulq	8(%rsp)                         # 8-byte Folded Reload
	movq	%r9, %rax
	movq	%rdx, %r9
	movq	%r15, %rdx
	imulq	%r9, %rdx
	subq	%rdx, %r8
	cmpq	%r15, %r8
	sbbq	$-1, %r9
	cmpq	%r15, %r8
	movl	$0, %edx
	cmovaeq	%r15, %rdx
	subq	%rdx, %r8
	movq	%r8, %rdx
	movq	%r9, %r8
.LBB9_32:                               #   in Loop: Header=BB9_12 Depth=1
	#APP
	divq	%r15
	#NO_APP
.LBB9_33:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%r12, %r9
	subq	%rsi, %r9
	movq	%r13, %rbx
	sbbq	%rdi, %rbx
	cmpq	%rax, %r9
	sbbq	%r8, %rbx
	jb	.LBB9_34
# %bb.35:                               #   in Loop: Header=BB9_12 Depth=1
	addq	%rsi, %rax
	adcq	%rdi, %r8
	cmpq	%rax, %r12
	sbbq	%r8, %r13
	movq	24(%rsp), %rbx                  # 8-byte Reload
	jb	.LBB9_20
# %bb.36:                               #   in Loop: Header=BB9_12 Depth=1
	testq	%rdx, %rdx
	movq	(%rsp), %r9                     # 8-byte Reload
	jne	.LBB9_37
# %bb.38:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%rax, %rdx
	negq	%rdx
	movl	$0, %ecx
	sbbq	%r8, %rcx
	testq	%rbp, %rbp
	cmovnsq	%r8, %rcx
	cmovnsq	%rax, %rdx
	movq	%rdx, %r14
	shrq	$8, %r14
	movb	$1, %al
.LBB9_39:                               #   in Loop: Header=BB9_12 Depth=1
	testb	%al, %al
	je	.LBB9_42
# %bb.40:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%r14, %rsi
	shlq	$8, %rsi
	movzbl	%dl, %eax
	orq	%rsi, %rax
	movq	-8(%r9,%r10), %rdx
	addq	%rax, %rdx
	movq	(%r9,%r10), %rsi
	adcq	%rcx, %rsi
	jo	.LBB9_41
# %bb.43:                               #   in Loop: Header=BB9_12 Depth=1
	movq	56(%rsp), %rdi                  # 8-byte Reload
	movl	(%rdi,%r11,4), %edi
	shlq	$4, %rdi
	addq	(%rbx,%rdi), %rax
	movq	8(%rbx,%rdi), %r8
	adcq	%rcx, %r8
	jo	.LBB9_41
# %bb.44:                               #   in Loop: Header=BB9_12 Depth=1
	movq	%r14, 16(%rsp)                  # 8-byte Spill
	addq	%rbx, %rdi
	movq	%rdx, -8(%r9,%r10)
	movq	%rsi, (%r9,%r10)
	movq	%rax, (%rdi)
	movq	%r8, 8(%rdi)
	incq	%r11
	addq	$16, %r10
	cmpq	%r11, 48(%rsp)                  # 8-byte Folded Reload
	jne	.LBB9_12
	jmp	.LBB9_45
.LBB9_20:                               #   in Loop: Header=BB9_12 Depth=1
	xorl	%eax, %eax
	xorl	%edx, %edx
.LBB9_21:                               #   in Loop: Header=BB9_12 Depth=1
	movq	(%rsp), %r9                     # 8-byte Reload
	movq	16(%rsp), %r14                  # 8-byte Reload
	jmp	.LBB9_39
.LBB9_34:                               #   in Loop: Header=BB9_12 Depth=1
	xorl	%eax, %eax
	xorl	%edx, %edx
	movq	24(%rsp), %rbx                  # 8-byte Reload
	jmp	.LBB9_21
.LBB9_37:                               #   in Loop: Header=BB9_12 Depth=1
	movb	$2, %dl
	xorl	%eax, %eax
	movq	16(%rsp), %r14                  # 8-byte Reload
	jmp	.LBB9_39
.LBB9_45:
	movq	40(%rsp), %rbx                  # 8-byte Reload
	movb	$1, %al
.LBB9_46:
	movb	%al, 16(%rbx)
	movq	%rbx, %rax
	addq	$72, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.LBB9_41:
	.cfi_def_cfa_offset 128
	xorl	%edx, %edx
.LBB9_42:
	movq	40(%rsp), %rbx                  # 8-byte Reload
	movq	%r11, (%rbx)
	movb	%dl, 8(%rbx)
	xorl	%eax, %eax
	jmp	.LBB9_46
.Lfunc_end9:
	.size	_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE, .Lfunc_end9-_ZN23fixedwide_batch_codegen14prepared_deltaILm1EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	.cfi_endproc
                                        # -- End function
	.text
	.globl	_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE # -- Begin function _ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE,@function
_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE: # @_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.cfi_startproc
# %bb.0:
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$16, %rsp
	.cfi_def_cfa_offset 32
	.cfi_offset %rbx, -16
	movq	%rdi, %rbx
	movq	32(%rsp), %rax
	movq	%rax, (%rsp)
	callq	_ZN23fixedwide_batch_codegen14prepared_deltaILm4EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	movq	%rbx, %rax
	addq	$16, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end10:
	.size	_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE, .Lfunc_end10-_ZN23fixedwide_batch_codegen13blocked_deltaEPKN9fixedwide11basic_fixedILm128ELj12EEEPS2_S5_PKjmNS1_ILm64ELj10EEE
	.cfi_endproc
                                        # -- End function
	.section	.text._ZN23fixedwide_batch_codegen14prepared_deltaILm4EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE,"axG",@progbits,_ZN23fixedwide_batch_codegen14prepared_deltaILm4EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE,comdat
	.weak	_ZN23fixedwide_batch_codegen14prepared_deltaILm4EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE # -- Begin function _ZN23fixedwide_batch_codegen14prepared_deltaILm4EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen14prepared_deltaILm4EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE,@function
_ZN23fixedwide_batch_codegen14prepared_deltaILm4EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE: # @_ZN23fixedwide_batch_codegen14prepared_deltaILm4EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r13
	.cfi_def_cfa_offset 40
	pushq	%r12
	.cfi_def_cfa_offset 48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	subq	$216, %rsp
	.cfi_def_cfa_offset 272
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%r8, %rbx
	movq	%rcx, %r12
	movq	%rdx, %r13
	movq	%rsi, %rbp
	movq	%rdi, 8(%rsp)                   # 8-byte Spill
	movq	272(%rsp), %rax
	movq	%rax, %rcx
	negq	%rcx
	movq	%rax, 16(%rsp)                  # 8-byte Spill
	cmovsq	%rax, %rcx
	movabsq	$-2601111570856684097, %rdx     # imm = 0xDBE6FECEBDEDD5BF
	movq	%rcx, %rax
	mulq	%rdx
	movq	%rdx, %rsi
	shrq	$33, %rsi
	movabsq	$10000000000, %rdx              # imm = 0x2540BE400
	imulq	%rdx, %rsi
	movq	%rcx, %rax
	subq	%rsi, %rax
	jmp	.LBB11_2
	.p2align	4
.LBB11_3:                               #   in Loop: Header=BB11_2 Depth=1
                                        # kill: def $eax killed $eax killed $rax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $edx killed $edx def $rdx
	movq	%rsi, %rax
	testq	%rdx, %rdx
	je	.LBB11_4
.LBB11_2:                               # =>This Inner Loop Header: Depth=1
	movq	%rdx, %rsi
	movq	%rax, %rdx
	orq	%rsi, %rdx
	shrq	$32, %rdx
	je	.LBB11_3
# %bb.1:                                #   in Loop: Header=BB11_2 Depth=1
	xorl	%edx, %edx
	divq	%rsi
	movq	%rsi, %rax
	testq	%rdx, %rdx
	jne	.LBB11_2
.LBB11_4:
	movq	%rcx, %rax
	orq	%rsi, %rax
	shrq	$32, %rax
	je	.LBB11_6
# %bb.5:
	movq	%rcx, %rax
	xorl	%edx, %edx
	divq	%rsi
	jmp	.LBB11_7
.LBB11_6:
	movl	%ecx, %eax
	xorl	%edx, %edx
	divl	%esi
                                        # kill: def $eax killed $eax def $rax
.LBB11_7:
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	$0, (%rsp)                      # 8-byte Folded Spill
	movabsq	$10000000000, %rax              # imm = 0x2540BE400
	xorl	%edx, %edx
	divq	%rsi
	movq	%rax, %r14
	cmpq	$1, %rax
	movq	%r9, 144(%rsp)                  # 8-byte Spill
	je	.LBB11_9
# %bb.8:
	movl	$1, %esi
	xorl	%edi, %edi
	movq	%r14, %rdx
	xorl	%ecx, %ecx
	callq	__udivti3@PLT
	movq	144(%rsp), %r9                  # 8-byte Reload
	movq	%rax, (%rsp)                    # 8-byte Spill
.LBB11_9:
	shrq	$63, 16(%rsp)                   # 8-byte Folded Spill
	cmpq	$4, %r9
	movq	%rbx, 56(%rsp)                  # 8-byte Spill
	movq	%rbp, 32(%rsp)                  # 8-byte Spill
	jb	.LBB11_135
# %bb.10:
	movl	$12, %ecx
                                        # implicit-def: $al
	movq	%rax, 136(%rsp)                 # 8-byte Spill
                                        # implicit-def: $al
	movq	%rax, 128(%rsp)                 # 8-byte Spill
                                        # implicit-def: $al
	movq	%rax, 120(%rsp)                 # 8-byte Spill
                                        # implicit-def: $al
	movq	%rax, 112(%rsp)                 # 8-byte Spill
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rbx
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
                                        # implicit-def: $rax
                                        # kill: killed $rax
	xorl	%r15d, %r15d
	.p2align	4
.LBB11_11:                              # =>This Inner Loop Header: Depth=1
	movq	-48(%rbp,%rcx,4), %rdx
	movq	-40(%rbp,%rcx,4), %rsi
	movq	%rsi, %r11
	shrq	$63, %r11
	movq	%rdx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rsi, %rdi
	testq	%rsi, %rsi
	cmovnsq	%rdx, %rax
	cmovnsq	%rsi, %rdi
	movl	$0, %r8d
	cmpq	$1, %r14
	movq	%r15, 24(%rsp)                  # 8-byte Spill
	movq	%r9, 208(%rsp)                  # 8-byte Spill
	movq	%rbx, 152(%rsp)                 # 8-byte Spill
	je	.LBB11_18
# %bb.12:                               #   in Loop: Header=BB11_11 Depth=1
	testq	%rdi, %rdi
	je	.LBB11_15
# %bb.13:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rdi, %r14
	jbe	.LBB11_16
# %bb.14:                               #   in Loop: Header=BB11_11 Depth=1
	xorl	%esi, %esi
	jmp	.LBB11_17
.LBB11_15:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r14, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%r14, %r8
	sbbq	$-1, %rdx
	cmpq	%r14, %r8
	movl	$0, %eax
	cmovaeq	%r14, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB11_18
.LBB11_16:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%r14, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %rsi
	cmpq	%r14, %rdi
	movl	$0, %edx
	cmovaeq	%r14, %rdx
	subq	%rdx, %rdi
.LBB11_17:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%r14
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB11_18:                              #   in Loop: Header=BB11_11 Depth=1
	movq	$0, 104(%rsp)                   # 8-byte Folded Spill
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpq	%rdx, %r11
	movl	$0, %ebx
	movq	$-1, %rsi
	cmoveq	%rsi, %rbx
	cmpl	%edx, %r11d
	movabsq	$-9223372036854775808, %r15     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rdx      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rdx, %r15
	movq	40(%rsp), %r10                  # 8-byte Reload
	mulq	%r10
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r10
	addq	%r9, %rax
	adcq	$0, %rdx
	jne	.LBB11_21
# %bb.19:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rsi, %rbx
	movq	%r15, %rdx
	sbbq	%rax, %rdx
	jae	.LBB11_26
.LBB11_20:                              #   in Loop: Header=BB11_11 Depth=1
	movq	$0, 136(%rsp)                   # 8-byte Folded Spill
	movq	$0, 104(%rsp)                   # 8-byte Folded Spill
	movq	$0, 80(%rsp)                    # 8-byte Folded Spill
	jmp	.LBB11_22
	.p2align	4
.LBB11_21:                              #   in Loop: Header=BB11_11 Depth=1
	movl	$0, %eax
	movq	%rax, 80(%rsp)                  # 8-byte Spill
	movl	$0, %eax
	movq	%rax, 136(%rsp)                 # 8-byte Spill
.LBB11_22:                              #   in Loop: Header=BB11_11 Depth=1
	movq	-32(%rbp,%rcx,4), %rdx
	movq	-24(%rbp,%rcx,4), %rsi
	movq	%rsi, %r11
	shrq	$63, %r11
	movq	%rdx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rsi, %rdi
	testq	%rsi, %rsi
	cmovnsq	%rdx, %rax
	cmovnsq	%rsi, %rdi
	movl	$0, %r8d
	cmpq	$1, %r14
	je	.LBB11_32
# %bb.23:                               #   in Loop: Header=BB11_11 Depth=1
	testq	%rdi, %rdi
	je	.LBB11_29
# %bb.24:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rdi, %r14
	jbe	.LBB11_30
# %bb.25:                               #   in Loop: Header=BB11_11 Depth=1
	xorl	%esi, %esi
	jmp	.LBB11_31
	.p2align	4
.LBB11_26:                              #   in Loop: Header=BB11_11 Depth=1
	testq	%r8, %r8
	je	.LBB11_66
# %bb.27:                               #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r9
	movq	%r8, %rax
	mulq	40(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rdi
	cmpq	$1, %r14
	jne	.LBB11_73
# %bb.28:                               #   in Loop: Header=BB11_11 Depth=1
	xorl	%edx, %edx
	jmp	.LBB11_91
.LBB11_29:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r14, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%r14, %r8
	sbbq	$-1, %rdx
	cmpq	%r14, %r8
	movl	$0, %eax
	cmovaeq	%r14, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB11_32
.LBB11_30:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%r14, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %rsi
	cmpq	%r14, %rdi
	movl	$0, %edx
	cmovaeq	%r14, %rdx
	subq	%rdx, %rdi
.LBB11_31:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%r14
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB11_32:                              #   in Loop: Header=BB11_11 Depth=1
	movq	$0, 96(%rsp)                    # 8-byte Folded Spill
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpq	%rdx, %r11
	movl	$0, %ebx
	movq	$-1, %rsi
	cmoveq	%rsi, %rbx
	cmpl	%edx, %r11d
	movabsq	$-9223372036854775808, %r15     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rdx      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rdx, %r15
	movq	40(%rsp), %r10                  # 8-byte Reload
	mulq	%r10
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r10
	addq	%r9, %rax
	adcq	$0, %rdx
	jne	.LBB11_35
# %bb.33:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rsi, %rbx
	movq	%r15, %rdx
	sbbq	%rax, %rdx
	jae	.LBB11_40
.LBB11_34:                              #   in Loop: Header=BB11_11 Depth=1
	movq	$0, 128(%rsp)                   # 8-byte Folded Spill
	movq	$0, 96(%rsp)                    # 8-byte Folded Spill
	movq	$0, 72(%rsp)                    # 8-byte Folded Spill
	jmp	.LBB11_36
	.p2align	4
.LBB11_35:                              #   in Loop: Header=BB11_11 Depth=1
	movl	$0, %eax
	movq	%rax, 72(%rsp)                  # 8-byte Spill
	movl	$0, %eax
	movq	%rax, 128(%rsp)                 # 8-byte Spill
.LBB11_36:                              #   in Loop: Header=BB11_11 Depth=1
	movq	-16(%rbp,%rcx,4), %rdx
	movq	-8(%rbp,%rcx,4), %rsi
	movq	%rsi, %rbx
	shrq	$63, %rbx
	movq	%rdx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rsi, %rdi
	testq	%rsi, %rsi
	cmovnsq	%rdx, %rax
	cmovnsq	%rsi, %rdi
	movl	$0, %r8d
	cmpq	$1, %r14
	je	.LBB11_46
# %bb.37:                               #   in Loop: Header=BB11_11 Depth=1
	testq	%rdi, %rdi
	je	.LBB11_43
# %bb.38:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rdi, %r14
	jbe	.LBB11_44
# %bb.39:                               #   in Loop: Header=BB11_11 Depth=1
	xorl	%esi, %esi
	jmp	.LBB11_45
	.p2align	4
.LBB11_40:                              #   in Loop: Header=BB11_11 Depth=1
	testq	%r8, %r8
	je	.LBB11_68
# %bb.41:                               #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r9
	movq	%r8, %rax
	mulq	40(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rdi
	cmpq	$1, %r14
	jne	.LBB11_76
# %bb.42:                               #   in Loop: Header=BB11_11 Depth=1
	xorl	%edx, %edx
	jmp	.LBB11_98
.LBB11_43:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r14, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%r14, %r8
	sbbq	$-1, %rdx
	cmpq	%r14, %r8
	movl	$0, %eax
	cmovaeq	%r14, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB11_46
.LBB11_44:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%r14, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %rsi
	cmpq	%r14, %rdi
	movl	$0, %edx
	cmovaeq	%r14, %rdx
	subq	%rdx, %rdi
.LBB11_45:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%r14
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB11_46:                              #   in Loop: Header=BB11_11 Depth=1
	movq	$0, 88(%rsp)                    # 8-byte Folded Spill
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpq	%rdx, %rbx
	movl	$0, %r15d
	movq	$-1, %rsi
	cmoveq	%rsi, %r15
	cmpl	%edx, %ebx
	movabsq	$-9223372036854775808, %r11     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rdx      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rdx, %r11
	movq	40(%rsp), %r10                  # 8-byte Reload
	mulq	%r10
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r10
	addq	%r9, %rax
	adcq	$0, %rdx
	jne	.LBB11_49
# %bb.47:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rsi, %r15
	movq	%r11, %rdx
	sbbq	%rax, %rdx
	jae	.LBB11_54
.LBB11_48:                              #   in Loop: Header=BB11_11 Depth=1
	movq	$0, 120(%rsp)                   # 8-byte Folded Spill
	movq	$0, 88(%rsp)                    # 8-byte Folded Spill
	movq	$0, 64(%rsp)                    # 8-byte Folded Spill
	jmp	.LBB11_50
	.p2align	4
.LBB11_49:                              #   in Loop: Header=BB11_11 Depth=1
	movl	$0, %eax
	movq	%rax, 64(%rsp)                  # 8-byte Spill
	movl	$0, %eax
	movq	%rax, 120(%rsp)                 # 8-byte Spill
.LBB11_50:                              #   in Loop: Header=BB11_11 Depth=1
	movq	(%rbp,%rcx,4), %rdx
	movq	8(%rbp,%rcx,4), %rsi
	movq	%rsi, %rbx
	shrq	$63, %rbx
	movq	%rdx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rsi, %rdi
	testq	%rsi, %rsi
	cmovnsq	%rdx, %rax
	cmovnsq	%rsi, %rdi
	movl	$0, %r8d
	cmpq	$1, %r14
	je	.LBB11_60
# %bb.51:                               #   in Loop: Header=BB11_11 Depth=1
	testq	%rdi, %rdi
	je	.LBB11_57
# %bb.52:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rdi, %r14
	jbe	.LBB11_58
# %bb.53:                               #   in Loop: Header=BB11_11 Depth=1
	xorl	%esi, %esi
	jmp	.LBB11_59
	.p2align	4
.LBB11_54:                              #   in Loop: Header=BB11_11 Depth=1
	testq	%r8, %r8
	je	.LBB11_70
# %bb.55:                               #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r9
	movq	%r8, %rax
	mulq	40(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rdi
	cmpq	$1, %r14
	jne	.LBB11_79
# %bb.56:                               #   in Loop: Header=BB11_11 Depth=1
	xorl	%edx, %edx
	jmp	.LBB11_105
.LBB11_57:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r14, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%r14, %r8
	sbbq	$-1, %rdx
	cmpq	%r14, %r8
	movl	$0, %eax
	cmovaeq	%r14, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB11_60
.LBB11_58:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rsi, %rax
	movq	%rdx, %rsi
	movq	%r14, %rdx
	imulq	%rsi, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %rsi
	cmpq	%r14, %rdi
	movl	$0, %edx
	cmovaeq	%r14, %rdx
	subq	%rdx, %rdi
.LBB11_59:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%r14
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB11_60:                              #   in Loop: Header=BB11_11 Depth=1
	xorl	%esi, %esi
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpq	%rdx, %rbx
	movl	$0, %r11d
	movq	$-1, %r9
	cmoveq	%r9, %r11
	cmpl	%edx, %ebx
	movabsq	$-9223372036854775808, %rbp     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rdx      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rdx, %rbp
	movq	40(%rsp), %r10                  # 8-byte Reload
	mulq	%r10
	movq	%rdx, %r9
	movq	%rax, %r15
	movq	%rdi, %rax
	mulq	%r10
	addq	%r9, %rax
	adcq	$0, %rdx
	movl	$0, %edx
	jne	.LBB11_65
# %bb.61:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%r15, %r11
	movq	%rbp, %rdx
	sbbq	%rax, %rdx
	jb	.LBB11_114
# %bb.62:                               #   in Loop: Header=BB11_11 Depth=1
	testq	%r8, %r8
	je	.LBB11_72
# %bb.63:                               #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %rsi
	movq	%r8, %rax
	mulq	40(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %rdi
	cmpq	$1, %r14
	jne	.LBB11_82
# %bb.64:                               #   in Loop: Header=BB11_11 Depth=1
	xorl	%edx, %edx
	jmp	.LBB11_112
	.p2align	4
.LBB11_65:                              #   in Loop: Header=BB11_11 Depth=1
	movl	$0, %eax
	movq	%rax, 112(%rsp)                 # 8-byte Spill
	jmp	.LBB11_115
.LBB11_66:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rsi, %r8
	negq	%r8
	movl	$0, %edi
	sbbq	%rax, %rdi
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpl	%edx, %r11d
	cmoveq	%rax, %rdi
	movq	%rdi, 48(%rsp)                  # 8-byte Spill
	cmpq	%rdx, %r11
	cmoveq	%rsi, %r8
.LBB11_67:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%r8, %rax
	movq	%r8, 104(%rsp)                  # 8-byte Spill
	shrq	$8, %rax
	movq	%rax, 192(%rsp)                 # 8-byte Spill
	movb	$1, %al
	movq	%rax, 80(%rsp)                  # 8-byte Spill
	jmp	.LBB11_22
.LBB11_68:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rsi, %r8
	negq	%r8
	movl	$0, %edi
	sbbq	%rax, %rdi
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpl	%edx, %r11d
	cmoveq	%rax, %rdi
	movq	%rdi, 184(%rsp)                 # 8-byte Spill
	cmpq	%rdx, %r11
	cmoveq	%rsi, %r8
.LBB11_69:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%r8, %rax
	movq	%r8, 96(%rsp)                   # 8-byte Spill
	shrq	$8, %rax
	movq	%rax, 176(%rsp)                 # 8-byte Spill
	movb	$1, %al
	movq	%rax, 72(%rsp)                  # 8-byte Spill
	jmp	.LBB11_36
.LBB11_70:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rsi, %r8
	negq	%r8
	movl	$0, %edi
	sbbq	%rax, %rdi
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpl	%edx, %ebx
	cmoveq	%rax, %rdi
	movq	%rdi, 200(%rsp)                 # 8-byte Spill
	cmpq	%rdx, %rbx
	cmoveq	%rsi, %r8
.LBB11_71:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%r8, %rax
	movq	%r8, 88(%rsp)                   # 8-byte Spill
	shrq	$8, %rax
	movq	%rax, 168(%rsp)                 # 8-byte Spill
	movb	$1, %al
	movq	%rax, 64(%rsp)                  # 8-byte Spill
	jmp	.LBB11_50
.LBB11_72:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%r15, %rsi
	negq	%rsi
	movl	$0, %edi
	sbbq	%rax, %rdi
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpl	%edx, %ebx
	cmoveq	%rax, %rdi
	movq	%rdi, 152(%rsp)                 # 8-byte Spill
	cmpq	%rdx, %rbx
	cmoveq	%r15, %rsi
	movq	%rsi, %rax
	shrq	$8, %rax
	movq	%rax, 160(%rsp)                 # 8-byte Spill
	movb	$1, %dl
	jmp	.LBB11_115
.LBB11_73:                              #   in Loop: Header=BB11_11 Depth=1
	testq	%rdi, %rdi
	je	.LBB11_85
# %bb.74:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rdi, %r14
	jbe	.LBB11_89
# %bb.75:                               #   in Loop: Header=BB11_11 Depth=1
	movq	%rdi, %rdx
	xorl	%edi, %edi
	jmp	.LBB11_90
.LBB11_76:                              #   in Loop: Header=BB11_11 Depth=1
	testq	%rdi, %rdi
	je	.LBB11_86
# %bb.77:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rdi, %r14
	jbe	.LBB11_96
# %bb.78:                               #   in Loop: Header=BB11_11 Depth=1
	movq	%rdi, %rdx
	xorl	%edi, %edi
	jmp	.LBB11_97
.LBB11_79:                              #   in Loop: Header=BB11_11 Depth=1
	testq	%rdi, %rdi
	je	.LBB11_87
# %bb.80:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rdi, %r14
	jbe	.LBB11_103
# %bb.81:                               #   in Loop: Header=BB11_11 Depth=1
	movq	%rdi, %rdx
	xorl	%edi, %edi
	jmp	.LBB11_104
.LBB11_82:                              #   in Loop: Header=BB11_11 Depth=1
	testq	%rdi, %rdi
	je	.LBB11_88
# %bb.83:                               #   in Loop: Header=BB11_11 Depth=1
	cmpq	%rdi, %r14
	jbe	.LBB11_110
# %bb.84:                               #   in Loop: Header=BB11_11 Depth=1
	movq	%rdi, %rdx
	xorl	%edi, %edi
	jmp	.LBB11_111
.LBB11_85:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %rdi
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%r14, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %rax
	cmpq	%r14, %rdi
	movl	$0, %r8d
	cmovaeq	%r14, %r8
	movq	%rdi, %rdx
	subq	%r8, %rdx
	xorl	%edi, %edi
	jmp	.LBB11_91
.LBB11_86:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %rdi
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%r14, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %rax
	cmpq	%r14, %rdi
	movl	$0, %r8d
	cmovaeq	%r14, %r8
	movq	%rdi, %rdx
	subq	%r8, %rdx
	xorl	%edi, %edi
	jmp	.LBB11_98
.LBB11_87:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %rdi
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%r14, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %rax
	cmpq	%r14, %rdi
	movl	$0, %r8d
	cmovaeq	%r14, %r8
	movq	%rdi, %rdx
	subq	%r8, %rdx
	xorl	%edi, %edi
	jmp	.LBB11_105
.LBB11_88:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %rdi
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%r14, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %rax
	cmpq	%r14, %rdi
	movl	$0, %r8d
	cmovaeq	%r14, %r8
	movq	%rdi, %rdx
	subq	%r8, %rdx
	xorl	%edi, %edi
	jmp	.LBB11_112
.LBB11_89:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	movq	%rdi, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r8, %rax
	movq	%rdx, %r8
	movq	%r14, %rdx
	imulq	%r8, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %r8
	cmpq	%r14, %rdi
	movl	$0, %edx
	cmovaeq	%r14, %rdx
	subq	%rdx, %rdi
	movq	%rdi, %rdx
	movq	%r8, %rdi
.LBB11_90:                              #   in Loop: Header=BB11_11 Depth=1
	#APP
	divq	%r14
	#NO_APP
.LBB11_91:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rbx, %r8
	subq	%rsi, %r8
	movq	%r15, %rbp
	sbbq	%r9, %rbp
	cmpq	%rax, %r8
	sbbq	%rdi, %rbp
	jae	.LBB11_93
# %bb.92:                               #   in Loop: Header=BB11_11 Depth=1
	movq	$0, 136(%rsp)                   # 8-byte Folded Spill
	movq	$0, 104(%rsp)                   # 8-byte Folded Spill
	movq	$0, 80(%rsp)                    # 8-byte Folded Spill
	movq	32(%rsp), %rbp                  # 8-byte Reload
	jmp	.LBB11_22
.LBB11_93:                              #   in Loop: Header=BB11_11 Depth=1
	addq	%rsi, %rax
	adcq	%r9, %rdi
	cmpq	%rax, %rbx
	sbbq	%rdi, %r15
	movq	32(%rsp), %rbp                  # 8-byte Reload
	jb	.LBB11_20
# %bb.94:                               #   in Loop: Header=BB11_11 Depth=1
	testq	%rdx, %rdx
	je	.LBB11_131
# %bb.95:                               #   in Loop: Header=BB11_11 Depth=1
	movl	$2, %eax
	movq	%rax, 104(%rsp)                 # 8-byte Spill
	movb	$2, %al
	movq	%rax, 136(%rsp)                 # 8-byte Spill
	movq	$0, 80(%rsp)                    # 8-byte Folded Spill
	jmp	.LBB11_22
.LBB11_96:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	movq	%rdi, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r8, %rax
	movq	%rdx, %r8
	movq	%r14, %rdx
	imulq	%r8, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %r8
	cmpq	%r14, %rdi
	movl	$0, %edx
	cmovaeq	%r14, %rdx
	subq	%rdx, %rdi
	movq	%rdi, %rdx
	movq	%r8, %rdi
.LBB11_97:                              #   in Loop: Header=BB11_11 Depth=1
	#APP
	divq	%r14
	#NO_APP
.LBB11_98:                              #   in Loop: Header=BB11_11 Depth=1
	movq	%rbx, %r8
	subq	%rsi, %r8
	movq	%r15, %rbp
	sbbq	%r9, %rbp
	cmpq	%rax, %r8
	sbbq	%rdi, %rbp
	jae	.LBB11_100
# %bb.99:                               #   in Loop: Header=BB11_11 Depth=1
	movq	$0, 128(%rsp)                   # 8-byte Folded Spill
	movq	$0, 96(%rsp)                    # 8-byte Folded Spill
	movq	$0, 72(%rsp)                    # 8-byte Folded Spill
	movq	32(%rsp), %rbp                  # 8-byte Reload
	jmp	.LBB11_36
.LBB11_100:                             #   in Loop: Header=BB11_11 Depth=1
	addq	%rsi, %rax
	adcq	%r9, %rdi
	cmpq	%rax, %rbx
	sbbq	%rdi, %r15
	movq	32(%rsp), %rbp                  # 8-byte Reload
	jb	.LBB11_34
# %bb.101:                              #   in Loop: Header=BB11_11 Depth=1
	testq	%rdx, %rdx
	je	.LBB11_132
# %bb.102:                              #   in Loop: Header=BB11_11 Depth=1
	movl	$2, %eax
	movq	%rax, 96(%rsp)                  # 8-byte Spill
	movb	$2, %al
	movq	%rax, 128(%rsp)                 # 8-byte Spill
	movq	$0, 72(%rsp)                    # 8-byte Folded Spill
	jmp	.LBB11_36
.LBB11_103:                             #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	movq	%rdi, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r8, %rax
	movq	%rdx, %r8
	movq	%r14, %rdx
	imulq	%r8, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %r8
	cmpq	%r14, %rdi
	movl	$0, %edx
	cmovaeq	%r14, %rdx
	subq	%rdx, %rdi
	movq	%rdi, %rdx
	movq	%r8, %rdi
.LBB11_104:                             #   in Loop: Header=BB11_11 Depth=1
	#APP
	divq	%r14
	#NO_APP
.LBB11_105:                             #   in Loop: Header=BB11_11 Depth=1
	movq	%r15, %r8
	subq	%rsi, %r8
	movq	%r11, %rbp
	sbbq	%r9, %rbp
	cmpq	%rax, %r8
	sbbq	%rdi, %rbp
	jae	.LBB11_107
# %bb.106:                              #   in Loop: Header=BB11_11 Depth=1
	movq	$0, 120(%rsp)                   # 8-byte Folded Spill
	movq	$0, 88(%rsp)                    # 8-byte Folded Spill
	movq	$0, 64(%rsp)                    # 8-byte Folded Spill
	movq	32(%rsp), %rbp                  # 8-byte Reload
	jmp	.LBB11_50
.LBB11_107:                             #   in Loop: Header=BB11_11 Depth=1
	addq	%rsi, %rax
	adcq	%r9, %rdi
	cmpq	%rax, %r15
	sbbq	%rdi, %r11
	movq	32(%rsp), %rbp                  # 8-byte Reload
	jb	.LBB11_48
# %bb.108:                              #   in Loop: Header=BB11_11 Depth=1
	testq	%rdx, %rdx
	je	.LBB11_133
# %bb.109:                              #   in Loop: Header=BB11_11 Depth=1
	movl	$2, %eax
	movq	%rax, 88(%rsp)                  # 8-byte Spill
	movb	$2, %al
	movq	%rax, 120(%rsp)                 # 8-byte Spill
	movq	$0, 64(%rsp)                    # 8-byte Folded Spill
	jmp	.LBB11_50
.LBB11_110:                             #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	movq	%rdi, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r8, %rax
	movq	%rdx, %r8
	movq	%r14, %rdx
	imulq	%r8, %rdx
	subq	%rdx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %r8
	cmpq	%r14, %rdi
	movl	$0, %edx
	cmovaeq	%r14, %rdx
	subq	%rdx, %rdi
	movq	%rdi, %rdx
	movq	%r8, %rdi
.LBB11_111:                             #   in Loop: Header=BB11_11 Depth=1
	#APP
	divq	%r14
	#NO_APP
.LBB11_112:                             #   in Loop: Header=BB11_11 Depth=1
	movq	%r11, %r8
	subq	%r15, %r8
	movq	%rbp, %r9
	sbbq	%rsi, %r9
	cmpq	%rax, %r8
	sbbq	%rdi, %r9
	jb	.LBB11_114
# %bb.113:                              #   in Loop: Header=BB11_11 Depth=1
	addq	%r15, %rax
	adcq	%rsi, %rdi
	cmpq	%rax, %r11
	sbbq	%rdi, %rbp
	jae	.LBB11_129
	.p2align	4
.LBB11_114:                             #   in Loop: Header=BB11_11 Depth=1
	movq	$0, 112(%rsp)                   # 8-byte Folded Spill
	xorl	%esi, %esi
	xorl	%edx, %edx
.LBB11_115:                             #   in Loop: Header=BB11_11 Depth=1
	movq	56(%rsp), %r15                  # 8-byte Reload
	movq	32(%rsp), %r10                  # 8-byte Reload
	movq	48(%rsp), %r11                  # 8-byte Reload
.LBB11_116:                             #   in Loop: Header=BB11_11 Depth=1
	cmpb	$0, 80(%rsp)                    # 1-byte Folded Reload
	movq	200(%rsp), %rbx                 # 8-byte Reload
	je	.LBB11_171
# %bb.117:                              #   in Loop: Header=BB11_11 Depth=1
	movq	192(%rsp), %rdi                 # 8-byte Reload
	shlq	$8, %rdi
	movzbl	104(%rsp), %eax                 # 1-byte Folded Reload
	orq	%rdi, %rax
	movq	-48(%r13,%rcx,4), %rdi
	addq	%rax, %rdi
	movq	-40(%r13,%rcx,4), %r8
	adcq	%r11, %r8
	jo	.LBB11_170
# %bb.118:                              #   in Loop: Header=BB11_11 Depth=1
	movq	24(%rsp), %r9                   # 8-byte Reload
	movl	(%r15,%r9,4), %r9d
	shlq	$4, %r9
	addq	(%r12,%r9), %rax
	movq	%r11, %rbp
	movq	8(%r12,%r9), %r11
	adcq	%rbp, %r11
	jo	.LBB11_170
# %bb.119:                              #   in Loop: Header=BB11_11 Depth=1
	addq	%r12, %r9
	movq	%rdi, -48(%r13,%rcx,4)
	movq	%r8, -40(%r13,%rcx,4)
	movq	%rax, (%r9)
	movq	%r11, 8(%r9)
	cmpb	$0, 72(%rsp)                    # 1-byte Folded Reload
	je	.LBB11_172
# %bb.120:                              #   in Loop: Header=BB11_11 Depth=1
	movq	176(%rsp), %rdi                 # 8-byte Reload
	shlq	$8, %rdi
	movzbl	96(%rsp), %eax                  # 1-byte Folded Reload
	orq	%rdi, %rax
	movq	-32(%r13,%rcx,4), %rdi
	addq	%rax, %rdi
	movq	-24(%r13,%rcx,4), %r8
	movq	184(%rsp), %rbp                 # 8-byte Reload
	adcq	%rbp, %r8
	jo	.LBB11_173
# %bb.121:                              #   in Loop: Header=BB11_11 Depth=1
	movq	24(%rsp), %r9                   # 8-byte Reload
	movl	4(%r15,%r9,4), %r9d
	shlq	$4, %r9
	addq	(%r12,%r9), %rax
	movq	8(%r12,%r9), %r11
	adcq	%rbp, %r11
	jo	.LBB11_174
# %bb.122:                              #   in Loop: Header=BB11_11 Depth=1
	addq	%r12, %r9
	movq	%rdi, -32(%r13,%rcx,4)
	movq	%r8, -24(%r13,%rcx,4)
	movq	%rax, (%r9)
	movq	%r11, 8(%r9)
	cmpb	$0, 64(%rsp)                    # 1-byte Folded Reload
	je	.LBB11_175
# %bb.123:                              #   in Loop: Header=BB11_11 Depth=1
	movq	168(%rsp), %rdi                 # 8-byte Reload
	shlq	$8, %rdi
	movzbl	88(%rsp), %eax                  # 1-byte Folded Reload
	orq	%rdi, %rax
	movq	-16(%r13,%rcx,4), %rdi
	addq	%rax, %rdi
	movq	-8(%r13,%rcx,4), %r8
	adcq	%rbx, %r8
	jo	.LBB11_176
# %bb.124:                              #   in Loop: Header=BB11_11 Depth=1
	movq	24(%rsp), %r9                   # 8-byte Reload
	movl	8(%r15,%r9,4), %r9d
	shlq	$4, %r9
	addq	(%r12,%r9), %rax
	movq	8(%r12,%r9), %r11
	adcq	%rbx, %r11
	jo	.LBB11_177
# %bb.125:                              #   in Loop: Header=BB11_11 Depth=1
	addq	%r12, %r9
	movq	%rdi, -16(%r13,%rcx,4)
	movq	%r8, -8(%r13,%rcx,4)
	movq	%rax, (%r9)
	movq	%r11, 8(%r9)
	testb	%dl, %dl
	je	.LBB11_178
# %bb.126:                              #   in Loop: Header=BB11_11 Depth=1
	movq	160(%rsp), %rdx                 # 8-byte Reload
	shlq	$8, %rdx
	movzbl	%sil, %eax
	orq	%rdx, %rax
	movq	(%r13,%rcx,4), %rdx
	addq	%rax, %rdx
	movq	8(%r13,%rcx,4), %rsi
	movq	152(%rsp), %rbx                 # 8-byte Reload
	adcq	%rbx, %rsi
	jo	.LBB11_181
# %bb.127:                              #   in Loop: Header=BB11_11 Depth=1
	movq	24(%rsp), %r8                   # 8-byte Reload
	movl	12(%r15,%r8,4), %edi
	movq	%r8, %r15
	shlq	$4, %rdi
	addq	(%r12,%rdi), %rax
	movq	8(%r12,%rdi), %r8
	adcq	%rbx, %r8
	movq	208(%rsp), %r9                  # 8-byte Reload
	jo	.LBB11_182
# %bb.128:                              #   in Loop: Header=BB11_11 Depth=1
	addq	%r12, %rdi
	movq	%rdx, (%r13,%rcx,4)
	movq	%rsi, 8(%r13,%rcx,4)
	movq	%rax, (%rdi)
	movq	%r8, 8(%rdi)
	addq	$4, %r15
	addq	$16, %rcx
	addq	$-4, %r9
	cmpq	$3, %r9
	movq	%r10, %rbp
	ja	.LBB11_11
	jmp	.LBB11_136
.LBB11_129:                             #   in Loop: Header=BB11_11 Depth=1
	testq	%rdx, %rdx
	movq	56(%rsp), %r15                  # 8-byte Reload
	movq	32(%rsp), %r10                  # 8-byte Reload
	movq	48(%rsp), %r11                  # 8-byte Reload
	je	.LBB11_134
# %bb.130:                              #   in Loop: Header=BB11_11 Depth=1
	movl	$2, %esi
	movb	$2, %al
	movq	%rax, 112(%rsp)                 # 8-byte Spill
	xorl	%edx, %edx
	jmp	.LBB11_116
.LBB11_131:                             #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	negq	%r8
	movl	$0, %esi
	sbbq	%rdi, %rsi
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpl	%edx, %r11d
	cmoveq	%rdi, %rsi
	movq	%rsi, 48(%rsp)                  # 8-byte Spill
	cmpq	%rdx, %r11
	cmoveq	%rax, %r8
	jmp	.LBB11_67
.LBB11_132:                             #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	negq	%r8
	movl	$0, %esi
	sbbq	%rdi, %rsi
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpl	%edx, %r11d
	cmoveq	%rdi, %rsi
	movq	%rsi, 184(%rsp)                 # 8-byte Spill
	cmpq	%rdx, %r11
	cmoveq	%rax, %r8
	jmp	.LBB11_69
.LBB11_133:                             #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %r8
	negq	%r8
	movl	$0, %esi
	sbbq	%rdi, %rsi
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpl	%edx, %ebx
	cmoveq	%rdi, %rsi
	movq	%rsi, 200(%rsp)                 # 8-byte Spill
	cmpq	%rdx, %rbx
	cmoveq	%rax, %r8
	jmp	.LBB11_71
.LBB11_134:                             #   in Loop: Header=BB11_11 Depth=1
	movq	%rax, %rsi
	negq	%rsi
	movl	$0, %r8d
	sbbq	%rdi, %r8
	movq	16(%rsp), %rdx                  # 8-byte Reload
	cmpl	%edx, %ebx
	cmoveq	%rdi, %r8
	movq	%r8, 152(%rsp)                  # 8-byte Spill
	cmpq	%rdx, %rbx
	cmoveq	%rax, %rsi
	movq	%rsi, %rax
	shrq	$8, %rax
	movq	%rax, 160(%rsp)                 # 8-byte Spill
	movb	$1, %dl
	jmp	.LBB11_116
.LBB11_135:
	xorl	%r15d, %r15d
.LBB11_136:
	cmpq	144(%rsp), %r15                 # 8-byte Folded Reload
	jae	.LBB11_163
# %bb.137:
	leaq	2(,%r15,4), %r11
	.p2align	4
.LBB11_138:                             # =>This Inner Loop Header: Depth=1
	movq	-8(%rbp,%r11,4), %rcx
	movq	(%rbp,%r11,4), %rdx
	movq	%rdx, %rbx
	shrq	$63, %rbx
	movq	%rcx, %rax
	negq	%rax
	movl	$0, %edi
	sbbq	%rdx, %rdi
	testq	%rdx, %rdx
	cmovnsq	%rcx, %rax
	cmovnsq	%rdx, %rdi
	movl	$0, %r8d
	cmpq	$1, %r14
	movq	%r15, 24(%rsp)                  # 8-byte Spill
	je	.LBB11_145
# %bb.139:                              #   in Loop: Header=BB11_138 Depth=1
	testq	%rdi, %rdi
	je	.LBB11_142
# %bb.140:                              #   in Loop: Header=BB11_138 Depth=1
	cmpq	%rdi, %r14
	jbe	.LBB11_143
# %bb.141:                              #   in Loop: Header=BB11_138 Depth=1
	xorl	%esi, %esi
	jmp	.LBB11_144
.LBB11_142:                             #   in Loop: Header=BB11_138 Depth=1
	movq	%rax, %r8
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r14, %rax
	imulq	%rdx, %rax
	subq	%rax, %r8
	cmpq	%r14, %r8
	sbbq	$-1, %rdx
	cmpq	%r14, %r8
	movl	$0, %eax
	cmovaeq	%r14, %rax
	subq	%rax, %r8
	movq	%rdx, %rax
	xorl	%edi, %edi
	jmp	.LBB11_145
.LBB11_143:                             #   in Loop: Header=BB11_138 Depth=1
	movq	%rax, %rcx
	movq	%rdi, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rcx, %rax
	movq	%rdx, %rsi
	movq	%r14, %rcx
	imulq	%rdx, %rcx
	subq	%rcx, %rdi
	cmpq	%r14, %rdi
	sbbq	$-1, %rsi
	cmpq	%r14, %rdi
	movl	$0, %ecx
	cmovaeq	%r14, %rcx
	subq	%rcx, %rdi
.LBB11_144:                             #   in Loop: Header=BB11_138 Depth=1
	movq	%rdi, %rdx
	#APP
	divq	%r14
	#NO_APP
	movq	%rdx, %r8
	movq	%rsi, %rdi
.LBB11_145:                             #   in Loop: Header=BB11_138 Depth=1
	xorl	%ebp, %ebp
	movq	16(%rsp), %rcx                  # 8-byte Reload
	cmpq	%rcx, %rbx
	movl	$0, %r15d
	movq	$-1, %rdx
	cmoveq	%rdx, %r15
	cmpl	%ecx, %ebx
	movabsq	$-9223372036854775808, %rcx     # imm = 0x8000000000000000
	movabsq	$9223372036854775807, %rdx      # imm = 0x7FFFFFFFFFFFFFFF
	cmoveq	%rdx, %rcx
	movq	40(%rsp), %r10                  # 8-byte Reload
	mulq	%r10
	movq	%rdx, %r9
	movq	%rax, %rsi
	movq	%rdi, %rax
	mulq	%r10
	movq	%rax, %rdi
	addq	%r9, %rdi
	adcq	$0, %rdx
	jne	.LBB11_165
# %bb.146:                              #   in Loop: Header=BB11_138 Depth=1
	cmpq	%rsi, %r15
	movq	%rcx, %rax
	sbbq	%rdi, %rax
	movq	24(%rsp), %r10                  # 8-byte Reload
	jb	.LBB11_164
# %bb.147:                              #   in Loop: Header=BB11_138 Depth=1
	testq	%r8, %r8
	je	.LBB11_150
# %bb.148:                              #   in Loop: Header=BB11_138 Depth=1
	movq	%r8, %rax
	mulq	40(%rsp)                        # 8-byte Folded Reload
	movq	%rdx, %r8
	cmpq	$1, %r14
	jne	.LBB11_151
# %bb.149:                              #   in Loop: Header=BB11_138 Depth=1
	xorl	%edx, %edx
	jmp	.LBB11_157
	.p2align	4
.LBB11_150:                             #   in Loop: Header=BB11_138 Depth=1
	movq	%rsi, %rax
	movq	%rdi, %r8
	movq	32(%rsp), %rbp                  # 8-byte Reload
	movq	%r10, %r15
	jmp	.LBB11_160
	.p2align	4
.LBB11_151:                             #   in Loop: Header=BB11_138 Depth=1
	testq	%r8, %r8
	je	.LBB11_154
# %bb.152:                              #   in Loop: Header=BB11_138 Depth=1
	cmpq	%r8, %r14
	jbe	.LBB11_155
# %bb.153:                              #   in Loop: Header=BB11_138 Depth=1
	movq	%r8, %rdx
	xorl	%r8d, %r8d
	jmp	.LBB11_156
.LBB11_154:                             #   in Loop: Header=BB11_138 Depth=1
	movq	%rax, %r8
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%rdx, %rax
	movq	%r14, %rdx
	imulq	%rax, %rdx
	subq	%rdx, %r8
	cmpq	%r14, %r8
	sbbq	$-1, %rax
	cmpq	%r14, %r8
	movl	$0, %edx
	cmovaeq	%r14, %rdx
	subq	%rdx, %r8
	movq	%r8, %rdx
	xorl	%r8d, %r8d
	jmp	.LBB11_157
.LBB11_155:                             #   in Loop: Header=BB11_138 Depth=1
	movq	%rax, %r9
	movq	%r8, %rax
	mulq	(%rsp)                          # 8-byte Folded Reload
	movq	%r9, %rax
	movq	%rdx, %r9
	movq	%r14, %rdx
	imulq	%r9, %rdx
	subq	%rdx, %r8
	cmpq	%r14, %r8
	sbbq	$-1, %r9
	cmpq	%r14, %r8
	movl	$0, %edx
	cmovaeq	%r14, %rdx
	subq	%rdx, %r8
	movq	%r8, %rdx
	movq	%r9, %r8
.LBB11_156:                             #   in Loop: Header=BB11_138 Depth=1
	#APP
	divq	%r14
	#NO_APP
.LBB11_157:                             #   in Loop: Header=BB11_138 Depth=1
	movq	%r15, %r9
	subq	%rsi, %r9
	movq	%rcx, %rbp
	sbbq	%rdi, %rbp
	cmpq	%rax, %r9
	sbbq	%r8, %rbp
	jb	.LBB11_164
# %bb.158:                              #   in Loop: Header=BB11_138 Depth=1
	addq	%rsi, %rax
	adcq	%rdi, %r8
	cmpq	%rax, %r15
	sbbq	%r8, %rcx
	jb	.LBB11_164
# %bb.159:                              #   in Loop: Header=BB11_138 Depth=1
	testq	%rdx, %rdx
	movq	32(%rsp), %rbp                  # 8-byte Reload
	movq	%r10, %r15
	jne	.LBB11_166
.LBB11_160:                             #   in Loop: Header=BB11_138 Depth=1
	movq	%rax, %rcx
	negq	%rcx
	movl	$0, %edx
	sbbq	%r8, %rdx
	movq	16(%rsp), %rsi                  # 8-byte Reload
	cmpl	%esi, %ebx
	cmoveq	%r8, %rdx
	cmpq	%rsi, %rbx
	cmoveq	%rax, %rcx
	movq	-8(%r13,%r11,4), %rax
	addq	%rcx, %rax
	movq	(%r13,%r11,4), %rsi
	adcq	%rdx, %rsi
	movq	56(%rsp), %rdi                  # 8-byte Reload
	jo	.LBB11_185
# %bb.161:                              #   in Loop: Header=BB11_138 Depth=1
	movl	(%rdi,%r15,4), %edi
	shlq	$4, %rdi
	addq	(%r12,%rdi), %rcx
	adcq	8(%r12,%rdi), %rdx
	jo	.LBB11_185
# %bb.162:                              #   in Loop: Header=BB11_138 Depth=1
	addq	%r12, %rdi
	movq	%rax, -8(%r13,%r11,4)
	movq	%rsi, (%r13,%r11,4)
	movq	%rcx, (%rdi)
	movq	%rdx, 8(%rdi)
	incq	%r15
	addq	$4, %r11
	cmpq	%r15, 144(%rsp)                 # 8-byte Folded Reload
	jne	.LBB11_138
.LBB11_163:
	movq	8(%rsp), %rax                   # 8-byte Reload
	movb	$1, 16(%rax)
	jmp	.LBB11_169
.LBB11_164:
	xorl	%ebp, %ebp
	movq	8(%rsp), %rax                   # 8-byte Reload
	movq	%r10, %r15
	jmp	.LBB11_167
.LBB11_165:
	movq	8(%rsp), %rax                   # 8-byte Reload
	movq	24(%rsp), %r15                  # 8-byte Reload
	jmp	.LBB11_167
.LBB11_166:
	movb	$2, %bpl
	movq	8(%rsp), %rax                   # 8-byte Reload
.LBB11_167:
	movq	%r15, (%rax)
	movb	%bpl, 8(%rax)
.LBB11_168:
	movb	$0, 16(%rax)
.LBB11_169:
	addq	$216, %rsp
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%r12
	.cfi_def_cfa_offset 40
	popq	%r13
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.LBB11_170:
	.cfi_def_cfa_offset 272
	xorl	%ecx, %ecx
	movq	8(%rsp), %rax                   # 8-byte Reload
	movq	24(%rsp), %r15                  # 8-byte Reload
	jmp	.LBB11_184
.LBB11_171:
	movq	8(%rsp), %rax                   # 8-byte Reload
	movq	136(%rsp), %rcx                 # 8-byte Reload
	xorl	%edx, %edx
	jmp	.LBB11_180
.LBB11_172:
	movl	$1, %edx
	movq	128(%rsp), %rcx                 # 8-byte Reload
	jmp	.LBB11_179
.LBB11_173:
	movq	24(%rsp), %r15                  # 8-byte Reload
	orq	$1, %r15
	jmp	.LBB11_183
.LBB11_174:
	movq	24(%rsp), %r15                  # 8-byte Reload
	incq	%r15
	jmp	.LBB11_183
.LBB11_175:
	movl	$2, %edx
	movq	120(%rsp), %rcx                 # 8-byte Reload
	jmp	.LBB11_179
.LBB11_176:
	movq	24(%rsp), %r15                  # 8-byte Reload
	addq	$2, %r15
	jmp	.LBB11_183
.LBB11_177:
	movq	24(%rsp), %r15                  # 8-byte Reload
	orq	$2, %r15
	jmp	.LBB11_183
.LBB11_178:
	movl	$3, %edx
	movq	112(%rsp), %rcx                 # 8-byte Reload
.LBB11_179:
                                        # kill: def $cl killed $cl killed $rcx def $rcx
	movq	8(%rsp), %rax                   # 8-byte Reload
.LBB11_180:
	movq	24(%rsp), %r15                  # 8-byte Reload
	addq	%rdx, %r15
	jmp	.LBB11_184
.LBB11_181:
	movq	24(%rsp), %r15                  # 8-byte Reload
	orq	$3, %r15
	jmp	.LBB11_183
.LBB11_182:
	addq	$3, %r15
.LBB11_183:
	xorl	%ecx, %ecx
	movq	8(%rsp), %rax                   # 8-byte Reload
.LBB11_184:
	movq	%r15, (%rax)
	movb	%cl, 8(%rax)
	jmp	.LBB11_168
.LBB11_185:
	movq	8(%rsp), %rax                   # 8-byte Reload
	movq	%r15, (%rax)
	movb	$0, 8(%rax)
	jmp	.LBB11_168
.Lfunc_end11:
	.size	_ZN23fixedwide_batch_codegen14prepared_deltaILm4EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE, .Lfunc_end11-_ZN23fixedwide_batch_codegen14prepared_deltaILm4EEESt8expectedIvN9fixedwide5batch5ErrorEEPKNS2_11basic_fixedILm128ELj12EEEPS7_SA_PKjmNS6_ILm64ELj10EEE
	.cfi_endproc
                                        # -- End function
	.text
	.globl	_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m # -- Begin function _ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m,@function
_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m: # @_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m
	.cfi_startproc
# %bb.0:
	movq	%rdi, %rax
	testq	%r8, %r8
	je	.LBB12_12
# %bb.1:
	cmpq	$12, %r8
	jb	.LBB12_2
# %bb.3:
	movq	%rsi, %rdi
	subq	%rdx, %rdi
	cmpq	$32, %rdi
	setb	%dil
	movq	%rsi, %r9
	subq	%rcx, %r9
	cmpq	$32, %r9
	setb	%r9b
	orb	%dil, %r9b
	je	.LBB12_5
.LBB12_2:
	xorl	%edi, %edi
.LBB12_8:
	movq	%rdi, %r9
	orq	$1, %r9
	testb	$1, %r8b
	je	.LBB12_10
# %bb.9:
	movl	(%rdx,%rdi,4), %r10d
	movl	(%rcx,%rdi,4), %r11d
	cmpl	%r11d, %r10d
	cmovll	%r10d, %r11d
	movl	%r11d, (%rsi,%rdi,4)
	movq	%r9, %rdi
.LBB12_10:
	cmpq	%r9, %r8
	je	.LBB12_12
	.p2align	4
.LBB12_11:                              # =>This Inner Loop Header: Depth=1
	movl	(%rdx,%rdi,4), %r9d
	movl	(%rcx,%rdi,4), %r10d
	cmpl	%r10d, %r9d
	cmovll	%r9d, %r10d
	movl	%r10d, (%rsi,%rdi,4)
	movl	4(%rdx,%rdi,4), %r9d
	movl	4(%rcx,%rdi,4), %r10d
	cmpl	%r10d, %r9d
	cmovll	%r9d, %r10d
	movl	%r10d, 4(%rsi,%rdi,4)
	addq	$2, %rdi
	cmpq	%rdi, %r8
	jne	.LBB12_11
	jmp	.LBB12_12
.LBB12_5:
	movq	%r8, %rdi
	andq	$-8, %rdi
	xorl	%r9d, %r9d
	.p2align	4
.LBB12_6:                               # =>This Inner Loop Header: Depth=1
	movdqu	(%rdx,%r9,4), %xmm0
	movdqu	16(%rdx,%r9,4), %xmm1
	movdqu	(%rcx,%r9,4), %xmm2
	movdqu	16(%rcx,%r9,4), %xmm3
	movdqa	%xmm2, %xmm4
	pcmpgtd	%xmm0, %xmm4
	pand	%xmm4, %xmm0
	pandn	%xmm2, %xmm4
	por	%xmm0, %xmm4
	movdqa	%xmm3, %xmm0
	pcmpgtd	%xmm1, %xmm0
	pand	%xmm0, %xmm1
	pandn	%xmm3, %xmm0
	por	%xmm1, %xmm0
	movdqu	%xmm4, (%rsi,%r9,4)
	movdqu	%xmm0, 16(%rsi,%r9,4)
	addq	$8, %r9
	cmpq	%r9, %rdi
	jne	.LBB12_6
# %bb.7:
	cmpq	%rdi, %r8
	jne	.LBB12_8
.LBB12_12:
	movb	$1, 16(%rax)
	retq
.Lfunc_end12:
	.size	_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m, .Lfunc_end12-_ZN23fixedwide_batch_codegen12select_min32EPN9fixedwide11basic_fixedILm32ELj4EEEPKS2_S5_m
	.cfi_endproc
                                        # -- End function
	.globl	_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m # -- Begin function _ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m
	.p2align	4
	.type	_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m,@function
_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m: # @_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m
	.cfi_startproc
# %bb.0:
	movq	%rdi, %rax
	testq	%r8, %r8
	je	.LBB13_4
# %bb.1:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r14
	.cfi_def_cfa_offset 24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset %rbx, -32
	.cfi_offset %r14, -24
	.cfi_offset %rbp, -16
	movl	$8, %edi
	.p2align	4
.LBB13_2:                               # =>This Inner Loop Header: Depth=1
	movq	-8(%rdx,%rdi), %r9
	movq	(%rdx,%rdi), %r10
	movq	-8(%rcx,%rdi), %r11
	movq	(%rcx,%rdi), %rbx
	xorl	%ebp, %ebp
	cmpq	%r11, %r9
	setb	%bpl
	xorl	%r14d, %r14d
	cmpq	%rbx, %r10
	setl	%r14b
	cmovel	%ebp, %r14d
	testb	%r14b, %r14b
	cmovneq	%r9, %r11
	cmovneq	%r10, %rbx
	movq	%r11, -8(%rsi,%rdi)
	movq	%rbx, (%rsi,%rdi)
	addq	$16, %rdi
	decq	%r8
	jne	.LBB13_2
# %bb.3:
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	.cfi_restore %rbx
	.cfi_restore %r14
	.cfi_restore %rbp
.LBB13_4:
	movb	$1, 16(%rax)
	retq
.Lfunc_end13:
	.size	_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m, .Lfunc_end13-_ZN23fixedwide_batch_codegen13select_min128EPN9fixedwide11basic_fixedILm128ELj12EEEPKS2_S5_m
	.cfi_endproc
                                        # -- End function
	.section	.text._ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE,"axG",@progbits,_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE,comdat
	.weak	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE # -- Begin function _ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	.p2align	4
	.type	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE,@function
_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE: # @_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	.cfi_startproc
# %bb.0:
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$176, %rsp
	.cfi_def_cfa_offset 192
	.cfi_offset %rbx, -16
	movq	%rdi, %rbx
	movq	%rsi, 96(%rsp)
	movq	%rdx, 104(%rsp)
	sarq	$63, %rdx
	movq	%rdx, 112(%rsp)
	movq	%rdx, 120(%rsp)
	movq	%rcx, 64(%rsp)
	sarq	$63, %rcx
	movq	%rcx, 72(%rsp)
	movq	%rcx, 80(%rsp)
	movq	%rcx, 88(%rsp)
	movups	64(%rsp), %xmm0
	movups	80(%rsp), %xmm1
	movups	%xmm1, 48(%rsp)
	movups	%xmm0, 32(%rsp)
	movups	96(%rsp), %xmm0
	movups	112(%rsp), %xmm1
	movups	%xmm1, 16(%rsp)
	movups	%xmm0, (%rsp)
	leaq	128(%rsp), %rdi
	movl	$128, %r9d
	movl	$12, %esi
	movl	$10, %edx
	movl	$12, %ecx
	callq	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	cmpb	$0, 160(%rsp)
	je	.LBB14_1
# %bb.2:
	movaps	128(%rsp), %xmm0
	movups	%xmm0, (%rbx)
	movb	$1, %al
	jmp	.LBB14_3
.LBB14_1:
	movzbl	128(%rsp), %eax
	movb	%al, (%rbx)
	xorl	%eax, %eax
.LBB14_3:
	movb	%al, 16(%rbx)
	movq	%rbx, %rax
	addq	$176, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end14:
	.size	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE, .Lfunc_end14-_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_NS2_ILm64ELj10EEEEESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	.cfi_endproc
                                        # -- End function
	.section	.text._ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE,"axG",@progbits,_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE,comdat
	.weak	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE # -- Begin function _ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	.p2align	4
	.type	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE,@function
_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE: # @_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	.cfi_startproc
# %bb.0:
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$176, %rsp
	.cfi_def_cfa_offset 192
	.cfi_offset %rbx, -16
	movl	%r9d, %eax
	movq	%rdi, %rbx
	movq	%rsi, 96(%rsp)
	movq	%rdx, 104(%rsp)
	sarq	$63, %rdx
	movq	%rdx, 112(%rsp)
	movq	%rdx, 120(%rsp)
	movq	%rcx, 64(%rsp)
	movq	%r8, 72(%rsp)
	sarq	$63, %r8
	movq	%r8, 80(%rsp)
	movq	%r8, 88(%rsp)
	movups	64(%rsp), %xmm0
	movups	80(%rsp), %xmm1
	movups	%xmm1, 48(%rsp)
	movups	%xmm0, 32(%rsp)
	movups	96(%rsp), %xmm0
	movups	112(%rsp), %xmm1
	movups	%xmm1, 16(%rsp)
	movups	%xmm0, (%rsp)
	leaq	128(%rsp), %rdi
	movl	$128, %r9d
	movl	$12, %esi
	movl	$12, %edx
	movl	%eax, %r8d
	movl	$12, %ecx
	callq	_ZN9fixedwide6detail16mixed_mul_kernelENS_4wide6int256EjS2_jjNS_8RoundingEm@PLT
	cmpb	$0, 160(%rsp)
	je	.LBB15_1
# %bb.2:
	movaps	128(%rsp), %xmm0
	movups	%xmm0, (%rbx)
	movb	$1, %al
	jmp	.LBB15_3
.LBB15_1:
	movzbl	128(%rsp), %eax
	movb	%al, (%rbx)
	xorl	%eax, %eax
.LBB15_3:
	movb	%al, 16(%rbx)
	movq	%rbx, %rax
	addq	$176, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end15:
	.size	_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE, .Lfunc_end15-_ZN9fixedwide6detail20runtime_mul_fallbackINS_11basic_fixedILm128ELj12EEES3_S3_EESt8expectedIT_NS_15ArithmeticErrorEET0_T1_NS_8RoundingE
	.cfi_endproc
                                        # -- End function
	.section	".linker-options","e",@llvm_linker_options
	.ident	"Ubuntu clang version 20.1.2 (0ubuntu1~24.04.3)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym __gxx_personality_v0
