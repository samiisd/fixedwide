# Exact decimal exponent versus floating-point log10

Measured 2026-10-21 in GitHub Actions CI run [39653814479](https://github.com/samiisd/fixedwide/actions/runs/39653814479), job [119408700155](https://github.com/samiisd/fixedwide/actions/runs/39653814479/job/119408700155).

## Provenance

- Branch source commit: `f9575e4b42a9f7e85482dc3c1a5603f30cbaa5af`.
- Measured PR merge checkout: `b51af43b43d984906a2d1130703249895b588133` (base `84f887d454dda71bdb566fab20374b6c3dea05a6`).
- Artifact: `instruction-counts`, ID `12369999488`, SHA-256 `4c61a7297ac64262d9ef98a219bfa4fdac3c6d2d3664a6b89e3f448e70f74cb0`.
- AMD EPYC 7763 host, Ubuntu 24.04 x86-64 guest, GCC 14.2.0, glibc 2.39; CPU affinity 0, shared virtual host, no frequency lock.
- Release `-O3 -DNDEBUG`, scalar loops, fast-math disabled. `long double`: 16-byte storage, radix 2, 64 significand bits (x87 extended precision).
- 262,144 operations per sample, 11 repetitions, 4,096 warmup operations. Runtime-generated 1,024-entry shuffled corpora.

The CSV files here preserve all seven methods and all eleven raw samples for the log-uniform distribution, on both backends. They are selected rows from the original artifact, not a second run. The artifact additionally contains all uniform and decimal-boundary timing rows, full environment metadata, header-only test output, generated assembly, binaries' hashes, and `source.tar.gz`. Its working tree listed only untracked CI outputs/install scripts, not tracked source changes.

Subsequent formatting corrections change whitespace, not the measured implementation. These numbers remain attributed to the source/merge hashes above, not to an unmeasured later commit. Reproduction and the complete method are in [docs/log10.md](../../docs/log10.md).

## Native backend: log-uniform corpus

Median nanoseconds per operation, including indexing, the scalar loop and the observable-result sink. **These are throughput measurements, not instruction or dependency-chain latency.**

| Source type | Exact fixedwide exponent | double log only | long double log only | double log + floor, preconverted | long double log + floor, preconverted | double conversion + log + floor | long double conversion + log + floor |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Fixed32<4> | 2.806 | 8.312 | 31.826 | 12.126 | 33.448 | 18.881 | 87.501 |
| Fixed64<8> | 2.188 | 8.271 | 31.957 | 12.066 | 33.770 | 21.063 | 87.845 |
| Fixed128<12> | 2.769 | 8.171 | 31.148 | 11.971 | 172.994 | 23.164 | 98.530 |
| Fixed256<18> | 3.705 | 8.315 | 31.056 | 12.289 | 173.876 | 53.140 | 671.183 |

For Fixed64<8>, the exact query took 3.78x less time than double log-only and 14.61x less than long-double log-only. Starting with the same fixed-point input and obtaining an integer exponent, the approximate floating routes took 9.63x and 40.15x as long, respectively. Ratios are calculated from the unrounded medians in `samples-native.csv`.

The contracts are different: **fixedwide returns an exact integer exponent, while log-only returns a fractional logarithm**. This is not evidence that fixedwide computes a full fractional logarithm faster. Floating log-plus-floor is closer in intent but still not an exact replacement.

## Forced-portable backend: log-uniform corpus

| Source type | Exact fixedwide exponent | double log only | long double log only | double conversion + log + floor | long double conversion + log + floor |
| --- | ---: | ---: | ---: | ---: | ---: |
| Fixed32<4> | 2.805 | 8.472 | 31.978 | 18.563 | 225.397 |
| Fixed64<8> | 2.806 | 8.403 | 32.111 | 22.189 | 85.945 |
| Fixed128<12> | 2.698 | 8.190 | 30.897 | 24.084 | 86.141 |
| Fixed256<18> | 3.584 | 8.379 | 31.007 | 53.129 | 616.828 |

The unexpectedly expensive long-double floor paths are retained rather than discarded. Full fractional log-only is consistently much cheaper than some log-plus-floor paths in this build. These measurements do not isolate whether that difference comes from inlining, register allocation, FP-control-word handling, or another code-generation detail. Do not present those large ratios as a property of log10l itself. Fixed256 end-to-end timing also includes the library's existing legacy floating conversion, not just log10.

The implementation was checked in the generated assembly: fixedwide uses integer operations and exact table comparisons; the floating wrappers call libm `log10`/`log10l`. Small differences between native and portable timings of otherwise equivalent narrow operations should not be attributed to an algorithmic backend advantage. Re-run on the deployment CPU; the methods run sequentially rather than as a randomized paired trial.

## Correctness, independently of speed

Each backend's header-only, exceptions-disabled test executable reported:

```text
log10_floor: 364697 checks OK
```

The benchmark also validates every fixedwide result against an independent integer oracle before timing. Both backends produced identical mismatch counts for the approximate floating routes. The deliberately adversarial decimal-boundary corpus has 1,024 entries including repeated power-of-ten neighbours; these are not real-world error probabilities or counts of distinct mathematical inputs.

| Type | fixedwide mismatches | double log + floor mismatches | long double log + floor mismatches |
| --- | ---: | ---: | ---: |
| Fixed32<4> | 0 | 0 | 36 |
| Fixed64<8> | 0 | 72 | 75 |
| Fixed128<12> | 0 | 201 | 228 |
| Fixed256<18> | 0 | 263 | 288 |

`accuracy.csv` records all distributions. Extra significand bits do not guarantee that flooring a floating-point logarithm recovers the exact decimal exponent: conversion and logarithm rounding both matter at discontinuities. No correction is silently applied to those baselines.

## Existing regression gate

The GCC 14 and Clang 20 instruction-count comparisons passed in the same CI job. Historical baseline files, existing workloads, and the 1% threshold were not changed. Wall-clock results above are descriptive and never replace that gate. For the current PR's full test/coverage/platform status, consult its checks rather than treating this measurement snapshot as a permanent all-green claim.
