# Exact checked arithmetic over SoA arrays

Implementation and qualification report, 2026-10-28. The baseline is the public
scalar implementation at `86266a04bca03c914698a1e81e1123e065c77558` (v0.6.10).
Existing scalar entry points, storage, rounding policies and dependency versions
are unchanged. This change adds `<fixedwide/batch.hpp>` and its private multiply
helpers. It does not integrate with or qualify an HFT owner.

## Recommendation and scope

Use prepared broadcast multiplication and a single-pass row kernel first. The
default block size is one. Explicit four-row blocks remain available when a
measurement of the complete consumer kernel justifies them. Independent products
may be computed ahead; dependent additions and replacements still commit in
physical row order. There is no tree reassociation, array transaction, retained
status array, heap allocation, ISA dispatch or persistent packing.

The native optimization avoids building an unnecessarily wide raw product before
rescaling. For unsigned magnitudes, `a = q*d + r` implies
`floor(a*b/d) = q*b + floor(r*b/d)`. The remainder is exact, and both the quotient
range and the selected rounding policy are checked. A prepared broadcast cancels
common decimal factors and reuses an exact reciprocal divider. For rounded
products with two one-limb magnitudes, an unsigned128 product followed by one
division avoids splitting twice. On x86, a dividend high limb smaller than the
divisor feeds the hardware division directly; only larger high limbs need the
upper reciprocal reduction. Signed minima are handled through unsigned magnitudes.

This covers `Fixed128<12> * Fixed64<10> -> Fixed128<12>` over its full scalar
domain, and `Fixed128<12> * Fixed128<12> -> Fixed128<12>` when an actual operand
magnitude, or the reduced broadcast factor, fits unsigned64. Other narrow-width
products with a nonnegative scale gap no larger than 19 use the same method.
Unsupported cases delegate to the original checked scalar operation. Forced
portable builds expose the same API through the original portable arithmetic.

Existing add/sub, wide replacement, mixed-width division/casts, ordered sums,
comparisons and scalar floating-conversion contracts compose through the row
API. These adapters do not imply a new arithmetic fast path for every operation.
Explicit multi-limb SIMD, specialized segmented kernels, general 256-bit batch
division and new floating-conversion algorithms are deferred.

The complete [API contract and workload examples](../../docs/batch.md) cover
delta updates, anchored valuation with separate rounding boundaries, cancellation
through a wide temporary, and publication with per-entry rounding followed by
per-position FX. [The executable example](../../examples/11_batch_soa.cpp) is a
CTest target.

## Correctness qualification

The batch tests compare results and exact errors with the unchanged scalar API.
The independent Boost.Multiprecision oracle additionally covers exhaustive small
domains, full-width randomized operands, all six rounding modes, prepared
divisors, signed limits and overflow-versus-inexact precedence. Focused tests
cover accepted-prefix writes, failure precedence between maps and commits,
in-place aliasing, empty/tail spans, duplicate destinations, sink rows, ordered
sum overflow, half-quantum entries, wide replacement cancellation and the actual
mixed-width cold preparation chains.

The Google Benchmark executable validates its entire fixture corpus before any
timing, including values, errors, untouched output suffixes and repeated quotes.
The portable build and sanitizer checks are correctness gates, independent of
the timing comparison. No timing threshold is used as a CI gate.

Clang 20 passed 155,654 batch checks and 4,785,848 independent rational-oracle
checks. The ASan+UBSan run passed all four focused tests; LeakSanitizer was
disabled after the sandbox prevented process inspection. Address and undefined-
behavior instrumentation remained enabled. The allocation test observed zero
heap allocations. The native coverage gate passed with 99.1099% distinct source
lines, 95.7998% LLVM summary lines and 91.8347% LLVM summary branches. See the
[sanitizer record](validation/review-sanitize.txt), [Clang release tests](validation/review-clang-focused.txt),
and [coverage summary](validation/review-coverage/SUMMARY.md).

