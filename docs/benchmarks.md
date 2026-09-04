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

`mul_to`, `div_to` and `add_to` have two implementations. When the aligned
intermediate fits 126 bits, `detail::mixed_native` does the whole thing in one
`__int128`. Otherwise a multi-limb kernel in `src/mixed.cpp` evaluates the exact
rational and rounds once.

Instructions per operation:

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

What remains is real work: the general path evaluates an exact rational and
performs a division that the native path avoids entirely. If a mixed operation
is on a hot path, keeping the destination scale close to what the operands need
still keeps you on the native side, and
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
