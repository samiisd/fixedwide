# Checked 0.6.8: coverage and code-generation follow-up

Investigation date: 2026-10-15. Original PR head: `16bee6d`.
Final implementation measured below: `4a534a86af9e055b2db71776dd956052048a0354`.
The residual overhead below is accepted; see [benchmarks.md](../docs/benchmarks.md#accumulated-checked-chains).

## Keep the standard result type

Every numerical operation still calls the original `std::expected<T,
ArithmeticError>`-returning checked function. `take` consumes that result;
`result` returns an ordinary `std::expected`. There is no replacement result
class, bespoke arithmetic, tagged number, expression template or hidden global
state. Public call sites and eager post-error behavior have not changed.

The stored first error is now `std::optional<ArithmeticError>`, replacing the
custom 16-bit success-sentinel encoding. Its first value is constructed with
`emplace` after checking that it is empty. It retains the original two-byte,
trivially-copyable context on the tested configurations, including all 256 error
representations. Copies are independent and a success never clears an error.

A stored `std::expected<void, ArithmeticError>` was tried first. It does not
satisfy `std::is_trivially_copyable_v` in the tested libstdc++; changing to it
would silently drop an explicit requirement. `expected<unsigned char,E>` and
`expected<E,E>` also fail that trait. Optional error storage with expected
operation/result boundaries avoids that compromise using standard types.

## Why the short chain was slow

Clang 20 if-converted the zero-on-failure paths, generating conditional-zero
moves, retaining overflow flags and saving registers across the chain. Explicit
early-return checks had a shorter success trace. These costs came from consuming
checked results and maintaining sticky state, not different numerical kernels.

Targeted `always_inline` on the two value `take` overloads and `add`/`sub`, plus
optional emplacement, helped Clang 17 substantially. It did NOT fix Clang 20:
a matched Clang 20 CI run still showed approximately 48% short-chain overhead.
This compiler-version difference is why local Clang 17 results are not used as
Clang 20 evidence.

The retained Clang-only workaround is an error-path
`std::atomic_signal_fence(std::memory_order_acq_rel)`, skipped during constant
evaluation. It inhibits the harmful speculation/if-conversion without adding
inline assembly, a CPU fence, a runtime state variable, or any check before a
successful operation. The measured x86-64 output contains no fence instruction.
This is an intentional code-generation barrier, NOT an inter-thread
synchronization mechanism or a claim that Checked is thread-safe.

The standard specifies the signal fence's compiler-ordering role without the
hardware fence of a thread fence: [atomics.fences](https://eel.is/c++draft/atomics.fences).
The particular branch shape/performance improvement is empirical, not a C++
standard guarantee. The workaround is restricted to Clang; GCC retains the
unfenced path. Inlining attributes are compiler hints, not numerical machinery.

## Alternatives measured on the actual CI compiler

Diagnostic run: [38731081200](https://github.com/samiisd/fixedwide/actions/runs/38731081200),
commit `49953bd`. It retains generated headers, test logs, raw samples and assembly
in `checked-chain-clang++-20` and `checked-chain-g++-14` artifacts. Every variant
first passed 275,811 checked assertions with exceptions disabled. The temporary
variant-generating workflow step was subsequently removed; the normal evidence
workflow and every gate are unchanged.

For Clang 20, three trials of 262,144 iterations and eleven repetitions per policy
were run with rotating policy order. Pooled medians below are ns per whole
8-operation chain, not ns per operation:

| Variant | Expected | Accumulated | Decision |
|---|---:|---:|---|
| Optional + targeted inlining | 3.380 | 5.075 | Insufficient on Clang 20 |
| Guarded `expected::value()` access | 3.393 | 5.032 | No useful improvement |
| Error-only standard signal fence | 3.391 | 3.589 | Retained for Clang |
| Error-only empty volatile assembly | 3.382 | 3.595 | No benefit over standard fence |
| Cold, non-inlined error recorder | 3.389 | 3.818 | Slower; additional call/ABI cost |
| Cold recorder with `preserve_most` | 3.393 | 5.172 | No useful improvement |

Earlier local experiments also covered expected status storage, monadic
`or_else`/`transform_error`/`transform`, by-value consumption and branch hints.
None supplied a portable proof of the requested parity. Changing zero fallback,
skipping later operations, or using unchecked arithmetic was not accepted.

## Final implementation: canonical CI measurements

[Evidence run 38732097080](https://github.com/samiisd/fixedwide/actions/runs/38732097080),
commit `4a534a8`, measured the committed source (no overlay). Source archives from
both compiler artifacts match the locally tested header, coverage harness and
restored workflow byte-for-byte.

GCC 14.2 ran on a virtualized AMD EPYC 9V74; Clang 20.1.8 ran on a virtualized
Intel Xeon Platinum 8573C. Both used Ubuntu 24.04, CPU 0 affinity, C++23,
`-O3 -DNDEBUG`, no LTO, no forced architecture, disabled loop/SLP vectorization,
and `-ffp-contract=off`. Do not compare absolute times across these two hosts.
The existing fixture validates all runtime inputs outside timing and consumes
every result equally. There are 1,048,576 iterations, fifteen repetitions and
three rotating-order trials: 45 samples per policy. No exclusive-host or
frequency-control guarantee is claimed.

| Compiler / whole chain | Expected ns | Accumulated ns | Unchecked ns | Accumulated vs expected |
|---|---:|---:|---:|---:|
| Clang 20: Fixed128 add/sub x8 | 3.381 | 3.586 | 3.354 | +6.07% |
| Clang 20: Fixed256 multiply chain | 142.459 | 144.249 | 147.667 | +1.26% |
| Clang 20: floating/mixed chain | 95.974 | 96.313 | 96.792 | +0.35% |
| GCC 14: Fixed128 add/sub x8 | 3.334 | 3.325 | 2.859 | -0.26% |
| GCC 14: Fixed256 multiply chain | 224.886 | 236.847 | 247.347 | +5.32% |
| GCC 14: floating/mixed chain | 107.073 | 108.614 | 104.670 | +1.44% |

[Exact pooled summary](checked-0.6.8-followup-summary.csv). Raw `trial-*.csv`,
`environment.json`, compile commands, source archives, validation logs and full
assembly are retained by the evidence workflow. Timings are batch-throughput
costs, not dependency-chain latency. Faster-than-unchecked rows are observations
of these generated binaries, not a claim that checking inherently speeds up
arithmetic. The approximately 6% (+0.2 ns) Clang short-chain and 5% GCC
wide-chain differences are accepted as the cost of `Checked`.

In Clang 20's eight-operation core, the previous conditional-zero moves and all
context-related saved registers/stack operands disappear. The success trace has
eight arithmetic overflow branches, a final sticky-state check and one retained
status bit. There is no error-helper call. Cold paths still perform the required
safe later operations. Wrapper ABI housekeeping is present for BOTH expected
and accumulated probes and must not be attributed solely to Checked.

The wide chain still has additional state/register pressure across the compiled
kernel calls. For example, Clang's wide core saves six registers versus four in
the explicit chain. Static counts include cold blocks and are not dynamic
instruction counts. No claim of no-extra-spill parity is made for every chain.

## Coverage failure and fix

The failed report already had 99.2117% physical source-line coverage and 100%
coverage of `checked.hpp`. The failing independent gate was LLVM summary lines:
3804/4059 = 93.7177%, below 95%. The same report was reproduced with matching local
Clang/LLVM 17.

Some arithmetic/from-integer instantiations in `checked_cases.hpp` were exercised
only by `static_assert`. They had no runtime counters in the single coverage
executable. `test_coverage_checked.cpp` now executes those same success/failure
chains at runtime across all public widths, and checks integer-construction
parity at zero, both signs and representative boundaries. Existing compile-time
assertions remain. No thresholds, exclusions, denominator rewriting, or merging
of profiles from different binaries/backends was introduced.

Final Clang/LLVM 20 CI coverage: [job 116412676926](https://github.com/samiisd/fixedwide/actions/runs/38732097070/job/116412676926).
Both backends passed all three gates and 73/73 CTest cases each; the comprehensive
coverage exercise ran separately, as before:

| Gate | Required | Native | Portable |
|---|---:|---:|---:|
| Physical source lines | 99% | 4280/4314 = 99.2119% | 3807/3837 = 99.2181% |
| LLVM summary lines | 95% | 3805/3982 = 95.5550% | 3451/3533 = 97.6790% |
| LLVM summary branches | 90% | 1650/1810 = 91.1602% | 1515/1616 = 93.7500% |

LLVM's overlapping template mappings change when those instantiations execute;
this explains the changed LLVM denominator while physical coverage stays nearly
unchanged. The report/tool invocation itself is unmodified. Separate local
Clang/LLVM 17 runs also passed both backends, with the same line percentages;
the CI figures above are the actual Clang 20 results, not extrapolations.

Final local portable Clang ASan+UBSan passed the Checked suite, allocation test
and monetary example (3/3). GCC 14 and Clang 17 checked suites, including
constexpr success/failure and copyability assertions, also passed. CI's
compiler-specific variant tests passed 275,811 assertions each; the full normal
cross-platform CI is separate from the completed coverage/evidence jobs.

## Reproduce

Use the existing `scripts/checked_chain_evidence.py` commands documented in the
main report for both compilers, then inspect the actual chain bodies in
`checked-chain.asm`, not only the forwarding probe wrappers. For the diagnostic
alternatives, the temporary workflow at `49953bd` retains exact variant source
and commands. Normal coverage uses `scripts/coverage.sh` independently for
native and portable backends with matching Clang/LLVM versions.
