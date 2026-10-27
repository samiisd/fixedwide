# Native guard selection experiment

This small Google Benchmark experiment guided the initial implementation of the
runtime signed-128 multiplication guard. It measures out-of-line helper calls,
including the loop, result handling, and barriers. The complete operation
benchmarks in the parent report are the performance evidence for the library.
The [later full-API measurements](../inlining-experiment/README.md) also required
preserving raw operand widths and refining the inline and fallback boundaries;
this helper-only experiment does not establish the final public-API cost.

This experiment supported a compiler-specific choice: retain GCC's signed
overflow builtin and use a partial unsigned product on Clang. Clang 19's
generated signed-builtin helper contained eight multiply instructions and
signed 256-bit correction. The measured partial-product candidate used one
multiply when both magnitudes fit 64 bits, two when exactly one was wider, and
rejected two wider operands before multiplying. GCC 14 already generated an
efficient narrow case for its signed builtin.

## Recorded results

The unmodified Google Benchmark JSON files are [clang19.json](clang19.json)
and [gcc14.json](gcc14.json). Each benchmark iteration processes 1,024 pairs.
Every value below is the median `cpu_time` in nanoseconds **divided by 1,024**.
Five repetitions were run with random interleaving and a 0.05-second minimum.
The exploratory command used `--benchmark_report_aggregates_only=true`, so
these files retain mean, median, standard deviation, and coefficient of
variation; individual repetition records were not saved. The final whole
operation benchmark retains individual repetitions.

| Compiler | Dataset | Signed builtin | Split unsigned builtin | Partial product |
| --- | --- | ---: | ---: | ---: |
| Clang 19.1.1 | 0: small FX-like magnitudes | 4.445 ns | 3.875 ns | 3.709 ns |
| Clang 19.1.1 | 1: price-like magnitudes | 4.434 ns | 4.571 ns | 3.804 ns |
| Clang 19.1.1 | 2: two wide operands | 4.295 ns | 2.325 ns | 1.848 ns |
| Clang 19.1.1 | 3: one wide operand | 4.363 ns | 3.717 ns | 2.794 ns |
| GCC 14.2 | 0: small FX-like magnitudes | 2.010 ns | 4.320 ns | 3.997 ns |
| GCC 14.2 | 1: price-like magnitudes | 2.958 ns | 5.287 ns | 4.478 ns |
| GCC 14.2 | 2: two wide operands | 2.669 ns | 3.497 ns | 2.498 ns |
| GCC 14.2 | 3: one wide operand | 3.925 ns | 6.071 ns | 3.528 ns |

These are **within-compiler comparisons**. Each helper reads the same immutable
dataset within its compiler's executable. The archived exploratory generator
contains two RNG calls in bitwise expressions for the wide datasets; GCC and
Clang can evaluate them in different orders. Its exact wide samples therefore
must not be used to compare the compilers. The main operation benchmark
sequences its RNG draws explicitly. The small datasets are representative
helper inputs, not the complete financial workload distributions.

The partial product improves all four Clang cases, while it roughly doubles
the GCC small-input helper cost. This supports the internal `__clang__`
selection rather than a common manual implementation.

The GCC JSON also records the constant-scale guard. For multiplication by
`10^12`, its checked builtin measured 1.657 ns on dataset 0 and 3.545 ns on
dataset 2; checking the compile-time input bounds before multiplying measured
1.718 ns and 1.919 ns respectively. The bound is
`min128 / factor <= value <= max128 / factor`; the positive factor and signed
division's truncation toward zero make both bounds exact. A rejected input
never needs a multiplication.

## Sources and reproduction

[benchmark.cpp](benchmark.cpp) and [candidates.cpp](candidates.cpp) preserve
the measured source, with only whitespace changed by the repository's pinned
clang-format 22.1.8. No semantic edits or post-format reruns were made. The
original pre-format SHA-256 hashes were:

```text
e4478c4c338933b57dff9ac70d72bd9423e314b1484e8846aa2e182e41dd38b1  benchmark.cpp
577da8f0a8101e359a9b06f784b58db9b061fcfe64c3d157659f52b789fdec5d  candidates.cpp
```

The host was Linux x86-64, with Ubuntu Clang 19.1.1
(`1ubuntu1~24.04.2`) and GCC 14.2.0 (`14.2.0-4ubuntu2~24.04.1`). Both
executables linked the same Google Benchmark 1.9.4 static library, built with
GCC 14 in Release mode (`-O3 -DNDEBUG`), from commit
`eddb0241389718a23a42db6af5f0164b6e0139af`. Its SHA-256 was
`cbd1f97fe375f90be1b95928bc4fdeb633fad562c50ac6bc6b71f8499b158f6a`.
Google Benchmark records the host, caches, load, and timestamps in each JSON.
Both runs were pinned to CPU 0; no CPU-frequency control or `-march` was used.

From the repository root, use a checkout of that exact Google Benchmark
commit. The original library build configuration was:

```bash
gb_source=/tmp/fixedwide-google-benchmark
gb_build=/tmp/fixedwide-google-benchmark-build
cmake -S "$gb_source" -B "$gb_build" -G Ninja \
  -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_BUILD_TYPE=Release \
  -DBENCHMARK_ENABLE_TESTING=OFF -DBENCHMARK_ENABLE_GTEST_TESTS=OFF \
  -DBENCHMARK_ENABLE_INSTALL=OFF -DBENCHMARK_ENABLE_WERROR=OFF
cmake --build "$gb_build" --target benchmark
```

The following reproduces the compiler flags, separate translation units,
filters, and timing settings used for the saved final files. The original
source paths were `/tmp/fixedwide-mul-candidate-bench.cpp` and
`/tmp/fixedwide-mul-candidates.cpp`; the archived paths below contain the same
code after formatting.

```bash
experiment=reports/runtime-narrow/guard-experiment
clang++-19 -std=c++23 -O3 -DNDEBUG -fno-vectorize -fno-slp-vectorize \
  -I"$gb_source/include" "$experiment/benchmark.cpp" "$experiment/candidates.cpp" \
  "$gb_build/src/libbenchmark.a" -lpthread -o /tmp/fixedwide-guard-clang
g++-14 -std=c++23 -O3 -DNDEBUG -fno-tree-vectorize -fno-tree-slp-vectorize \
  -I"$gb_source/include" "$experiment/benchmark.cpp" "$experiment/candidates.cpp" \
  "$gb_build/src/libbenchmark.a" -lpthread -o /tmp/fixedwide-guard-gcc

taskset -c 0 /tmp/fixedwide-guard-clang \
  '--benchmark_filter=product<(signed_builtin|split_unsigned|partial_product)>.*' \
  --benchmark_min_time=0.05s --benchmark_repetitions=5 \
  --benchmark_enable_random_interleaving=true --benchmark_report_aggregates_only=true \
  --benchmark_out=/tmp/fixedwide-guard-clang.json --benchmark_out_format=json
taskset -c 0 /tmp/fixedwide-guard-gcc \
  '--benchmark_filter=product<(signed_builtin|split_unsigned|partial_product)>.*|scale.*' \
  --benchmark_min_time=0.05s --benchmark_repetitions=5 \
  --benchmark_enable_random_interleaving=true --benchmark_report_aggregates_only=true \
  --benchmark_out=/tmp/fixedwide-guard-gcc.json --benchmark_out_format=json
```

The production helper's exactness, signed extrema, carry boundaries, and
fallback/error behavior are checked by the independent arithmetic suites;
this experiment is not the correctness oracle.
