# Full-API guard and call-layout investigation

The initial Google Benchmark campaign exposed a limitation of the standalone
[guard experiment](../guard-experiment/README.md): Clang outlined the larger
`try_mul` helper in the real API. The nested `optional<expected>` result crossed
an ABI boundary, its state was stored in memory, and the helper retained a
runtime rounding parameter even when the caller supplied a constant.

These screens use the **complete, unchanged operation benchmark**, comparing
public operations with direct general-kernel calls on the same immutable
fixtures. They explain the selected implementation; the final campaign in
[`../google-benchmark/`](../google-benchmark/) is the performance report for the
final merged source.

## Findings and selection

Forcing only `try_mul` inline caused Clang to outline `mul_to` instead. Forcing
both boundaries removed that call overhead, but the manual product still lost
the known 64-bit width of the Price operand. Preserving raw widths eliminated
its unnecessary high-limb work and size branch. Checking the high cross product
before multiplying the low limbs also made rejected products cheaper.

Moving the original general-kernel call and result conversion into an outlined
fallback adapter removed the general path's large stack frame from the native
attempt. Price then inlined automatically; the 128-by-128 FX operation still
benefited from an explicit public `mul_to` inline hint. The selected shape is
`outline-inline`: type-aware magnitudes, cross product first, the fallback
adapter, and both inline boundaries. In production, the new hints and adapter
are restricted to Clang's native backend, and the adapter dispatch is limited to
the runtime-eligible operand/destination widths.

One final candidate added a conservative signed-high-limb multiplication check
before magnitude formation for a statically narrow operand. Its measured fast
path was slower and its successful fallback did not improve, so that extra
check was rejected. No additional guard redesign followed this screen.

| Variant | Price realistic | FX realistic | Price successful-fallback overhead | FX successful-fallback overhead |
| --- | ---: | ---: | ---: | ---: |
| `current` | 12.471 ns | 19.313 ns | +6.908 ns | +3.386 ns |
| `inline` | 9.780 ns | 17.652 ns | +1.763 ns | +10.118 ns |
| `inline-builtin` | 7.564 ns | 20.763 ns | +4.516 ns | +3.810 ns |
| `full-inline` | 8.936 ns | 16.646 ns | +5.278 ns | +3.112 ns |
| `full-inline-builtin` | 7.196 ns | 19.127 ns | +5.279 ns | +5.932 ns |
| `typeaware` | 5.552 ns | 17.088 ns | +4.952 ns | +3.029 ns |
| `outline` | 5.182 ns | 16.991 ns | +3.777 ns | +3.249 ns |
| **`outline-inline`** | **5.080 ns** | **16.927 ns** | **+2.962 ns** | **+0.294 ns** |
| `preflight` | 5.516 ns | — | +5.643 ns | — |

Values are medians of Google Benchmark's `cpu_time`. Each iteration performs
one complete operation, so these are already nanoseconds per operation:
**do not divide by 1,024**, as in the separate helper experiment. An overhead is
the public median minus its general-kernel median from the same screen. These
shared-host samples contain visible variation; the first three variants were
shorter exploratory screens, and individual medians are not precision claims.
The selected screen still measured about 3 ns of successful Price fallback
overhead, rather than establishing a universal 1 ns guard cost.

Every run first validated 983,040 complete results across all fixtures,
including error codes. Raw repetitions, aggregates, and validation output are
retained under [`results/`](results/). The `preflight` screen timed Price only;
its change is discarded at compile time for two 128-bit operands.

## Exact source and run settings

The benchmark source was identical throughout, with SHA-256:

```text
8099c949162231e22f65180d862cada4c7fe83ecfb73380507324d1b40da0810  benchmarks/runtime_narrow_bench.cpp
```

The baseline headers are preserved by the
[initial campaign source patch](../google-benchmark-initial/initial-inputs.patch)
against commit `544c4ab09735cc809f83850e9a380211bfa4a1bc`. Their SHA-256 hashes are:

```text
9ce414c2454400468688b8d736bff54601704a4f45234980708afacbfc04c3ef  include/fixedwide/detail/mixed_native.hpp
78a512be1f4b3c423aa886d59533c210ffa48572b800969847ef0035418cadcd  include/fixedwide/mixed.hpp
```

Each file in [`variants/`](variants/) is an independent patch against those
baseline headers. `current` applies no additional patch. The patches have zero
context lines, so apply them with `git apply --unidiff-zero`. Applying each patch
was checked to reproduce the exact measured header hashes recorded in
[`variants.json`](variants.json); the patches preserve the experimental source
bytes without formatting edits.

