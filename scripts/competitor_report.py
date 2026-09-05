#!/usr/bin/env python3
"""Validate retained competitor samples and render reports without embedded timings."""
from __future__ import annotations

import argparse
import csv
import hashlib
import io
import math
import pathlib
import re

BEGIN = "<!-- BEGIN GENERATED COMPETITOR SUMMARY -->"
END = "<!-- END GENERATED COMPETITOR SUMMARY -->"
WITHDRAWN = (
    "Previously retained schema-2 timings are withdrawn: their process executed "
    "an overflowing CNL binary multiplication. They must not support performance claims. "
    "The corrected schema-3 benchmark bounds binary operands before execution. "
    "Fresh Release CSV, validation logs and generated reports are available from the "
    "[Competitor benchmark workflow](https://github.com/samiisd/fixedwide/actions/workflows/competitors.yml). "
    "No replacement timings are claimed until that evidence is retained."
)


def fail(message: str) -> None:
    raise SystemExit(f"competitor report: {message}")


def required_rows() -> set[tuple[str, str, str, str]]:
    rows: set[tuple[str, str, str, str]] = set()
    def add(lib: str, typ: str, sem: str, ops: tuple[str, ...]) -> None:
        rows.update((lib, typ, sem, op) for op in ops)
    arithmetic = ("add", "mul", "div")
    all_ops = (*arithmetic, "parse", "format_fixed")
    for d in (4, 12):
        sem = f"decimal_fixed_exact_{d}"
        add("fixedwide", f"Fixed64<{d}>", sem, all_ops)
        add("decimal_for_cpp", f"decimal<{d},half_even>", sem, all_ops)
    add("cnl", "scaled_integer<int64,power<-4,10>>", "decimal_fixed_exact_4", ("add", "mul"))
    add("cnl", "scaled_integer<int64,power<-4,10>>", "decimal_fixed_adjacent", ("div_same_type",))
    add("boost.decimal", "decimal64_t", "decimal_float_exact_4", all_ops)
    add("mpdecimal", "Decimal", "arbitrary_decimal_exact_4", all_ops)
    add("boost.multiprecision", "cpp_dec_float_50", "arbitrary_decimal_exact_4", all_ops)
    add("cnl", "scaled_integer<int64,power<-32>>", "binary_fixed_approx", ("add", "mul", "div_same_type"))
    add("fpm", "fixed<int64,int128,32>", "binary_fixed_approx", arithmetic)
    add("std", "double", "hardware_baseline", all_ops)
    add("std", "int64_t", "hardware_baseline",
        ("add_unchecked", "mul_unchecked", "div_unchecked", "memcpy_store", "memcpy_load"))
    add("fixedwide", "Fixed64<4>", "serialization", ("to_bytes_little", "from_bytes_little"))
    return rows


