# Batch code generation: GCC 14 and Clang 20

Generated from [batch_codegen.cpp](../../../benchmarks/batch_codegen.cpp) and the accompanying batch headers on 2026-10-28. These are assembly and compiler diagnostics, not timing results. The public default is one lane; the probes explicitly compare blocks of one and four lanes.

| Compiler | Version | Assembly | Vectorization diagnostics |
|---|---|---|---|
| GCC | Ubuntu GCC 14.2.0 | [gcc14.s](gcc14.s) | [gcc14-vectorization.txt](gcc14-vectorization.txt) |
| Clang | Ubuntu Clang 20.1.2 | [clang20.s](clang20.s) | [clang20-vectorization.txt](clang20-vectorization.txt) |

Full compiler banners are in [gcc14-version.txt](gcc14-version.txt) and [clang20-version.txt](clang20-version.txt). Both use their default x86-64 target and the installed libstdc++ 14 headers. No native ISA flags, `restrict`, extra alignment assumptions, fast math, LTO, or profiling instrumentation are used. This replaces earlier GCC 13 / Clang 18 exploratory output.

## Findings

### The 32-bit comparison really vectorizes with Clang

Clang reports vectorization width four and interleave count two for the infallible transform in `select_min32`. Its emitted function contains a reachable SSE2 loop: each iteration loads two vectors from each input, compares four signed 32-bit values per vector with `pcmpgtd`, and selects with `pand`, `pandn`, and `por`. Eight rows are written per iteration with unaligned `movdqu` stores. This is arithmetic across rows, not merely a vector register copying one fixed-point value.

The vector path is reached when the row count is at least 12 and runtime pointer-distance checks admit the memory access pattern. Separate sufficiently spaced buffers can reach it. Exact in-place inputs take the scalar path under these conservative checks. A scalar remainder handles a row count that is not a multiple of eight; small or conservatively overlapping inputs also use scalar code. The supported in-place semantics do not depend on vectorization.

GCC 14 emits a scalar load/compare/conditional-move/store loop for the same `select_min32` probe. `select_min128` is scalar with both compilers. There is no claim that every transform, compiler, or fixed-point width vectorizes.

### The 128-bit exact multiply and delta update remain scalar

The `Money = Fixed128<12>` / `Price = Fixed64<10>` multiplication probes use scalar `mulq`/`imulq`, comparisons, and conditional `divq`. Delta updates additionally use ordinary carry chains such as `addq`/`adcq`, with checked writes in physical row order. No AVX or SIMD 128-bit integer multiplication is emitted. XMM transfers in these paths copy values or result storage; they do not multiply independent rows.

The prepared multiplier performs GCD cancellation and reciprocal setup before the row loop. The emitted `__udivti3` call belongs to that preparation, not to every row. The common exact case with a one-limb magnitude and no rescaling remainder uses the prepared reciprocal, or the divisor-one shortcut, followed by multiplication. Larger numerator/remainder cases retain exact scalar division. The complete function therefore still contains `divq`; counting all instructions in its assembly would not count the work executed by a particular row.

Both compilers inline the prepared arithmetic in the block-one multiply and delta loops. Clang also inlines the explicit block-four path without per-row helper calls. GCC outlines calls to `broadcast_multiplier::operator()` in some block-four paths. Both compilers retain more code and live intermediate state for four lanes. Explicit blocking is a tuning option, not evidence of automatic SIMD or a universal improvement.

The unchanged scalar reference retains its general mixed multiplication fallback when its signed-128 native attempt declines. GCC calls `mixed_mul_kernel`; Clang reaches it through the existing outlined `runtime_mul_fallback`. The prepared `128 x 64 -> 128` exact path covers that complete operand domain through quotient/remainder decomposition, so these particular prepared probes do not need a general rational fallback. Other supported operand combinations still retain the scalar fallback in the library.

GCC's `__stack_chk_fail` sites are compiler stack-protection failure handlers, not numerical fallbacks or normal row calls. The wrapper call to a complete `prepared_delta<1>` or `prepared_delta<4>` function occurs once per invocation, not once per row.

