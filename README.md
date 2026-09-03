# fixedwide

A modern C++23 library for **checked fixed-point decimal arithmetic** and **portable fixed-width wide integers** (128/256-bit).

`fixedwide` provides distinct compile-time types for different bit-widths and decimal scales. It guarantees zero heap allocation, zero virtual dispatch, no runtime scale overhead, and no compiler `_BitInt` in the public storage types. (Two Clang-only inline fast
paths use `_BitInt(256)` as a local computation type, where it has no ABI
surface.)

| Type Alias | Storage Type | Width | Fractional Digits | In-Memory Size | Alignment |
| :--- | :--- | :---: | :---: | :---: | :---: |
| `Fixed8<D>` | `std::int8_t` | 8 bits | $0 \le D \le 2$ | 1 byte | 1 |
| `Fixed16<D>` | `std::int16_t` | 16 bits | $0 \le D \le 4$ | 2 bytes | 2 |
| `Fixed32<D>` | `std::int32_t` | 32 bits | $0 \le D \le 9$ | 4 bytes | 4 |
| `Fixed64<D>` | `std::int64_t` | 64 bits | $0 \le D \le 18$ | 8 bytes | 8 |
| `Fixed128<D>` | `wide::int128` | 128 bits | $0 \le D \le 38$ | 16 bytes | 8 |
| `Fixed256<D>` | `wide::int256` | 256 bits | $0 \le D \le 76$ | 32 bytes | 8 |

Backward compatibility aliases `FP64` (alias for `Fixed64<12>`) and `FP128` (alias for `Fixed128<12>`) are provided out of the box.

---

## Key Features

