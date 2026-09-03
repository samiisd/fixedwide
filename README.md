# fixedwide

A modern C++23 library for **checked fixed-point decimal arithmetic** and **portable fixed-width wide integers** (128/256-bit).

`fixedwide` provides distinct compile-time types for different bit-widths and decimal scales. It guarantees zero heap allocation, zero virtual dispatch, no runtime scale overhead, and no compiler `_BitInt` ABI leak in public types.

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
- **Strict Portability**:
  - Linux x86-64, Linux AArch64, macOS, Windows MSVC / clang-cl
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
- C++23 compiler (Clang 17+, GCC 13+, or MSVC 19.36+)
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

Microbenchmarking against the untouched 0.4.0 baseline confirms that the generalized multi-scale architecture matches native inline performance:
- Common operations (`Fixed64<12>` and `Fixed128<12>` multiplication and division): **~2.3 – 2.7 ns**
- Exact dependent arithmetic chains: **Within ±2% of baseline 0.4**
- Competitor comparisons:
  - Checked `Fixed64` addition: **0.20 ns** (matching native integer and double)
  - Checked `Fixed64` division: **~3x faster** than Boost.Decimal `decimal64_t`

See [`reports/benchmark_summary.md`](reports/benchmark_summary.md) and [`reports/competitors.csv`](reports/competitors.csv) for full raw tables.

---

## Ethical Policy & License

- **License**: Released under the permissive [MIT License](LICENSE).
- **Ethics**: See [`ETHICS.md`](ETHICS.md) for the author's non-binding statement on military and weapons development.
