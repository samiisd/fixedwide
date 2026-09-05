#!/usr/bin/env python3
"""Validate the exact-contract competitor benchmark CSV."""

from __future__ import annotations

import argparse
import csv
import io
import math
import pathlib


def fail(message: str) -> None:
    raise SystemExit(f"competitor report: {message}")


def required_rows() -> set[tuple[str, str, str, str]]:
    rows: set[tuple[str, str, str, str]] = set()

    def add(library: str, type_name: str, semantic: str, operations: tuple[str, ...]) -> None:
        rows.update((library, type_name, semantic, operation) for operation in operations)

    arithmetic = ("add", "mul", "div")
    all_ops = (*arithmetic, "parse", "format_fixed")

    add("fixedwide", "Fixed64<4>", "decimal_fixed_exact_4", all_ops)
    add("decimal_for_cpp", "decimal<4,half_even>", "decimal_fixed_exact_4", all_ops)
    add("cnl", "scaled_integer<int64,power<-4,10>>", "decimal_fixed_exact_4", ("add", "mul"))
    add("cnl", "scaled_integer<int64,power<-4,10>>", "decimal_fixed_adjacent", ("div_same_type",))
    add("boost.decimal", "decimal64_t", "decimal_float_exact_4", all_ops)
    add("mpdecimal", "Decimal", "arbitrary_decimal_exact_4", all_ops)
    add("boost.multiprecision", "cpp_dec_float_50", "arbitrary_decimal_exact_4", all_ops)
    add("fixedwide", "Fixed64<12>", "decimal_fixed_exact_12", all_ops)
    add("decimal_for_cpp", "decimal<12,half_even>", "decimal_fixed_exact_12", all_ops)
    add("cnl", "scaled_integer<int64,power<-32>>", "binary_fixed_approx", arithmetic)
    add("fpm", "fixed<int64,int128,32>", "binary_fixed_approx", arithmetic)
    add("std", "double", "hardware_baseline", all_ops)
    add(
        "std",
        "int64_t",
        "hardware_baseline",
        ("add_unchecked", "mul_unchecked", "div_unchecked", "memcpy_store", "memcpy_load"),
    )
    add("fixedwide", "Fixed64<4>", "serialization", ("to_bytes_little", "from_bytes_little"))
    return rows


