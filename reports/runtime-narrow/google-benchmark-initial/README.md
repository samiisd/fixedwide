# Initial Google Benchmark measurements

This is the completed Google Benchmark campaign **before the final inlining
refinement**. Clang's successful fallback paths still showed approximately
3–6% overhead, so these measurements are retained as investigation evidence,
not the final performance claim for the PR.

Each of the four compiler/backend configurations validated 983,040 complete
results and produced 270 raw timing repetitions, plus Google Benchmark's
aggregate statistics. All comparisons passed.

The environment record contains exact source/library/binary hashes and commands.
To reproduce the measured source, check out `544c4ab09735cc809f83850e9a380211bfa4a1bc` and apply
`initial-inputs.patch` from the repository root with `git apply --unidiff-zero`.
The patch uses zero context and captures the modified
production header, benchmark, dependency configuration and runner before further
inlining changes. `initial-source-base.txt` also records this base commit.
The [reconstruction check](source-reproduction.txt) verifies that the patch
reproduces all six recorded source hashes exactly. Converting the patch to zero
context and the summary CSV to LF line endings preserved the measured source
bytes and summary records.

The final campaign is recorded separately in `../google-benchmark/`.
