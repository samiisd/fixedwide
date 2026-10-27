# Final Google Benchmark evidence

These records measure the final frozen production headers with Google Benchmark
1.9.4. Each configuration contains 15 workloads, with paired direct-general-kernel
and public-API registrations. The [summary](summary.csv) uses CPU-time median
aggregates in nanoseconds; positive deltas mean public-API overhead. The parent
[report](../README.md) discusses the results.

## Records and provenance

| Configuration | Raw JSON | Aggregate console output | Final validation |
| --- | --- | --- | --- |
| GCC 14 native | [JSON](gcc14-native.json) | [Text](gcc14-native.txt) | [Log](gcc14-native-validation.txt) |
| Clang 19 native | [JSON](clang19-native.json) | [Text](clang19-native.txt) | [Log](clang19-native-validation.txt) |
| GCC 14 portable | [JSON](gcc14-portable.json) | [Text](gcc14-portable.txt) | [Log](gcc14-portable-validation.txt) |
| Clang 19 portable | [JSON](clang19-portable.json) | [Text](clang19-portable.txt) | [Log](clang19-portable-validation.txt) |

The native configurations were rerun after the final refinements: nine
repetitions, minimum CPU time 0.1 seconds, warmup 0.02 seconds, random interleaving,
and CPU 0 affinity. [Environment metadata](environment.txt) records exact
commands, compiler flags, framework version, CPU details, and source, executable,
and static-library hashes.

Both final rebuilt portable executables **and** their static libraries were
byte-identical to those from the [initial campaign](../google-benchmark-initial/).
Their JSON and console output are reused unchanged, explicitly marked
`identical_binary_reuse` in the summary. The portable validation logs above are
fresh final-build runs; the original timing-run logs are retained separately for
[GCC](gcc14-portable-timing-validation.txt) and
[Clang](clang19-portable-timing-validation.txt).

All four final rebuilt configurations passed 983,040 complete-result comparisons
each, totaling **3,932,160 comparisons**. Each JSON contains 270 raw repetitions
and 120 aggregates. Across the four configurations, the evidence therefore holds
**1,080 raw records and 480 aggregates**, including 540 freshly measured native
repetitions and 540 reused portable repetitions.

## Interpretation limits

The final Clang run retains successful-fallback overhead of **4.628 ns (4.71%)**
for FX multiplication and **6.504 ns (4.77%)** for mixed addition. Other fallback
deltas, including improvements, are preserved in the summary. These results do
not establish negligible overhead for every fallback.

Measurements used a shared virtual host without a frequency lock or exclusive
CPU reservation. Variability remained visible despite CPU affinity and random
interleaving: Clang general/public coefficients of variation were 6.48%/8.27%
for the FX-success fallback and 1.34%/4.47% for the mixed-add-success fallback.
Some other rows had larger outliers. Raw repetitions and aggregate dispersion
are retained so small differences can be assessed in that context.

## Reconstructing the measured inputs

In a separate checkout of the commit recorded in [source-base.txt](source-base.txt),
apply [final-inputs.patch](final-inputs.patch) from the repository root with
`git apply --unidiff-zero`. The patch uses zero context and
captures both production headers, benchmark source, dependency configuration,
and runner. Applying it in an isolated temporary tree reproduced all six source
hashes recorded in the environment exactly, as documented in
[source-reproduction.txt](source-reproduction.txt). The reconstructed runner also
matched the frozen working-tree script. No new builds or timings were needed for
this verification. Converting the patch to zero context and the summary CSV to
LF line endings preserved the measured source bytes and summary records.