### Rounded FX uses scalar multiply and division, with the general fallback retained

`prepared_scalar_fx`, `blocked_fx`, and `varying_fx` cover nearest-even `Fixed128<12> x Fixed128<12> -> Fixed128<12>`. The small-magnitude branch forms an exact unsigned `64 x 64 -> 128` product with scalar `mulq` and divides once before the tie/range checks. It does not need a signed128 intermediate, so a product between `2^127` and `2^128-1` is not prematurely refused. Larger magnitudes use the quotient/remainder decomposition when one operand fits an unsigned64 magnitude limb.

The varying-operand probe keeps the scale divisor constant and has no `__udivti3` call in either compiler's function. It still has conditional `divq` instructions for two-limb numerators; the absence of an out-of-line divide helper does not mean division disappears. Prepared-broadcast setup retains `__udivti3` before row processing for reduction/reciprocal setup. Both compilers keep the general mixed multiplication call for two large operand magnitudes that the specialized path cannot handle. GCC calls `mixed_mul_kernel`; Clang calls the existing outlined `runtime_mul_fallback`.

The final prepared divider also tests whether a two-limb numerator's high limb is smaller than the divisor. On x86-64, that condition already proves the low quotient fits a single `divq`; the emitted branch bypasses the otherwise required upper-limb reciprocal multiplication and correction. The upper reduction remains when needed, and one-limb numerators still use the reciprocal directly. This is an exact scheduling shortcut within the same quotient/remainder algorithm, not a changed numerical contract.

Clang inlines the owned binary/broadcast multipliers and the scheduler's map/commit boundaries in these probes. GCC retains an outlined broadcast-multiplier call in parts of `blocked_fx`. Four-lane functions are substantially larger and maintain more live intermediate state than block-one functions. These observations support measuring the complete kernel before choosing a larger block; they do not establish a speedup.

### Concrete vectorization blockers

Diagnostics identify checked early exits, conditional control flow, the inline scalar division, possible memory dependencies, and indirect balance destinations. These are consistent with the emitted scalar arithmetic. Ordered duplicate-destination updates and their overflow checks have not been relaxed to satisfy the vectorizer.

The probes cover exact price multiplication, rounded broadcast/varying FX, complete exact delta updates, and infallible 32-/128-bit comparison/select. They do not provide assembly evidence for every mixed-width fallback, the complete anchored/publication chains, or the full HFT owner. Those chains have separate correctness fixtures and complete-loop benchmarks; their throughput cannot be inferred from a multiply probe.

## Reproduce

Run from the repository root with GCC 14 and Clang 20 installed. The commands compile only this probe translation unit; they do not link or execute any workload. Truncating GCC's diagnostic file avoids appending output from a previous invocation.

```sh
mkdir -p reports/batch/codegen

g++-14 --version > reports/batch/codegen/gcc14-version.txt
: > reports/batch/codegen/gcc14-vectorization.txt
g++-14 -std=c++23 -O3 -DNDEBUG -ffp-contract=off -fno-fast-math \
  -I include -S benchmarks/batch_codegen.cpp \
  -o reports/batch/codegen/gcc14.s \
  -fopt-info-vec-all=reports/batch/codegen/gcc14-vectorization.txt

clang++-20 --version > reports/batch/codegen/clang20-version.txt
clang++-20 -std=c++23 -O3 -DNDEBUG -ffp-contract=off -fno-fast-math \
  -I include -S benchmarks/batch_codegen.cpp \
  -o reports/batch/codegen/clang20.s \
  '-Rpass=(loop-vectorize|slp-vectorizer)' \
  '-Rpass-missed=(loop-vectorize|slp-vectorizer)' \
  '-Rpass-analysis=(loop-vectorize|slp-vectorizer)' \
  2> reports/batch/codegen/clang20-vectorization.txt
```

The assembly retains compiler-generated symbols and labels. Search for `select_min32`, `prepared_scalar_multiply`, `prepared_scalar_fx`, `varying_fx`, `prepared_deltaILm1`, and `prepared_deltaILm4` to inspect the corresponding functions; use `c++filt` to demangle names when needed.
