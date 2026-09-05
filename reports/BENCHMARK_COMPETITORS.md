# Competitor benchmark

## What this is, and what it is not

Every row below was produced by `benchmarks/competitor/` on an isolated, core-pinned x86-64 Linux environment.
Rows are grouped by **semantic class**. Cost may be compared across classes; correctness may not.
A binary fixed-point multiply and a decimal fixed-point multiply are not the same operation, and only one of them can represent `0.01`.

Each number is the **median** of 11 timed repetitions of 262144 operations.
Minimum, median, p95, maximum and raw samples are preserved in `reports/raw/competitors.csv`.
Every timed loop's output was validated against exact oracles before timing.

## Libraries Evaluated

| Library | Type | Architecture / Representation | Error Handling | Allocation |
|---|---|---|---|---|
| **fixedwide** | `Fixed64<D>` | Decimal Fixed-Point (scaled 64-bit integer, 128-bit intermediate) | Checked (`std::expected`) | Zero (core arithmetic, parsing, caller-buffer formatting) |
| **decimal_for_cpp** | `dec::decimal<D>` | Decimal Fixed-Point (scaled 64-bit integer) | Unchecked / wraps | Zero arithmetic / allocates on `toString` |
| **cnl** | `scaled_integer` | Fixed-Point (binary or decimal radix) | Unchecked / wraps | Zero |
| **fpm** | `fixed` | Binary Fixed-Point (scaled 64-bit integer) | Unchecked / wraps | Zero |
| **Boost.Decimal** | `decimal64_t` | Decimal Floating-Point (IEEE 754-2008 decimal64) | IEEE flags / status | Zero |
| **mpdecimal** | `decimal::Decimal` | Arbitrary-Precision Decimal Float (libmpdec++) | Context status / exception | Dynamic |
| **Boost.Multiprecision** | `cpp_dec_float_50` | Arbitrary-Precision Decimal Float (50 decimal digits) | Exceptions | Dynamic |
| *std (baseline)* | `double` | Binary Floating-Point (IEEE 754 binary64) | Hardware NaN/inf | Zero |
| *std (baseline)* | `int64_t` | Raw 64-bit integer (unscaled machine word) | Undefined behavior | Zero |

## Test Environment & Methodology

- **Compiler**: Clang 22.1.8 (`-O3 -DNDEBUG -fno-vectorize -fno-slp-vectorize -ffp-contract=off`)
- **Execution**: Thread pinned to single CPU core
- **Workload**: 262,144 operations per repetition, 11 timed repetitions
- **Reporting**: Medians are reported as the primary metric, alongside minimum and 95th-percentile samples
- **Validation**: All operations verified against exact oracles prior to timed loops
- **Raw Data**: Full per-sample timing records are preserved in `reports/raw/competitors.csv`

## Results

### decimal fixed, matched scale (scale 4)

The like-for-like comparison: the same scale, the same operand integers, and results brought back to the declared type.

| library | type | operation | median ns/op | min ns | p95 ns | error handling |
|---|---|---|---:|---:|---:|---|
| fixedwide | `Fixed64<4>` | add | 0.406 | 0.403 | 0.408 | checked |
| fixedwide | `Fixed64<4>` | mul | 1.298 | 1.289 | 1.524 | checked |
| fixedwide | `Fixed64<4>` | div | 1.430 | 1.422 | 1.442 | checked |
| fixedwide | `Fixed64<4>` | parse | 9.334 | 9.270 | 9.350 | checked |
| fixedwide | `Fixed64<4>` | format_fixed | 10.948 | 10.883 | 10.955 | checked |
| decimal_for_cpp | `decimal<4,half_even>` | add | 0.283 | 0.283 | 0.287 | unchecked |
| decimal_for_cpp | `decimal<4,half_even>` | mul | 6.388 | 6.335 | 6.461 | unchecked |
| decimal_for_cpp | `decimal<4,half_even>` | div | 6.373 | 6.345 | 6.583 | unchecked |
| decimal_for_cpp | `decimal<4,half_even>` | parse | 75.296 | 74.906 | 76.461 | unchecked |
| decimal_for_cpp | `decimal<4,half_even>` | format_fixed | 106.498 | 106.213 | 107.453 | unchecked |
| cnl | `scaled_integer<int64,power<-4,10>>` | add | 0.288 | 0.287 | 0.290 | unchecked |
| cnl | `scaled_integer<int64,power<-4,10>>` | mul | 0.578 | 0.571 | 0.584 | unchecked |
| cnl | `scaled_integer<int64,power<-4,10>>` | div_same_type | 1.219 | 1.209 | 1.227 | unchecked |

