# Benchmarks

Two different questions, measured two different ways.

| | Question | Where it runs | Determinism |
|---|---|---|---|
| **Instruction counts** | Did this change make the library do more work? | Every pull request, `benchmark` job | Exact. Two runs are byte-identical |
| **Wall-clock** | How long does an operation actually take? | Locally, results in [`reports/`](../reports) | Medians over many samples |

The gate on CI is the first one. A shared GitHub runner has more timing noise
than the regression worth catching, so gating on wall-clock there would produce
either false alarms or a threshold so loose it catches nothing. Retired
instructions do not have that problem: the same binary on the same input
executes exactly the same number of them.

## SoA batch benchmark

The opt-in `fixedwide_batch_bench` target uses the existing pinned Google
Benchmark dependency. It compares scalar loops with batch sizes 1 and 4 over
identical retained SoA inputs, with vectorization enabled, strict floating-point
flags and no `-march=native`. The scalar baseline is the public API from the
same revision. The four-row exact broadcast path dispatches to AVX2 on capable
x86-64 CPUs; its fallback and the one-row path need no AVX2 support.

```bash
# Build and verify every fixture, without running a timing workload.
scripts/bench_batch.sh --validate-only reports/batch-local

# CPU is an available physical CPU reserved for this measurement.
CXX=g++ BATCH_FILTER='delta.*|anchored_replace.*|publication.*' \
  scripts/bench_batch.sh --cpu "$CPU" reports/batch-local
```

The script generates logs and Google Benchmark JSON locally in the specified
output directory; `reports/batch-local` is ignored by Git. Without `--cpu` or
`BATCH_CPU`, it only builds and validates. CPU affinity does not reserve a core:
coordinate with other workloads and avoid concurrent builds or timing runs.
`BATCH_BACKEND=portable` selects the portable backend, and
`GOOGLE_BENCHMARK_SOURCE` can name an offline checkout of the pinned dependency.

Fixtures cover 64 and 125,000 rows: vector/broadcast products, exact refusals,
wide scalar fallbacks, delta updates with duplicate destinations and sink holes,
anchored FX with wide replacement, cancellation, and per-position publication.
Exact broadcast cases include 32-bit coefficients with small and wide factors,
64-bit and wider coefficients that reject the SIMD certificate, and complete
delta updates with 5% wide-coefficient fallback lanes.
Each fixture verifies outputs and accepted-prefix errors before timing.
Repeated quote fixtures alternate bounded marks/deltas without timed resets.

JSON counters `seconds_per_row` and `seconds_per_op` use CPU time; multiply by
`1e9` for nanoseconds. Refused rows are excluded from completed work.
`mapped_rows` includes speculative lanes; `scalar_api_product_rows` and
`general_product_rows` classify the required prefix outside timing.
`simd_product_rows` counts successful SIMD coefficient certificates in that
prefix; it is zero for one-row or unsupported configurations. Classification
is untimed and does not insert hardware counters into the loop. Modeled
bytes and lane-result storage are estimates, not measured hardware traffic.
`lane_result_bytes` is the generic scheduler's storage bound; the SIMD path can
use less storage or write a fully accepted tile directly to the output.

`benchmarks/batch_codegen.cpp` provides matching probes for assembly and
vectorization diagnostics (`-S`, GCC's `-fopt-info-vec-all`, or Clang's
`-Rpass` / `-Rpass-missed`). Measure complete application loops separately from
these library fixtures.

## Manual Zen 5 workstation measurements

The `Zen 5 SIMD Benchmarks` workflow (`zen5-benchmarks.yml`) is manually
dispatched on a reviewed branch in this repository. Its runner labels are
`self-hosted` and `ryzen-9950x3d`; fork pull requests never trigger it. The
workflow uses read-only repository permissions and does not persist checkout
credentials. Review the selected branch before dispatching it on the personal
workstation.

`ccd=vcache` and `ccd=frequency` discover the larger and smaller L3 cache
groups from sysfs, then pin to one allowed logical CPU on a physical core in
that group. CPU numbering, cache indices and SMT offsets are not assumed.
`ccd=all` is an unpinned diagnostic, not a multicore scaling experiment. The
shared Actions concurrency group serializes these jobs across branches and
compiler choices. It does not exclude unrelated workstation workloads; load
and frequency observations accompany the results, and no global power, SMT or
process settings are changed.

