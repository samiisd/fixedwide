# Mixed-scale and Fixed256 benchmark

These are the operations the paired 0.4 comparison structurally cannot cover,
because 0.4 has neither mixed-width arithmetic nor a 256-bit type. Until
alpha.4 they had no performance evidence at all.

The reference point for a mixed operation is the spec's own bar: the **same-type
operation in the destination domain**, which is what "the abstraction must cost
essentially nothing beyond the arithmetic actually required" means in numbers.

Medians of 11 repetitions, core-pinned, every result validated outside the timed
region (20,480 checks). Raw samples in `reports/raw/mixed.csv`.

## The floor

| operation | ns |
|---|---:|
| `Money.add` on `Fixed128<12>` | 0.53 |
| `Money.mul` on `Fixed128<12>` | 1.85 |
| `Money.div` on `Fixed128<12>` | 4.94 |

## Mixed width and scale

| operation | before | after | speedup | floor |
|---|---:|---:|---:|---:|
| `mul_to.Money.from.Price.Rate` | 338.79 | 3.50 | 97x | 1.85 |
| `div_to.Rate.from.Price.Small` | 369.88 | 4.03 | 92x | 4.94 |
| `add_to.Money.from.Price.Rate` | 418.22 | 0.56 | 747x | 0.53 |
| `mul_div_to.Money.from.Price.Rate.Small` | 309.40 | 4.37 | 71x | 4.94 |
| `fixed_cast.Money.from.Price` | 215.34 | 0.35 | 612x | - |
| `compare.Price.vs.Rate` | 268.79 | 0.47 | 569x | - |
| `mul_to.Rate.from.Small.Small` | 232.74 | 0.31 | 762x | 1.85 |

Every mixed operation previously built an exact rational in 1024-bit limbs and
divided it with Knuth's algorithm, including comparison, which needs no division
at all. `add_to`, `fixed_cast` and comparison now sit on the same-type floor,
and `div_to` is faster than the same-type divide it is measured against.

The narrow paths are guarded entirely by compile-time bounds derived from the
widths and scales in the types, so anything outside them runs the original
kernel unchanged. `tests/test_mixed_native.cpp` requires the two to return the
same value and the same error across 157,250 comparisons, including type
combinations the bounds must reject.

## Fixed256

| operation | before | after | speedup |
|---|---:|---:|---:|
| `Fixed256.add` | 1.49 | 1.52 | 1.0x |
| `Fixed256.mul` | 38.13 | 28.23 | 1.4x |
| `Fixed256.div` | 35.28 | 27.92 | 1.3x |
| `Fixed256.mul_div` | 24.90 | 18.81 | 1.3x |
| `Fixed256.quantize` | 27.65 | 26.98 | 1.0x |

`quantize` is unchanged and is now the slowest `Fixed256` operation. It is
recorded as an open item rather than claimed as improved.