### decimal fixed, high precision (scale 12)

Decimal fixed point at 12 decimal places.

| library | type | operation | median ns/op | min ns | p95 ns | error handling |
|---|---|---|---:|---:|---:|---|
| fixedwide | `Fixed64<12>` | add | 0.425 | 0.423 | 0.426 | checked |
| fixedwide | `Fixed64<12>` | mul | 1.971 | 1.951 | 1.992 | checked |
| fixedwide | `Fixed64<12>` | div | 1.915 | 1.903 | 1.931 | checked |
| fixedwide | `Fixed64<12>` | parse | 17.633 | 17.585 | 17.662 | checked |
| fixedwide | `Fixed64<12>` | format_fixed | 13.340 | 13.260 | 13.361 | checked |
| decimal_for_cpp | `decimal<12,half_even>` | add | 0.282 | 0.281 | 0.283 | unchecked |
| decimal_for_cpp | `decimal<12,half_even>` | mul | 40.081 | 39.901 | 40.309 | unchecked |
| decimal_for_cpp | `decimal<12,half_even>` | div | 41.099 | 40.794 | 41.564 | unchecked |
| decimal_for_cpp | `decimal<12,half_even>` | parse | 102.805 | 101.504 | 103.430 | unchecked |
| decimal_for_cpp | `decimal<12,half_even>` | format_fixed | 111.808 | 111.670 | 111.881 | unchecked |

### decimal float

IEEE 754 decimal floating point: a decimal significand with a moving exponent.

| library | type | operation | median ns/op | min ns | p95 ns | error handling |
|---|---|---|---:|---:|---:|---|
| boost.decimal | `decimal64_t` | add | 3.968 | 3.960 | 3.975 | ieee status |
| boost.decimal | `decimal64_t` | mul | 3.621 | 3.570 | 3.633 | ieee status |
| boost.decimal | `decimal64_t` | div | 9.299 | 9.285 | 9.425 | ieee status |
| boost.decimal | `decimal64_t` | parse | 11.993 | 11.793 | 12.008 | ieee status |
| boost.decimal | `decimal64_t` | format_fixed | 23.038 | 22.994 | 23.199 | ieee status |

### arbitrary-precision decimal

Arbitrary-precision decimal representations.

| library | type | operation | median ns/op | min ns | p95 ns | error handling |
|---|---|---|---:|---:|---:|---|
| mpdecimal | `Decimal` | add | 13.654 | 13.483 | 13.698 | exceptions |
| mpdecimal | `Decimal` | mul | 11.457 | 11.414 | 11.508 | exceptions |
| mpdecimal | `Decimal` | div | 39.270 | 39.193 | 39.575 | exceptions |
| mpdecimal | `Decimal` | parse | 23.530 | 23.479 | 23.932 | exceptions |
| mpdecimal | `Decimal` | format_fixed | 47.743 | 47.332 | 47.787 | exceptions |
| boost.multiprecision | `cpp_dec_float_50` | add | 12.260 | 12.219 | 12.288 | exceptions |
| boost.multiprecision | `cpp_dec_float_50` | mul | 43.571 | 43.560 | 43.600 | exceptions |
| boost.multiprecision | `cpp_dec_float_50` | div | 333.356 | 332.868 | 334.136 | exceptions |
| boost.multiprecision | `cpp_dec_float_50` | parse | 59.000 | 58.729 | 59.011 | exceptions |
| boost.multiprecision | `cpp_dec_float_50` | format_fixed | 92.122 | 91.909 | 92.281 | exceptions |

### binary fixed

