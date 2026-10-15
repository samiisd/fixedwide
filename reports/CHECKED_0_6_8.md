# Accumulated checked arithmetic — 0.6.8

Base: `6de5a19fa04226e33bc5bcede0d6426ec820fa8d` (main, includes 0.6.7 / PR #14).
Implementation and investigation date: 15 October 2026.

## Acceptance status

The additive API and numerical/error parity tests are implemented. The requested
universal happy-path parity and absence of extra context spills are **not yet
established**. Do not interpret working code or passing correctness tests as
performance sign-off. The PR remains draft until the outstanding evidence is
reviewed. No existing arithmetic kernel was changed to obtain nicer benchmarks.

## Operation inventory

`checked.hpp` provides **28 fallible overloads across 25 operation names**,
mirroring the public `ArithmeticError` APIs at the pinned base. This count
excludes the generic `take` and `result` facilities.

| Header | Covered operation(s) | Errors the underlying operation can report |
|---|---|---|
| arithmetic.hpp | `add`, `sub`, `negate`, `abs` | overflow |
| arithmetic.hpp | `mul` | overflow, inexact |
| arithmetic.hpp | `div`, fixed-point `mul_div` | division_by_zero, overflow, inexact |
| arithmetic.hpp | `remainder` | division_by_zero |
| arithmetic.hpp | `quantize(a, decimals, rounding)` | invalid_precision, overflow, inexact |
| arithmetic.hpp | `midpoint(a,b,rounding)` | inexact |
| arithmetic.hpp | `midpoint(a,b,decimals,rounding)` | invalid_precision, overflow, inexact |
| arithmetic.hpp | `from_integer<Target>` | overflow |
| arithmetic.hpp | legacy `narrow(FP128)` | overflow |
| arithmetic.hpp | legacy `mul_wide(FP64,FP64,rounding)` | inexact; the widened destination bounds the product |
| mixed.hpp | `add_to`, `sub_to`, `mul_to`, `fixed_cast` | overflow, inexact |
| mixed.hpp | `div_to`, `mul_div_to` | division_by_zero, overflow, inexact |
| floating.hpp | `from_float<Target>`, `from_double<Target>`, `from_double64`, `from_double128` | invalid_value, overflow, inexact |
| bigint.hpp | `divmod(u256,u128)` | division_by_zero |
| bigint.hpp | `divmod(i256,i128)` | division_by_zero, overflow |
| bigint.hpp | `divide_to_i128(i256,i128,rounding)`, raw `mul_div(i128,i128,i128,rounding)` | division_by_zero, overflow, inexact |

Every operation forwards directly to its existing free function and absorbs its
result. This preserves error **precedence within each operation**, not just its
list of possible errors. Sticky first-error ordering applies between operations.

### Actual base API, not invented overloads

There is one checked `quantize` overload on this base, not two. Both checked
`midpoint` overloads are covered. The compile-time-rounding
`midpoint<Rounding != exact>(a,b)` returns a value without `ArithmeticError`, so
it stays an infallible free function.

Public fixed-point storage widths are 8, 16, 32, 64, 128 and 256, with maximum
fractional digits 2, 4, 9, 18, 38 and 76 respectively. Members are templates over
the same widths/scales as the originals. Mixed numerical kernels still use their
existing 512/1024-bit intermediates; this release does not introduce wider public
storage. There is no width-specific fallback to unchecked arithmetic.

Same-domain arithmetic, integer construction and `narrow` preserve constexpr.
Mixed, floating and raw bigint members are not constexpr because their existing
free functions are not. Merely decorating a wrapper would not make those compiled
kernels usable in constant evaluation.

### Explicit exclusions

| API family | Reason |
|---|---|
| `parse` / character input | `ParseError`: boundary validation, not arithmetic accumulation. |
| Formatting / character output | `FormatError`: output handling; string helpers may allocate. |
| `from_bytes` / binary serialization | `BinaryError`, not `ArithmeticError`. No binary error is translated or lost. |
| `to_float`, `to_double`, comparisons, raw construction/access, infallible midpoint | No `ArithmeticError` result to accumulate. |
| Unchecked functions/operators | Their preconditions remain caller obligations; they must never be used to process failed-chain placeholders. |
| `detail` and implementation kernels | Not public API. Covered transitively by the public member that calls the original checked entry point. |

No other public `ArithmeticError`-returning operation was found at the pinned
base. `take(expected)` remains the extension point for caller-defined functions.

## Semantic decisions

`Checked` retains the proposed name: it describes the policy and is consistent
with the existing `ArithmeticError` / `Rounding` naming. It is not a stored-value
wrapper. There are no overloaded arithmetic operators on a new value type.
Explicit destination types and per-operation rounding still belong at `mul_to`
and `mul_div_to` call sites.

A failed operation returns `T{}`. A later successful operation returns its own
computed value even though the context remains failed; it does not automatically
return zero. Neither these values nor values computed earlier in the same failed
chain may be published as the chain's successful result. `result(v)` refuses
publication after failure and does not move from `v` on that error path.

Every member always invokes its checked free function. `take(custom())` evaluates
`custom()` even after failure. There is no rollback or suppression of caller
side effects. The no-UB guarantee is for the checked operations with their normal
valid argument contracts, including zero placeholders; it cannot sanitize an
unchecked expression the caller evaluates before entering a member.

First error means first in actual evaluation order. Separate statements make
that order explicit. Sibling arguments in a nested expression are not guaranteed
to execute left-to-right. Context copies are independent; no reset is provided.
Neither querying status nor producing a result consumes or resets the context.
One context is local to one chain/thread and is not a synchronization primitive.

To reconcile a generic hook with `noexcept`, `take<T>` requires nonthrowing default
construction and the selected move/copy. `result<T>` requires only its selected
construction to be nonthrowing. Move-only nonthrowing values work. Throwing value
operations are rejected by constraints, not hidden behind `noexcept` termination.
`take(expected<void,E>)` and `result()` cover status-only custom computations.
The context itself never allocates; a caller-defined type/function may still
allocate despite being nonthrowing. Numeric library operations do not.

The implementation stores `std::optional<ArithmeticError>` and consumes/returns
`std::expected` at the operation and publication boundaries. It is two bytes and
trivially copyable on the supported CI configurations; no custom sentinel or
result representation is needed. The first error is constructed with `emplace`.
Tests also round-trip `static_cast<ArithmeticError>(255)`. A stored
`std::expected<void, ArithmeticError>` was tested and rejected because it does
not preserve the required trivial-copyability trait on the tested libstdc++.
See [the follow-up](CHECKED_0_6_8_FOLLOWUP.md) for the measured compiler workaround.

## Tests and adoption

`tests/checked_cases.hpp` compares member results/errors with the exact checked
free functions, with always-active `CHECK` assertions. It covers all operation
names/overloads, all five error classes, six rounding policies, all six public
widths, mixed scales including 8/10/12 and extreme 0/76 combinations, signed
minima/maxima, floating extrema/NaN/infinity, raw division records and aliases.
All 25 pairs of first/later error classes exercise sticky state. Zero-result
failures followed by checked division demonstrate safe continued execution.

The standalone suite runs without exceptions on GCC/Clang; the same cases are
included in comprehensive unity coverage, not counted only in a separate binary.
`test_no_alloc` executes successful and failing chains. Compile-time chains at
all widths include both success and failure. Existing independent kernel oracle
tests remain unchanged: delegation parity supplements rather than replaces them.

`10_checked_chain.cpp` computes available equity and a margin percentage, then
publishes a custom aggregate through `result`. The mixed-scale and invoice
examples now use accumulated checks where a formula becomes easier to follow.
`03_error_handling.cpp` deliberately retains early-return/monadic examples because
they teach short-circuit semantics, which differ from eager accumulation.
Internal numerical kernels are unchanged: their local error precedence and hot
paths are not rewritten without a demonstrated readability/performance benefit.

## Reproducible performance protocol

Run from the repository root, separately for each compiler:

```sh
python3 scripts/checked_chain_evidence.py --compiler g++-14 \
  --output /tmp/checked-gcc --cpu 2
python3 scripts/checked_chain_evidence.py --compiler clang++-20 \
  --output /tmp/checked-clang --cpu 2
```

The existing `measurement.hpp` harness runs 4,096 warmup iterations, 1,048,576
measured iterations and 15 repetitions per policy, over three trials with policy
order rotated. All 4,096 seeded runtime inputs are checked against explicit
expected chains before timing. Success at every intermediate proves the
unchecked preconditions for those inputs. Every iteration consumes its result
through the same compiler escape; RNG and validation are outside timed regions.
Small formula bodies enter the harness through inline lambdas, not indirect
function pointers. The assembly driver separately exports ordinary callable
functions for inspection, preserving each policy's natural result ABI.

Flags: Release `-O3 -DNDEBUG -std=c++23`, `-ffp-contract=off`, and both loop and
SLP vectorization disabled (`-fno-tree-vectorize -fno-tree-slp-vectorize` for GCC;
`-fno-vectorize -fno-slp-vectorize` for Clang). No LTO or forced architecture is
added. CMake compile commands, exact compiler/CPU, affinity, git state, UTC time,
raw timing samples, pooled sample medians and disassembly are retained.

Workloads are eight Fixed128<10> additions/subtractions, a Fixed256<12>
`mul_to` / `mul_div_to` chain with wide operands and add/sub stages, and a mixed
binary-floating-ingress/conversion/equity/margin chain. There is no unchecked
floating-input API: that comparison retains checked floating conversion and
uses a prevalidated engaged result, rather than substituting an unrelated cast.
Unchecked cancellation/reassociation permitted by its proven bounds is not
artificially disabled to flatter the checked policies.

Static assembly counts cover entire functions, including cold/error paths and
ABI housekeeping. They are navigation aids, **not** measured executed instruction
counts or proof of happy-path branch parity. Compare success traces manually.
No noisy hosted timing threshold is installed as a substitute for that review.

## Measurements and code-generation review

The original scalar-state implementation had substantial Clang overhead. The
follow-up replaces that custom state with standard optional storage and keeps
all arithmetic/results on the existing `std::expected` boundary. Targeted
inlining and a Clang-only compiler barrier on the failure path remove the short
chain's conditional-zero moves and context spills on the measured Clang 20
configuration. Numerical kernels and eager post-error semantics are unchanged.

The full benchmark still measures about 6% overhead for Clang's eight-operation
chain and about 5% for GCC's wide chain. These differences are not asserted to be
noise; universal parity and no-extra-spill acceptance remain open. Complete
measurements, rejected alternatives, coverage diagnosis and reproduction:
[CHECKED_0_6_8_FOLLOWUP.md](CHECKED_0_6_8_FOLLOWUP.md).
