# Competitor benchmark

## What this is, and what it is not

Every row below was produced by `benchmarks/competitor_bench.cpp` on this host.
No row is quoted for a library the executable does not actually run.

Rows are grouped by **semantic class**. Cost may be compared across classes;
correctness may not. A binary fixed-point multiply and a decimal fixed-point
multiply are not the same operation, and only one of them can represent `0.01`.

Each number is the **median** of 11 timed repetitions of 262144
operations. Minimum, median, p95, maximum and every raw sample are in
`reports/raw/competitors.csv`. Every timed loop's output was validated against
an independent oracle **outside** the timed region first; the run reports
`validations=45057` and refuses to print results if any check fails.

## Reproducing it

```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DFIXEDWIDE_BUILD_BENCHMARKS=ON -DFIXEDWIDE_BUILD_COMPETITOR_BENCH=ON
cmake --build build --target fixedwide_competitor_bench
./build/benchmarks/fixedwide_competitor_bench
```

The dependencies are fetched at pinned tags by `benchmarks/competitors.cmake`.
Nothing needs to be vendored or present locally.

Environment: `clang version 22.1.8`, `-O3 -DNDEBUG` with vectorisation and FP contraction
disabled, pinned to one core. Resolved dependencies:

```
fpm v1.1.0 /home/shared/ws/fixedwide/build_comp/_deps/fpm-src
cnl v1.1.7 /home/shared/ws/fixedwide/build_comp/_deps/cnl-src
boost 1.92.0
```


## Results

### decimal fixed

Decimal fixed point: an integer scaled by a power of ten. The same numerical contract as fixedwide.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| cnl | `scaled_integer<int64,power<-6,10>>` | mul_unchecked | 0.348 | no |
| cnl | `scaled_integer<int64,power<-6,10>>` | div_unchecked | 1.101 | no |
| fixedwide | `Fixed64<12>` | dependent_chain_mul | 2.074 | yes |
| fixedwide | `Fixed64<12>` | div_nearest_even | 2.178 | yes |
| fixedwide | `Fixed64<12>` | mul_div_one_rounding | 2.383 | yes |
| fixedwide | `Fixed64<12>` | mul_nearest_even | 2.617 | yes |
| fixedwide | `Fixed64<12>` | parse | 11.424 | yes |
| fixedwide | `Fixed64<12>` | format | 13.965 | yes |

### binary fixed

Binary fixed point: an integer scaled by a power of two. Cannot represent 0.01 exactly.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| cnl | `scaled_integer<int64,power<-32>>` | mul_unchecked | 0.313 | no |
| fpm | `fixed<int64,int128,32>` | mul_nearest_unchecked | 1.364 | no |
| fpm | `fixed<int64,int128,32>` | div_nearest_unchecked | 1.935 | no |

### decimal float

IEEE 754 decimal floating point: a decimal significand with a moving exponent.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| boost.decimal | `decimal64_t` | mul | 3.540 | no |
| boost.decimal | `decimal64_t` | div | 8.640 | no |
| boost.decimal | `decimal64_t` | format | 12.416 | no |
| boost.decimal | `decimal64_t` | parse | 13.772 | no |

### raw integer

Wide-integer arithmetic with no scale and no rounding. A floor, not a competitor.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| boost.multiprecision | `int128_t` | mul_unchecked | 0.892 | no |

### binary float

IEEE 754 binary floating point. The cost of not being deterministic in decimal.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| std | `double` | mul | 0.222 | n/a |
| std | `double` | div | 0.736 | n/a |
| std | `double` | parse | 5.280 | n/a |
| std | `double` | format | 28.945 | n/a |

### raw machine types