Binary fixed point: an integer scaled by a power of two. Cannot represent 0.01 exactly.

| library | type | operation | median ns/op | min ns | p95 ns | error handling |
|---|---|---|---:|---:|---:|---|
| cnl | `scaled_integer<int64,power<-32>>` | add | 0.286 | 0.286 | 0.288 | unchecked |
| cnl | `scaled_integer<int64,power<-32>>` | mul | 0.516 | 0.506 | 0.517 | unchecked |
| cnl | `scaled_integer<int64,power<-32>>` | div | 1.296 | 1.284 | 1.300 | unchecked |
| fpm | `fixed<int64,int128,32>` | add | 0.278 | 0.277 | 0.279 | unchecked |
| fpm | `fixed<int64,int128,32>` | mul | 1.438 | 1.428 | 1.452 | unchecked |
| fpm | `fixed<int64,int128,32>` | div | 2.070 | 2.039 | 2.079 | unchecked |

### raw machine types

Hardware baselines and serialization floor.

| library | type | operation | median ns/op | min ns | p95 ns | error handling |
|---|---|---|---:|---:|---:|---|
| std | `double` | add | 0.371 | 0.371 | 0.371 | n/a |
| std | `double` | mul | 0.277 | 0.273 | 0.278 | n/a |
| std | `double` | div | 0.745 | 0.742 | 0.750 | n/a |
| std | `double` | parse | 5.005 | 4.898 | 5.019 | n/a |
| std | `double` | format_fixed | 28.456 | 28.382 | 28.531 | n/a |
| std | `int64_t` | add_unchecked | 0.278 | 0.278 | 0.280 | n/a |
| std | `int64_t` | mul_unchecked | 0.317 | 0.316 | 0.321 | n/a |
| std | `int64_t` | div_unchecked | 1.198 | 1.183 | 1.208 | n/a |
| std | `int64_t` | memcpy_store | 0.274 | 0.272 | 0.278 | n/a |
| std | `int64_t` | memcpy_load | 0.183 | 0.183 | 0.183 | n/a |
| fixedwide | `Fixed64<4>` | to_bytes_little | 0.198 | 0.198 | 0.199 | n/a |
| fixedwide | `Fixed64<4>` | from_bytes_little | 0.279 | 0.277 | 0.280 | n/a |

## The raw-type floor

Hardware operations (unscaled 64-bit integer and binary float) represent the absolute execution floor of the host CPU, not comparable decimal libraries. A checked decimal add costs only two instructions more than a raw 64-bit integer add, and byte-order-defined serialization (`to_bytes` / `from_bytes`) operates at the native `memcpy` floor.

## Reading these numbers honestly

- **Against CNL at matched scale**: CNL decimal multiply (0.578 ns) performs single-word 64-bit integer multiplication without intermediate widening or overflow checking. `fixedwide::mul` (1.298 ns) computes a 128-bit intermediate, rescales, verifies destination bounds, and returns `std::expected`.
- **Intermediate precision limits**: At scale 12, non-widening 64-bit fixed-point arithmetic overflows for values above ~0.003 during multiplication. `fixedwide` uses a 128-bit intermediate representation and computes exact products.
- **Binary fixed-point (`cnl`, `fpm`)**: Binary fixed-point cannot represent decimal fractions like 0.01 exactly in binary radix.
- **Against Boost.Decimal**: At scale 4, fixedwide is faster for arithmetic multiplication (1.30 ns vs 3.62 ns), division (1.43 ns vs 9.30 ns), parsing (9.33 ns vs 11.99 ns), and fixed formatting (10.95 ns vs 23.04 ns). Boost.Decimal provides IEEE 754 decimal floating-point dynamic range.
- **Standard library binary float (`double`)**: Binary floats are fast in hardware but suffer from decimal representation error (e.g. `0.01` cannot be represented exactly).

## Reproducing it

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DFIXEDWIDE_BUILD_BENCHMARKS=ON -DFIXEDWIDE_BUILD_COMPETITOR_BENCH=ON
cmake --build build --target fixedwide_competitor_bench
./build/benchmarks/fixedwide_competitor_bench
```
