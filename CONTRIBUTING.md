# Contributing to fixedwide

## Principles

1. **Correctness first.** Numerical routines must match exact integer rational
   arithmetic. No silent overflow, no dropped remainder, no wrong answer
   returned in place of an error.
2. **Minimalism.** No speculative abstractions. Prefer the standard library and
   standard C++23 to anything written here.
3. **Nothing is claimed until it is executed.** A platform is described as
   supported only when a CI job builds it *and runs its tests*. Everything else
   is `not-configured` in `reports/EXECUTION_MATRIX.csv`. This applies to
   performance numbers too: quote a row in `reports/`, or do not quote one.
4. **Portable, and tested that way.** The library must build on GCC 14+,
   Clang 18+ with libc++ (or Clang 19+ with either), AppleClang 15+ and MSVC
   19.3x, on x86-64 and AArch64, with the native and the portable backend.
   `docs/ci.md` records which combinations actually work and why some do not.

## Formatting

```bash
pip install clang-format==22.1.8   # the exact version CI uses
./scripts/format.sh                # rewrite
./scripts/format.sh --check        # what CI runs
```

The version is pinned because clang-format majors format the same file
differently; `scripts/format.sh` refuses to run with a different one rather than
producing a diff nobody asked for. A few files are deliberately excluded and the
script says why — most importantly `benchmarks/rounding_bench.cpp`, which the
paired 0.4 comparison requires to stay byte-identical to 0.4's copy.

Warnings are errors in CI over three toolchains and both backends. The library
builds with `-Wall -Wextra -Wconversion -Wsign-conversion -Wshadow
-Wold-style-cast` and is clean.

## Before opening a pull request

```bash
# tests and examples, both backends
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DFIXEDWIDE_BUILD_TESTS=ON -DFIXEDWIDE_BUILD_EXAMPLES=ON \
      -DFIXEDWIDE_BUILD_ORACLE_TESTS=ON
cmake --build build && ctest --test-dir build --output-on-failure

# sanitizers
cmake --preset sanitize && ctest --preset sanitize
```

CI runs the rest — every platform, both backends, Conan, packaging, the fuzzer
and the regression gate. `./scripts/verify_all.sh` runs the local half of it in
one go and rewrites `reports/EXECUTION_MATRIX.csv`.

## Performance

Hot paths must not regress. The gate is `benchmarks/baseline/*.csv`: CI counts
retired instructions per operation under Valgrind and fails if any workload
grows by more than 1%. It is deterministic, so a failure is a real change in the
generated code and not noise.

If a change is meant to move a number:

```bash
./scripts/icount.sh --update      # re-record the baseline
```

and say in the commit message why it moved. If you add an operation worth
protecting, add a row to `benchmarks/icount.cpp` and re-record.

Wall-clock work — the paired comparison against 0.4 and the competitor suite —
is in `docs/benchmarks.md`. Neither can gate CI: the first needs a 0.4 source
tree that is not in this repository, and the second is a comparison rather than
a threshold.

## Adding an example

Examples are documentation that cannot go stale, because each one is a ctest
test that checks its own output before printing `OK`. Add the file, add its name
to `examples/CMakeLists.txt`, and add a row to `examples/README.md` and to the
table in `README.md`. Keep it to one file and one idea.

## Adding a test

Anything with a branch, a loop, or a platform assumption needs one. Two things
this repository has learned the hard way:

- **Test the concept, not the call.** `std::format("{}", v)` compiling under
  libstdc++ did not mean `std::formattable<T, char>` was satisfied under libc++.
  Assert the concept.
- **Do not encode your platform's numbers.** A test that perturbed a
  `long double` by 2^-60 assumed 61 significand bits and failed on Apple
  silicon, where `long double` is `double`. Ask `std::numeric_limits` what the
  platform has.