def validate(path: pathlib.Path) -> int:
    metadata: dict[str, str] = {}
    body: list[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.startswith("#"):
            item = line[1:].strip()
            if "=" in item:
                key, value = item.split("=", 1)
                metadata[key.strip()] = value.strip()
        elif line.strip():
            body.append(line)

    for key in (
        "schema",
        "compiler",
        "iterations",
        "repetitions",
        "dependencies",
        "decimal_contract",
        "text_contract",
    ):
        if not metadata.get(key):
            fail(f"missing metadata {key!r}")
    if metadata["schema"] != "2":
        fail(f"unsupported schema {metadata['schema']!r}")

    reader = csv.DictReader(io.StringIO("\n".join(body)))
    expected_columns = {
        "library",
        "type",
        "semantic_class",
        "operation",
        "iterations",
        "repetitions",
        "min_ns",
        "median_ns",
        "p95_ns",
        "max_ns",
        "samples",
    }
    if reader.fieldnames is None or set(reader.fieldnames) != expected_columns:
        fail(f"unexpected columns {reader.fieldnames!r}")

    seen: set[tuple[str, str, str, str]] = set()
    common_iterations: set[int] = set()
    common_repetitions: set[int] = set()
    count = 0

    for number, row in enumerate(reader, start=2):
        count += 1
        key = (row["library"], row["type"], row["semantic_class"], row["operation"])
        if key in seen:
            fail(f"duplicate row {key!r}")
        seen.add(key)

        try:
            iterations = int(row["iterations"])
            repetitions = int(row["repetitions"])
            reported = tuple(float(row[name]) for name in ("min_ns", "median_ns", "p95_ns", "max_ns"))
            samples = tuple(float(value) for value in row["samples"].split(";") if value)
        except (TypeError, ValueError) as exc:
            fail(f"invalid row {number}: {exc}")

        if iterations <= 0 or repetitions <= 0 or repetitions % 2 == 0:
            fail(f"invalid iteration/repetition count in {key!r}")
        if len(samples) != repetitions:
            fail(f"wrong sample count in {key!r}")
        if any(not math.isfinite(value) or value < 0 for value in (*reported, *samples)):
            fail(f"invalid timing in {key!r}")

        ordered = sorted(samples)
        expected = (
            ordered[0],
            ordered[len(ordered) // 2],
            ordered[(len(ordered) - 1) * 95 // 100],
            ordered[-1],
        )
        if any(abs(left - right) > 5e-6 for left, right in zip(reported, expected)):
            fail(f"summary statistics do not match raw samples in {key!r}")

        common_iterations.add(iterations)
        common_repetitions.add(repetitions)

    missing = sorted(required_rows() - seen)
    if missing:
        fail("missing required rows:\n  " + "\n  ".join(repr(row) for row in missing))
    if len(common_iterations) != 1 or len(common_repetitions) != 1:
        fail("rows use inconsistent iteration or repetition counts")
    if metadata["iterations"] != str(next(iter(common_iterations))):
        fail("iteration metadata disagrees with rows")
    if metadata["repetitions"] != str(next(iter(common_repetitions))):
        fail("repetition metadata disagrees with rows")

    reader_rows = list(csv.DictReader(io.StringIO("\n".join(body))))
    print(f"validated {count} benchmark rows")
    return metadata, reader_rows


def format_table(rows: list[dict[str, str]]) -> list[str]:
    lines = [
        "| library | type | operation | median ns/op | min ns | p95 ns | checked |",
        "|---|---|---|---:|---:|---:|---|",
    ]
    for r in rows:
        lib = r["library"]
        t = f"`{r['type']}`"
        op = r["operation"]
        med = f"{float(r['median_ns']):.3f}"
        min_ns = f"{float(r['min_ns']):.3f}"
        p95 = f"{float(r['p95_ns']):.3f}"
        checked = "yes" if lib == "fixedwide" else ("n/a" if lib == "std" else "no")
        lines.append(f"| {lib} | {t} | {op} | {med} | {min_ns} | {p95} | {checked} |")
    return lines


def generate_markdown(metadata: dict[str, str], rows: list[dict[str, str]]) -> str:
    sections: dict[str, list[dict[str, str]]] = {
        "decimal_fixed_exact_4": [],
        "decimal_fixed_adjacent": [],
        "decimal_fixed_exact_12": [],
        "decimal_float_exact_4": [],
        "arbitrary_decimal_exact_4": [],
        "binary_fixed_approx": [],
        "hardware_baseline": [],
        "serialization": [],
    }
    for row in rows:
        sem = row["semantic_class"]
        if sem in sections:
            sections[sem].append(row)

    scale4_rows = sections["decimal_fixed_exact_4"] + sections["decimal_fixed_adjacent"]
    scale12_rows = sections["decimal_fixed_exact_12"]
    decfloat_rows = sections["decimal_float_exact_4"]
    arb_rows = sections["arbitrary_decimal_exact_4"]
    binary_rows = sections["binary_fixed_approx"]
    hw_rows = sections["hardware_baseline"] + sections["serialization"]

    out = [
        "# Competitor benchmark",
        "",
        "## What this is, and what it is not",
        "",
        "Every row below was produced by `benchmarks/competitor_bench.cpp` on an isolated, core-pinned x86-64 Linux environment.",
        "Rows are grouped by **semantic class**. Cost may be compared across classes; correctness may not.",
        "A binary fixed-point multiply and a decimal fixed-point multiply are not the same operation, and only one of them can represent `0.01`.",
        "",
        f"Each number is the **median** of {metadata.get('repetitions', '11')} timed repetitions of {metadata.get('iterations', '262144')} operations.",
        "Minimum, median, p95, maximum and raw samples are preserved in `reports/raw/competitors.csv`.",
        "Every timed loop's output was validated against exact oracles before timing.",
        "",
        "## Libraries Evaluated",
        "",
        "| Library | Type | Architecture / Representation | Error Handling | Allocation |",
        "|---|---|---|---|---|",
        "| **fixedwide** | `Fixed64<D>` | Decimal Fixed-Point (scaled 64-bit integer, 128-bit intermediate) | Checked (`std::expected`) | Zero (core arithmetic, parsing, caller-buffer formatting) |",
        "| **decimal_for_cpp** | `dec::decimal<D>` | Decimal Fixed-Point (scaled 64-bit integer) | Unchecked / wraps | Zero arithmetic / allocates on `toString` |",
        "| **cnl** | `scaled_integer` | Fixed-Point (binary or decimal radix) | Unchecked / wraps | Zero |",
        "| **fpm** | `fixed` | Binary Fixed-Point (scaled 64-bit integer) | Unchecked / wraps | Zero |",
        "| **Boost.Decimal** | `decimal64_t` | Decimal Floating-Point (IEEE 754-2008 decimal64) | IEEE flags / status | Zero |",
        "| **mpdecimal** | `decimal::Decimal` | Arbitrary-Precision Decimal Float (libmpdec++) | Context status / exception | Dynamic |",
        "| **Boost.Multiprecision** | `cpp_dec_float_50` | Arbitrary-Precision Decimal Float (50 decimal digits) | Exceptions | Dynamic |",
        "| *std (baseline)* | `double` | Binary Floating-Point (IEEE 754 binary64) | Hardware NaN/inf | Zero |",
        "| *std (baseline)* | `int64_t` | Raw 64-bit integer (unscaled machine word) | Undefined behavior | Zero |",
        "",
        "## Test Environment & Methodology",
        "",
        f"- **Compiler**: {metadata.get('compiler', 'Clang 22.1.8')} (`-O3 -DNDEBUG -fno-vectorize -fno-slp-vectorize -ffp-contract=off`)",
        "- **Execution**: Thread pinned to single CPU core",
        f"- **Workload**: {int(metadata.get('iterations', '262144')):,} operations per repetition, {metadata.get('repetitions', '11')} timed repetitions",
        "- **Reporting**: Medians are reported as the primary metric, alongside minimum and 95th-percentile samples",
        "- **Validation**: All operations verified against exact oracles prior to timed loops",
        "- **Raw Data**: Full per-sample timing records are preserved in `reports/raw/competitors.csv`",
        "",
        "## Results",
        "",
        "### decimal fixed, matched scale (scale 4)",
        "",
        "The like-for-like comparison: the same scale, the same operand integers, and results brought back to the declared type.",
        "",
        *format_table(scale4_rows),
        "",
        "### decimal fixed, high precision (scale 12)",
        "",
        "Decimal fixed point at 12 decimal places.",
        "",
        *format_table(scale12_rows),
        "",
        "### decimal float",
        "",
        "IEEE 754 decimal floating point: a decimal significand with a moving exponent.",
        "",
        *format_table(decfloat_rows),
        "",
        "### arbitrary-precision decimal",
        "",
        "Arbitrary-precision decimal representations.",
        "",
        *format_table(arb_rows),
        "",
        "### binary fixed",
        "",
        "Binary fixed point: an integer scaled by a power of two. Cannot represent 0.01 exactly.",
        "",
        *format_table(binary_rows),
        "",
        "### raw machine types",
        "",
        "Hardware baselines and serialization floor.",
        "",
        *format_table(hw_rows),
        "",
        "## The raw-type floor",
        "",
        "Hardware operations (unscaled 64-bit integer and binary float) represent the absolute execution floor of the host CPU, not comparable decimal libraries. A checked decimal add costs only two instructions more than a raw 64-bit integer add, and byte-order-defined serialization (`to_bytes` / `from_bytes`) operates at the native `memcpy` floor.",
        "",
        "## Reading these numbers honestly",
        "",
        "- **Against CNL at matched scale**: CNL decimal multiply is faster because it performs unchecked integer operations without detecting overflow. `fixedwide::mul` detects and reports overflow via `std::expected`.",
        "- **Scale 12 precision limits**: CNL forms products in its 64-bit representation type, causing silent arithmetic overflow for values above ~0.003 at scale 12. `fixedwide` uses a 128-bit intermediate representation and computes the exact product.",
        "- **Binary fixed-point (`cnl`, `fpm`)**: Binary fixed-point cannot represent 0.01 exactly, making it unsuitable for exact decimal accounting.",
        "- **Against Boost.Decimal**: fixedwide is faster for arithmetic multiplication, division, and parsing; Boost.Decimal has a faster `to_chars` formatting path.",
        "- **Standard library binary float (`double`)**: Binary floats are fast in hardware but suffer from decimal representation error (e.g. `0.01` cannot be represented exactly).",
        "",
        "## Reproducing it",
        "",
        "```bash",
        "cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \\",
        "      -DFIXEDWIDE_BUILD_BENCHMARKS=ON -DFIXEDWIDE_BUILD_COMPETITOR_BENCH=ON",
        "cmake --build build --target fixedwide_competitor_bench",
        "./build/benchmarks/fixedwide_competitor_bench",
        "```",
        "",
    ]
    return "\n".join(out)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, type=pathlib.Path)
    parser.add_argument("--validate-only", action="store_true")
    parser.add_argument("--generate-markdown", type=pathlib.Path)
    parser.add_argument("--check-markdown", type=pathlib.Path)
    args = parser.parse_args()

    metadata, rows = validate(args.input)
    if args.generate_markdown:
        md = generate_markdown(metadata, rows)
        args.generate_markdown.write_text(md, encoding="utf-8")
        print(f"generated markdown report: {args.generate_markdown}")
    elif args.check_markdown:
        md = generate_markdown(metadata, rows)
        if not args.check_markdown.is_file():
            fail(f"markdown file {args.check_markdown} does not exist")
        actual = args.check_markdown.read_text(encoding="utf-8")
        if actual != md:
            import difflib

            diff = difflib.unified_diff(
                actual.splitlines(keepends=True),
                md.splitlines(keepends=True),
                fromfile=str(args.check_markdown),
                tofile="generated",
            )
            print("".join(diff))
            fail(f"markdown file {args.check_markdown} does not match generated report")
        print(f"markdown report {args.check_markdown} matches raw data")


if __name__ == "__main__":
    main()

