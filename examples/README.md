# Examples

Eight short programs, each one file, each about a single idea. Every one of them
is a `ctest` test: `ctest -R fixedwide.example` compiles and runs all eight, and
each checks its own output before printing `OK`. An example that stops being
correct fails the build, so nothing here can drift away from the library.

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFIXEDWIDE_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build -R fixedwide.example --output-on-failure

./build/examples/example_01_quick_start      # or just run one
```

Read them in order; each assumes the one before it.

| # | Example | What it shows | Reference |
|---|---|---|---|
| 01 | [`01_quick_start.cpp`](01_quick_start.cpp) | Parse text, multiply across scales, format the result — the three steps almost every use goes through | [Primary types](../docs/api_reference.md#primary-types) |
| 02 | [`02_rounding_modes.cpp`](02_rounding_modes.cpp) | All six `Rounding` values on the same division, side by side, including the one that refuses | [Rounding modes](../docs/api_reference.md#rounding-modes) |
| 03 | [`03_error_handling.cpp`](03_error_handling.cpp) | Overflow, divide-by-zero and inexact as values; chaining with `and_then` / `transform` / `value_or` | [Error types](../docs/api_reference.md#error-types) |
| 04 | [`04_mixed_scales.cpp`](04_mixed_scales.cpp) | `mul_to` / `div_to` / `add_to` / `fixed_cast`, exact cross-scale comparison, and what is compile-rejected | [Mixed-scale operations](../docs/api_reference.md#mixed-scale-operations) |
| 05 | [`05_text_io.cpp`](05_text_io.cpp) | `to_chars` into your own buffer, `FormatOptions`, `std::format`, `operator<<` | [Text conversion](../docs/api_reference.md#text-conversion) |
| 06 | [`06_binary_roundtrip.cpp`](06_binary_roundtrip.cpp) | `to_bytes` / `from_bytes` with the byte order named, and unaligned packet access | [Binary serialization](../docs/api_reference.md#binary-serialization) |
| 07 | [`07_money_ledger.cpp`](07_money_ledger.cpp) | An invoice with tax, computed twice — in `double` and in `Fixed64<2>` — so the drift is visible | [Primary types](../docs/api_reference.md#primary-types) |
| 08 | [`08_constexpr.cpp`](08_constexpr.cpp) | The whole arithmetic surface at compile time, errors included | [Arithmetic functions](../docs/api_reference.md#arithmetic-functions) |

## `consumer/`

Not a tutorial. [`consumer/`](consumer) is a standalone downstream project that
finds the library through `find_package` and nothing else; CI builds it against
an install tree to prove the exported CMake package works. `test_package/` at
the repository root is the same check for the Conan package.
