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
| Clang 17 | 48 | 30 | 21 | 12 | 9 | +63.8% | +0.4% |
| Clang 18 | 53 | 24 | 16 | 8 | 5 | +59.4% | -0.2% |
| Clang 22 | 41 | 53 | 35 | 21 | 8 | +56.0% | +3.3% |

## Progress against the previous release, on the audit's compiler

| Clang 17, versus 0.4 | alpha.3 | alpha.4 |
|---|---:|---:|
| rows faster than 0.4 | 42 | 48 |
| rows >5% slower | 32 | 21 |
| rows >10% slower | 22 | 12 |
| rows >25% slower | 15 | 9 |
| worst row | +161.0% | +63.8% |
| median row | +0.9% | +0.4% |

**The gate still does not pass.** What remains is concentrated in the wide
`Fixed128` paths, chiefly `mul_div`.

That row has been investigated to the point of exhausting the mechanisms this
host can measure. Against 0.4, on the same workload, it executes:

| | 0.4 | alpha.4 |
|---|---:|---:|
| instructions | 576M | 622M |
| branch misses | 40.9K | 44.7K |
| divide ops | 1.675M | 1.679M |
| divider busy cycles | 19.81M | 19.77M |
| frontend stalls | 3.09M | 1.59M |
| **cycles** | **128M** | **184M** |

Same work, same divider occupancy, no extra mispredicts, fewer frontend stalls,
44% more cycles. That is a dependency-chain and scheduling difference, not extra
computation, and it will not yield to further micro-optimisation of this shape.
It is reported rather than explained away.

## Largest remaining regressions, Clang 17

| workload | rounding | 0.4 (ns) | alpha.4 (ns) | alpha.3 delta | alpha.4 delta |
|---|---|---:|---:|---:|---:|
| `inexact_chain.FP128.mul` | nearest_even | 4.7092 | 7.7114 | +64.1% | +63.8% |
| `throughput256.FP128.mul_div` | toward_zero | 2.8656 | 4.4068 | +4.5% | +53.8% |
| `throughput4096.FP128.mul_div` | toward_zero | 2.8827 | 4.3948 | +3.7% | +52.5% |
| `wide_product.FP128.mul_div` | toward_zero | 12.6598 | 18.8796 | +132.6% | +49.1% |
| `wide_product.FP128.mul_div` | nearest_even | 13.3993 | 19.3319 | +161.0% | +44.3% |
| `format_2digits.FP128` | nearest_even | 15.8517 | 22.0760 | +75.9% | +39.3% |
| `wide_product.FP128.mul` | toward_zero | 6.0801 | 7.7132 | -17.5% | +26.9% |
| `throughput4096.FP128.mul_div` | nearest_even | 4.5596 | 5.7729 | -1.7% | +26.6% |
| `throughput256.FP128.mul_div` | nearest_even | 4.5448 | 5.7369 | -1.3% | +26.2% |
| `native_by64.FP128.div` | toward_zero | 4.0758 | 4.6917 | -1.0% | +15.1% |
| `halfway_ties.FP128.mul_div` | toward_zero | 2.8771 | 3.2544 | +3.1% | +13.1% |
| `native_by128.FP128.div` | toward_zero | 8.5100 | 9.3691 | +30.5% | +10.1% |

## Where this version is faster than 0.4

| workload | rounding | 0.4 (ns) | alpha.4 (ns) | delta |
|---|---|---:|---:|---:|
| `throughput4096.FP64.quantize4` | toward_zero | 3.7483 | 1.8983 | -49.4% |
| `throughput4096.FP128.quantize4` | toward_zero | 6.4994 | 3.5471 | -45.4% |
| `throughput4096.FP64.quantize4` | nearest_even | 4.7000 | 2.7022 | -42.5% |
| `throughput4096.FP128.quantize4` | nearest_even | 7.2716 | 4.4484 | -38.8% |
| `format_2digits.FP64` | nearest_even | 15.9361 | 11.5720 | -27.4% |
| `parse_extra_digit.FP64` | nearest_even | 32.9739 | 24.7240 | -25.0% |
| `format_2digits.FP64` | toward_zero | 14.5757 | 11.0177 | -24.4% |
| `parse_extra_digit.FP64` | toward_zero | 32.4268 | 24.5256 | -24.4% |
| `parse_extra_digit.FP128` | toward_zero | 32.6243 | 25.3308 | -22.4% |
| `parse_extra_digit.FP128` | nearest_even | 33.0250 | 26.7022 | -19.1% |
| `fullrange.FP64.mul_wide` | nearest_even | 3.0852 | 2.5153 | -18.5% |
| `halfway_ties.FP128.div` | toward_zero | 2.9203 | 2.4404 | -16.4% |

## What this benchmark cannot cover

0.4 has no mixed-width arithmetic and no `Fixed256`, so neither can appear here.
Both are measured in `benchmarks/mixed_bench.cpp` against the same-type
operation in the destination domain, and reported in `reports/BENCHMARK_MIXED.md`.

* **No paired GCC row exists.** The 0.4 baseline hard-requires C++ `_BitInt(256)`
  and refuses to configure without it; GCC does not implement `_BitInt` in C++.
* **No AArch64 timings.** The AArch64 evidence is correctness on real hardware
  (a Pixel 6). A phone under thermal control is not a benchmark host.
* Raw samples for every row and compiler are in `reports/raw/<compiler>/`.

