# CI

Every configuration listed as supported is built **and its tests run** by a job
in [`.github/workflows/ci.yml`](../.github/workflows/ci.yml). A configuration
with no job is marked `not-configured` in
[`reports/EXECUTION_MATRIX.csv`](../reports/EXECUTION_MATRIX.csv) and is not
claimed anywhere, including in the README.

## What each job proves

| Job | What would break it |
|---|---|
| `linux gcc-14` / `clang-18-libc++`, Debug and Release | Anything, on the two toolchains most people have. Includes the Boost.Multiprecision differential oracle |
| `linux arm64` | A little-endian assumption that is really an x86 assumption |
| `macos` (arm64) | An assumption about `long double`, or about libc++ |
| `windows` MSVC and clang-cl | A GNU builtin, GNU inline asm, or `__int128` used without a guard |
| `forced-portable`, `no-int128` | The multi-limb fallback diverging from the native path |
| `asan+ubsan`, both backends | Undefined behaviour or an out-of-bounds access |
| `shared library` | A missing export or an ODR problem |
| `install + find_package consumer` | A broken exported CMake package |
| `conan create` | A recipe that packages something not usable |
| `instruction-count regression gate` | A change that makes the library do measurably more work |
| `fuzz smoke` | A crash reachable from arbitrary text input |
| `coverage` | Nothing; it reports |

Examples are `ctest` tests, so every platform job also proves that all eight
examples compile, run, and still print the right answers.

The long-running checks — a 30-minute fuzz run and the competitor benchmark —
are in [`nightly.yml`](../.github/workflows/nightly.yml), so a pull request is
not held up by them but they cannot silently rot either.

## Compiler and standard-library combinations

This is the part that is easy to get wrong, so it is written down.

**Clang 17 and 18 cannot compile this library against libstdc++**, and neither
can anything else that uses `std::expected`. Those releases report
`__cpp_concepts` as `201907`; libstdc++ gates `<expected>` on `202002`, so the
header defines nothing. It is not a fixable problem on this side.

Measured on `ubuntu:24.04`:

| Compiler | Standard library | `std::expected` |
|---|---|---|
| GCC 13 | libstdc++ | works, but `std::frexpl` is missing — see below |
| GCC 14 | libstdc++ | works |
| Clang 17 | libstdc++ | **no** |
| Clang 18 | libstdc++ | **no** |
| Clang 18 | libc++ | works |
| Clang 20 | libstdc++ | works |

**GCC 13 cannot build this library either.** libstdc++ did not put `frexpl` and
`ldexpl` in namespace `std` until GCC 14, and `src/floating.cpp` needs them.
The library now calls the C++ `std::frexp` / `std::ldexp` overloads instead, so
this is fixed — but GCC 14 remains the floor, and `conanfile.py` says so.

**The fuzz and coverage jobs use Clang 20 with libstdc++, not Clang 18 with
libc++.** The libFuzzer runtime Ubuntu ships is compiled against libstdc++, so
linking it into a libc++ binary leaves `std::thread::hardware_concurrency` and
several `std::string` symbols undefined. One standard library per binary.

**MSVC** has no `__int128` and no GNU inline assembly, so it takes the portable
multi-limb backend automatically — the same code path the `forced-portable` job
exercises on Linux. **clang-cl** does expose `__int128`, but on Windows its
runtime may not provide the 128-bit division builtins; `CMakeLists.txt` probes
that at configure time and auto-selects the portable backend when needed. MSVC
also needs `/Zc:__cplusplus`, `/Zc:preprocessor` and `/utf-8`, which
`CMakeLists.txt` adds.

## Running a job's work locally

Every job runs commands that work on a workstation too:

```bash
# what the linux job does
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DFIXEDWIDE_BUILD_TESTS=ON -DFIXEDWIDE_BUILD_EXAMPLES=ON \
      -DFIXEDWIDE_BUILD_ORACLE_TESTS=ON
cmake --build build && ctest --test-dir build --output-on-failure

# every configuration at once, recording what actually happened
./scripts/verify_all.sh          # writes reports/EXECUTION_MATRIX.csv

# the regression gate
./scripts/icount.sh > current.csv
python3 scripts/compare_icount.py benchmarks/baseline/x86_64-gcc-14.csv current.csv

# the conan package
conan create . --build=missing -s compiler.cppstd=23
```

`scripts/Dockerfile.bench` pins an `ubuntu:24.04` image with the CI compilers,
which is the quickest way to reproduce a Linux job failure without pushing.

## Releases

Pushing a `v*` tag runs [`release.yml`](../.github/workflows/release.yml). It
calls `scripts/release.sh`, which builds the archive with `git archive` — so it
contains exactly what is committed and nothing from a working tree — then
extracts it somewhere else and verifies the manifest inside the extracted tree.
The workflow then builds and tests that extracted copy before publishing it.
