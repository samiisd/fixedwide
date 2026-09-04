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
    add("cnl", "scaled_integer<int64,power<-4,10>>", "decimal_fixed_exact_4", arithmetic)
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

    print(f"validated {count} benchmark rows")
    return count


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, type=pathlib.Path)
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args()
    validate(args.input)


if __name__ == "__main__":
    main()
