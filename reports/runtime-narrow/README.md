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

Scale-up checks compare against compile-time `INT128_MIN / 10^k` and
`INT128_MAX / 10^k` bounds before multiplying. This rejects an overflowing
alignment without first constructing a larger product. GCC retains its
efficient signed multiplication-overflow builtin. Clang preserves each raw
operand's width when forming unsigned magnitudes, checks the high cross product
before the low product, then checks the carry and exact signed-result limit.
This avoids its more expensive signed-builtin lowering.

The full-API benchmark also exposed an optimizer boundary: Clang outlined the
runtime multiply attempt, materializing the nested result on the stack and
losing constant rounding information. Scoped inline hints preserve that
information, and small general-fallback adapters keep the kernel's argument
and return storage out of rejected native attempts. The adapters call the
unchanged kernels; they apply only to Clang's native runtime-eligible paths.
GCC, forced-portable dispatch, and general-only wide operations keep their
previous fallback structure.

Initial helper measurements and the reason for keeping GCC's builtin are in
[guard-experiment](guard-experiment/README.md). The
[initial full-API run](google-benchmark-initial/README.md) preserves the evidence
that prompted the final refinement, with the intermediate alternatives and
their exact source patches in [inlining-experiment](inlining-experiment/README.md).
The full-operation measurements below are the final performance results for
the public API.

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
or fail on mismatches. Both timed sides use equivalent noinline wrappers and
`benchmark::DoNotOptimize` on the complete result. Registered pairs share
immutable fixture snapshots so later fixture generation cannot alter their
inputs.

Google Benchmark owns iteration calibration, timing, warmup, repetitions,
random interleaving, statistics, and JSON output. The script keeps every raw
repetition and aggregate, using nine repetitions, a configured minimum-time
target of 0.1 seconds per measurement and 0.02 seconds of warmup. Results include indexing, the loop and
result barriers; they describe throughput on the recorded shared virtual host,
not isolated instruction latency. No `-march`, fast-math, CPU-frequency lock,
or HFT end-to-end result is claimed.

Reproduce with:

```bash
CXX=g++-14 MIN_TIME=0.1s REPETITIONS=9 \
  scripts/bench_runtime_narrow.sh reports/runtime-narrow-local
```

The script runs native and forced-portable binaries, pins to an allowed CPU
when available, and records compiler/CPU information, compile commands, source
and binary hashes, validation counts, and raw Google Benchmark JSON.

The target is opt-in, preserving the dependency-free instruction-count gate.
It uses an installed Google Benchmark >=1.9.4 when available; otherwise CMake
fetches v1.9.4 at immutable commit
`eddb0241389718a23a42db6af5f0164b6e0139af`, with its tests and install rules
disabled. A manual build and correctness-only run are:

```bash
cmake -B build-runtime -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=g++-14 -DFIXEDWIDE_BUILD_BENCHMARKS=ON \
  -DFIXEDWIDE_BUILD_RUNTIME_NARROW_BENCHMARK=ON \
  -DFIXEDWIDE_BUILD_TESTS=OFF -DFIXEDWIDE_BUILD_EXAMPLES=OFF
cmake --build build-runtime --target fixedwide_runtime_narrow_bench
build-runtime/benchmarks/fixedwide_runtime_narrow_bench --validate-only
```

Use `FETCHCONTENT_SOURCE_DIR_GOOGLEBENCHMARK` for a local source checkout, or
`GOOGLE_BENCHMARK_SOURCE` with the script. Standard `--benchmark_*` flags work
directly on the executable. CI builds this optional target on both backends,
validates every fixture, and runs short timing smoke tests without a wall-clock
threshold. Earlier custom-harness measurements are explicitly marked
[superseded](exploratory-custom/README.md) and are not the final results.

## Final Google Benchmark results

These are CPU-time median aggregates in nanoseconds per operation on the
recorded x86-64 host. Both native configurations were rebuilt and rerun against
the final production headers. Each comparison uses identical input data and
measures the complete operation through its public API or direct general kernel.
The speedup is the general-kernel median divided by the public-API median.

| Compiler | Realistic workload | General kernel | Public API | Speedup |
| --- | --- | ---: | ---: | ---: |
| GCC 14.2 | Price multiplication | 67.471 ns | 13.360 ns | 5.05× |
| GCC 14.2 | FX multiplication | 77.314 ns | 23.629 ns | 3.27× |
| GCC 14.2 | Replacement | 199.038 ns | 3.801 ns | 52.36× |
| Clang 19.1.1 | Price multiplication | 53.228 ns | 5.152 ns | 10.33× |
| Clang 19.1.1 | FX multiplication | 61.056 ns | 17.185 ns | 3.55× |
| Clang 19.1.1 | Replacement | 168.647 ns | 3.301 ns | 51.10× |

The same-scale replacement path also helps full-range input: GCC measures
217.335 → 13.496 ns (16.10×), and Clang 187.373 → 12.263 ns (15.28×).
The [complete summary](google-benchmark/summary.csv) includes all 15 workloads,
including mixed addition/subtraction, casts, and wide cancellation.

Forced fallbacks are reported separately. Positive deltas below mean the
public API was slower than calling the general kernel directly; these are
differences between medians, not a separately isolated guard cost.

| Forced-fallback workload | GCC 14.2 delta | Clang 19.1.1 delta |
| --- | ---: | ---: |
| Price, successful rounded result | −0.431 ns (−0.39%) | +1.589 ns (+1.69%) |
| FX, successful rounded result | +2.064 ns (+1.85%) | +4.628 ns (+4.71%) |
| Price, overflow result | −4.117 ns (−5.28%) | +0.256 ns (+0.34%) |
| FX, overflow result | −4.859 ns (−5.33%) | −2.381 ns (−2.72%) |
| Mixed addition, successful result | +0.872 ns (+0.60%) | +6.504 ns (+4.77%) |
| Mixed subtraction, successful result | −12.163 ns (−7.59%) | +2.205 ns (+1.59%) |

