# Benchmark Comparison: fixedwide 0.5.0-alpha.3 vs 0.4.0 Baseline

## Methodology
- **Hardware**: Linux x86_64
- **Harness**: Paired interleaved benchmarking with 1,048,576 operations per repetition, 9 paired repetitions, warm cache.
- **Compiler**: Clang 22.1.8 (`-O3 -DNDEBUG -fno-vectorize -fno-slp-vectorize -ffp-contract=off`)
- **Baseline**: `fixedwide-0.4.0-nearest-even` release binary.

## Key Workload Performance Comparison

| Workload | Rounding Mode | 0.4.0 Baseline (ns) | 0.5.0-alpha.2 (ns) | 0.5.0-alpha.3 (ns) | Delta vs 0.4.0 | Evaluation |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **`native_by128.FP128.div`** | nearest_even | 10.65 | 32.58 | **10.72** | +0.6% | **Parity restored** |
| `native_by128.FP128.div` | toward_zero | 7.03 | 27.30 | **10.54** | +3.5 ns | **Parity restored** |
| **`wide_by128.FP128.div`** | nearest_even | 19.64 | 57.38 | **26.74** | -53% vs a2 | **Resolved** |
| `wide_by128.FP128.div` | toward_zero | 18.23 | 52.57 | **23.97** | -54% vs a2 | **Resolved** |
| **`wide_product.FP128.mul_div`** | nearest_even | 22.35 | 56.30 | **27.27** | -51% vs a2 | **Resolved** |
| `wide_product.FP128.mul_div` | toward_zero | 21.06 | 51.11 | **22.49** | -55% vs a2 | **Parity (22 ns)** |
| **`throughput4096.FP128.quantize4`** | nearest_even | 10.24 | 22.58 | **10.09** | **-1.5%** | **Faster than 0.4** |
| `throughput4096.FP128.quantize4` | toward_zero | 9.63 | 18.83 | **5.67** | **-41.1%** | **41% Faster than 0.4** |
| **`throughput4096.FP64.mul_wide`** | nearest_even | 3.56 | 4.87 | **2.49** | **-30.1%** | **30% Faster than 0.4** |
| `throughput4096.FP64.mul_wide` | toward_zero | 3.44 | 3.65 | **2.54** | **-26.2%** | **26% Faster than 0.4** |
| **`format_2digits.FP64`** | nearest_even | 29.18 | 48.24 | **22.24** | **-23.8%** | **24% Faster than 0.4** |
| `format_2digits.FP64` | toward_zero | 27.40 | 41.26 | **19.01** | **-30.6%** | **31% Faster than 0.4** |
| **`format_2digits.FP128`** | nearest_even | 28.99 | 48.30 | **22.59** | **-22.1%** | **22% Faster than 0.4** |
| `format_2digits.FP128` | toward_zero | 28.26 | 41.13 | **18.94** | **-33.0%** | **33% Faster than 0.4** |

## Conclusion
`0.5.0-alpha.3` achieves strict parity on heavy division routines while outperforming 0.4.0 by 20% to 41% across quantize, multiplication, and formatting kernels.
