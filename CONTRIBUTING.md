# Contributing to fixedwide

Thank you for your interest in contributing to `fixedwide`!

## Code Quality and Design Principles

1. **Correctness First**: Numerical routines must match mathematical integer rational arithmetic exactly. No silent overflows, no truncated remainders.
2. **Minimalism**: Avoid speculative abstractions. Use standard C++23 features and standard library components wherever possible.
3. **Deterministic & Portable**: Code must compile and pass all tests cleanly with both Clang (>= 17) and GCC (>= 14) on x86_64 and AArch64.
4. **Performance Parity**: Hot paths must not regress against assembly baselines. Any changes to arithmetic cores must be benchmarked with `./build/benchmarks/fixedwide_bench` and `fixedwide_rounding_bench`.

## Testing Workflow

Before opening a pull request, ensure all tests pass:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DFIXEDWIDE_BUILD_TESTS=ON -DFIXEDWIDE_BUILD_ORACLE_TESTS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Also run the sanitizers:

```bash
cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DFIXEDWIDE_SANITIZE=ON -DFIXEDWIDE_BUILD_TESTS=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```
