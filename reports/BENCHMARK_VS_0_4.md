# Paired benchmark against untouched fixedwide 0.4

## Method

Both implementations build the **byte-identical** `benchmarks/rounding_bench.cpp`
(sha256 `3afa755e84ef51879f75faf029c7e035cbd55b2ae86b6745cb44598ad42fabdc`, the
source the independent audit hashed) with the same compiler and flags. The 0.4
tree is the untouched release; only its build glue is adjusted, never its source.

The two binaries run **interleaved, one seed at a time, alternating which goes
first**, both **pinned to the same core**. Running one to completion and then the
other let frequency and placement drift land entirely on one side, which on this
host was worth more than the differences being measured.

Every row is the median of 27 samples (3 seeds x 9 repetitions) of 1,048,576
operations in CPU nanoseconds. Every process validates 363,520 oracle cases
before timing anything.

### Noise floor

`SELF_CHECK=1 scripts/paired_bench.sh` compares the 0.4 baseline **against
itself** through the identical pipeline. Over 100 rows:

| median | p90 | p99 | max |
|---:|---:|---:|---:|
| 0.30% | 2.01% | 5.49% | 5.49% |

A row within about 2% of parity is not distinguishable from noise here, and a
few can cross a 3% gate on measurement alone. Rows are still reported
individually and never averaged into a category score.

## Summary per compiler

Negative means faster than 0.4. Gates: 3% common throughput, 5% dependent
chains, wide arithmetic and text.

| compiler | rows faster | >3% | >5% | >10% | >25% | worst row | median row |
|---|---:|---:|---:|---:|---:|---:|---:|
| Clang 17 | 48 | 27 | 20 | 11 | 7 | +63.0% | +0.5% |
| Clang 18 | 45 | 28 | 21 | 8 | 5 | +60.0% | +0.4% |
| Clang 22 | 45 | 47 | 31 | 22 | 9 | +57.3% | +1.4% |

## Progress against the previous release, on the audit's compiler

Both columns come from this same pipeline.

| Clang 17, versus 0.4 | alpha.3 | alpha.4 |
|---|---:|---:|
| rows faster than 0.4 | 42 | 48 |
| rows >5% slower | 32 | 20 |
| rows >10% slower | 22 | 11 |
| rows >25% slower | 15 | 7 |
| worst row | +161.0% | +63.0% |
| median row | +0.9% | +0.5% |

**The gate still does not pass**, and that is reported rather than averaged into
something that does. What remains is the wide `Fixed128` paths, chiefly
`mul_div`, whose divisor is a runtime value and so has no constant for the
kernel to fold.

The earlier claim that the gap was *structural* -- that a generalized
`basic_fixed<Bits, D>` cannot give a compiled kernel a constant scale -- was
wrong. `D` is a compile-time constant at every call site. The kernels are now
templated on it and explicitly instantiated per decimal count, so they see the
scale exactly as 0.4's did.

## Largest remaining regressions, Clang 17

| workload | rounding | 0.4 (ns) | alpha.4 (ns) | alpha.3 delta | alpha.4 delta |
|---|---|---:|---:|---:|---:|
| `inexact_chain.FP128.mul` | nearest_even | 4.7130 | 7.6839 | +64.1% | +63.0% |
| `throughput256.FP128.mul_div` | toward_zero | 2.8821 | 4.3624 | +4.5% | +51.4% |
| `throughput4096.FP128.mul_div` | toward_zero | 2.8850 | 4.3613 | +3.7% | +51.2% |
| `wide_product.FP128.mul_div` | toward_zero | 12.6910 | 18.8648 | +132.6% | +48.6% |
| `wide_product.FP128.mul_div` | nearest_even | 13.4208 | 19.3205 | +161.0% | +44.0% |
| `format_2digits.FP128` | nearest_even | 16.8326 | 22.0716 | +75.9% | +31.1% |
| `wide_product.FP128.mul` | toward_zero | 6.0761 | 7.6659 | -17.5% | +26.2% |
| `throughput256.FP128.mul_div` | nearest_even | 4.5439 | 5.4776 | -1.3% | +20.6% |
| `throughput4096.FP128.mul_div` | nearest_even | 4.5504 | 5.4803 | -1.7% | +20.4% |
| `native_by64.FP128.div` | toward_zero | 4.0542 | 4.8623 | -1.0% | +19.9% |
| `native_by128.FP128.div` | toward_zero | 8.5289 | 9.8595 | +30.5% | +15.6% |
| `wide_product.FP128.mul` | nearest_even | 7.3642 | 8.0710 | +50.3% | +9.6% |

## Where this version is faster than 0.4

| workload | rounding | 0.4 (ns) | alpha.4 (ns) | delta |
|---|---|---:|---:|---:|
| `throughput4096.FP64.quantize4` | toward_zero | 3.7243 | 1.8955 | -49.1% |
| `throughput4096.FP128.quantize4` | toward_zero | 6.4238 | 3.5459 | -44.8% |
| `throughput4096.FP64.quantize4` | nearest_even | 4.7008 | 2.6988 | -42.6% |
| `throughput4096.FP128.quantize4` | nearest_even | 7.6569 | 4.4466 | -41.9% |
| `parse_extra_digit.FP64` | nearest_even | 33.0175 | 23.6051 | -28.5% |
| `parse_extra_digit.FP64` | toward_zero | 32.4748 | 23.3997 | -27.9% |
| `format_2digits.FP64` | nearest_even | 15.8710 | 11.5809 | -27.0% |
| `parse_extra_digit.FP128` | toward_zero | 32.6073 | 23.9168 | -26.6% |
| `format_2digits.FP64` | toward_zero | 14.5107 | 11.0869 | -23.6% |
| `parse_extra_digit.FP128` | nearest_even | 33.0308 | 25.7855 | -21.9% |
| `fullrange.FP64.mul_wide` | nearest_even | 3.0805 | 2.5143 | -18.4% |
| `halfway_ties.FP128.div` | toward_zero | 2.8872 | 2.4397 | -15.5% |

## What is not measured here

* **No paired GCC row exists.** The 0.4 baseline hard-requires C++ `_BitInt(256)`
  and refuses to configure without it; GCC does not implement `_BitInt` in C++.
  GCC is covered for correctness in the execution matrix, but there is no GCC
  build of 0.4 to compare against, and modifying the baseline would stop it
  being the baseline.
* **No AArch64 timings.** The AArch64 evidence is correctness on real hardware
  (a Pixel 6). A phone under thermal control is not a benchmark host.
* Raw samples for every row and compiler are in `reports/raw/<compiler>/`, with
  the environment, the benchmark source hash and both binary hashes.

