# Compile-Time & Header Parsing Benchmark: fixedwide 0.5.0-alpha.3

## Methodology
Measured parsing and syntax-checking duration using Clang 22.1.8:
```bash
clang++ -std=c++23 -Iinclude -fsyntax-only -x c++ -
```

## Results Across Releases

| Header Included | 0.4.0 Baseline | 0.5.0-alpha.2 | 0.5.0-alpha.3 | Delta vs alpha.2 | Gate Status |
| :--- | :---: | :---: | :---: | :---: | :---: |
| `<fixedwide/fixed.hpp>` | 52.5 ms | 79.3 ms | **30.0 ms** | **-62.2%** | **PASS** |
| `<fixedwide/arithmetic.hpp>` | 77.2 ms | 152.8 ms | **47.1 ms** | **-69.2%** | **PASS** |
| `<fixedwide/chars.hpp>` | 176.9 ms | 198.1 ms | **71.7 ms** | **-63.8%** | **PASS** |
| `<fixedwide/wide.hpp>` | 38.1 ms | 41.2 ms | **24.5 ms** | **-40.5%** | **PASS** |

## Key Optimizations
1. **`max_integer_allowed`**: Replaced consteval 256-iteration Knuth long-division loop with direct single-word division for `Bits <= 64`.
2. **Modular Inlining**: Shifted heavy multi-precision fallback routines to compiled source translation units (`arithmetic.cpp`, `chars.cpp`) while retaining inlined hardware fast paths in headers.
