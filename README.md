<div align="center">

<img src="docs/assets/logo.svg" alt="" width="112" height="112">

# fixedwide

**Checked decimal fixed-point arithmetic for C++23.**  
*For discrete measurements, deterministic simulation, coordinates and financial ledgers.*

[![CI](https://github.com/samiisd/fixedwide/actions/workflows/ci.yml/badge.svg)](https://github.com/samiisd/fixedwide/actions/workflows/ci.yml)
[![coverage](docs/assets/coverage.svg)](docs/ci.md)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)

[Why fixedwide?](#why-fixedwide) • [Domains](#domains) • [Quick Start](#the-problem-in-three-snippets) • [Performance](#performance) • [The Types](#the-types) • [Install](#install)

</div>

---

## Why fixedwide?

When values live on a **discrete decimal grid**, a scaled integer can represent them exactly. fixedwide makes the scale part of the type, checks arithmetic overflow, and makes rounding policy explicit. It does not eliminate quantization error or the need to choose where a calculation rounds.

| Approach | Useful properties | Trade-offs |
|---|---|---|
| `double` | Hardware arithmetic and wide dynamic range | Decimal fractions such as `0.01` are not exactly representable. |
| Manually scaled `int64_t` | Exact discrete values and compact storage | Scale management and overflow checks belong to the caller. |
| Boost.Decimal | Decimal significand with a moving exponent | Different range, precision and error semantics from fixed point. |
| mpdecimal | Runtime decimal precision and a configurable arithmetic context | Different storage/allocation and context costs. |
| fixedwide | Compile-time decimal scale, checked rescaling, fixed-size storage | Bounded range; division/multiplication can still require rounding. |

Core arithmetic, decimal parsing and caller-buffer formatting allocate no heap memory and return errors as values. Convenience string formatting such as `to_string` may allocate. The guarantee does not extend to throwing adapters or to `.value()` on an unsuccessful `std::expected`.

## Domains

- **Instrumentation:** store measurements on a declared decimal grid while preserving their recorded resolution. Calibration and conversion still need an explicit rounding boundary.
- **Simulation and coordinates:** use deterministic checked integer arithmetic for quantities represented on fixed grids. Coordinate transformations are not automatically lossless.
- **Ledgers and billing:** preserve decimal amounts and apply the rounding rule required by the application. Nearest-even is the library default, not a universal tax or settlement rule.

## The Problem in Three Snippets

```cpp
double binary_total = 0.0;
for (int i = 0; i < 100; ++i) binary_total += 0.01;
// A typical binary64 result is 1.0000000000000007, not exactly 1.
```

```cpp
#include <cstdint>
std::int64_t a = 5'000'000'000'000'000'000;
// a + a would overflow signed int64_t: undefined behaviour, not a checked result.
```

Inside a function with the arithmetic, chars and string headers included:

```cpp
using Money = fixedwide::Fixed64<2>;
auto checked_total = Money::from_raw(0);
for (int i = 0; i < 100; ++i) {
    checked_total = fixedwide::add(
        checked_total, fixedwide::parse<Money>("0.01").value()).value();
}
// fixedwide::to_string(checked_total) == "1.00"
auto overflow = fixedwide::add(Money::max(), fixedwide::parse<Money>("0.01").value());
// overflow.error() == fixedwide::ArithmeticError::overflow
```

These known constants make the successful `.value()` calls safe in this example. Check results before dereferencing when processing external values.

Different widths/scales are distinct types. Two aliases with identical width and scale are the **same** type: this is scale safety, not dimensional analysis.

## Intermediate precision widening vs single-word fixed point

For scale `S = 10^D`, multiplication computes `(raw_a * raw_b) / S` before rounding to the destination. A positive economic product `a * b` fits an un-widened signed 64-bit intermediate only when:

```text
a * b <= (2^63 - 1) / S^2
```

At `D = 12`, that limit is approximately `9.223372e-6`. For **equal positive operands**, the largest value that can be squared without such intermediate overflow is approximately `0.003037`. This is not a universal limit for each operand independently.

fixedwide widens the `Fixed64` multiplication intermediate to 128 bits, rescales, and checks the destination:

```cpp
using F = fixedwide::Fixed64<12>;
auto a = fixedwide::parse<F>("123.456789012345").value();
auto b = fixedwide::parse<F>("2.000000000000").value();
auto result = fixedwide::mul(a, b); // 246.913578024690
```

Other fixed-point libraries can be configured with widened representations or overflow policies. Compare the configuration actually being measured, not just the library name.

## Performance

[Full report and contracts](reports/BENCHMARK_COMPETITORS.md). The summary below is generated from the same retained CSV as that report. Exact-result throughput is not a claim about all rounding modes, widths, or dependency-chain latency. The separate rounding benchmark covers inexact arithmetic.

<!-- BEGIN GENERATED COMPETITOR SUMMARY -->

Previously retained schema-2 timings are withdrawn: their process executed an overflowing CNL binary multiplication. They must not support performance claims. The corrected schema-3 benchmark bounds binary operands before execution. Fresh Release CSV, validation logs and generated reports are available from the [Competitor benchmark workflow](https://github.com/samiisd/fixedwide/actions/workflows/competitors.yml). No replacement timings are claimed until that evidence is retained.

<!-- END GENERATED COMPETITOR SUMMARY -->

The instruction-count CI gate checks core workloads against its committed baseline. This is distinct from a timing comparison against 0.4; see [benchmark methodology](docs/benchmarks.md).

## Install

```cmake
include(FetchContent)
FetchContent_Declare(fixedwide
    GIT_REPOSITORY https://github.com/samiisd/fixedwide.git
    GIT_TAG        v0.6.0)
FetchContent_MakeAvailable(fixedwide)
target_link_libraries(app PRIVATE fixedwide::fixedwide)
```

Alternatively, build the local Conan recipe with `conan create .`, or install with CMake and consume `fixedwide::fixedwide` through `find_package(fixedwide)`. Local recipe testing does not imply publication on ConanCenter.

The `<format>` and `<iostream>` adapters are opt-in rather than pulled into every arithmetic translation unit. Include-cost measurements and their environment are documented separately.

## The types

| Type | Storage | Decimals | Size |
|---|---|---|---:|
| `Fixed8<D>` | `std::int8_t` | 0–2 | 1 B |
| `Fixed16<D>` | `std::int16_t` | 0–4 | 2 B |
| `Fixed32<D>` | `std::int32_t` | 0–9 | 4 B |
| `Fixed64<D>` | `std::int64_t` | 0–18 | 8 B |
| `Fixed128<D>` | `wide::int128` | 0–38 | 16 B |
| `Fixed256<D>` | `wide::int256` | 0–76 | 32 B |

A value is its scaled integer, with no runtime scale member.

```cpp
using namespace fixedwide;
auto price = parse<Fixed64<4>>("19.9900").value();
auto rate = parse<Fixed64<8>>("1.07500000").value();

// mul(price, rate);              // rejected: two different types
auto total = mul_to<Fixed128<2>>(price, rate); // 21.49; one final rounding
bool same = price == parse<Fixed64<8>>("19.99000000").value(); // true

auto zero = div(price, Fixed64<4>::from_raw(0)); // division_by_zero
auto inexact = div(price, parse<Fixed64<4>>("3.0000").value(), Rounding::exact);
// inexact.error() == ArithmeticError::inexact

// Nearest-even is the arithmetic default. Only exact halfway cases use parity.
auto even = quantize(parse<Fixed64<2>>("2.50").value(), 0); // 2.00
auto odd = quantize(parse<Fixed64<2>>("3.50").value(), 0);  // 4.00
```

Nearest-even reduces systematic tie-breaking bias; it does **not** prevent accumulated rounding error. Rounding `0.005` individually to two places gives `0.00`, whereas adding 100 original values and rounding once gives `0.50`. Preserve intermediates and round at the intended calculation boundary.

All six rounding policies remain available. Decimal parsing and `fixed_cast` default to exact; full-precision serialization does not discard digits. Raw binary encoding contains no scale tag, so both endpoints must agree on the type.

## Examples

| | | |
|---|---|---|
| 01 | [Quick start](examples/01_quick_start.cpp) | parse → mixed multiply → format |
| 02 | [Rounding](examples/02_rounding_modes.cpp) | all six policies |
| 03 | [Errors](examples/03_error_handling.cpp) | failures as values |
| 04 | [Mixed scales](examples/04_mixed_scales.cpp) | explicit destinations |
| 05 | [Text](examples/05_text_io.cpp) | chars, format and streams |
| 06 | [Binary](examples/06_binary_roundtrip.cpp) | both byte orders |
| 07 | [Money ledger](examples/07_money_ledger.cpp) | invoice example |
| 08 | [constexpr](examples/08_constexpr.cpp) | compile-time arithmetic |

## Verification

The repository's [CI documentation](docs/ci.md) and [execution matrix](reports/EXECUTION_MATRIX.csv) distinguish executed platforms from untested targets. They cover Linux, macOS, Windows, portable/no-int128 configurations, sanitizers and emulated s390x. Do not infer WebAssembly or Windows ARM64 execution from the portable implementation alone.

| Documentation | |
|---|---|
| [API reference](docs/api_reference.md) | public functions and headers |
| [Architecture](docs/architecture.md) | storage and arithmetic backends |
| [Benchmarks](docs/benchmarks.md) | methods and limitations |
| [STATUS](STATUS.md) | executed work and open items |

Pre-1.0: the API may still change. [Changelog](CHANGELOG.md) · [MIT](LICENSE) · [Contributing](CONTRIBUTING.md) · [Ethics](ETHICS.md).
