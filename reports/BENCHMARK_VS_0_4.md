# Paired benchmark against untouched fixedwide 0.4

## Method

Both implementations build the **byte-identical** `benchmarks/rounding_bench.cpp`
(sha256 `3afa755e84ef51879f75faf029c7e035cbd55b2ae86b6745cb44598ad42fabdc`, the
source the independent audit hashed) with the same compiler and flags. The 0.4
tree is the untouched release; only its build glue is adjusted, never its source.

The two binaries run **interleaved, one seed at a time, alternating which goes
first**, both **pinned to the same core**. Running one to completion and then the
other let frequency and placement drift land entirely on one side.

Every row is the median of 27 samples (3 seeds x 9 repetitions) of 1,048,576
operations, in CPU nanoseconds. Every process validates 363,520 oracle cases
before timing anything.

### Noise floor

`SELF_CHECK=1 scripts/paired_bench.sh` compares the 0.4 baseline **against
itself** through the identical pipeline. Over 100 rows:

| median | p90 | p99 | max |
|---:|---:|---:|---:|
| 0.15% | 0.53% | 3.73% | 3.73% |

Separately, re-measuring an unchanged tree on different days moved the median
row by about 1.6 percentage points, so a comparison is only meaningful when both
sides are measured in the same session. Every before/after in this report is.

## Summary per compiler

| compiler | rows faster | >3% | >5% | >10% | >25% | worst row | median row |
|---|---:|---:|---:|---:|---:|---:|---:|
| Clang 17 | 62 | 14 | 7 | 3 | 0 | +13.1% | -1.22% |
| Clang 18 | 60 | 14 | 12 | 5 | 0 | +19.7% | -1.20% |
| Clang 22 | 49 | 26 | 18 | 11 | 0 | +24.1% | +0.06% |

## Progress on the audit's compiler

| Clang 17, versus 0.4 | alpha.3 | alpha.4 | alpha.5 |
|---|---:|---:|---:|
| rows faster than 0.4 | 42 | 48 | 62 |
| rows >5% slower | 32 | 21 | 7 |
| rows >10% slower | 22 | 12 | 3 |
| rows >25% slower | 15 | 9 | 0 |
| worst row | +161.0% | +63.8% | +13.1% |
| median row | +0.9% | +0.4% | -1.22% |

No row on any of the three compilers is more than 25% slower than 0.4, and the
median row is now faster on two of them. The wide `Fixed128` paths that dominated
every previous list -- `wide_product.FP128.mul_div` at +56%, `wide_product.FP128.mul`
at +30%, `native_by128.FP128.div` at +41%, `inexact_chain.FP128.mul` at +64% --
are all within 3% of 0.4 or faster.

## What the remaining gap turned out to be

The previous release reported this gap as a scheduling difference it could not
explain: the same divider occupancy, no extra branch misses, fewer frontend
stalls, 8% more instructions and 44% more cycles. Sampling the *instructions*
retired rather than the cycles located it. On the worst row, 79% of retired
instructions were attributed to the public wrapper -- a function whose only job
is a range test and a call -- and 12% to the kernel that does the arithmetic.
Two instructions in that wrapper held almost all of the samples:

```
    mov    QWORD PTR [rsp+0x18],r11     ; the divisor's low limb
    mov    QWORD PTR [rsp+0x20],r14     ; its high limb
    movups xmm0,XMMWORD PTR [rsp+0x18]  ; reloaded 16 bytes wide
    movups XMMWORD PTR [rsp],xmm0       ; into the outgoing argument slot
```

