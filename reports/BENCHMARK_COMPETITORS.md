# Competitor benchmark

These are independent-operation throughput microbenchmarks, not dependency-chain latency. Decimal multiplication/division fixtures are deliberately exact at the selected scale; they do not measure the general cost of inexact nearest-even rounding. Decimal preflight checks compare raw values or canonical fixed-format text against integer-derived expectations. Binary fixed-point and double checks use documented floating tolerances; cpp_dec_float_50 division uses a 1e-45 residual plus exact four-place text. CNL div_same_type discards fractional quotient digits and is NOT an equivalent division result.

## Recorded provenance

CSV SHA-256: `a5c973e1beeecd6825ae319cdddea2e4fdeee0d4aa799a1bd9d06ae22f666c68`

- source_commit: `bdea3667a71542a987d310524a3438c52bc9bb0b`
- compiler: `Ubuntu Clang 22.1.8 (++20260714014902+ca7933e47d3a-1~exp1~20260714135019.80)`
- cpu: `AMD EPYC 9V74 80-Core Processor`
- flags: `-O3 -DNDEBUG -Werror -fno-vectorize -fno-slp-vectorize -ffp-contract=off (compile_commands.json retained)`
- affinity: `0`
- run_url: `https://github.com/samiisd/fixedwide/actions/runs/33954651604`
- binary_sha256: `68b89a47e4808eb6b481a0d588bfa82a8617b3d4705cb283434a5a3879336bb8`
- iterations: `262144`
- repetitions: `11`
- validations: `212993`
- dependencies: `fixedwide=0.6.0; decimal_for_cpp=599372ee214ab37b5c0fc68148352321978f20ed; CNL=v1.1.7; fpm=v1.1.0; Boost.Decimal=1297a5efcb2368969f322d0addb3149ed4cbdd50; Boost.Multiprecision=1.83.0; mpdecimal=4.0.1`
- decimal_contract: `signed exact scaled-integer fixtures; multiplication and division exact at declared scale`
- binary_contract: `shared scale-4 multiplication inputs divided by 32; raw product bounds checked before execution; tolerance-based validation`
- text_contract: `fixed notation with all declared fractional digits`

## Reading the results

The binary CNL/fpm inputs share the scale-4 multiplication fixtures divided by 32. Every CNL raw product is checked in __int128 before the int64 operation executes. These bounded binary workloads do not share the decimal workloads' economic range.

fixedwide uses checked decimal rescaling. decimal_for_cpp explicitly selects half-even; CNL and fpm use their configured arithmetic without fixedwide-style checked overflow. Unconfigured signed overflow is not promised to wrap. Only successful bounded inputs are timed.

Boost.Decimal has a moving decimal exponent. mpdecimal has runtime precision and may allocate. The default cpp_dec_float_50 stores its fixed-precision digits inside the object, without a digit-storage allocator; string conversions may allocate. Allocating string formatters are labelled by API/type and should not be mistaken for caller-buffer formatting.

Serialization load rows traverse prepared buffers. They are microbenchmarks, not a universal memcpy floor. The p95 column follows the harness's lower order statistic: sorted[(n-1)*95/100]. All samples remain in the CSV. Sanitized runs must not supply timing tables.

## decimal_fixed_exact_4

| library | type | operation | median ns/op | min ns | p95 ns |
|---|---|---|---:|---:|---:|
| fixedwide | `Fixed64<4>` | add | 0.875 | 0.874 | 0.912 |
| fixedwide | `Fixed64<4>` | mul | 2.500 | 2.441 | 2.569 |
| fixedwide | `Fixed64<4>` | div | 3.052 | 2.993 | 3.082 |
| fixedwide | `Fixed64<4>` | parse | 21.861 | 21.446 | 22.369 |
| fixedwide | `Fixed64<4>` | format_fixed | 26.137 | 26.072 | 26.169 |
| decimal_for_cpp | `decimal<4,half_even>` | add | 0.714 | 0.713 | 0.743 |
| decimal_for_cpp | `decimal<4,half_even>` | mul | 11.880 | 11.855 | 12.058 |
| decimal_for_cpp | `decimal<4,half_even>` | div | 11.902 | 11.886 | 11.928 |
| decimal_for_cpp | `decimal<4,half_even>` | parse | 198.274 | 198.072 | 199.958 |
| decimal_for_cpp | `decimal<4,half_even>` | format_fixed | 266.114 | 265.580 | 266.952 |
| cnl | `scaled_integer<int64,power<-4,10>>` | add | 0.714 | 0.713 | 0.747 |
| cnl | `scaled_integer<int64,power<-4,10>>` | mul | 1.357 | 1.354 | 1.391 |

## decimal_fixed_adjacent

| library | type | operation | median ns/op | min ns | p95 ns |
|---|---|---|---:|---:|---:|
| cnl | `scaled_integer<int64,power<-4,10>>` | div_same_type | 2.281 | 2.253 | 2.285 |

## decimal_float_exact_4

| library | type | operation | median ns/op | min ns | p95 ns |
|---|---|---|---:|---:|---:|
| boost.decimal | `decimal64_t` | add | 7.646 | 7.635 | 7.717 |
| boost.decimal | `decimal64_t` | mul | 7.536 | 7.533 | 7.545 |
| boost.decimal | `decimal64_t` | div | 20.781 | 20.744 | 20.993 |
| boost.decimal | `decimal64_t` | parse | 23.935 | 23.901 | 24.005 |
| boost.decimal | `decimal64_t` | format_fixed | 40.500 | 40.439 | 40.749 |

