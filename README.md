<div align="center">

<img src="docs/assets/logo.svg" alt="" width="112" height="112">

# fixedwide

**Checked fixed-point decimal arithmetic for C++23.**

[![CI](https://github.com/samiisd/fixedwide/actions/workflows/ci.yml/badge.svg)](https://github.com/samiisd/fixedwide/actions/workflows/ci.yml)
[![coverage](docs/assets/coverage.svg)](docs/ci.md)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)

</div>

---

```cpp
double binary_total = 0.0;
for (int i = 0; i < 100; ++i) binary_total += 0.01;
// 1.0000000000000007          ← binary floats do not have 0.01
```

```cpp
std::int64_t a = 5'000'000'000'000'000'000;
std::int64_t raw_total = a + a;
// signed integer overflow      ← undefined behavior in standard C++
```

```cpp
using Money = fixedwide::Fixed64<2>;               // an int64 of cents

auto checked_total = Money::from_raw(0);
for (int i = 0; i < 100; ++i)
    checked_total = add(checked_total, parse<Money>("0.01").value()).value();
// to_string(checked_total) == "1.00"   ← exactly

add(Money::max(), parse<Money>("0.01").value());
// ArithmeticError::overflow    ← reported, not wrapped
```

The scale lives in the type, so different fixed-point widths and scales are distinct types:
when a price and quantity carry different scales (such as 4 and 2 decimals),
mixing them without specifying the target scale does not compile.
Note that `basic_fixed` enforces scale and bit-width distinctness, not dimensional analysis:
types with identical width and scale are the same type.

---

## Intermediate precision widening vs single-word fixed point

Fixed-point multiplication rescales by dividing by the scale factor after multiplication ($(\text{raw}_a \times \text{raw}_b) / 10^D$).
At high fractional scales, computing the intermediate product within the same storage type overflows: at 12 decimals, any product $\ge 10^{12} \cdot 2^{63} \approx 9.22$ overflows a 64-bit integer before the scale factor can be applied.

`fixedwide` evaluates 64-bit multiplications using a 128-bit intermediate integer, detecting overflow against destination bounds before returning:

```cpp
// 123.456789012345 × 2 at Fixed64<12>
auto a = parse<Fixed64<12>>("123.456789012345").value();
auto b = parse<Fixed64<12>>("2.000000000000").value();
auto res = mul(a, b); // 246.913578024690 (128-bit intermediate, rescaled once)
```

In contrast, single-word fixed-point models (such as `cnl::scaled_integer<int64_t, power<-12, 10>>`) overflow 64-bit integer representation on the intermediate product for values above ~0.003 when intermediate widening is not configured.

---

## Performance

Median ns/op, Clang 22, `-O3`, core-pinned, every timed result validated against
an oracle prior to measurement.
[Full table and methodology.](reports/BENCHMARK_COMPETITORS.md)

### Scale 4 comparison

| | **fixedwide**<br>`Fixed64<4>` | Boost.Decimal<br>`decimal64_t` | CNL<br>`scaled_integer` | `double`<br>*(binary float)* |
|---|---:|---:|---:|---:|
| multiply | **1.30** | 3.62 | 0.58 | 0.28 |
| divide | **1.43** | 9.30 | 1.22 *(same-type)* | 0.75 |
| parse | **9.33** | 11.99 | — | 5.00 |
| format | **10.95** | 23.04 | — | 28.46 |
| error model | `std::expected` | IEEE status flags | unchecked | hardware NaN/inf |

At scale 12 (`Fixed64<12>`), fixedwide measures **1.97 ns** multiply, **1.92 ns** divide, **17.63 ns** parse, and **13.34 ns** format.

### Against raw machine hardware

| | **fixedwide** | raw `int64_t` | `double` |
|---|---:|---:|---:|
| add | 0.41 | 0.28 | 0.37 |
| store 8 bytes | 0.20 `to_bytes` | 0.27 `memcpy` | — |
| load 8 bytes | 0.28 `from_bytes` | 0.18 `memcpy` | — |

**Performance trade-offs:** At scale 4, CNL's unscaled 64-bit multiply is ~2.2× faster (0.58 ns vs 1.30 ns) because it performs single-word integer multiplication without 128-bit intermediate widening or overflow checking. `fixedwide::mul` widens to 128 bits, rescales, verifies destination bounds, and returns `std::expected`. That is the safety trade-off this library is designed for.

**Deterministic instruction-count CI gate:** Every pull request measures retired instructions under Valgrind on Linux x86-64 (GCC 14) and fails if any core workload grows by more than 1%. This prevents algorithmic bloat independently of wall-clock timing jitter. [How.](docs/benchmarks.md)


---

## Install

```cmake
include(FetchContent)
FetchContent_Declare(fixedwide
    GIT_REPOSITORY https://github.com/samiisd/fixedwide.git
    GIT_TAG        v0.6.0)
FetchContent_MakeAvailable(fixedwide)
target_link_libraries(app PRIVATE fixedwide::fixedwide)
```

Or `conan create .`, or `cmake --build build --target install` then
`find_package(fixedwide)`. All three give you the identical two lines of CMake,
and [CI proves each of them still works](docs/ci.md).

`#include <fixedwide/all.hpp>` costs **187 ms** — the `<format>` and
`<iostream>` adapters are opt-in, because those two standard headers together
cost 885 ms, more than everything here.

---

## The types

| Type | Storage | Decimals | Size |
|---|---|---|---:|
| `Fixed8<D>` | `std::int8_t` | 0–2 | 1 B |
| `Fixed16<D>` | `std::int16_t` | 0–4 | 2 B |
| `Fixed32<D>` | `std::int32_t` | 0–9 | 4 B |
| `Fixed64<D>` | `std::int64_t` | 0–18 | 8 B |
| `Fixed128<D>` | `wide::int128` | 0–38 | 16 B |
| `Fixed256<D>` | `wide::int256` | 0–76 | 32 B |


A value *is* its scaled integer. No exponent, no tag, no indirection,
`sizeof(Fixed64<2>) == 8`.

```cpp
auto price = parse<Fixed64<4>>("19.9900").value();
auto rate  = parse<Fixed64<8>>("1.07500000").value();

mul(price, rate);                    // ✗ compile error: two different types
mul_to<Fixed128<2>>(price, rate);    // ✓ 21.49 — exact product, ONE rounding
price == parse<Fixed64<8>>("19.99000000").value();   // ✓ true, exactly

div(price, Fixed64<4>::from_raw(0)); // ArithmeticError::division_by_zero
div(price, parse<Fixed64<4>>("3.0000").value(), Rounding::exact); // ArithmeticError::inexact
```

Six rounding modes. `constexpr` arithmetic. Core arithmetic, parsing, and
caller-buffer formatting allocate no heap memory and return failures through
`std::expected` (proved by a test that replaces `operator new` and counts).
Convenience string formatting like `to_string` allocates by design.

---

## Examples

Eight programs, one file and one idea each. All eight are `ctest` tests that
check their own output, so they cannot drift away from the library.

| | | |
|---|---|---|
| 01 | [Quick start](examples/01_quick_start.cpp) | parse → multiply across scales → format |
| 02 | [Rounding](examples/02_rounding_modes.cpp) | all six modes on one division, side by side |
| 03 | [Errors](examples/03_error_handling.cpp) | overflow and divide-by-zero as values, `and_then` chaining |
| 04 | [Mixed scales](examples/04_mixed_scales.cpp) | `mul_to` / `fixed_cast`, and what is compile-rejected |
| 05 | [Text](examples/05_text_io.cpp) | `to_chars`, `std::format`, streams |
| 06 | [Binary](examples/06_binary_roundtrip.cpp) | `to_bytes` / `from_bytes`, both byte orders |
| 07 | [Money ledger](examples/07_money_ledger.cpp) | an invoice in `double` and in `Fixed64<2>`, side by side |
| 08 | [constexpr](examples/08_constexpr.cpp) | the whole arithmetic surface at compile time |

---

## Verified, not asserted

Every row is a CI job that builds **and runs the tests** on every push. A
platform with no job is marked `not-configured` and is not claimed.

Linux x86-64 (GCC 14, Clang 18 + libc++) · Linux AArch64 · macOS 14 and 15 ·
Windows MSVC and clang-cl · **big-endian s390x** · portable backend ·
no-`__int128` · ASan + UBSan · shared library · Conan · FetchContent ·
`find_package` · libFuzzer · instruction-count gate

Big-endian runs under emulation and says so. Not configured: Windows ARM64.

| | |
|---|---|
| [API reference](docs/api_reference.md) | every header and function, naming rules, the 0.4 surface |
| [Architecture](docs/architecture.md) | storage, the three backends, how one is chosen |
| [Benchmarks](docs/benchmarks.md) | how every number was produced, and why not `-march=native` |
| [CI](docs/ci.md) | what each job proves, and which compilers genuinely do not work |
| [STATUS](STATUS.md) | what has been executed, and every open item |

---

Pre-1.0 — the API may still change, and `CHANGELOG.md` says when.
[MIT](LICENSE). [Contributing](CONTRIBUTING.md). [Ethics](ETHICS.md).