| Input | Purpose |
|---|---|
| `compiler` | Use the installed `clang++` or `g++`; record its actual version and standard library. |
| `extra_flags` | Default `-O3 -march=x86-64`; strict FP flags are always appended. |
| `suite` | `batch`, `runtime`, or the opt-in `zen5` candidate comparison executable. |
| `benchmark_filter` | Select a bounded group of cases; an empty selection fails. |
| `repetitions`, `min_time` | Retain every repetition; defaults are 5 and `0.2s`. |
| `baseline_sha` | Optional immutable commit from `main` history. Blank measures candidate backends in one executable. |

Compare baseline-compatible builds under matching flags. A separate
`-O3 -march=znver5` run measures a deliberately machine-specific build;
it does not establish fallback compatibility on older processors. Compiler
and standard-library requirements are the same as the library's normal build.
Do not install or upgrade dependencies as part of a timing run.

`scripts/zen5_bench.py` builds each revision in a fresh temporary directory,
checks the exact checkout SHA, validates the fixtures, and finishes all
compilation before timing. A five-second stabilization period precedes the
first measurement, with 0.05 seconds of Google Benchmark warm-up per case.
Candidate-only runs interleave benchmark repetitions. Cross-revision runs
alternate baseline/candidate and candidate/baseline order; their filters must
select identical case names. Use `suite=zen5` with no baseline SHA for new
candidate-only fixtures that do not exist in an older revision.

Actions artifacts contain the raw JSON repetitions, build and validation logs,
topology, CPUID/XCR0 and affinity observations, run IDs and SHAs, compile
commands, and Release binaries for inspection of the actual timed code.
The job summary separates CPU and wall time, reports medians and repetition
ranges, and uses completed rows rather than the full input length after a
refusal. These ranges are not per-quote latency percentiles. Check the run
conclusion and `completion.json`; partial artifacts from a failed run are not
successful qualification. Keep generated evidence out of the source diff.

## The regression gate

```bash
scripts/icount.sh                     # measure, CSV to stdout
scripts/icount.sh --update            # re-record the committed baseline
python3 scripts/compare_icount.py benchmarks/baseline/x86_64-gcc-14.csv current.csv
```

`benchmarks/icount.cpp` runs one workload exactly *n* times and nothing else.
`scripts/icount.sh` runs it twice under Valgrind, at *n* and *2n*, and reports
`(I(2n) − I(n)) / n`. The subtraction cancels process startup, dynamic loading,
page faults, the fixture setup and the final print — everything that does not
scale with *n* — so what is left is the marginal cost of one operation.

The tolerance is **1%**, and any workload above it fails the build. Baselines
are per toolchain, because instruction counts are, and are recorded on the
machine the gate runs on rather than on a workstation.

A number that moves for a good reason is re-recorded with `--update` and
explained in the commit message. A number that moves for no reason is a bug.

### Current baseline, GCC 14, x86-64

Instructions per operation. `baseline.empty` is the harness floor: the loop, the
index mask and the sink, which every other row also pays.

| Workload | Fixed64 | Fixed128 | Fixed256 |
|---|---:|---:|---:|
| `add` | 9 | 22 | — |
| `mul` | 37 | 52 | 697 |
| `div` | 50 | 131 | 523 |
| `mul_div` | 27 | 47 | — |
| `quantize` | 72 | 155 | 557 |
| `remainder` | 15 | — | — |

| Text and cross-scale | |
|---|---:|
| `parse.Fixed64` | 511 |
| `to_chars.Fixed64` | 413 |
| `to_chars.Fixed128` | 562 |
| `compare.Price.Rate` | 15 |
| `fixed_cast.Price.MixedFast` | 8 |
| `baseline.empty` | 5 |

### The raw-type floor

The same harness measures plain `std::int64_t`, so the price of the contract is
a number rather than an argument. Subtract `baseline.empty` (5) from each to get
the marginal cost of one operation:

| Operation | fixedwide `Fixed64<12>` | raw `int64_t` | the difference is |
|---|---:|---:|---|
| add | 9 (4 marginal) | 7 (2 marginal) | the overflow check: two instructions |
| multiply | 37 (32) | 7 (2) | widening to 128 bits and dividing by the scale |
| divide | 50 (45) | 8 (3) | the same, plus the rounding mode |
| store 8 bytes | 115 (110) | 59 (54) | pinning the byte order |

Wall-clock cannot separate these — they are all a fraction of a nanosecond — and
in a timed loop `to_bytes` and `memcpy` measure the same 0.19 ns, because the
extra work is cheap ALU that pipelines away at this size. That is a real
disagreement between the two measurements, and both are reported:
`reports/BENCHMARK_COMPETITORS.md` has the timings, this table has the work.

## The cost of a mixed operation

