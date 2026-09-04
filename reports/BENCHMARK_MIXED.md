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
| `Money.mul` on `Fixed128<12>` | 1.90 |
| `Money.div` on `Fixed128<12>` | 2.91 |

`Money.div` was 4.94 ns in alpha.4. The alpha.5 divide work moved the floor
itself, which is why the mixed rows measured against it are compared to the
number in the same run and not to the previous release's.

`Money`, `Price`, `Rate` and `Small` are type aliases local to
`benchmarks/mixed_bench.cpp` (`Fixed128<12>`, `Fixed64<8>`, `Fixed64<12>`,
`Fixed32<6>`), named for readability in a row label. The library has no such
types; its public API is parameterised on width and scale throughout.

## Mixed width and scale

| operation | before | now | speedup | floor |
|---|---:|---:|---:|---:|
| `mul_to.Money.from.Price.Rate` | 338.79 | 2.48 | 137x | 1.90 |
| `div_to.Rate.from.Price.Small` | 369.88 | 4.12 | 90x | 2.91 |
| `add_to.Money.from.Price.Rate` | 418.22 | 0.56 | 747x | 0.55 |
| `mul_div_to.Money.from.Price.Rate.Small` | 309.40 | 4.37 | 71x | 2.91 |
| `fixed_cast.Money.from.Price` | 215.34 | 0.35 | 612x | - |
| `compare.Price.vs.Rate` | 268.79 | 0.47 | 569x | - |
| `mul_to.Rate.from.Small.Small` | 232.74 | 0.30 | 762x | 1.90 |

`mul_to.Money.from.Price.Rate` was 3.50 ns in alpha.4 and is 2.48 ns here. Every
rescale in this file divides by a power of ten, and every one that a mixed
operation reaches fits 64 bits -- but the value being divided is 128 bits, so the
quotient was a `__udivti3` call out to libgcc. One or two hardware divisions do
the same work. `detail::mixed_native::divide_magnitude` takes that path when the
divisor fits one limb, and keeps the generic form for a wider divisor and for
constant evaluation, where no instruction runs. Measured in isolation, the same
loop went from 7.47 ns to 2.09 ns.

`tests/test_mixed_native.cpp` sweeps every magnitude width from 0 to 128 against
every divisor width from 1 to 128 and requires the narrow path to return exactly
what the compiler's own division returns: `divq` faults rather than wraps when
its quotient does not fit a limb, so the reduction that makes it safe is
checked, not assumed.

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
| `Fixed256.add` | 1.49 | 1.51 | 1.0x |
| `Fixed256.mul` | 38.13 | 28.34 | 1.3x |
| `Fixed256.div` | 35.28 | 28.28 | 1.2x |
| `Fixed256.mul_div` | 24.90 | 19.01 | 1.3x |
| `Fixed256.quantize` | 27.65 | 26.69 | 1.0x |

`quantize` is effectively unchanged and is still the slowest `Fixed256`
operation. Its divisor is always a power of ten that occupies one limb, yet both
its division and its multiply-back run the general four-limb routines. Recorded
as an open item rather than claimed as improved.

The 256-bit entry points now take their operands by reference. A 32-byte struct
is passed in memory by value, so each call copied three of them onto the stack;
the measured effect was under 2%, which is what the table shows.