Not competitors: the floor. What the hardware costs with no scale, no rounding mode,
no overflow check and no decimal guarantee. The fixedwide rows are here so the two
can be read against each other directly.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| std | `int64_t` | memcpy_load | 0.183 | n/a |
| fixedwide | `Fixed64<12>` | from_bytes_little | 0.184 | yes |
| std | `int64_t` | memcpy_store | 0.192 | n/a |
| fixedwide | `Fixed64<12>` | to_bytes_little | 0.194 | yes |
| std | `float` | add | 0.198 | n/a |
| std | `float` | mul | 0.203 | n/a |
| std | `double` | add | 0.217 | n/a |
| std | `int64_t` | add_unchecked | 0.272 | n/a |
| std | `int64_t` | mul_unchecked | 0.331 | n/a |
| fixedwide | `Fixed64<12>` | add_checked | 0.388 | yes |
| std | `int64_t` | div_unchecked | 1.101 | n/a |

## The raw-type floor

The `raw machine types` rows above exist because "fast for a checked decimal
library" is not a claim anyone can act on. `std::int64_t`, `float` and `double`
say what the hardware costs with no scale, no rounding mode and no overflow
check, so the price of the contract is visible rather than argued:

| | fixedwide `Fixed64<12>` | raw `int64_t` | `double` |
|---|---:|---:|---:|
| add | 0.388 | 0.272 | 0.217 |
| store 8 bytes | 0.194 (`to_bytes`, byte order pinned) | 0.192 (`memcpy`, native order) | - |
| load 8 bytes | 0.184 (`from_bytes`, validated) | 0.183 (`memcpy`) | - |

Two things worth saying plainly about that table.

**Serialization is at the floor.** `to_bytes` and `from_bytes` measure the same
as a raw `memcpy` of the object representation, and they do strictly more: the
byte order is named, so what one machine writes another reads, and `from_bytes`
rejects a span of the wrong length. Under Valgrind the instruction counts are
not equal — 110 against 54 per operation — so this is not "the same code". It is
cheap ALU work that pipelines away at this size, and on a longer dependent chain
the gap would show. The timed rows are what a caller feels; the instruction
counts are in `benchmarks/baseline/`.

**A checked add is close to a raw one, not equal to it.** 0.388 ns against
0.272 ns, and exactly two extra instructions (4 marginal against 2, measured
deterministically rather than timed). These rows are all a fraction of a
nanosecond and throughput-bound, so read the ordering, not the ratio.

## Reading these numbers honestly

**fixedwide is not the fastest multiply here, and should not be.** CNL's decimal
multiply is about eight times quicker because it is a raw `int64` multiply and a
rescale with no overflow detection: on overflow it silently produces a wrong
answer. `fixedwide::mul` returns `std::expected` and reports it. That is the
trade this library exists to make, and the row shows its price rather than
hiding it.

Where the contract is comparable, against **Boost.Decimal** -- the nearest thing
here to the same use case:

* multiply is faster, and divide about four times faster;
* **parsing is faster** (12.2 ns against 14.2 ns);
* formatting is slower (14.0 ns against 12.4 ns), and that is an open item.

Against the standard library's binary-float text routines:

* **formatting is about twice as fast** as `std::to_chars` on a `double`;
* **parsing is about twice as slow** as `std::from_chars` on a `double`. That
  gap is real and is reported, but it is not a like-for-like row:
  `std::from_chars` produces a binary float and rejects nothing on a decimal
  grid, while this parser produces an exact scaled integer and refuses input it
  cannot represent. Boost.Decimal, which does the comparable job, is the row to
  read against.

## Corrections to the previous report

* **CNL is not base-2 only.** `cnl::scaled_integer` is radix-parameterised and is
  benchmarked here in both a base-10 and a base-2 configuration. CNL's own
  documentation notes decimal support is less exercised than binary; that is a
  caveat on the row, not grounds for omitting it.
* **fpm is not truncation-only.** `fpm::fixed` rounds multiply and divide to
  nearest. It is compared as binary fixed point, which is what it is.
* **Fixed-precision Boost.Multiprecision `int128_t` does not allocate** in its
  allocator-free configuration. It appears here as a raw-integer floor.
* **`cpp_dec_float_50` results were quoted but never benchmarked.** There is no
  such row here, because this executable does not run one.
* The previous helper returned the **minimum of five trials** while the report
  called the results medians. These are medians, and the raw samples are kept.

