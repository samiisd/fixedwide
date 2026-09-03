# Benchmark Report: Generalized fixedwide vs 0.4.0 Baseline

## Configuration
- **Hardware**: Linux x86_64
- **Repetitions**: 9
- **Iterations per Repetition**: 1,048,576
- **Compiler**: Clang 22.1.8 (`-O3 -DNDEBUG -fno-vectorize -fno-slp-vectorize -ffp-contract=off`)
- **Baseline**: Untouched `fixedwide-0.4.0-nearest-even` release binary
- **Target**: Generalized `fixedwide` C++23 library with distinct compile-time scales

## Paired Benchmark Results Table

| Workload & Rounding Mode | Baseline 0.4 (ns/op) | Generalized (ns/op) | Delta (%) | Status |
| :--- | :---: | :---: | :---: | :---: |
| `exact_chain.FP128.div_nearest_even` | 4.083 | 3.811 | -6.65% | PASS |
| `exact_chain.FP128.div_toward_zero` | 4.059 | 3.853 | -5.08% | PASS |
| `exact_chain.FP128.mul_div_nearest_even` | 3.871 | 3.945 | +1.90% | PASS |
| `exact_chain.FP128.mul_div_toward_zero` | 3.899 | 4.051 | +3.92% | PASS |
| `exact_chain.FP128.mul_nearest_even` | 3.785 | 3.746 | -1.02% | PASS |
| `exact_chain.FP128.mul_toward_zero` | 3.758 | 3.776 | +0.48% | PASS |
| `exact_chain.FP64.div_nearest_even` | 3.834 | 3.771 | -1.64% | PASS |
| `exact_chain.FP64.div_toward_zero` | 3.819 | 3.741 | -2.06% | PASS |
| `exact_chain.FP64.mul_div_nearest_even` | 3.684 | 3.755 | +1.92% | PASS |
| `exact_chain.FP64.mul_div_toward_zero` | 3.662 | 3.736 | +2.03% | PASS |
| `exact_chain.FP64.mul_nearest_even` | 3.768 | 3.765 | -0.08% | PASS |
| `exact_chain.FP64.mul_toward_zero` | 3.703 | 3.734 | +0.83% | PASS |
| `exact_results.FP128.div_nearest_even` | 2.417 | 2.565 | +6.12% | CHECK |
| `exact_results.FP128.div_toward_zero` | 2.583 | 2.587 | +0.18% | PASS |
| `exact_results.FP128.mul_div_nearest_even` | 3.221 | 3.672 | +13.98% | CHECK |
| `exact_results.FP128.mul_div_toward_zero` | 3.119 | 3.742 | +19.97% | CHECK |
| `exact_results.FP128.mul_nearest_even` | 2.646 | 2.558 | -3.31% | PASS |
| `exact_results.FP128.mul_toward_zero` | 2.713 | 2.513 | -7.34% | PASS |
| `exact_results.FP64.div_nearest_even` | 2.553 | 2.684 | +5.12% | CHECK |
| `exact_results.FP64.div_toward_zero` | 2.604 | 2.670 | +2.54% | PASS |
| `exact_results.FP64.mul_div_nearest_even` | 2.549 | 2.643 | +3.66% | PASS |
| `exact_results.FP64.mul_div_toward_zero` | 2.556 | 2.622 | +2.58% | PASS |
| `exact_results.FP64.mul_nearest_even` | 2.593 | 2.596 | +0.10% | PASS |
| `exact_results.FP64.mul_toward_zero` | 2.570 | 2.600 | +1.19% | PASS |
| `format_2digits.FP128_nearest_even` | 16.716 | 25.632 | +53.34% | CHECK |
| `format_2digits.FP128_toward_zero` | 15.489 | 20.780 | +34.16% | CHECK |
| `format_2digits.FP64_nearest_even` | 16.799 | 25.173 | +49.85% | CHECK |
| `format_2digits.FP64_toward_zero` | 15.525 | 20.566 | +32.47% | CHECK |
| `fullrange.FP64.mul_wide_nearest_even` | 2.938 | 4.140 | +40.95% | CHECK |
| `fullrange.FP64.mul_wide_toward_zero` | 2.474 | 4.080 | +64.88% | CHECK |
| `halfway_ties.FP128.div_nearest_even` | 3.258 | 3.988 | +22.40% | CHECK |
| `halfway_ties.FP128.div_toward_zero` | 2.574 | 2.587 | +0.50% | PASS |
| `halfway_ties.FP128.mul_div_nearest_even` | 4.701 | 4.898 | +4.20% | PASS |
| `halfway_ties.FP128.mul_div_toward_zero` | 3.121 | 3.779 | +21.07% | CHECK |
| `halfway_ties.FP128.mul_nearest_even` | 2.644 | 3.992 | +50.99% | CHECK |
| `halfway_ties.FP128.mul_toward_zero` | 2.737 | 2.520 | -7.94% | PASS |
| `halfway_ties.FP64.div_nearest_even` | 2.476 | 2.956 | +19.39% | CHECK |
| `halfway_ties.FP64.div_toward_zero` | 2.589 | 2.669 | +3.07% | PASS |
| `halfway_ties.FP64.mul_div_nearest_even` | 2.452 | 2.993 | +22.06% | CHECK |
| `halfway_ties.FP64.mul_div_toward_zero` | 2.557 | 2.618 | +2.40% | PASS |
| `halfway_ties.FP64.mul_nearest_even` | 2.631 | 2.366 | -10.07% | PASS |
| `halfway_ties.FP64.mul_toward_zero` | 2.628 | 2.604 | -0.91% | PASS |
| `inexact_chain.FP128.div_nearest_even` | 5.439 | 5.067 | -6.85% | PASS |
| `inexact_chain.FP128.div_toward_zero` | 4.058 | 3.854 | -5.04% | PASS |
| `inexact_chain.FP128.mul_div_nearest_even` | 5.136 | 5.243 | +2.09% | PASS |
| `inexact_chain.FP128.mul_div_toward_zero` | 3.898 | 4.047 | +3.82% | PASS |
| `inexact_chain.FP128.mul_nearest_even` | 5.210 | 7.675 | +47.30% | CHECK |
| `inexact_chain.FP128.mul_toward_zero` | 3.733 | 3.782 | +1.32% | PASS |
| `inexact_chain.FP64.div_nearest_even` | 5.143 | 5.067 | -1.47% | PASS |
| `inexact_chain.FP64.div_toward_zero` | 3.812 | 3.747 | -1.70% | PASS |
| `inexact_chain.FP64.mul_div_nearest_even` | 5.088 | 5.165 | +1.51% | PASS |
| `inexact_chain.FP64.mul_div_toward_zero` | 3.655 | 3.738 | +2.27% | PASS |
| `inexact_chain.FP64.mul_nearest_even` | 5.002 | 4.815 | -3.75% | PASS |
| `inexact_chain.FP64.mul_toward_zero` | 3.725 | 3.716 | -0.24% | PASS |
| `native_by128.FP128.div_nearest_even` | 9.910 | 20.586 | +107.73% | CHECK |
| `native_by128.FP128.div_toward_zero` | 5.976 | 18.826 | +215.00% | CHECK |
| `native_by64.FP128.div_nearest_even` | 6.281 | 15.808 | +151.67% | CHECK |
| `native_by64.FP128.div_toward_zero` | 3.942 | 12.947 | +228.48% | CHECK |
| `parse_extra_digit.FP128_nearest_even` | 33.859 | 39.369 | +16.27% | CHECK |
| `parse_extra_digit.FP128_toward_zero` | 33.452 | 37.585 | +12.36% | CHECK |
| `parse_extra_digit.FP64_nearest_even` | 34.308 | 38.387 | +11.89% | CHECK |
| `parse_extra_digit.FP64_toward_zero` | 33.926 | 37.395 | +10.23% | CHECK |
| `throughput256.FP128.div_nearest_even` | 3.263 | 3.399 | +4.18% | PASS |
| `throughput256.FP128.div_toward_zero` | 2.586 | 2.589 | +0.13% | PASS |
| `throughput256.FP128.mul_div_nearest_even` | 4.687 | 4.896 | +4.47% | PASS |
| `throughput256.FP128.mul_div_toward_zero` | 3.112 | 3.790 | +21.78% | CHECK |
| `throughput256.FP128.mul_nearest_even` | 2.610 | 2.569 | -1.57% | PASS |
| `throughput256.FP128.mul_toward_zero` | 2.728 | 3.782 | +38.63% | CHECK |
| `throughput256.FP64.div_nearest_even` | 2.461 | 2.944 | +19.60% | CHECK |
| `throughput256.FP64.div_toward_zero` | 2.433 | 2.433 | +0.02% | PASS |
| `throughput256.FP64.mul_div_nearest_even` | 2.425 | 2.980 | +22.89% | CHECK |
| `throughput256.FP64.mul_div_toward_zero` | 2.533 | 2.579 | +1.82% | PASS |
| `throughput256.FP64.mul_nearest_even` | 2.757 | 2.369 | -14.09% | PASS |
| `throughput256.FP64.mul_toward_zero` | 2.539 | 2.610 | +2.78% | PASS |
| `throughput4096.FP128.div_nearest_even` | 3.258 | 3.396 | +4.24% | PASS |
| `throughput4096.FP128.div_toward_zero` | 2.572 | 2.593 | +0.81% | PASS |
| `throughput4096.FP128.mul_div_nearest_even` | 4.698 | 4.901 | +4.33% | PASS |
| `throughput4096.FP128.mul_div_toward_zero` | 3.128 | 3.788 | +21.09% | CHECK |
| `throughput4096.FP128.mul_nearest_even` | 2.623 | 7.667 | +192.31% | CHECK |
| `throughput4096.FP128.mul_toward_zero` | 2.738 | 3.906 | +42.66% | CHECK |
| `throughput4096.FP128.quantize4_nearest_even` | 6.036 | 14.224 | +135.65% | CHECK |
| `throughput4096.FP128.quantize4_toward_zero` | 5.699 | 9.319 | +63.53% | CHECK |
| `throughput4096.FP64.div_nearest_even` | 2.486 | 2.942 | +18.36% | CHECK |
| `throughput4096.FP64.div_toward_zero` | 2.419 | 2.415 | -0.15% | PASS |
| `throughput4096.FP64.mul_div_nearest_even` | 2.449 | 2.989 | +22.07% | CHECK |
| `throughput4096.FP64.mul_div_toward_zero` | 2.554 | 2.587 | +1.28% | PASS |
| `throughput4096.FP64.mul_nearest_even` | 2.730 | 2.382 | -12.76% | PASS |
| `throughput4096.FP64.mul_toward_zero` | 2.543 | 2.625 | +3.21% | PASS |
| `throughput4096.FP64.mul_wide_nearest_even` | 2.427 | 2.584 | +6.49% | CHECK |
| `throughput4096.FP64.mul_wide_toward_zero` | 2.583 | 2.561 | -0.86% | PASS |
| `throughput4096.FP64.quantize4_nearest_even` | 3.234 | 6.240 | +92.99% | CHECK |
| `throughput4096.FP64.quantize4_toward_zero` | 3.190 | 3.339 | +4.66% | PASS |
| `wide_by128.FP128.div_nearest_even` | 14.008 | 35.503 | +153.44% | CHECK |
| `wide_by128.FP128.div_toward_zero` | 12.569 | 31.799 | +153.00% | CHECK |
| `wide_by64.FP128.div_nearest_even` | 7.909 | 30.756 | +288.85% | CHECK |
| `wide_by64.FP128.div_toward_zero` | 6.435 | 20.500 | +218.58% | CHECK |
| `wide_product.FP128.mul_div_nearest_even` | 13.263 | 28.391 | +114.07% | CHECK |
| `wide_product.FP128.mul_div_toward_zero` | 12.144 | 24.189 | +99.19% | CHECK |
| `wide_product.FP128.mul_nearest_even` | 6.613 | 26.323 | +298.02% | CHECK |
| `wide_product.FP128.mul_toward_zero` | 5.946 | 19.682 | +230.99% | CHECK |