The full GCC 14 native suite passed all 89 tests. After the final x86 divisor
tuning, the batch, independent oracle, allocation and example tests were rebuilt
and passed again under GCC 14 and Clang 20;
[the final GCC focused run](validation/review-gcc14-tuned.txt) is retained. One generated executable initially
lacked its execute permission; restoring that bit and rerunning the single
affected test passed without a source change. The complete initial run and retry
are retained in the [native test record](validation/review-gcc14-native.txt).
The GCC 14 forced-portable suite also passed all 89 tests; its two process-launch
permission repairs and focused rerun are retained in the
[portable test record](validation/review-gcc14-portable.txt).
Both compiler benchmark binaries validated 9,956,064 output-row comparisons
before timing; see the [GCC fixture log](validation/review-gcc14-benchmark.txt)
and [Clang fixture log](validation/review-clang-benchmark.txt).

## Measurement method and limits

The benchmark compares the unchanged scalar loop, optimized arithmetic through
`for_each<1>`, and explicit `for_each<4>` on identical retained inputs. Broadcast
preparation is included once per invocation. Complete delta, anchored/replacement
and publication loops include the dependent arithmetic and stores; they are not
standalone-multiply extrapolations. Publication here is the prepared-entry
PnL/FX/clipping/balance-fold arithmetic fixture, not the application's entire
publication function. Fixtures include 64-row blocks and streaming
arrays of 125,000 rows, duplicate balances, zeroed holes, wide successes and
refused lanes.

Measurements use Google Benchmark 1.9.4, Release (`-O3 -DNDEBUG`), normal compiler
vectorization and `-ffp-contract=off -fno-fast-math`, with no `-march=native`, LTO
or explicit non-aliasing assumption. The runner is an Intel Xeon Platinum 8370C
virtual machine, not the brief's Ryzen reference machine. A bounded selection of
workloads is pinned to one virtual CPU, with no concurrent builds, tests or other
timings launched by this task. CPU affinity is not exclusive ownership of the
underlying host and there is no frequency lock; VM scheduling noise limits
interpretation of small differences.

Report time per completed row/chain, not per requested row when a lane refuses.
The JSON retains completed rows/operations, refusals, speculative rows, modeled
bytes, bounded lane-result storage and zero extra array passes. Product fixtures
also classify scalar/general fallback outside the timed loop. Those classifications
are not hardware counters; modeled byte counts are not measured memory traffic.
The full-loop cost includes ordered reductions and scratch work.

### Timing results

Final implementation, 125,000 input rows, three randomly interleaved repetitions.
Values are **median CPU ns per completed row**; speedup is scalar time divided
by the default single-row batch time. For publication, a row is an entry,
with its share of the per-position FX/clipping and ordered balance fold.
The refused-price fixture completes 62,501 rows before refusing; its time is
normalized by that accepted prefix.

| Workload | GCC scalar | GCC batch | Ratio | Clang scalar | Clang batch | Ratio |
|---|---:|---:|---:|---:|---:|---:|
| Delta update, unique balances | 35.23 | 25.01 | 1.41x | 11.27 | 12.64 | 0.89x |
| Delta update, duplicate balances + 5% holes | 37.72 | 23.77 | 1.59x | 10.60 | 10.24 | 1.03x |
| Grid delta, duplicates + 5% holes | 35.28 | 24.26 | 1.45x | 17.33 | 10.70 | 1.62x |
| Anchored valuation + FX + wide replacement | 91.01 | 68.97 | 1.32x | 49.81 | 42.54 | 1.17x |
| Publication PnL, per entry | 49.39 | 52.49 | 0.94x | 29.30 | 24.34 | 1.20x |
| Wide cancellation replacement | 19.11 | 19.62 | 0.97x | 9.44 | 9.26 | 1.02x |
| Exact price multiply, broadcast | 26.86 | 13.66 | 1.97x | 14.03 | 8.72 | 1.61x |
| Exact price multiply, varying | 26.56 | 20.83 | 1.28x | 16.94 | 9.44 | 1.80x |
| Price multiply, wide raw product | 247.32 | 45.96 | 5.38x | 229.39 | 32.23 | 7.12x |
| Exact price multiply, refused prefix | 24.62 | 16.90 | 1.46x | 14.85 | 8.46 | 1.76x |
| Nearest-even FX, broadcast | 40.11 | 31.35 | 1.28x | 26.93 | 23.01 | 1.17x |
| Nearest-even FX, varying | 33.71 | 44.06 | 0.77x | 26.32 | 23.17 | 1.14x |
| Wide FX fallback, broadcast | 183.29 | 185.29 | 0.99x | 243.74 | 176.46 | 1.38x |
| Wide FX fallback, varying | 203.91 | 187.55 | 1.09x | 175.82 | 300.39 | 0.59x |

