#!/usr/bin/env python3
"""Report distinct instrumented source lines, alongside untouched LLVM totals.

LLVM's summary weights some overlapping template mappings differently from its
LCOV DA records. Do not relabel one as the other: publish both. This script uses
all DA records under include/fixedwide and src, never exclusions for particular
branches/functions. Native and portable profiles are reported separately.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import sys


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def library_file(filename: str, root: Path) -> str | None:
    try:
        path = Path(filename).resolve().relative_to(root.resolve())
    except ValueError:
        return None
    if path.parts[0] == "src" or path.parts[:2] == ("include", "fixedwide"):
        return path.as_posix()
    return None


def read_lcov(text: str, root: Path) -> dict[str, dict[int, int]]:
    result: dict[str, dict[int, int]] = {}
    current: str | None = None
    for line in text.splitlines():
        if line.startswith("SF:"):
            current = library_file(line[3:], root)
            if current is not None:
                result.setdefault(current, {})
        elif line == "end_of_record":
            current = None
        elif line.startswith("DA:") and current is not None:
            fields = line[3:].split(",")
            require(len(fields) in (2, 3), "malformed LCOV DA record")
            number, hits = int(fields[0]), int(fields[1])
            require(number > 0 and hits >= 0, "invalid LCOV line/count")
            result[current][number] = max(result[current].get(number, 0), hits)
    require(bool(result) and any(result.values()), "no instrumented library source lines")
    return result


def percentage(covered: int, total: int) -> float:
    require(total > 0 and 0 <= covered <= total, "invalid coverage totals")
    return 100.0 * covered / total


def summarize(lcov: str, llvm: dict, root: Path, backend: str) -> dict:
    lines = read_lcov(lcov, root)
    required_sources = {path.relative_to(root).as_posix() for path in (root / "src").glob("*.cpp")}
    require(bool(required_sources), "source directory has no .cpp files")
    require(required_sources <= lines.keys(), "library translation units missing from coverage: " + str(sorted(required_sources - lines.keys())))
    require(len(llvm.get("data", [])) == 1, "expected one LLVM export")
    data = llvm["data"][0]
    branches: dict[str, dict[tuple[int, ...], tuple[int, int]]] = {}
    for entry in data["files"]:
        name = library_file(entry["filename"], root)
        if name is None:
            continue
        table = branches.setdefault(name, {})
        for branch in entry.get("branches", []):
            require(len(branch) >= 6, "malformed LLVM branch record")
            key = tuple(branch[:4])
            counts = tuple(branch[4:6])
            require(all(isinstance(count, int) and count >= 0 for count in counts), "invalid branch count")
            previous = table.get(key, (0, 0))
            table[key] = (max(counts[0], previous[0]), max(counts[1], previous[1]))
    files = []
    for name, table in sorted(lines.items()):
        branch_table = branches.get(name, {})
        covered = sum(count > 0 for count in table.values())
        files.append({
            "file": name, "source_lines": len(table), "covered_source_lines": covered,
            "source_line_percent": percentage(covered, len(table)),
            "uncovered_lines": sorted(line for line, count in table.items() if count == 0),
            "branch_outcomes": len(branch_table) * 2,
            "covered_branch_outcomes": sum(count > 0 for pair in branch_table.values() for count in pair),
            "uncovered_branches": [list(key) + list(pair) for key, pair in sorted(branch_table.items()) if 0 in pair],
        })
    line_total = sum(file["source_lines"] for file in files)
    line_hit = sum(file["covered_source_lines"] for file in files)
    branch_total = sum(file["branch_outcomes"] for file in files)
    branch_hit = sum(file["covered_branch_outcomes"] for file in files)
    totals = data["totals"]
    for metric in ("lines", "branches", "functions", "regions"):
        value = totals[metric]
        require(math.isfinite(value["percent"]), "non-finite LLVM metric")
        require(value["count"] >= value["covered"] >= 0, "invalid LLVM metric")
    return {
        "schema": 1, "backend": backend,
        "metric": "distinct instrumented source lines (LCOV DA records), all library files",
        "source_lines": line_total, "covered_source_lines": line_hit,
        "source_line_percent": percentage(line_hit, line_total),
        "branch_outcomes": branch_total, "covered_branch_outcomes": branch_hit,
        "source_branch_percent": percentage(branch_hit, branch_total),
        "llvm_totals": totals,
        "files": files,
    }


def markdown(summary: dict) -> str:
    llvm = summary["llvm_totals"]
    out = [
        f"# Coverage: {summary['backend']}", "",
        "Profiles are collected from one comprehensive exercise executable. The full CTest suite is run separately as a correctness gate.",
        "No profiles from different executables or backends are merged.", "",
        "| Metric | Covered / total | Percent |", "|---|---:|---:|",
        f"| Distinct instrumented source lines (LCOV) | {summary['covered_source_lines']} / {summary['source_lines']} | {summary['source_line_percent']:.4f}% |",
        f"| Distinct source branch outcomes | {summary['covered_branch_outcomes']} / {summary['branch_outcomes']} | {summary['source_branch_percent']:.4f}% |",
    ]
    for name in ("lines", "branches", "functions", "regions"):
        row = llvm[name]
        out.append(f"| LLVM summary {name} (unmodified) | {row['covered']} / {row['count']} | {row['percent']:.4f}% |")
    out += ["", "The two line metrics are not interchangeable. LLVM summary totals include overlapping template mappings; LCOV counts each physical instrumented source line once.",
            "These figures do not prove every template instantiation, platform-specific branch, or possible input is covered.", "",
            "## Files and remaining gaps", "", "| File | Source lines | Percent | Uncovered line numbers |", "|---|---:|---:|---|"]
    for file in summary["files"]:
        missing = ", ".join(map(str, file["uncovered_lines"])) or "none"
        out.append(f"| `{file['file']}` | {file['covered_source_lines']} / {file['source_lines']} | {file['source_line_percent']:.3f}% | {missing} |")
    return "\n".join(out) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lcov", type=Path, required=True)
    parser.add_argument("--llvm-json", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--backend", choices=("native", "portable"), required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--min-source-lines", type=float, default=99.0)
    parser.add_argument("--min-llvm-lines", type=float, default=95.0)
    parser.add_argument("--min-llvm-branches", type=float, default=90.0)
    args = parser.parse_args()
    try:
        summary = summarize(args.lcov.read_text(), json.loads(args.llvm_json.read_text()), args.source_root, args.backend)
        summary["inputs_sha256"] = {path.name: hashlib.sha256(path.read_bytes()).hexdigest() for path in (args.lcov, args.llvm_json)}
        args.output.mkdir(parents=True, exist_ok=True)
        (args.output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
        report = markdown(summary)
        (args.output / "SUMMARY.md").write_text(report)
        print(report)
        failures = []
        for label, actual, threshold in (
            ("distinct source lines", summary["source_line_percent"], args.min_source_lines),
            ("LLVM summary lines", summary["llvm_totals"]["lines"]["percent"], args.min_llvm_lines),
            ("LLVM summary branches", summary["llvm_totals"]["branches"]["percent"], args.min_llvm_branches),
        ):
            require(0 <= threshold <= 100, "coverage threshold outside [0,100]")
            if actual < threshold:
                failures.append(f"{label}: {actual:.6f}% < {threshold}%")
        if failures:
            raise ValueError("coverage gate failed: " + "; ".join(failures))
        return 0
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"coverage report: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
