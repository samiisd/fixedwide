# fixedwide

**Exact decimal arithmetic for C++23, with the overflow checked.** Fixed-point
decimal types and portable 128/256-bit integers. No heap allocation, no virtual
dispatch, no exceptions — every fallible operation returns `std::expected`.

[![CI](https://github.com/Samiisd/fixedwide/actions/workflows/ci.yml/badge.svg)](https://github.com/Samiisd/fixedwide/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)

---

## The problem

Money in a `double` drifts, because `0.01` is not a number a binary float has:

```cpp
double total = 0.0;
for (int i = 0; i < 100; ++i) total += 0.01;
// total == 1.0000000000000007
```

Money in a hand-rolled `int64_t` of cents does not drift, but it wraps, and it
wraps silently:

```cpp
std::int64_t a = 5'000'000'000'000'000'000;  // 5e18 cents
std::int64_t total = a + a;                  // undefined behaviour, no diagnostic
```

`fixedwide` gives you the integer, keeps the decimal point in the type, and
makes the failure a value you cannot ignore:

```cpp
#include <fixedwide/all.hpp>
using Money = fixedwide::Fixed64<2>;        // an int64 of cents

auto total = Money::from_raw(0);
for (int i = 0; i < 100; ++i) {
    total = add(total, parse<Money>("0.01").value()).value();
}
// to_string(total) == "1.00", exactly

auto overflowed = add(Money::max(), parse<Money>("0.01").value());
// overflowed.error() == ArithmeticError::overflow  -- reported, not wrapped
```

The scale is part of the type, so a price and a quantity are different types and
mixing them without saying what you meant does not compile.

---

## Install

**Conan**

```bash
conan create .            # or add fixedwide to your conanfile
```
```cmake
find_package(fixedwide REQUIRED)
target_link_libraries(app PRIVATE fixedwide::fixedwide)
```

**CMake FetchContent / CPM**

```cmake
include(FetchContent)
FetchContent_Declare(fixedwide
    GIT_REPOSITORY https://github.com/Samiisd/fixedwide.git
    GIT_TAG        v0.5.0)
FetchContent_MakeAvailable(fixedwide)
target_link_libraries(app PRIVATE fixedwide::fixedwide)
```

**System install**

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DFIXEDWIDE_BUILD_TESTS=OFF
cmake --build build --target install
```

All three give you the identical two lines of CMake. Requires a C++23 compiler
with `std::expected` — see [supported platforms](#supported-platforms).

---

## The types

| Alias | Storage | Width | Decimals | Size |
|---|---|---:|---|---:|
| `Fixed8<D>` | `std::int8_t` | 8 | 0–2 | 1 |
| `Fixed16<D>` | `std::int16_t` | 16 | 0–4 | 2 |
| `Fixed32<D>` | `std::int32_t` | 32 | 0–9 | 4 |
| `Fixed64<D>` | `std::int64_t` | 64 | 0–18 | 8 |
| `Fixed128<D>` | `wide::int128` | 128 | 0–38 | 16 |
| `Fixed256<D>` | `wide::int256` | 256 | 0–76 | 32 |

A value is its scaled integer and nothing else — no exponent field, no tag, no
indirection. `sizeof(Fixed64<2>) == 8`.

What the library does with that:

- **Distinct types per scale.** `Fixed64<4>` and `Fixed64<2>` do not implicitly
  convert. Unit mix-ups are compile errors.
- **One rounding per expression.** `mul_to<Dest>(a, b)` forms the exact rational
  product at full width and rounds once, into the destination. Not twice.
- **Exact cross-scale comparison.** `==` and `<=>` work across any two types
  without rounding either side, and are `constexpr`.
- **Six rounding modes**, including `exact`, which refuses to round and tells
  you instead.
- **Errors as values.** `overflow`, `division_by_zero`, `inexact`,
  `invalid_precision`, `invalid_value` — never a wrong answer.
- **`constexpr` arithmetic.** A rate table can be computed and checked at
  compile time.
- **No allocation on any arithmetic, parsing or formatting path.** `to_chars`
  and `from_chars` write into your buffer; a test that replaces `operator new` and
  counts every call proves it. `to_string` is the one convenience that allocates,
  and is in its own header. Works with `-fno-exceptions` and `-fno-rtti`.

---

## Performance

Median ns/op, Clang 22, `-O3`, pinned to one core, every timed result validated
against an independent oracle outside the timed region. Full table, method and
raw samples: [`reports/BENCHMARK_COMPETITORS.md`](reports/BENCHMARK_COMPETITORS.md).

| | **fixedwide**<br>`Fixed64<12>` | Boost.Decimal<br>`decimal64_t` | CNL<br>`scaled_integer` | `double` |
|---|---:|---:|---:|---:|
| multiply | **2.61** | 3.53 | 0.35 | 0.23 |
| divide | **2.17** | 8.53 | 1.09 | 0.74 |
| parse | **12.17** | 14.17 | — | 5.18 |
| format | 13.97 | **12.36** | — | 28.30 |
| exact in decimal | yes | yes | yes | **no** |
| overflow detected | **yes** | no | no | no |

Read honestly:

- Against **Boost.Decimal**, the closest comparable contract here, multiply is
  faster, divide is about **four times** faster, and parsing is faster.
  Formatting is the row it loses, and that is an open item.
- **CNL's decimal multiply is about 8x quicker**, because it is a raw `int64`
  multiply and a rescale with no overflow detection: on overflow it silently
  produces a wrong answer. Returning `std::expected` is what that row costs, and
  it is the trade this library exists to make.
- Formatting is about **twice as fast as `std::to_chars` on a `double`**.
  Parsing is about twice as slow — but `std::from_chars` produces a binary float
  and rejects nothing on a decimal grid, so it is not the same job.
  Boost.Decimal is the row to read against.

Side by side, the same calculation three ways:

```cpp
// double: fast, and wrong in the last place
double notional = 123.4567 * 10.50;              // 1296.2953499999999

// hand-rolled int64: you now own the scale, the rounding and the overflow check
int64_t notional = 1234567LL * 1050LL / 100;     // 12962953 -- but at which scale?
                                                 // truncated, and unchecked

// fixedwide: exact, one rounding, overflow reported, scale in the type
auto notional = mul_to<Fixed128<6>>(price, qty); // 1296.295350, or an error
```

### Regressions

Every pull request re-measures **retired instructions per operation** under
Valgrind and fails if any workload grows by more than 1%. Instruction counts
are deterministic — two runs are byte-identical — which is what makes a 1% gate
possible on a shared CI runner, where wall-clock noise is far larger than that.
Baselines are committed in [`benchmarks/baseline/`](benchmarks/baseline).

There is one performance cliff in the API, and it is documented rather than
hidden: cross-scale `mul_to` / `div_to` are about 160x cheaper when the aligned
intermediate fits 126 bits. [Where the edge is](docs/benchmarks.md#the-mixed-path-cliff).

---

## Examples

Eight short programs, each one file, each one idea. All eight are `ctest` tests
that check their own output, so they cannot drift away from the library.

| # | Example | What it shows |
|---|---|---|
| 01 | [Quick start](examples/01_quick_start.cpp) | Parse, multiply across scales, format |
| 02 | [Rounding modes](examples/02_rounding_modes.cpp) | All six, side by side on the same division |
| 03 | [Error handling](examples/03_error_handling.cpp) | Overflow and divide-by-zero as values; `and_then` chaining |
| 04 | [Mixed scales](examples/04_mixed_scales.cpp) | `mul_to` / `fixed_cast`, and what is compile-rejected |
| 05 | [Text I/O](examples/05_text_io.cpp) | `to_chars`, `FormatOptions`, `std::format`, streams |
| 06 | [Binary round-trip](examples/06_binary_roundtrip.cpp) | `to_bytes` / `from_bytes`, both byte orders |
| 07 | [Money ledger](examples/07_money_ledger.cpp) | An invoice in `double` and in `Fixed64<2>`, side by side |
| 08 | [constexpr](examples/08_constexpr.cpp) | The whole arithmetic surface at compile time |

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build -R fixedwide.example --output-on-failure
```

More in [`examples/README.md`](examples/README.md).

---

## Documentation

| | |
|---|---|
| [API reference](docs/api_reference.md) | Every header, every function, naming rules, the 0.4 compatibility surface |
| [Architecture](docs/architecture.md) | Storage layout and the arithmetic pipeline |
| [Benchmarks](docs/benchmarks.md) | How each number was produced, and how to reproduce it |
| [CI](docs/ci.md) | What each job proves, and what is deliberately not claimed |

Every public declaration carries a `///` doc comment, and a top-level build
writes `build/compile_commands.json`, so `clangd` — and VS Code, Neovim, CLion
and Qt Creator with it — shows all of this on hover with no configuration.

---

## Supported platforms

A platform is listed here only when a CI job builds it **and runs its tests**.
Anything else is marked `not-configured` in
[`reports/EXECUTION_MATRIX.csv`](reports/EXECUTION_MATRIX.csv) and is not
claimed.

Every row below is `executed-pass` in
[the last CI run](https://github.com/Samiisd/fixedwide/actions/workflows/ci.yml).

| Platform | Toolchain | Backend |
|---|---|---|
| Linux x86-64 | GCC 14, Debug and Release | native |
| Linux x86-64 | Clang 18 + libc++, Debug and Release | native |
| Linux x86-64 | Clang 20 + libstdc++ | native |
| Linux AArch64 | GCC 14 | native |
| macOS (Apple silicon) | AppleClang, macos-14 and macos-15 | native |
| Windows x64 | MSVC | portable |
| Windows x64 | clang-cl | portable |
| Linux x86-64 | GCC 14, `FIXEDWIDE_FORCE_PORTABLE` | portable |
| Linux x86-64 | GCC 14, `__SIZEOF_INT128__` undefined | portable |
| Linux x86-64 | Clang 18, ASan + UBSan, both backends | both |

Also executed on every push: the shared library, install plus an external
`find_package` consumer, `conan create` with `test_package` in both backends,
a libFuzzer smoke run, and the instruction-count gate. Executed on the
maintainer's hardware and recorded in `reports/`: Clang 22 and GCC 16, a
no-exceptions / no-RTTI build, and AArch64 on a real device.

Not configured, and therefore not claimed: Windows ARM64, and big-endian
hardware — `binary.hpp` implements both byte orders but only little-endian has
ever been executed.

Note on Clang: Clang 17 and 18 report `__cpp_concepts` as `201907`, and
libstdc++ gates `<expected>` on `202002`, so **this library cannot compile with
those compilers against libstdc++** — nor can anything else that uses
`std::expected`. Pair them with libc++ (`-stdlib=libc++`), or use Clang 19 or
newer. See [docs/ci.md](docs/ci.md).

---

## Building

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

| Option | Default | |
|---|---|---|
| `FIXEDWIDE_BUILD_TESTS` | `ON` when top-level | Unit, audit and negative-compile tests |
| `FIXEDWIDE_BUILD_EXAMPLES` | `ON` when top-level | The eight examples, as ctest tests |
| `FIXEDWIDE_BUILD_ORACLE_TESTS` | `OFF` | Boost.Multiprecision differential tests |
| `FIXEDWIDE_BUILD_BENCHMARKS` | `OFF` | Benchmark harnesses |
| `FIXEDWIDE_FORCE_PORTABLE` | `OFF` | Multi-limb backend, no `__int128`, no inline asm |
| `FIXEDWIDE_SANITIZE` | `OFF` | ASan + UBSan |
| `FIXEDWIDE_COVERAGE` | `OFF` | Clang source coverage |
| `FIXEDWIDE_BUILD_FUZZER` | `OFF` | libFuzzer harness (needs `FIXEDWIDE_SANITIZE`) |
| `BUILD_SHARED_LIBS` | `OFF` | Build `libfixedwide.so` |

`CMakePresets.json` has `default`, `release`, `sanitize`, `coverage` and
`benchmarks` configured.

---

## Status

Pre-1.0: the API may still change, and `CHANGELOG.md` will say when it does.
What has actually been executed, and what has not, is in
[`STATUS.md`](STATUS.md) and `reports/`. Contributions: [`CONTRIBUTING.md`](CONTRIBUTING.md).

## License

[MIT](LICENSE). See [`ETHICS.md`](ETHICS.md) for the author's non-binding
statement on military and weapons development.