| Variant | Change from baseline |
| --- | --- |
| `inline` | Force internal `try_mul` inline. |
| `inline-builtin` | The same, using the signed product builtin. |
| `full-inline` | Force internal `try_mul` and public `mul_to` inline. |
| `full-inline-builtin` | Both inline hints, using the signed product builtin. |
| `typeaware` | Both inline hints; preserve raw widths and check the high cross product first. |
| `outline` | Type-aware product and internal inline hint; outline the general fallback, omit the public hint. |
| `outline-inline` | The same fallback adapter, retaining the public inline hint. |
| `preflight` | Add the rejected signed-high-limb check to `outline-inline`. |

The first three screens used five repetitions, a 0.05-second minimum, and
0.01 seconds of warmup. The remaining screens used nine repetitions, a
0.1-second minimum, and 0.02 seconds of warmup. All used random interleaving,
CPU affinity 0, `--benchmark_report_aggregates_only=false`, and
`--benchmark_display_aggregates_only=true`. The compiler was Ubuntu Clang
19.1.1 (`1ubuntu1~24.04.2`), with the same Google Benchmark 1.9.4 library and
unchanged compiled general kernels throughout. No `-march` or CPU-frequency
control was used. Binary hashes and per-variant filters/settings are in
`variants.json`; Google Benchmark records the host and timestamps in each JSON.
The `current` control's binary hash comes from the preserved initial campaign
environment, because that build path was subsequently reused for the final
executable. The isolated candidate binaries had separate temporary paths.

The final production merge additionally gates the hints/adapters by compiler
and backend, restricts fallback adapters to the newly eligible widths, uses
`sizeof(std::uint64_t)` instead of the experimental literal `8`, and adds
comments/formatting. These refinements preserve the selected arithmetic for
the measured Clang/native types. The final campaign validates and measures the
actual merged source separately.

## Reproducing a screen

The following rebuilds the baseline compiled kernels, then applies the selected
header-only variant. Use fresh temporary paths. Google Benchmark's pinned
source/library setup is documented in the
[guard experiment](../guard-experiment/README.md#sources-and-reproduction).

```bash
evidence="$PWD/reports/runtime-narrow/inlining-experiment"
repro_checkout=/tmp/fixedwide-layout-repro
repro_build=/tmp/fixedwide-layout-library
gb_source=/tmp/fixedwide-google-benchmark
gb_build=/tmp/fixedwide-google-benchmark-build
variant=outline-inline

git worktree add --detach "$repro_checkout" 544c4ab09735cc809f83850e9a380211bfa4a1bc
git -C "$repro_checkout" apply --unidiff-zero "$evidence/../google-benchmark-initial/initial-inputs.patch"
cmake -S "$repro_checkout" -B "$repro_build" -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++-19 -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS=-Werror -DFIXEDWIDE_BUILD_TESTS=OFF \
  -DFIXEDWIDE_BUILD_EXAMPLES=OFF
cmake --build "$repro_build" --target fixedwide
git -C "$repro_checkout" apply --unidiff-zero "$evidence/variants/$variant.patch"

clang++-19 -DBENCHMARK_STATIC_DEFINE \
  '-DFIXEDWIDE_GOOGLE_BENCHMARK_VERSION="1.9.4"' \
  -I"$repro_checkout/include" -isystem "$gb_source/include" \
  -Werror -O3 -DNDEBUG -std=c++23 -fno-vectorize -fno-slp-vectorize \
  -ffp-contract=off -fno-fast-math \
  "$repro_checkout/benchmarks/runtime_narrow_bench.cpp" \
  "$repro_build/libfixedwide.a" "$gb_build/src/libbenchmark.a" \
  -lrt -o /tmp/fixedwide-layout-screen

taskset -c 0 /tmp/fixedwide-layout-screen \
  '--benchmark_filter=mul_(price|fx)\.' \
  --benchmark_min_time=0.1s --benchmark_repetitions=9 \
  --benchmark_min_warmup_time=0.02 --benchmark_enable_random_interleaving=true \
  --benchmark_report_aggregates_only=false --benchmark_display_aggregates_only=true \
  --benchmark_out_format=json --benchmark_out=/tmp/fixedwide-layout-screen.json
```

For another variant, start from a fresh baseline and use its corresponding
patch/settings. The two earliest manually built variants, `inline` and
`inline-builtin`, linked `-lpthread` in place of `-lrt`; all other screens used
`-lrt`, matching the CMake-built control. The rejected preflight's filter was
`mul_price\.`. No binaries or large disassembly files are stored in this report.
