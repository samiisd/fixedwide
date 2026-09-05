# Competitor evidence provenance

Schema-2 competitor CSVs and their accompanying logs are historical evidence, not a valid current performance baseline. The executable that produced them deliberately ran overflowing signed CNL binary multiplication. Those timings are withdrawn, and the report generator renders only a withdrawal notice for schema 2.

Schema 3 bounds the shared binary fixtures before arithmetic and labels CNL same-type division separately. It also traverses prepared serialization buffers rather than decoding one invariant value repeatedly. Fresh results must be rerun; do not relabel old measurements as schema 3.

The competitor workflow uploads Release CSVs, their matching stderr validation count, generated reports, source snapshot, build commands, CPU metadata, executable digest and source commit. It separately uploads sanitizer preflight evidence; those timings cannot generate performance tables.

To retain a new measured baseline, copy its actual `competitors.csv` and matching `competitors.stderr` from one successful workflow artifact into `competitors.csv` and `competitors.log` here, then run:

```sh
python3 scripts/competitor_report.py --input reports/raw/competitors.csv --require-provenance \
  --generate-markdown reports/BENCHMARK_COMPETITORS.md --update-readme README.md
```

The measured source commit and workflow URL belong to the recorded run. A later documentation-only commit must not be misrepresented as the source of those measurements.
