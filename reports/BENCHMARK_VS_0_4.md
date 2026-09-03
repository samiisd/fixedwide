# Paired benchmark against untouched fixedwide 0.4

## Method

Both implementations build the **byte-identical** `benchmarks/rounding_bench.cpp`
(sha256 `3afa755e84ef51879f75faf029c7e035cbd55b2ae86b6745cb44598ad42fabdc`, the
same source the independent audit hashed) with the same compiler and the same
flags. The 0.4 tree is the untouched release; only its build glue is adjusted,
never its sources.

For each compiler the two binaries run **interleaved, one seed at a time,
alternating which goes first**, both **pinned to the same core**. The previous
methodology ran one implementation to completion and then the other, which let
CPU frequency and placement drift land entirely on one side; on this host that
was worth more than the differences being measured.

Every row is the median of 27 samples (3 seeds x 9 repetitions) of 1,048,576
operations, in CPU nanoseconds. Every process validates 363,520 oracle cases
before timing anything.

### Noise floor

`SELF_CHECK=1 scripts/paired_bench.sh` compares the 0.4 baseline **against
itself** through the identical pipeline. Over 100 rows on this host:

| median | p90 | p99 | max |
|---:|---:|---:|---:|
| 0.51% | 1.77% | 5.33% | 5.33% |

A row within about 2% of parity is therefore not distinguishable from noise
here, and a few rows can cross a 3% gate on measurement alone. Rows are still
reported individually and never averaged into a category score.

## Summary per compiler

Negative means faster than 0.4. Gates: 3% for common throughput, 5% for
dependent chains, wide arithmetic and text conversion.

| compiler | rows faster | >3% | >5% | >10% | >25% | worst row | median row |
|---|---:|---:|---:|---:|---:|---:|---:|
| Clang 17 | 43 | 33 | 27 | 16 | 10 | +66.9% | +0.7% |
| Clang 18 | 49 | 30 | 22 | 16 | 10 | +74.9% | +0.1% |
| Clang 22 | 28 | 60 | 46 | 34 | 11 | +78.1% | +4.5% |

**The gate does not pass.** That is reported rather than averaged into something
that does. The wide `Fixed128` paths are the remaining gap, and the cause is
structural: 0.4 was a single-scale library whose kernels saw the scale as a
compile-time constant, so the optimiser folded every bound, branch and 64-bit
division that used it. `basic_fixed<Bits, D>` cannot do that inside a compiled
kernel without instantiating one kernel per decimal count. That was built and
measured, and it lost: it turned a 71-instruction entry point into a
992-instruction one and was slower overall.

## Progress against the previous release

Both columns come from this same pipeline, so they are comparable to each other.

| Clang 17, versus 0.4 | alpha.3 | alpha.4 |
|---|---:|---:|
| rows faster than 0.4 | 42 | 43 |
| rows >5% slower | 32 | 27 |
| rows >10% slower | 22 | 16 |
| rows >25% slower | 15 | 10 |
| worst row | +161.0% | +66.9% |
| median row | +0.9% | +0.7% |

## Largest remaining regressions, Clang 17

| workload | rounding | 0.4 (ns) | alpha.4 (ns) | alpha.3 delta | alpha.4 delta |
|---|---|---:|---:|---:|---:|
| `wide_product.FP128.mul_div` | nearest_even | 13.5692 | 22.6482 | +161.0% | +66.9% |
| `inexact_chain.FP128.mul` | nearest_even | 4.6802 | 7.7771 | +64.1% | +66.2% |
| `wide_product.FP128.mul` | nearest_even | 7.3505 | 11.2693 | +50.3% | +53.3% |
| `native_by64.FP128.div` | nearest_even | 6.3909 | 9.7385 | +58.0% | +52.4% |
| `wide_product.FP128.mul_div` | toward_zero | 13.0780 | 18.5857 | +132.6% | +42.1% |
| `format_2digits.FP128` | nearest_even | 15.7644 | 22.1392 | +75.9% | +40.4% |
| `native_by128.FP128.div` | toward_zero | 8.5290 | 11.2780 | +30.5% | +32.2% |
| `wide_by64.FP128.div` | nearest_even | 8.0675 | 10.6383 | +37.5% | +31.9% |
| `halfway_ties.FP128.mul` | nearest_even | 2.4932 | 3.1841 | +20.5% | +27.7% |
| `wide_by128.FP128.div` | toward_zero | 12.4979 | 15.7511 | +136.8% | +26.0% |
| `exact_results.FP128.mul` | nearest_even | 2.5371 | 3.0273 | +2.6% | +19.3% |
| `throughput4096.FP64.quantize4` | nearest_even | 4.7205 | 5.5712 | +52.7% | +18.0% |

## Where this version is faster than 0.4

Improvements get the same per-row detail as the regressions.

| workload | rounding | 0.4 (ns) | alpha.4 (ns) | delta |
|---|---|---:|---:|---:|
| `throughput4096.FP64.quantize4` | toward_zero | 3.7552 | 1.8150 | -51.7% |
| `throughput4096.FP128.quantize4` | toward_zero | 6.2088 | 3.3026 | -46.8% |
| `wide_by64.FP128.div` | toward_zero | 6.7896 | 5.0177 | -26.1% |
| `format_2digits.FP64` | toward_zero | 14.5547 | 10.9668 | -24.6% |
| `wide_product.FP128.mul` | toward_zero | 6.0714 | 4.9235 | -18.9% |
| `fullrange.FP64.mul_wide` | nearest_even | 3.0949 | 2.5347 | -18.1% |
| `halfway_ties.FP128.div` | toward_zero | 2.8801 | 2.4375 | -15.4% |
| `exact_results.FP128.div` | toward_zero | 2.7134 | 2.4400 | -10.1% |
| `format_2digits.FP64` | nearest_even | 15.9125 | 14.6772 | -7.8% |
| `inexact_chain.FP128.div` | toward_zero | 4.1428 | 3.8350 | -7.4% |

## What is not measured here

* **No paired GCC row exists.** The 0.4 baseline hard-requires C++ `_BitInt(256)`
  and refuses to configure without it; GCC does not implement `_BitInt` in C++.
  GCC is covered for correctness in the execution matrix, but there is no GCC
  build of 0.4 to compare against, and modifying the baseline would stop it
  being the baseline.
* **No AArch64 timings.** The AArch64 evidence is correctness on real hardware
  (a Pixel 6). A phone under thermal control is not a benchmark host, so no
  timing is claimed from it.
* Raw samples for every row and compiler are in `reports/raw/<compiler>/`, with
  the environment, the benchmark source hash and both binary hashes.

