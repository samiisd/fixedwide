# Runtime native paths for mixed arithmetic, 0.6.10

This change builds on fixedwide 0.6.9, commit
`87d828754d7d9f98f84c4b89c1dfde538756ad5d`. That release commit is one commit
ahead of `main` at the start of this work; the PR retains it as its prerequisite.

## Implementation and boundaries

The previous native dispatch proved bounds from operand types. A 128-bit type
therefore missed that path even when its actual values were small. The general
kernel already selects 128-, 256-, 512-, or 1024-bit working tiers; the remaining
overhead is building and evaluating the exact rational.

The existing compile-time native paths remain first. Additional native paths
try checked signed 128-bit products, aligned sums/differences, and rescaling.
They share `divide_rounded`, including its exact quotient/remainder and its
destination-overflow-before-inexact rule. There is no floating-point estimate.

| Operation | Runtime native eligibility | When the attempt cannot finish |
| --- | --- | --- |
| `mul_to` | Both operands and destination at most 128 bits | General kernel evaluates the complete product and rescaling |
| Mixed-scale `add_to` / `sub_to` | Operands at most 128 bits; destination at most 256 bits | General kernel handles alignment, cancellation and rescaling |
| Same-scale add/sub | Any supported operand/destination width | Direct checked 256-bit limb arithmetic after a native attempt; no rational kernel |
| `fixed_cast` | Destination at most 128 bits, source raw value fits signed int128 | General rescaling; same-scale non-narrow values are proven overflow immediately |

Native powers of ten are instantiated only through exponent 38. Larger scale
gaps retain the general implementation. A Fixed256 source is narrow only when
both upper limbs exactly extend **bit 127's sign**. All paths preserve the
public result/error contract, including `INT128_MIN` and a wide intermediate
that becomes representable after cancellation or rescaling.

The only existing helper correction restores a rounded negative result with
unsigned subtraction before the signed conversion. Negating a signed value of
`INT128_MIN` would have been undefined behavior. A regression covers that
boundary in every rounding mode.

Public signatures, storage, and constant-evaluation support are unchanged.
`FIXEDWIDE_FORCE_PORTABLE` keeps its previous dispatch. Division and fused
multiply/divide retain their previous dispatch on both backends.

## Benchmark method

`benchmarks/runtime_narrow_bench.cpp` has 15 workloads, each with 65,536
deterministic samples. Before timing, it compares the complete `expected`
result against **direct calls to the general kernels**: status, raw value,
and error code. A mismatch or an invalid fallback fixture exits unsuccessfully.

The realistic data spans 0.01–1000 lots of 100,000 units, price differences
through ±500, pre-FX amounts through ±10 million, FX rates 0.2–5, and totals
through ±1 billion. Separate rows force native-product overflow while the
rescaled result remains valid, force final overflow, require wide replacement
cancellation, and exercise uniformly distributed full-range replacement inputs.

This corrects three problems in the supplied standalone benchmark: its slope
range exceeded the stated range by four orders of magnitude, most replacement
`next` values were read before initialization, and it did not compare errors
or fail on mismatches. Both timed sides now use equivalent noinline wrappers,
the same result consumption, and a defined unsigned checksum.

Timings alternate pair order, preserve every raw sample, and report medians.
They include indexing, the loop, and result handling; they describe throughput
on the recorded shared virtual host, not isolated instruction latency. No
`-march`, fast-math, CPU-frequency lock, or HFT end-to-end result is claimed.

Reproduce with:

```bash
CXX=g++-14 ITERATIONS=1048576 REPETITIONS=9 \
  scripts/bench_runtime_narrow.sh reports/runtime-narrow-local
```

The script runs native and forced-portable binaries, pins to an allowed CPU
when available, and records compiler/CPU information, source and binary hashes,
validation counts, paired samples, deltas, and ratios.

## Optional reciprocal division

The experimental source and raw exploratory GCC 13 measurements are retained
here as evidence, explicitly labelled outside the supported GCC matrix. The
candidate computes the high half of a product with `ceil(2^128 / divisor)` and
corrects the quotient to recover an exact remainder. It is **not part of the
library**. Its additional arithmetic must show a consistent measured advantage
before replacing the existing native division.

## Validation scope

`test_mixed_runtime` compares all six rounding modes with the general kernels
over exhaustive small grids, exact ties and neighbours, signed boundaries,
mixed-scale alignment/rescaling, powers of ten at 38/39/76, contaminated sign
extensions, true 256-bit overflow, wide cancellation into narrow destinations,
and deterministic full-range and narrow-biased random data. The existing
libFuzzer target now performs the same kind of differential checks for the new
paths, including complete replacement sequences.

The separate Boost-based coverage executable also exercises the new paths so
that the repository's existing coverage gate measures them. Native, portable,
sanitizer, fuzz, and regression-gate execution results are recorded with the
final measurements below.
