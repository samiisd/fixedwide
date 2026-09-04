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
| `mul` | 37 | 52 | 701 |
| `div` | 50 | 131 | 523 |
| `mul_div` | 27 | 47 | — |
| `quantize` | 72 | 155 | 557 |
| `remainder` | 15 | — | — |

| Text and cross-scale | |
|---|---:|
| `parse.Fixed64` | 511 |
| `to_chars.Fixed64` | 476 |
| `to_chars.Fixed128` | 568 |
| `compare.Price.Rate` | 15 |
| `fixed_cast.Price.MixedFast` | 8 |
| `baseline.empty` | 5 |

## The mixed path cliff

The one performance surprise in this API, stated rather than hidden.

`mul_to`, `div_to` and `add_to` have two implementations. When the aligned
intermediate fits 126 bits, `detail::mixed_native` does the whole thing in
`__int128`. When it does not, a general kernel works in 1024-bit limbs. The two
differ by about **160x**:

| | native | general |
|---|---:|---:|
| `mul_to` | 53 | 8561 |
| `div_to` | 67 | 8377 |
| `add_to` | 57 | — |

Which one you get is decided at compile time by the widths and the scales, in
`include/fixedwide/detail/mixed_native.hpp`:

- `mul_to<Dest>`: native when both operands are ≤ 64 bits, `Dest` ≤ 128 bits,
  and either `Dest`'s scale is below the sum of the operands' scales, or the
  128-bit product plus the widening still fits 127 bits.
- `div_to<Dest>`: native when both operands are ≤ 64 bits, `Dest` ≤ 128 bits,
  and the numerator plus `10^(Dd + Db − Da)` fits 126 bits.

In practice: `mul_to<Fixed128<6>>(Fixed64<4>, Fixed64<8>)` is native;
`mul_to<Fixed128<18>>` of the same operands is not. If a mixed operation is on
a hot path, keep the destination scale close to what the operands need. Both
paths are in the baseline above, so neither can regress unnoticed and the cliff
stays visible.

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