The original compile-time bounds select unconditional native arithmetic when
the operand widths and scales prove that all intermediates fit. Since 0.6.10,
`mul_to`, `add_to` / `sub_to`, and `fixed_cast` also try checked signed 128-bit
arithmetic when their actual values fit, even if their types' worst cases do
not. Rescaling still rounds exactly once. An intermediate overflow declines
the attempt, allowing the general rational kernel in `src/mixed.cpp` to finish
the calculation without prematurely rejecting a representable result.

Same-scale add/sub uses native or 256-bit limb arithmetic directly, including
wide intermediate cancellation. A Fixed256 value whose upper limbs extend bit
127's sign can use native narrowing. These runtime paths are disabled by
`FIXEDWIDE_FORCE_PORTABLE`; existing compile-time paths and public contracts
are unchanged. `div_to` and `mul_div_to` retain their previous dispatch.

The Google Benchmark runtime-path target covers realistic magnitudes,
successful fallbacks, overflow results, and the complete wide replacement chain:

```bash
CXX=g++-14 scripts/bench_runtime_narrow.sh reports/runtime-narrow-local
```

Before timing, every workload compares 65,536 complete results, including error
codes, with direct calls to the general kernels. Google Benchmark controls the
timing loop, repetitions, random interleaving, result barriers, and statistics.
Raw repetitions and aggregate results are retained as JSON. The target is
opt-in, so the instruction-count gate does not acquire a Google Benchmark
dependency. Build details, results, and the optional reciprocal-division
decision are in [reports/runtime-narrow](../reports/runtime-narrow/README.md).

Historical instruction-count workloads (their type/scale combinations are
unchanged):

| | native | general, 64-bit operands | general, 256-bit operands |
|---|---:|---:|---:|
| `mul_to` | 53 | 416 | 1764 |
| `div_to` | 67 | 729 | 2570 |
| `add_to` | 57 | — | 1228 |

This used to be a cliff rather than a gradient: **every** operation that missed
the native path cost about 8500 instructions, because the kernel did all of its
arithmetic in sixteen 64-bit limbs regardless of the operands. A mixed multiply
of two `Fixed64` values into a `Fixed128` needs roughly eighty bits and was
running a 16x16 schoolbook multiply — 256 partial products, four of which were
not multiplying by zero.

The kernel now computes an upper bound on the bits it needs and dispatches to
the smallest of four tiers (128 / 256 / 512 / 1024 bits). Same code, same
results — 1,017,500 differential checks against Boost.Multiprecision, unchanged,
across the native and portable backends — for 8561 → 416 instructions on the
64-bit case and 8377 → 729 on the divide.

In wall-clock, timing the identical operation against the old kernel and the
new one on the same machine:

| `Fixed64<4>` × `Fixed64<8>` → `Fixed128<18>` | before | after |
|---|---:|---:|
| `mul_to` | 332.67 ns | **19.69 ns** |
| `div_to` | 590.95 ns | **35.36 ns** |
| `mul_to` on the native path, for reference | 1.45 ns | 1.47 ns |

The native row is the control: it does not go through this kernel and does not
move.