**The approximate 1 ns fallback-overhead target is not met on every row.** The
final full campaign retains a 4.628 ns FX and 6.504 ns mixed-add median penalty
under Clang. The implementation is retained for its substantial common-case
improvement, exact behavior, and unchanged portable dispatch; it does not claim
negligible overhead for every fallback.

The shared host also shows meaningful variance. For Clang's mixed-add fallback,
the general/public coefficients of variation are 1.34%/4.47%; for successful FX
fallback they are 6.48%/8.27%. Some GCC successful-product measurements have
larger outliers. CPU affinity and randomized repetitions reduce some sources
of bias but do not make small timing differences a portable guarantee. The raw
records are retained, including unfavorable results.

Both final portable executables and their static libraries were byte-identical
to those in the initial Google Benchmark campaign. Their original timing JSON
is reused unchanged, explicitly marked `identical_binary_reuse`, while all four
final configurations passed fresh validation: **983,040 comparisons each,
3,932,160 total**. The final evidence contains 1,080 raw timing records and 480
aggregates: 540 fresh native repetitions and 540 reused portable repetitions.
The portable results act as controls; the new native dispatch is disabled.

See [final evidence](google-benchmark/README.md) for the raw JSON, console output,
validation logs, exact-source patch, and compiler/CPU/command/hash provenance.

## Optional reciprocal division

The candidate computes the high half of a product with `ceil(2^128 / divisor)`
and corrects the quotient to recover an exact remainder. Each supported compiler
run checked 12,131,072 magnitudes/products with zero mismatches, then measured
nine alternating pairs. The same checked product and nearest-even rounding are
included on both sides.

| Compiler | Divisor | Existing division | Reciprocal candidate | Existing / candidate |
| --- | ---: | ---: | ---: | ---: |
| GCC 14.2 | 10^10 | 11.531 ns | 13.532 ns | 0.852× |
| GCC 14.2 | 10^12 | 18.431 ns | 17.616 ns | 1.046× |
| Clang 19.1.1 | 10^10 | 7.474 ns | 10.176 ns | 0.734× |
| Clang 19.1.1 | 10^12 | 17.949 ns | 17.943 ns | 1.000× |

The candidate regresses the price divisor on both compilers and provides no
consistent advantage for the FX divisor, so it is **not part of the library**.
This separate exploratory experiment retains its [source](reciprocal-experiment.cpp),
[commands and environment](reciprocal-environment.txt), and raw
[GCC](reciprocal-gcc14.txt) / [Clang](reciprocal-clang19.txt) samples. Its timings
come from the same shared host and are not an isolated-hardware latency claim.

## Validation scope

`test_mixed_runtime` compares all six rounding modes with the general kernels
over exhaustive small grids, exact ties and neighbours, signed boundaries,
mixed-scale alignment/rescaling, powers of ten at 38/39/76, contaminated sign
extensions, true 256-bit overflow, wide cancellation into narrow destinations,
and deterministic full-range and narrow-biased random data. The existing
libFuzzer target now performs the same kind of differential checks for the new
paths, including complete replacement sequences.

The separate Boost-based coverage executable also exercises the new paths so
that the repository's existing coverage gate measures them.

| Executed configuration | Result |
| --- | --- |
| GCC 14.2 Release, native, warnings as errors, tests/examples/Boost oracle | Earlier full suite: all 85 executed and passed |
| GCC 14.2 Release, forced portable, same configuration | Earlier full suite: 85/85 passed |
| Clang 19.1.1 RelWithDebInfo, native ASan + UBSan, warnings as errors, tests/examples/oracle | Earlier full suite: 77/77 passed |
| GCC 14.2 final standalone `-O2` differential suite, native / portable, warnings as errors | 1,226,146 / 1,226,134 checks passed |
| Clang 19.1.1 final focused ASan + UBSan differential suite | 1,226,146 checks passed |
| Clang 19 ASan + UBSan libFuzzer, native | 100,000 executions, no findings |
| Clang 19 ASan + UBSan libFuzzer, forced portable | 100,000 executions, no findings |
| Existing GCC 14 instruction-count gate | Passed against the unchanged committed baseline |
| Pinned clang-format 22.1.8 and shell syntax | Passed |

Both fuzz campaigns use the existing corpus plus deterministic edge/random
seeds (470 initial files per backend). LeakSanitizer was disabled because this
container cannot inspect process threads through `/proc`; address and undefined
behavior instrumentation remained enabled. Several generated executables lost
their executable permissions, and one generated executable had a corrupted
header. The affected outputs were restored/rebuilt and their tests rerun; the
sanitizer record distinguishes the initial run and successful targeted retries.
No numerical or sanitizer failures were observed.

Coverage was collected from `test_coverage all` alone, following the existing
gate. No thresholds or source exclusions changed:

| Backend | Independent-oracle assertions | Distinct source lines | LLVM lines | LLVM branches |
| --- | ---: | ---: | ---: | ---: |
| Native | 7,411,154 | 99.1989% | 95.7364% | 91.8630% |
| Forced portable | 7,067,896 | 99.2181% | 97.6790% | 93.7111% |
| Required minimum | — | 99% | 95% | 90% |

Commands, source/binary hashes, CTest logs, fuzz counts, coverage summaries, and
instruction-count results are retained in [validation](validation/). These are
local execution records; the PR's GitHub checks provide the separate platform
matrix, including Windows, macOS, ARM64, big-endian, and package consumers.
