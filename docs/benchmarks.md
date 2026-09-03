# Benchmarks & Performance Verification

## Methodology
Benchmarks are run against the 0.4 reference baseline across 100 workloads covering:
- Native single-limb operations
- Wide multi-limb arithmetic
- Rounding mode impacts
- Parsing and serialization throughput

## Benchmark Executables
- `fixedwide_bench`: Core FP64 and FP128 operations compared to 0.4 baselines.
- `fixedwide_rounding_bench`: 100 paired workload suite evaluating rounding modes and multi-digit scaling.
- `fixedwide_numeric_bench`: Stress testing edge values and large powers.
- `fixedwide_memory_bench`: Allocation-free verification under tight memory footprints.
- `fixedwide_competitor_bench`: Comparative benchmarking against competitor libraries.