The general path still constructs an exact rational and performs limb division;
the native path uses exact native rescaling when needed. Keeping the
destination scale close to the operands' scales increases the range of values
that fit the native intermediates, and
[api_reference.md](api_reference.md#mixed-scale-operations) says where that
boundary is. All three tiers are in the baseline above, so none of them can
regress unnoticed.

## Build flags: do not reach for `-march=native`

Measured, because it is the opposite of what most people assume.

| `Fixed256<38>`, GCC 14 | `mul` | `div` |
|---|---:|---:|
| `-march=x86-64` (default) | **53.4 ns** | **50.5 ns** |
| `-march=x86-64-v3` (BMI2 + AVX2) | 59.1 ns | 54.5 ns |
| `-march=x86-64-v3`, vectoriser off | 57.2 ns | 54.4 ns |
| `-march=x86-64`, vectoriser off | 58.6 ns | 59.4 ns |

Raising `-march` makes wide arithmetic about **10% slower**, reproducibly
(run-to-run spread is under 0.2 ns against a 5.6 ns effect). GCC does emit
`mulx`, `adcx` and `adox` at `-march=x86-64-v3` — 173 of them, against none at
the default — and the instruction count does fall slightly, from 697 to 675 per
`Fixed256` multiply. It still runs slower.

About half of the loss is the auto-vectoriser: AVX2's 256-bit lanes are a poor
fit for a four-limb loop, where the shuffling costs more than the parallelism
returns. The other half appears to be scheduling and register pressure around
the wider instruction forms.

The last row is the useful control: at the default `-march`, turning the
vectoriser **off** is worse still. SSE2's two lanes do fit a four-limb loop. So
the library does not disable vectorisation for its own sources, even though the
benchmark harnesses disable it for their measurement loops — those are different
questions and the answers differ.

**This is why there is no hand-written BMI2/ADX path.** The compiler already
emits those instructions when permitted, and permitting it makes this library
slower. Hand-writing them would be a large amount of unsafe code chasing a
number that measurement says goes the wrong way.

## Wall-clock, against the 0.4 release

The paired benchmark builds the byte-identical `benchmarks/rounding_bench.cpp`
against both this tree and an untouched 0.4 checkout, with the same compiler and
flags, core-pinned and interleaved, medians of 27 samples per row.

```bash
BASELINE_SRC=/path/to/fixedwide-0.4.0 ./scripts/paired_bench.sh
BASELINE_SRC=/path/to/fixedwide-0.4.0 ./scripts/docker_bench.sh   # in a pinned container
```

It cannot run on CI: it needs a 0.4 source tree, which is not in this
repository and cannot be, since vendoring it would stop it being the baseline.
Results and raw samples are in
[`reports/BENCHMARK_VS_0_4.md`](../reports/BENCHMARK_VS_0_4.md) and
`reports/raw/`.

Clang only. The 0.4 baseline hard-requires C++ `_BitInt(256)` and will not
configure without it, and GCC does not implement `_BitInt` in C++, so no paired
GCC row can exist.

## Against other libraries

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DFIXEDWIDE_BUILD_BENCHMARKS=ON -DFIXEDWIDE_BUILD_COMPETITOR_BENCH=ON
cmake --build build --target fixedwide_competitor_bench
./build/benchmarks/fixedwide_competitor_bench
```

CNL and fpm are fetched at pinned tags by `benchmarks/competitors.cmake`, so
this reproduces from a clean checkout with nothing vendored. Every timed loop's
output is validated against an independent oracle *outside* the timed region,
and the run refuses to print results if a check fails. The nightly workflow
rebuilds it so the comparison table in the README cannot silently rot.

Rows are grouped by semantic class. Cost may be compared across classes;
correctness may not — a binary fixed-point multiply and a decimal fixed-point
multiply are not the same operation, and only one of them can represent `0.01`.
Full table and the honest reading of it:
[`reports/BENCHMARK_COMPETITORS.md`](../reports/BENCHMARK_COMPETITORS.md).

## The other harnesses

| Executable | What it is for |
|---|---|
| `fixedwide_icount` | The CI gate above |
| `fixedwide_rounding_bench` | The 100-row paired suite against 0.4 |
| `fixedwide_competitor_bench` | CNL, fpm, Boost.Decimal, `double` |
| `fixedwide_bench` | Core operations against the 0.4 baselines |
| `fixedwide_numeric_bench` | Edge values and large powers |
| `fixedwide_memory_bench` | Allocation-free verification (Linux only) |
| `fixedwide_mixed_bench` | Cross-scale wall-clock |

## Accumulated checked chains

`fixedwide_checked_chain_bench` compares explicit `std::expected` early-return
chains, `Checked` accumulation and caller-proved unchecked arithmetic on the
same validated runtime inputs. It reuses `measurement.hpp`, checks all outputs
before timing, rotates policy order across trials and retains raw repetitions.
There are Fixed128 eight-add/sub, Fixed256 mixed-product/divisor, and floating
input plus mixed equity/margin workloads. Unchecked floating ingress has no
separate API, so that row retains the checked conversion with prevalidated
success rather than benchmarking a different numeric conversion.

```sh
python3 scripts/checked_chain_evidence.py --compiler g++-14 \
  --output /tmp/checked-gcc --cpu 2
python3 scripts/checked_chain_evidence.py --compiler clang++-20 \
  --output /tmp/checked-clang --cpu 2
```

The script retains machine/compiler/flags, compile commands, exact git state,
CSV samples and pooled medians, and ordinary callable formula disassembly.
Its static branch/call/stack-reference counts include cold paths and are not
executed instruction counts. The [0.6.8 evidence report](../reports/CHECKED_0_6_8.md)
records numerical coverage separately from performance acceptance. Eager
post-failure execution is intentionally different from early return. Existing
arithmetic kernels remain unchanged.

The residual happy-path overhead is accepted. On Clang 20 the eight-operation
Fixed128 add/sub chain measures 3.586 ns through `Checked` against 3.381 ns for
explicit `expected` checks, about +0.2 ns per chain. On GCC 14 the Fixed256
multiply chain measures +5.3%, below this setup's run-to-run variation: in the
same table unchecked (247 ns) measured slower than explicit (224 ns). The
instruction-count gate tracks the Clang 20 add/sub chain as
`checked.add8.Fixed128`, so a compiler that undoes the `record()` fence fails CI.
