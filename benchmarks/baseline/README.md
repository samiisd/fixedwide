# Instruction-count baselines

One CSV per toolchain, recorded on the machine the gate runs on. `.github/workflows/ci.yml`
re-measures on every pull request and fails if any workload grows by more than 1%.

Instruction counts are compiler specific, so a baseline names its toolchain and
is only ever compared against itself. They are not timings: they say how much
work the CPU is asked to do, which is the part a code change controls. Wall-clock
numbers live in `reports/`.

To re-record after an intentional change:

```
scripts/icount.sh --update          # writes benchmarks/baseline/<arch>-<compiler>.csv
```

and say in the commit message why the number moved. Two independent runs of
`scripts/icount.sh` on the same binary produce byte-identical output; if yours
do not, something in the harness is nondeterministic and the gate is not
trustworthy until that is found.
