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
`validations=57347` and refuses to print results if any check fails.

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

### decimal fixed, matched scale

The like-for-like comparison: the same scale, the same operand integers, and both
results brought back to the declared type. Validated against an exact integer oracle.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| cnl | `scaled_integer<int64,power<-6,10>>` | mul_unchecked | 0.538 | no |
| cnl | `scaled_integer<int64,power<-6,10>>` | div_unchecked | 1.194 | no |
| fixedwide | `Fixed64<6>` | mul_nearest_even | 1.518 | yes |
| fixedwide | `Fixed64<6>` | div_nearest_even | 1.995 | yes |

### decimal fixed

Decimal fixed point: an integer scaled by a power of ten. Note the CNL rows are
scale 6 against fixedwide's scale 12 -- see the matched-scale class above for the
comparison that controls for that.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| cnl | `scaled_integer<int64,power<-6,10>>` | mul_unchecked | 0.544 | no |
| cnl | `scaled_integer<int64,power<-6,10>>` | div_unchecked | 1.090 | no |
| fixedwide | `Fixed64<12>` | dependent_chain_mul | 2.039 | yes |
| fixedwide | `Fixed64<12>` | div_nearest_even | 2.171 | yes |
| fixedwide | `Fixed64<12>` | mul_div_one_rounding | 2.308 | yes |
| fixedwide | `Fixed64<12>` | mul_nearest_even | 2.626 | yes |
| fixedwide | `Fixed64<12>` | parse | 12.303 | yes |
| fixedwide | `Fixed64<12>` | format | 14.098 | yes |

### binary fixed

Binary fixed point: an integer scaled by a power of two. Cannot represent 0.01 exactly.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| cnl | `scaled_integer<int64,power<-32>>` | mul_unchecked | 0.488 | no |
| fpm | `fixed<int64,int128,32>` | mul_nearest_unchecked | 1.351 | no |
| fpm | `fixed<int64,int128,32>` | div_nearest_unchecked | 1.945 | no |

### decimal float

IEEE 754 decimal floating point: a decimal significand with a moving exponent.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| boost.decimal | `decimal64_t` | mul | 3.540 | no |
| boost.decimal | `decimal64_t` | div | 8.594 | no |
| boost.decimal | `decimal64_t` | format | 12.574 | no |
| boost.decimal | `decimal64_t` | parse | 13.669 | no |

### raw integer

Wide-integer arithmetic with no scale and no rounding. A floor, not a competitor.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| boost.multiprecision | `int128_t` | mul_unchecked | 0.888 | n/a |

### binary float

IEEE 754 binary floating point. The cost of not being deterministic in decimal.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| std | `double` | mul | 0.222 | n/a |
| std | `double` | div | 0.739 | n/a |
| std | `double` | parse | 5.403 | n/a |
| std | `double` | format | 28.648 | n/a |

### raw machine types

Not competitors: the floor. What the hardware costs with no scale, no rounding mode,
no overflow check and no decimal guarantee.

| library | type | operation | median ns/op | checked |
|---|---|---|---:|---|
| std | `int64_t` | memcpy_load | 0.182 | n/a |
| fixedwide | `Fixed64<12>` | from_bytes_little | 0.184 | yes |
| std | `int64_t` | memcpy_store | 0.190 | n/a |
| fixedwide | `Fixed64<12>` | to_bytes_little | 0.190 | yes |
| std | `float` | mul | 0.203 | n/a |
| std | `float` | add | 0.206 | n/a |
| std | `double` | add | 0.260 | n/a |
| std | `int64_t` | add_unchecked | 0.312 | n/a |
| std | `int64_t` | mul_unchecked | 0.330 | n/a |
| fixedwide | `Fixed64<12>` | add_checked | 0.384 | yes |
| std | `int64_t` | div_unchecked | 1.090 | n/a |

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

**Against CNL, at a matched scale, the multiply gap is about 2.8x — not the 8x
this report used to claim.** The old figure compared fixedwide's
multiply-widen-rescale-check against a CNL expression that never rescaled at
all: `cnl::scaled_integer::operator*` returns a type whose exponent is the *sum*
of the operands', so `a * b` is a bare 64-bit multiply leaving the product at a
different scale. Bringing the result back to the declared type is the comparable
operation, and the `decimal fixed, matched scale` rows above do that.

CNL is still faster, and the reason is unchanged and worth stating plainly: it
does not check for overflow. `fixedwide::mul` returns `std::expected` and
reports it. That is the trade this library exists to make.

**CNL cannot do twelve decimals at all**, which is the more important finding.
It forms the product in its representation type, so a scale-12 multiply
overflows `int64_t` for any value above roughly 0.003. Checked on every run of
this benchmark:

```
123.456789012345 * 2, at 12 decimals
  cnl        raw = -2111655         wrong, negative, and silent
  fixedwide  raw = 246913578024690  exact
```

fixedwide forms the intermediate at twice the width, so it returns the right
answer rather than either wrapping or erroring. That is why the matched-scale
comparison above is at scale 6: it is the widest scale at which both libraries
compute the same function.

Against **Boost.Decimal** — the nearest thing here to the same use case:

* multiply is faster (2.63 ns against 3.54), and divide about four times faster
  (2.17 against 8.59);
* **parsing is faster** (12.3 ns against 13.7);
* formatting is slower (14.1 ns against 12.6), and that is an open item.

Against the standard library's binary-float text routines:

* **formatting is about twice as fast** as `std::to_chars` on a `double`;
* **parsing is about twice as slow** as `std::from_chars` on a `double`. That
  gap is real and reported, but it is not a like-for-like row:
  `std::from_chars` produces a binary float and rejects nothing on a decimal
  grid, while this parser produces an exact scaled integer and refuses input it
  cannot represent. Boost.Decimal is the row to read against.

## Corrections to the previous report

* **The CNL rows did not rescale.** `scaled_integer::operator*` and
  `operator/` return a type whose exponent is the sum or difference of the
  operands', so the timed expression was a bare 64-bit multiply or divide with
  the result left at the wrong scale. Every CNL row is now forced back to its
  declared type, which is the operation fixedwide performs. The multiply gap
  went from a reported 8x to a measured 2.8x at a matched scale.
* **The CNL comparison was at the wrong scale.** `Fixed64<12>` was compared
  against a scale-6 CNL type. There is now a `decimal fixed, matched scale`
  class that pairs each scale with its own counterpart, seeds both libraries
  from the identical raw integers, and validates against an exact integer
  oracle instead of a 0.01 floating tolerance that could not tell a correct
  decimal result from a wrong one.
* **Operands were positive only.** The matched-scale fixture includes negatives,
  and derives magnitudes from the scale so both scale points see the same
  values.

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