## arbitrary_decimal_exact_4

| library | type | operation | median ns/op | min ns | p95 ns |
|---|---|---|---:|---:|---:|
| mpdecimal | `Decimal` | add | 22.463 | 22.377 | 22.849 |
| mpdecimal | `Decimal` | mul | 18.920 | 18.859 | 19.105 |
| mpdecimal | `Decimal` | div | 97.830 | 97.569 | 98.261 |
| mpdecimal | `Decimal` | parse | 45.627 | 45.560 | 45.817 |
| mpdecimal | `Decimal` | format_fixed | 106.464 | 106.092 | 106.706 |
| boost.multiprecision | `cpp_dec_float_50` | add | 25.763 | 25.697 | 25.803 |
| boost.multiprecision | `cpp_dec_float_50` | mul | 101.284 | 101.104 | 101.510 |
| boost.multiprecision | `cpp_dec_float_50` | div | 746.130 | 743.763 | 747.525 |
| boost.multiprecision | `cpp_dec_float_50` | parse | 144.773 | 141.975 | 145.895 |
| boost.multiprecision | `cpp_dec_float_50` | format_fixed | 214.879 | 214.084 | 216.442 |

## decimal_fixed_exact_12

| library | type | operation | median ns/op | min ns | p95 ns |
|---|---|---|---:|---:|---:|
| fixedwide | `Fixed64<12>` | add | 0.875 | 0.874 | 0.905 |
| fixedwide | `Fixed64<12>` | mul | 3.258 | 3.215 | 3.274 |
| fixedwide | `Fixed64<12>` | div | 3.172 | 3.144 | 3.183 |
| fixedwide | `Fixed64<12>` | parse | 38.741 | 38.367 | 40.183 |
| fixedwide | `Fixed64<12>` | format_fixed | 29.534 | 29.510 | 29.661 |
| decimal_for_cpp | `decimal<12,half_even>` | add | 0.713 | 0.713 | 0.745 |
| decimal_for_cpp | `decimal<12,half_even>` | mul | 67.736 | 67.551 | 68.564 |
| decimal_for_cpp | `decimal<12,half_even>` | div | 67.552 | 67.471 | 68.196 |
| decimal_for_cpp | `decimal<12,half_even>` | parse | 261.280 | 259.374 | 262.466 |
| decimal_for_cpp | `decimal<12,half_even>` | format_fixed | 272.358 | 271.865 | 273.273 |

## binary_fixed_approx

| library | type | operation | median ns/op | min ns | p95 ns |
|---|---|---|---:|---:|---:|
| cnl | `scaled_integer<int64,power<-32>>` | add | 0.713 | 0.708 | 0.739 |
| cnl | `scaled_integer<int64,power<-32>>` | mul | 1.169 | 1.166 | 1.198 |
| cnl | `scaled_integer<int64,power<-32>>` | div_same_type | 2.281 | 2.253 | 2.297 |
| fpm | `fixed<int64,int128,32>` | add | 0.713 | 0.708 | 0.740 |
| fpm | `fixed<int64,int128,32>` | mul | 3.194 | 3.161 | 3.217 |
| fpm | `fixed<int64,int128,32>` | div | 4.639 | 4.541 | 4.661 |

## hardware_baseline

| library | type | operation | median ns/op | min ns | p95 ns |
|---|---|---|---:|---:|---:|
| std | `double` | add | 0.713 | 0.713 | 0.747 |
| std | `double` | mul | 0.715 | 0.714 | 0.743 |
| std | `double` | div | 1.742 | 1.568 | 1.773 |
| std | `double` | parse | 12.466 | 12.429 | 12.532 |
| std | `double` | format_fixed | 46.258 | 46.070 | 46.877 |
| std | `int64_t` | add_unchecked | 0.713 | 0.713 | 0.742 |
| std | `int64_t` | mul_unchecked | 0.713 | 0.713 | 0.744 |
| std | `int64_t` | div_unchecked | 2.281 | 2.253 | 2.287 |
| std | `int64_t` | memcpy_store | 0.737 | 0.734 | 0.767 |
| std | `int64_t` | memcpy_load | 0.698 | 0.698 | 0.726 |

## serialization

| library | type | operation | median ns/op | min ns | p95 ns |
|---|---|---|---:|---:|---:|
| fixedwide | `Fixed64<4>` | to_bytes_little | 0.698 | 0.698 | 0.772 |
| fixedwide | `Fixed64<4>` | from_bytes_little | 0.697 | 0.697 | 0.725 |

## Reproduce

Build mpdecimal first with scripts/build_mpdecimal.sh and use its prefix as FIXEDWIDE_MPDECIMAL_ROOT. The complete Release and UBSan/ASan commands, recorded environment, binary hashes and raw outputs are retained by .github/workflows/competitors.yml. Do not reuse a schema-2 baseline; those measurements were withdrawn.

Generate this report and the README summary from the SAME validated CSV:

```bash
python3 scripts/competitor_report.py --input reports/raw/competitors.csv --require-provenance \
  --generate-markdown reports/BENCHMARK_COMPETITORS.md --update-readme README.md
```
