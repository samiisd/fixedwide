# fixedwide 0.5.0-alpha.4 — status

**Version**: 0.5.0-alpha.4
**Standard**: C++23
**Disposition**: pre-1.0. The performance gate does not pass; see below.

This file records what has been executed. Nothing here is claimed unless a
command in `scripts/` produced it and the output is retained under `reports/`.

## What alpha.4 changed

alpha.4 is a narrow performance, portability and reproducibility pass on top of
alpha.3. The type system is unchanged.

| Audit finding | What was done |
|---|---|
| 75/100 rows slower than 0.4 on Clang 17 | Root-caused and partly fixed. See `reports/BENCHMARK_VS_0_4.md`. |
| Performance reported from one compiler | The gate now runs on Clang 17, 18 and 22, with a measured noise floor. |
| `arithmetic.hpp` +68.9% compile time | Now +37.5% on this host, measured with instantiation included. |
| Competitor suite not reproducible | Rebuilt: pinned FetchContent, medians, validated outputs, semantic classes. |
| MSVC claim unsupported | The header-level blockers are removed. Still not executed; marked as such. |
| AArch64 claim unsupported | Executed on real hardware: 17/17 on a Pixel 6. |
| `_BitInt` in public headers | The conversion overloads are gone from `wide.hpp`. Two Clang-only inline fast paths still use `_BitInt(256)` as a local computation type; that is stated rather than denied. |
| Stale release metadata | Version, status, changelog and evidence regenerated from this commit. |

## Executed configurations

`scripts/verify_all.sh` produces `reports/EXECUTION_MATRIX.csv`. Every row is
labelled `executed-pass`, `executed-fail`, `configured-not-executed`,
`not-configured` or `not-applicable`. A platform that is not `executed-pass`
must not be described as supported.

Executed and passing on this host:

* Linux x86-64, Clang 22 and GCC 16, Release — 23/23
* Linux x86-64, Clang 17 in a pinned `ubuntu:24.04` container — 23/23
* Forced portable backend — 23/23
* Portable backend with `__SIZEOF_INT128__` undefined — 23/23
* ASan + UBSan over both the native and the portable backend — 23/23, no diagnostics
* Shared library — 23/23
* No-exceptions / no-RTTI library build
* Install plus an external `find_package` consumer
* **Linux AArch64 on real hardware** (Pixel 6, static cross build) — 17/17

Configured but not executed here: Windows MSVC, Windows clang-cl, macOS arm64,
macOS x86-64, Linux AArch64 CI. Not configured: Windows ARM64, big-endian.

## Performance

Paired against the untouched 0.4 release, byte-identical benchmark source,
core-pinned and interleaved, medians of 27 samples. Full per-row output in
`reports/BENCHMARK_VS_0_4.md` and `reports/raw/`.

| Clang 17, versus 0.4 | alpha.3 | alpha.4 |
|---|---:|---:|
| rows faster than 0.4 | 42 | 43 |
| rows >5% slower | 32 | 27 |
| rows >10% slower | 22 | 16 |
| rows >25% slower | 15 | 10 |
| worst row | +161.0% | +66.9% |

**This does not meet the release gate.** The remaining gap is the wide
`Fixed128` paths, and the cause is structural rather than an oversight: 0.4 was
a single-scale library whose kernels saw the scale as a compile-time constant.
The alternative — one compiled kernel per decimal count — was implemented and
measured, and it was slower.

## Known open items

1. Wide `Fixed128` multiply, divide and `mul_div` remain up to +67% on Clang 17.
2. Decimal parsing is about 4x slower than `std::from_chars` on a `double`, and
   6-11% slower than 0.4.
3. `arithmetic.hpp` costs about 37% more to include than 0.4's.
4. Windows and macOS are configured in CI but have never been executed.
5. Big-endian byte order is implemented in `binary.hpp` but never executed.
