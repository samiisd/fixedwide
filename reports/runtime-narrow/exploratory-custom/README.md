# Superseded custom-harness exploratory measurements

These measurements used the earlier custom timing loop at commit
`39765459396f9072ff322b5ae672b9e1ec6e1b91`. They were useful for identifying
Clang fallback overhead, but the user subsequently requested Google Benchmark.
They are retained as implementation evidence and are not the final performance
measurements for this change. The parent report uses Google Benchmark results.

All four compiler/backend runs completed with 983,040 value/error comparisons
each and zero mismatches. Each CSV retains nine individual measurements per
side and the environment record identifies exact source and binary hashes.
