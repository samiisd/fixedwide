#!/usr/bin/env python3
"""Write a coverage badge SVG from llvm-cov's TOTAL line.

shields.io cannot be used: it has to fetch a value over the network, and this
repository is private, so the badge is generated here and committed. The CI
coverage job regenerates it, which means it cannot drift from the number the
gate actually enforced.

    coverage_badge.py coverage.txt docs/assets/coverage.svg
"""
import re
import sys

# Shields' own palette, so the badge does not look foreign next to the others.
def colour(pct: float) -> str:
    for threshold, value in ((90, "#4c1"), (80, "#97ca00"), (70, "#a4a61d"),
                             (60, "#dfb317"), (40, "#fe7d37")):
        if pct >= threshold:
            return value
    return "#e05d44"


def total_line_coverage(report: str) -> float:
    for line in report.splitlines():
        if line.startswith("TOTAL"):
            percentages = re.findall(r"(\d+\.\d+)%", line)
            # llvm-cov TOTAL columns: regions, functions, lines, branches.
            if len(percentages) < 3:
                raise SystemExit("TOTAL line has too few percentage columns")
            return float(percentages[2])
    raise SystemExit("no TOTAL line in the coverage report")


def badge(label: str, message: str, fill: str) -> str:
    # 6.5px per character is a close enough advance width for DejaVu Sans at
    # 11px, which is what shields uses; the badge is decorative, not a layout.
    label_w = int(len(label) * 6.5) + 10
    msg_w = int(len(message) * 6.5) + 10
    total = label_w + msg_w
    return f'''<svg xmlns="http://www.w3.org/2000/svg" width="{total}" height="20" role="img" aria-label="{label}: {message}">
  <title>{label}: {message}</title>
  <linearGradient id="s" x2="0" y2="100%">
    <stop offset="0" stop-color="#bbb" stop-opacity=".1"/><stop offset="1" stop-opacity=".1"/>
  </linearGradient>
  <clipPath id="r"><rect width="{total}" height="20" rx="3" fill="#fff"/></clipPath>
  <g clip-path="url(#r)">
    <rect width="{label_w}" height="20" fill="#555"/>
    <rect x="{label_w}" width="{msg_w}" height="20" fill="{fill}"/>
    <rect width="{total}" height="20" fill="url(#s)"/>
  </g>
  <g fill="#fff" text-anchor="middle" font-family="Verdana,Geneva,DejaVu Sans,sans-serif" font-size="11">
    <text x="{label_w / 2}" y="15" fill="#010101" fill-opacity=".3">{label}</text>
    <text x="{label_w / 2}" y="14">{label}</text>
    <text x="{label_w + msg_w / 2}" y="15" fill="#010101" fill-opacity=".3">{message}</text>
    <text x="{label_w + msg_w / 2}" y="14">{message}</text>
  </g>
</svg>
'''


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__, file=sys.stderr)
        return 2
    pct = total_line_coverage(open(sys.argv[1]).read())
    with open(sys.argv[2], "w") as out:
        out.write(badge("coverage", f"{pct:.1f}%", colour(pct)))
    print(f"{sys.argv[2]}: {pct:.1f}%")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