A 16-byte load of two 8-byte stores does not forward in the store buffer. It
waits for both to reach L1: about twenty cycles, on the argument path of every
call. Three 16-byte operands and a returned `std::expected` do not fit the
argument registers, so the third operand goes in memory -- and Clang materialises
a struct argument in a temporary and copies it with a vector move, while it
passes a scalar `__int128` (which is what 0.4's `FP128` holds) by pushing the two
registers it is already in. The same mismatch appeared on the return path, where
a kernel writes two 8-byte halves into the caller's buffer and the caller reloads
them 16 bytes wide.

Both are fixed by not crossing the boundary with a struct: `mul_div128_impl`
takes the divisor as its two limbs, and the scale-specialised kernels return the
caller's own return type so the callee writes the caller's return slot directly.
Neither changes an instruction of arithmetic. `wide_product.FP128.mul_div` went
from +56% to +3% on Clang 17 and the row disappeared from the Clang 18 and
Clang 22 lists entirely.

Three smaller root causes were found the same way:

* the 128-bit `mul` fast path rounded with a branch on a coin flip, where the
  64-bit and `mul_div` paths beside it were already branchless. That branch was
  the whole of `inexact_chain.FP128.mul` at +64%, a serially dependent chain.
* `divide_native_n` was `noinline`, so a division whose operands were already in
  registers paid a call to reach two instructions. 0.4 marks the same function
  `always_inline`. Restoring that, plus 0.4's own toward-zero shortcut, took
  `native_by128.FP128.div` from +41% to parity.
* the 128-bit formatting kernel rounded through `wide::uint128` even when the
  quotient, remainder and divisor all fit 64 bits, which is every value a
  `Fixed128` shares with a `Fixed64`. Clang 22 undid that; Clang 17 and 18 did
  not, and it was the whole of `format_2digits.FP128` at +40%. Rounding narrow
  values in 64 bits made that row 26% *faster* than 0.4 on Clang 17.

## Largest remaining regressions

Every row on Clang 17 that exceeds the 3% gate:

| workload | rounding | 0.4 (ns) | now (ns) | delta |
|---|---|---:|---:|---:|
| `throughput256.FP64.mul_div` | nearest_even | 2.3944 | 2.7084 | +13.1% |
| `throughput4096.FP64.mul_div` | nearest_even | 2.4093 | 2.7071 | +12.4% |
| `halfway_ties.FP64.mul_div` | nearest_even | 2.4184 | 2.6857 | +11.1% |
| `throughput4096.FP64.mul_wide` | nearest_even | 2.3773 | 2.5527 | +7.4% |
| `fullrange.FP64.mul_wide` | toward_zero | 2.5170 | 2.6765 | +6.3% |
| `throughput256.FP64.div` | nearest_even | 2.3840 | 2.5289 | +6.1% |
| `throughput4096.FP64.div` | nearest_even | 2.3925 | 2.5259 | +5.6% |
| `inexact_chain.FP64.div` | toward_zero | 3.7032 | 3.8786 | +4.7% |
| `halfway_ties.FP128.div` | nearest_even | 3.2726 | 3.4227 | +4.6% |
| `throughput256.FP128.div` | nearest_even | 3.2756 | 3.4232 | +4.5% |
| `throughput4096.FP128.div` | nearest_even | 3.2642 | 3.4045 | +4.3% |
| `halfway_ties.FP64.div` | nearest_even | 2.4243 | 2.5199 | +4.0% |
| `exact_chain.FP64.div` | toward_zero | 3.7145 | 3.8568 | +3.8% |
| `wide_product.FP128.mul_div` | toward_zero | 12.6445 | 13.0429 | +3.1% |

Clang 18 additionally has `throughput{256,4096}.FP128.div` and
`halfway_ties.FP128.div` at +15% to +20% on toward-zero, where Clang 17 and
Clang 22 have the same rows at parity or faster: the shortcut that makes
`native_by128.FP128.div` fast on all three costs that row on Clang 18 alone.
Narrowing the shortcut to the case it was written for measured worse on all
three, so it stays as 0.4 wrote it.

### The small 64-bit rows

The 64-bit `div` and `mul_div` rows above are 2.4 ns operations, and the gap is
four instructions. Counted with `perf stat` over 184 million operations, 0.4
retires 65.2 instructions per operation and this version 69.2. Those four are
not arithmetic: this library inlines its narrow fast path into the caller, which
makes the *calling* function large enough that Clang gives it its own stack
frame, and `-fstack-protector-strong` then puts a canary prologue on it -- once
per operation. 0.4 keeps the same fast path behind a library call, so the caller
stays small enough to be inlined and the canary is paid once per timing run.

Removing the inline fast path was measured: it restores 0.4's exact call shape
and makes the loop *worse*, because the out-of-line kernel executes 84.7
instructions per operation instead of 69.2. The four instructions are the price
of the twenty-one this version saves elsewhere, and this row is where the trade
is visible.

Verified as pre-existing, not introduced by this pass: the same benchmark built
from the previous release executes the byte-identical instruction count on this
row (12,772,454,644 against 12,772,454,741 over the same workload).

## Where this version is faster than 0.4

Clang 17, the audit's compiler:

| workload | rounding | 0.4 (ns) | now (ns) | delta |
|---|---|---:|---:|---:|
| `throughput4096.FP64.quantize4` | toward_zero | 3.7296 | 1.8947 | -49.2% |
| `throughput4096.FP128.quantize4` | toward_zero | 6.8170 | 3.5729 | -47.6% |
| `throughput4096.FP64.quantize4` | nearest_even | 4.7048 | 2.6870 | -42.9% |
| `throughput4096.FP128.quantize4` | nearest_even | 7.7857 | 4.4821 | -42.4% |
| `format_2digits.FP64` | nearest_even | 15.8397 | 11.2811 | -28.8% |
| `format_2digits.FP128` | nearest_even | 15.7120 | 11.6357 | -25.9% |
| `format_2digits.FP64` | toward_zero | 14.4384 | 10.7862 | -25.3% |
| `parse_extra_digit.FP64` | nearest_even | 33.0023 | 25.1584 | -23.8% |
| `parse_extra_digit.FP64` | toward_zero | 32.4934 | 24.8744 | -23.4% |
| `parse_extra_digit.FP128` | toward_zero | 32.8115 | 25.3157 | -22.8% |
| `format_2digits.FP128` | toward_zero | 14.3282 | 11.1688 | -22.1% |
| `parse_extra_digit.FP128` | nearest_even | 33.1874 | 26.7685 | -19.3% |

## What this benchmark cannot cover

0.4 has no mixed-width arithmetic and no `Fixed256`, so neither can appear here.
Both are measured in `benchmarks/mixed_bench.cpp` against the same-type
operation in the destination domain, and reported in `reports/BENCHMARK_MIXED.md`.

* **No paired GCC row exists.** The 0.4 baseline hard-requires C++ `_BitInt(256)`
  and refuses to configure without it; GCC does not implement `_BitInt` in C++.
* **No AArch64 timings.** The AArch64 evidence is correctness on real hardware
  (a Pixel 6). A phone under thermal control is not a benchmark host.
* Raw samples for every row and compiler are in `reports/raw/<compiler>/`.
