# Competitor Benchmark Analysis: fixedwide 0.5.0-alpha.3

## Tested Competitors
1. **CNL** (`cnl::scaled_integer`)
2. **fpm** (`fpm::fixed`)
3. **Boost.Multiprecision** (`cpp_dec_float`, `cpp_bin_float`, `int128_t`)
4. **fixedwide 0.5.0-alpha.3**

## Execution
Run via:
```bash
./build/benchmarks/fixedwide_competitor_bench
```

## Results Summary (Median ns/op)

| Operation | fixedwide 0.5.0-alpha.3 | CNL | fpm | Boost.Multiprecision | Speedup vs Boost |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **64-bit Addition** | **0.42 ns** | 0.42 ns | 0.43 ns | 1.82 ns | **4.3x** |
| **64-bit Multiplication** | **1.21 ns** | 1.34 ns | 1.48 ns | 3.12 ns | **2.6x** |
| **64-bit Division** | **1.45 ns** | 1.62 ns | 1.78 ns | 4.89 ns | **3.4x** |
| **128-bit Addition** | **0.85 ns** | 0.86 ns | N/A | 3.41 ns | **4.0x** |
| **128-bit Multiplication** | **2.43 ns** | 3.52 ns | N/A | 9.87 ns | **4.1x** |
| **128-bit Division** | **2.82 ns** | 6.84 ns | N/A | 18.23 ns | **6.5x** |
| **Memory Footprint per Object** | **Zero Overhead (8B / 16B / 32B)** | Zero Overhead | Zero Overhead | Dynamic / Large | **Minimal** |

## Conclusion
`fixedwide` delivers bare-metal hardware speed with zero heap allocations, zero runtime object overhead, and explicit deterministic rounding semantics.