These are descriptive medians, not a significance test or a consumer latency
guarantee. The [complete summary CSV](summary.csv) includes both 64- and
125,000-row sizes, all three implementations, observed minima/maxima and
coefficients of variation. The retained [GCC JSON](gcc14/benchmark.json) and
[Clang JSON](clang20/benchmark.json) contain every repetition, aggregate and
work/fallback counter. Each directory also contains text output, fixture
validation, compiler/build commands and environment details.

### Interpretation and limitations

- Wide price products are the clearest win: the scalar raw product needs the
  general rational path, while quotient/remainder decomposition stays native.
  The final streaming medians improve by 5.38x with GCC and 7.12x with Clang;
  the observed repetition ranges do not overlap in either comparison.
- Exact broadcast price multiplication improves with both compilers. GCC also
  shows gains in the complete delta updates, including duplicate destinations.
  Clang's complete delta results vary by fixture; there is no universal
  full-loop gain to extrapolate to an owner.
- There are slower medians: GCC varying nearest-even FX and publication, for
  example. Existing scalar operations remain available in an ordinary loop
  or a generic batch transform. Benchmark the actual kernel before choosing
  optimized multiply helpers for an already-fast rounded path.
- Four-row blocking is workload-dependent. It helps GCC wide replacement and
  the wide-price fixture here, but frequently loses elsewhere; it remains
  opt-in. Neither its speed nor its width implies SIMD128 arithmetic.
- VM variability is substantial. For example, Clang's optimized wide-varying
  FX samples range from 158.93 to 353.99 ns/row (37.2% CV), against scalar
  samples from 168.85 to 187.40. Its 300.39 ns median is retained transparently,
  but these three observations do not establish a stable fallback-overhead
  estimate. Several other comparisons also have double-digit CVs; small
  differences should not be treated as performance conclusions.

Cold coefficient/division and general-maintenance expressions have correctness
coverage and generic API support. They are not assigned a new fast-path or
full-consumer throughput claim by this bounded timing run.

## Compiler evidence

The [GCC 14 / Clang 20 code-generation report](codegen/README.md) retains matching
assembly and vectorization diagnostics. The checked 128-bit price/FX/delta
kernels use scalar multiplication, exact division and carry chains. They do not
perform SIMD 128-bit multiplication. Prepared setup is outside the row loop;
wide numerator paths still need division, and genuinely wide operands retain
the general scalar fallback.

Clang emits SSE2 across rows for the infallible 32-bit comparison/select probe,
with runtime alias checks and a scalar remainder. GCC keeps that probe scalar.
Both keep 128-bit comparisons scalar. Four-lane kernels are larger and keep more
intermediate state live; GCC outlines parts of those kernels. These findings
explain why blocking is optional rather than an assumed performance win.

## Reproduce and qualify on the consumer machine

`scripts/bench_batch.sh --validate-only` builds and checks the fixtures without
timing. After reserving a suitable CPU, select a compiler and run a bounded
filter, for example:

```sh
CXX=clang++-20 BATCH_FILTER='^(delta|anchored_replace|publication)\.' \
  MIN_TIME=0.05s REPETITIONS=3 \
  scripts/bench_batch.sh --cpu 2 reports/batch-local
```

Use the exact consumer compiler, flags and stored widths. Keep owner performance
qualification separate: run the complete quote/publication workload with its
real scatter pattern, position boundaries and admission policy, then assess its
latency budget. The library does not narrow stored money, relax precision,
change rounding boundaries, infer unique destinations or change business policy.