- **Distinct Compile-Time Types**: Distinct scale/precision parameters prevent accidental unit mix-ups at compile time.
- **Single-Rounding Cross-Scale Mixed Arithmetic**: Explicit destination operations (`mul_to<Dest>`, `div_to<Dest>`, `add_to<Dest>`, `sub_to<Dest>`, `mul_div_to<Dest>`, `fixed_cast<Dest>`) evaluate exact rational expressions and round once directly to the target scale. Ambiguous implicit mixed arithmetic is compile-rejected.
- **Automatic Exact Comparisons**: Cross-scale `==` and `<=>` work seamlessly and exactly without intermediate rounding loss.
- **Checked Operations**: Overflow, division by zero, and precision loss return `std::expected<T, ArithmeticError>`.
- **Comprehensive Rounding Modes**:
  - `nearest_even` (banker's rounding, default for arithmetic)
  - `toward_zero` (truncation)
  - `floor` (downward directed)
  - `ceil` (upward directed)
  - `nearest_away` (commercial rounding, half away from zero)
  - `exact` (rejects any inexact division or cast, default for parsing and `fixed_cast`)
- **Modular Zero-Overhead Headers**:
  - `<fixedwide/fixed.hpp>`: Core template type `basic_fixed<Bits, Decimals>`
  - `<fixedwide/arithmetic.hpp>`: Checked same-scale arithmetic
  - `<fixedwide/mixed.hpp>`: Cross-scale operations and comparisons
  - `<fixedwide/chars.hpp>`: High-throughput decimal string parsing & formatting
  - `<fixedwide/binary.hpp>`: Explicit-endian zero-copy load/store (`to_bytes`, `from_bytes`)
  - `<fixedwide/floating.hpp>`: Explicit conversions to/from `float`, `double`
  - `<fixedwide/wide.hpp>`: Minimal public wide integers (`wide::int128`, `wide::uint128`, `wide::int256`, `wide::uint256`)
  - `<fixedwide/all.hpp>`: Convenience umbrella header
- **Portability**: every platform below is listed with what has actually been
  executed, in [`reports/EXECUTION_MATRIX.csv`](reports/EXECUTION_MATRIX.csv).
  A platform is not described as supported until a row there says
  `executed-pass`.
  - **Executed**: Linux x86-64 (Clang 17/18/22, GCC 16), Linux AArch64 on real
    hardware, forced-portable and no-`__int128` builds, both sanitizer backends
  - **Configured in CI, not yet executed**: macOS arm64 and x86-64,
    Windows MSVC and clang-cl
  - `FIXEDWIDE_FORCE_PORTABLE` build mode for standard-compliant multi-limb integer fallbacks without hardware assembly or compiler `__int128` extensions.
  - Zero heap allocation across all arithmetic and parsing functions.
  - Full support for `-fno-exceptions` and `-fno-rtti`.

---

## Quick Start

```cpp
#include <fixedwide/all.hpp>
#include <iostream>

int main() {
    using namespace fixedwide;

    // Define distinct domain types
    using Price = Fixed64<4>;
    using Qty = Fixed32<2>;
    using Notional = Fixed128<6>;

    // Exact string parsing
    auto price = parse<Price>("123.4567");
    auto qty = parse<Qty>("10.50");
    if (!price || !qty) return 1;

    // Cross-scale multiplication with single rounding
    auto total = mul_to<Notional>(*price, *qty, Rounding::nearest_even);
    if (!total) return 2;

    std::cout << "Price:    " << to_string(*price) << "\n";
    std::cout << "Quantity: " << to_string(*qty) << "\n";
    std::cout << "Notional: " << to_string(*total) << "\n";

    // Exact cross-scale comparison
    if (*price > Fixed32<2>::from_raw(10000)) { // 100.00
        std::cout << "Price is greater than 100.00\n";
    }

    return 0;
}
```

---

## Building and Testing

### Prerequisites
- C++23 compiler with `std::expected`. Executed here: Clang 17, 18 and 22,
  GCC 16, and GCC 16.1 cross-compiling for AArch64. MSVC is compiled-for but has
  not been executed.
- CMake 3.25+

### Build & Run Tests
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DFIXEDWIDE_BUILD_TESTS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### Build Options
| Option | Default | Description |
| :--- | :---: | :--- |
| `FIXEDWIDE_BUILD_TESTS` | `ON` | Deterministic unit & negative compile tests |
| `FIXEDWIDE_BUILD_ORACLE_TESTS` | `OFF` | Boost.Multiprecision differential oracle tests |
| `FIXEDWIDE_BUILD_BENCHMARKS` | `OFF` | Paired baseline & competitor benchmarks |
| `FIXEDWIDE_FORCE_PORTABLE` | `OFF` | Force portable multi-limb backend |
| `FIXEDWIDE_SANITIZE` | `OFF` | Enable ASan + UBSan memory/UB sanitizers |
| `FIXEDWIDE_BUILD_FUZZER` | `OFF` | Build libFuzzer harness |
| `BUILD_SHARED_LIBS` | `OFF` | Build shared library (`libfixedwide.so`) |

---

## Benchmarks and Performance

Measured against the **untouched 0.4.0 release**, building a byte-identical
benchmark source with the same compiler and flags, core-pinned and interleaved,
medians of 27 samples per row.

**The generalized version is not yet at parity with 0.4, and this README will
say so until it is.** On Clang 17, 20 of 100 rows are more than 5% slower and the
worst is +63%; the previous release's figures were 32 rows and +161%. What
remains is concentrated in wide `Fixed128` `mul_div`, whose divisor is a runtime
value with nothing for the compiler to fold.

Where this version is ahead of 0.4:

- decimal parsing: **17-19% faster**
- reduced-digit formatting: **12-32% faster**
- toward-zero `quantize`: **36% faster**
- 48 of 100 benchmark rows are at or faster than 0.4

Against other libraries, by semantic class and with every timed result validated
outside the timed region ([full table](reports/BENCHMARK_COMPETITORS.md)):

- versus **Boost.Decimal** `decimal64_t`, the closest comparable contract:
  multiply 2.6 ns vs 3.6 ns, divide 2.2 ns vs 8.7 ns, parse 11.5 ns vs 14.2 ns.
  Formatting is the one row it loses: 14.9 ns vs 12.4 ns.
- **formatting** is about twice as fast as `std::to_chars` on a `double`
- **CNL's unchecked decimal multiply is about 8x faster** than the checked one
  here. That is the cost of returning `std::expected` on overflow instead of
  silently producing a wrong answer, and it is stated rather than omitted.
- **parsing is about 2x slower** than `std::from_chars` on a `double`, which
  produces a binary float and rejects nothing on a decimal grid

Per-row results, raw samples, the measured noise floor and the environment for
every compiler are in [`reports/BENCHMARK_VS_0_4.md`](reports/BENCHMARK_VS_0_4.md)
and `reports/raw/`.

---

## Ethical Policy & License

- **License**: Released under the permissive [MIT License](LICENSE).
- **Ethics**: See [`ETHICS.md`](ETHICS.md) for the author's non-binding statement on military and weapons development.
