<div align="center">

<img src="docs/assets/logo.svg" alt="" width="112" height="112">

# fixedwide

**Exact decimal arithmetic for C++23 — with the overflow checked.**

[![CI](https://github.com/samiisd/fixedwide/actions/workflows/ci.yml/badge.svg)](https://github.com/samiisd/fixedwide/actions/workflows/ci.yml)
[![coverage](docs/assets/coverage.svg)](docs/ci.md)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)

</div>

---

```cpp
double total = 0.0;
for (int i = 0; i < 100; ++i) total += 0.01;
// 1.0000000000000007          ← binary floats do not have 0.01
```

```cpp
std::int64_t a = 5'000'000'000'000'000'000;
std::int64_t total = a + a;
// undefined behaviour          ← no diagnostic, no crash, just a wrong number
```

```cpp
using Money = fixedwide::Fixed64<2>;               // an int64 of cents

auto total = Money::from_raw(0);
for (int i = 0; i < 100; ++i)
    total = add(total, parse<Money>("0.01").value()).value();
// to_string(total) == "1.00"   ← exactly

add(Money::max(), parse<Money>("0.01").value());
// ArithmeticError::overflow    ← reported, not wrapped
```

The scale lives in the type, so a price and a quantity are different types and
mixing them without saying what you meant does not compile.

---

## It is not just faster than the alternative — the alternative is wrong

`cnl::scaled_integer` forms the product in its representation type. At twelve
decimals that overflows `int64_t` for any value above about `0.003`:

```cpp
// 123.456789012345 × 2, at 12 decimals

cnl        →       -0.000002      // silently wrong. and negative.
fixedwide  →  246.913578024690    // exact
```

The competitor benchmark [asserts this on every run](reports/BENCHMARK_COMPETITORS.md),
so the claim cannot rot.

---

## Performance

Median ns/op, Clang 22, `-O3`, core-pinned, every timed result validated against
an independent oracle **outside** the timed region.
[Full table and method.](reports/BENCHMARK_COMPETITORS.md)

| | **fixedwide**<br>`Fixed64<12>` | Boost.Decimal<br>`decimal64_t` | CNL | `double` |
|---|---:|---:|---:|---:|
| multiply | **2.63** | 3.54 | 0.54 | 0.22 |
| divide | **2.17** | 8.59 | 1.09 | 0.74 |
| parse | **12.30** | 13.67 | — | 5.40 |
| format | 14.10 | **12.57** | — | 28.65 |
| overflow detected | **yes** | no | no | no |
| usable at 12 decimals | **yes** | yes | **no** | no |

**Against the raw machine types** — because "fast for a checked decimal library"
is not a claim you can act on:

| | **fixedwide** | raw `int64_t` | `double` |
|---|---:|---:|---:|
| add | **0.38** | 0.31 | 0.26 |
| store 8 bytes | **0.19** `to_bytes` | 0.19 `memcpy` | — |
| load 8 bytes | **0.18** `from_bytes` | 0.18 `memcpy` | — |

A checked decimal add costs **two instructions** more than a raw `int64_t` add.
Byte-order-defined serialisation measures at the `memcpy` floor in this fixture
— though it is 110 instructions against 54, and
[the report says where the two measurements disagree](reports/BENCHMARK_COMPETITORS.md#the-raw-type-floor).

Read honestly: **CNL's multiply is 2.8× quicker at a matched scale**, because it
does not check for overflow. That is the trade this library exists to make.
Formatting loses to Boost.Decimal and that is an open item.

**No performance regression can merge.** Every pull request re-measures retired
instructions per operation under Valgrind and fails if any workload grows by
more than 1%. It is deterministic — two runs are byte-identical — which is what
makes a 1% gate possible where wall-clock noise is 15×
that. [How.](docs/benchmarks.md)

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

| | Storage | Decimals | Size |
|---|---|---|---:|
| `Fixed8<D>` … `Fixed64<D>` | `std::int8_t` … `std::int64_t` | 0–18 | 1–8 B |
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
div(a, b, Rounding::exact);          // refuses to round rather than guess
```

Six rounding modes. `constexpr` arithmetic. Zero allocation on every
arithmetic, parsing and formatting path — proved by a test that replaces
`operator new` and counts.

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