def load(path: pathlib.Path) -> tuple[dict[str, str], list[dict[str, str]]]:
    metadata: dict[str, str] = {}
    body: list[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.startswith("#"):
            item = line[1:].strip()
            if "=" in item:
                key, value = item.split("=", 1)
                key = key.strip()
                if key in metadata:
                    fail(f"duplicate metadata {key!r}")
                metadata[key] = value.strip()
        elif line.strip():
            body.append(line)
    reader = csv.DictReader(io.StringIO("\n".join(body)))
    columns = ("library", "type", "semantic_class", "operation", "iterations", "repetitions",
               "min_ns", "median_ns", "p95_ns", "max_ns", "samples")
    if reader.fieldnames != list(columns):
        fail(f"unexpected columns {reader.fieldnames!r}")
    return metadata, list(reader)


def validate(metadata: dict[str, str], rows: list[dict[str, str]], provenance: bool = False) -> None:
    for key in ("schema", "compiler", "iterations", "repetitions", "dependencies",
                "decimal_contract", "binary_contract", "text_contract", "validations"):
        if not metadata.get(key):
            fail(f"missing metadata {key!r}")
    if metadata["schema"] != "3":
        fail("only corrected schema-3 timings may be published")
    if metadata.get("mode", "timing") != "timing":
        fail("sanitizer timings are validation evidence, not performance results")
    if provenance:
        for key in ("source_commit", "binary_sha256", "cpu", "flags", "affinity", "run_url"):
            if not metadata.get(key):
                fail(f"missing provenance {key!r}")
        if not re.fullmatch(r"[0-9a-f]{40}", metadata["source_commit"]):
            fail("invalid source commit")
        if not re.fullmatch(r"[0-9a-f]{64}", metadata["binary_sha256"]):
            fail("invalid executable SHA-256")
    try:
        if int(metadata["validations"]) <= 0:
            fail("missing successful preflight checks")
        expected_iterations = int(metadata["iterations"])
        expected_repetitions = int(metadata["repetitions"])
    except ValueError:
        fail("invalid numeric metadata")
    seen: set[tuple[str, str, str, str]] = set()
    for row in rows:
        if None in row or any(v is None for v in row.values()):
            fail("malformed CSV row")
        key = tuple(row[name] for name in ("library", "type", "semantic_class", "operation"))
        if key in seen:
            fail(f"duplicate row {key!r}")
        seen.add(key)
        try:
            n, reps = int(row["iterations"]), int(row["repetitions"])
            values = tuple(float(row[x]) for x in ("min_ns", "median_ns", "p95_ns", "max_ns"))
            samples = tuple(float(x) for x in row["samples"].split(";"))
        except (ValueError, TypeError):
            fail(f"invalid numeric row {key!r}")
        if n <= 0 or reps <= 0 or reps % 2 == 0 or (n, reps) != (expected_iterations, expected_repetitions):
            fail(f"inconsistent workload in {key!r}")
        if len(samples) != reps or any(not math.isfinite(x) or x < 0 for x in (*samples, *values)):
            fail(f"invalid samples in {key!r}")
        ordered = sorted(samples)
        expected = (ordered[0], ordered[reps // 2], ordered[(reps - 1) * 95 // 100], ordered[-1])
        if any(abs(a - b) > 5e-6 for a, b in zip(values, expected)):
            fail(f"summary disagrees with samples in {key!r}")
    if seen != required_rows():
        fail(f"row set mismatch; missing={required_rows() - seen!r}; extra={seen - required_rows()!r}")


def scope() -> str:
    return (
        "These are independent-operation throughput microbenchmarks, not dependency-chain latency. "
        "Decimal multiplication/division fixtures are deliberately exact at the selected scale; "
        "they do not measure the general cost of inexact nearest-even rounding. "
        "Decimal preflight checks compare raw values or canonical fixed-format text against integer-derived expectations. "
        "Binary fixed-point and double checks use documented floating tolerances; "
        "cpp_dec_float_50 division uses a 1e-45 residual plus exact four-place text. "
        "CNL div_same_type discards fractional quotient digits and is NOT an equivalent division result."
    )


def summary(metadata: dict[str, str], rows: list[dict[str, str]]) -> str:
    if metadata.get("schema") == "2":
        return WITHDRAWN + "\n"
    lookup = {(r["library"], r["type"], r["operation"]): r for r in rows}
    types = (("fixedwide", "Fixed64<4>"), ("decimal_for_cpp", "decimal<4,half_even>"),
             ("boost.decimal", "decimal64_t"), ("std", "double"))
    out = ["Exact-result scale-4 throughput; median ns/op. Different error models are not equivalent contracts.",
           "", "| operation | fixedwide | decimal_for_cpp | Boost.Decimal | double |",
           "|---|---:|---:|---:|---:|"]
    for op in ("mul", "div", "parse", "format_fixed"):
        cells = [f"{float(lookup[(lib, typ, op)]['median_ns']):.3f}" for lib, typ in types]
        out.append("| " + op + " | " + " | ".join(cells) + " |")
    out += ["", f"Recorded compiler: `{metadata['compiler']}`. "
            f"Source commit: `{metadata.get('source_commit', 'not recorded')}`. "
            f"{metadata['repetitions']} repetitions of {metadata['iterations']} operations.", "", scope(), ""]
    return "\n".join(out)


def markdown(metadata: dict[str, str], rows: list[dict[str, str]], digest: str) -> str:
    if metadata.get("schema") == "2":
        return "# Competitor benchmark\n\n" + WITHDRAWN + "\n"
    out = ["# Competitor benchmark", "", scope(), "", "## Recorded provenance", "",
           f"CSV SHA-256: `{digest}`", ""]
    for key in ("source_commit", "compiler", "cpu", "flags", "affinity", "run_url", "binary_sha256",
                "iterations", "repetitions", "validations", "dependencies", "decimal_contract",
                "binary_contract", "text_contract"):
        out.append(f"- {key}: `{metadata.get(key, 'not recorded')}`")
    out += ["", "## Reading the results", "",
            "The binary CNL/fpm inputs share the scale-4 multiplication fixtures divided by 32. "
            "Every CNL raw product is checked in __int128 before the int64 operation executes. "
            "These bounded binary workloads do not share the decimal workloads' economic range.", "",
            "fixedwide uses checked decimal rescaling. decimal_for_cpp explicitly selects half-even; "
            "CNL and fpm use their configured arithmetic without fixedwide-style checked overflow. "
            "Unconfigured signed overflow is not promised to wrap. Only successful bounded inputs are timed.", "",
            "Boost.Decimal has a moving decimal exponent. mpdecimal has runtime precision and may allocate. "
            "The default cpp_dec_float_50 stores its fixed-precision digits inside the object, without a "
            "digit-storage allocator; string conversions may allocate. Allocating string formatters are "
            "labelled by API/type and should not be mistaken for caller-buffer formatting.", "",
            "Serialization load rows traverse prepared buffers. They are microbenchmarks, not a universal memcpy floor. "
            "The p95 column follows the harness's lower order statistic: sorted[(n-1)*95/100]. "
            "All samples remain in the CSV. Sanitized runs must not supply timing tables.", ""]
    for sem in dict.fromkeys(r["semantic_class"] for r in rows):
        out += [f"## {sem}", "", "| library | type | operation | median ns/op | min ns | p95 ns |",
                "|---|---|---|---:|---:|---:|"]
        for r in rows:
            if r["semantic_class"] == sem:
                out.append(f"| {r['library']} | `{r['type']}` | {r['operation']} | "
                           f"{float(r['median_ns']):.3f} | {float(r['min_ns']):.3f} | {float(r['p95_ns']):.3f} |")
        out.append("")
    out += ["## Reproduce", "", "Build mpdecimal first with scripts/build_mpdecimal.sh and use its prefix as "
            "FIXEDWIDE_MPDECIMAL_ROOT. The complete Release and UBSan/ASan commands, recorded environment, "
            "binary hashes and raw outputs are retained by .github/workflows/competitors.yml. "
            "Do not reuse a schema-2 baseline; those measurements were withdrawn.", "",
            "Generate this report and the README summary from the SAME validated CSV:", "", "```bash",
            "python3 scripts/competitor_report.py --input reports/raw/competitors.csv --require-provenance \\",
            "  --generate-markdown reports/BENCHMARK_COMPETITORS.md --update-readme README.md", "```", ""]
    return "\n".join(out)


def update_readme(text: str, generated: str) -> str:
    if text.count(BEGIN) != 1 or text.count(END) != 1:
        fail("README must contain exactly one generated-summary marker pair")
    before, rest = text.split(BEGIN, 1)
    _, after = rest.split(END, 1)
    return before + BEGIN + "\n\n" + generated.rstrip() + "\n\n" + END + after


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=pathlib.Path, required=True)
    parser.add_argument("--validate-only", action="store_true")
    parser.add_argument("--require-provenance", action="store_true")
    parser.add_argument("--generate-markdown", type=pathlib.Path)
    parser.add_argument("--check-markdown", type=pathlib.Path)
    parser.add_argument("--update-readme", type=pathlib.Path)
    parser.add_argument("--check-readme", type=pathlib.Path)
    args = parser.parse_args()
    meta, rows = load(args.input)
    withdrawn = meta.get("schema") == "2"
    if withdrawn and args.validate_only:
        fail("schema-2 data is withdrawn; rerun the corrected benchmark")
    if not withdrawn:
        validate(meta, rows, args.require_provenance)
        print(f"validated {len(rows)} benchmark rows")
    else:
        print("schema-2 data is withdrawn; rendering a notice without timings")
    report = markdown(meta, rows, hashlib.sha256(args.input.read_bytes()).hexdigest())
    if args.generate_markdown:
        args.generate_markdown.write_text(report, encoding="utf-8")
    if args.check_markdown and args.check_markdown.read_text(encoding="utf-8") != report:
        fail("Markdown differs from generated report")
    for path, check in ((args.update_readme, False), (args.check_readme, True)):
        if path:
            old = path.read_text(encoding="utf-8")
            new = update_readme(old, summary(meta, rows))
            if check and old != new:
                fail("README summary differs from retained data")
            if not check:
                path.write_text(new, encoding="utf-8")


if __name__ == "__main__":
    main()
