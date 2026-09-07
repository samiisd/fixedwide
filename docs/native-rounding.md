# Native rounding boundaries and performance evidence

The inline x86-64 IDIV paths must prove two distinct properties: the truncated
quotient fits the hardware destination, and the rounded result fits the public
result type. The second does not follow from the first.

## Safety invariants

For multiplication at positive scale S = 10^D, D >= 1, the predicate
`unsigned(high) + S/2 < S - 1` admits signed high limbs in
`[-S/2, S/2 - 2]`. With N = high * 2^64 + low, this implies
`-2^63 <= N/S < 2^63 - 2^64/S`. For S <= 10^18 there is room for the
rounding increment. The excluded slab uses the existing checked or wide-result
fallback. S = 1 admits no numerator through this conservative predicate.
Scale 19 never uses signed-64 IDIV: 10^19 fits uint64_t but not int64_t.

Variable divisors retain their quotient-fit predicate. Fixed64 division and
multiply-divide check the rounding addition and return overflow on failure.
Fixed128 multiply-divide uses `round_wide_quotient`: its adjustment is -1, 0 or
+1 directed away from zero. That restricted helper is not general addition;
it preserves a positive result of raw 2^63 without sign-extending the low limb.

For nearest-even rounding with a constant scale, let
`t = S/2 - (q & ~S & 1)`. The adjustment is `(rem > t) - (rem < -t)`.
For even S an odd quotient includes the exact halfway remainder; an even
quotient does not. For S = 1 the remainder is zero. The threshold and its
negation both fit int64_t. Signed comparisons avoid a remainder-sign branch
in the examined GCC 14 code, without signed absolute-value overflow.

For a runtime divisor, the doubled-remainder comparison has the narrower
precondition `0 <= r < d <= 2^63`. Therefore `2*r + (q & 1)` is at most
UINT64_MAX. Do not reuse that helper for unrestricted unsigned divisors.

## Regression tests

`audit_fastpath_rounding` checks all six policies, positive/negative boundaries,
zero and negative divisors, scale 19, the scale-12 mul_wide adapter and constexpr
behaviour. Runtime inputs are loaded through volatile storage so constant
expression evaluation cannot hide an inline-IDIV regression.

## Performance reproduction

Run on Linux with GCC 14, Boost headers, CMake, Ninja, binutils and Valgrind:

```sh
python3 scripts/test_compare_fastpath_rounding.py
python3 scripts/compare_fastpath_rounding.py /path/to/base /path/to/head \
    --output fastpath-results --icount
```

The historical runtime-policy driver is compiled against both revisions. A
separate 48-row driver adds Fixed64 scales 8 and 12, mul/div/mul_div, compile-time
nearest-even/toward-zero policies, exact/inexact results and positive/mixed-sign
operands. Its independent signed-128 oracle validates every fixture outside the
measured loop; paired result checksums must match. These extra rows measure
independent throughput, not dependent-chain latency.

The comparison retains every sample, source/driver hashes, hashes of all six
timed executables and both static libraries, their exact compile/link commands,
CMake compile databases, CPU affinity and paired matrix disassembly. Incomplete,
duplicate, non-finite, unbalanced or checksum-mismatched matrix evidence fails
validation. The comparison also rejects changes to the protected historical
baseline, driver, comparator, measurement script or general CI workflow.

The 48 supplemental Callgrind comparisons are reported separately. They neither
replace the original 34-row 1% gate nor silently establish a new baseline.
A green workflow means the validation ran successfully; it is not proof that
every CPU-time row improved. Retain and inspect slower rows as well as wins.
