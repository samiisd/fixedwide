# Assembly Audit: fixedwide 0.5.0-alpha.3

## Target Inspection: x86-64 Clang -O3 Release

### 1. `throughput4096.FP64.mul_wide`
Assembly verification reveals direct 1-instruction multiplication (`imulq`) and 1-instruction signed quotient division (`idivq`), matching bare-metal assembly.

```assembly
imulq   %rsi, %rax
idivq   %rcx
```
Total instruction count on the fast path is 11 instructions without function calls or multi-precision branching.

### 2. `native_by128.FP128.div` (Toward Zero & Nearest Even)
When operands are in-bounds (`ma <= bound128`), division lowers directly to hardware 128-bit operations:
```assembly
movq    %rdx, %rax
mulq    %rcx
divq    %rsi
```
Avoiding `divide_digit` and normalization loops.

### 3. `quantize128_impl`
Single 128-by-64 bit hardware division via inline assembly when scale difference fits in 64 bits:
```assembly
movq    %rdi, %rax
movq    %rsi, %rdx
divq    %rcx
```
Down to 5.67 ns (toward_zero) vs 0.4's 9.63 ns.
