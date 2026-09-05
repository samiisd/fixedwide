"""Fail-closed tests for coverage input handling and transparent metrics."""
import json
from pathlib import Path
import tempfile
import unittest

import coverage_report as report


class CoverageReportTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        (self.root / "src").mkdir()
        (self.root / "src" / "one.cpp").write_text("a\nb\n")
        self.filename = str(self.root / "src" / "one.cpp")

    def export(self):
        metric = {"count": 2, "covered": 1, "percent": 50.0}
        return {"data": [{"files": [{"filename": self.filename,
                     "branches": [[1, 1, 1, 3, 1, 0, 0, 0, 4]]}],
                         "totals": {key: metric.copy() for key in ("lines", "regions", "branches", "functions")}}]}

    def lcov(self, body="DA:1,1\nDA:2,0\n"):
        return f"SF:{self.filename}\n{body}end_of_record\n"

    def test_retains_uncovered_lines(self):
        result = report.summarize(self.lcov(), self.export(), self.root, "native")
        self.assertEqual(result["source_line_percent"], 50)
        self.assertEqual(result["files"][0]["uncovered_lines"], [2])

    def test_duplicate_mappings_are_not_double_counted(self):
        result = report.read_lcov(self.lcov() + self.lcov("DA:1,0\nDA:2,2\n"), self.root)
        self.assertEqual(result["src/one.cpp"], {1: 1, 2: 2})

    def test_does_not_replace_llvm_totals(self):
        result = report.summarize(self.lcov("DA:1,1\nDA:2,1\n"), self.export(), self.root, "native")
        self.assertEqual(result["source_line_percent"], 100)
        self.assertEqual(result["llvm_totals"]["lines"]["percent"], 50)
        self.assertIn("not interchangeable", report.markdown(result))

    def test_missing_translation_unit_fails(self):
        (self.root / "src" / "two.cpp").write_text("return 0;\n")
        with self.assertRaises(ValueError):
            report.summarize(self.lcov(), self.export(), self.root, "native")

    def test_no_library_code_fails(self):
        with self.assertRaises(ValueError):
            report.read_lcov("SF:/usr/include/x.hpp\nDA:1,4\nend_of_record\n", self.root)

    def test_negative_count_fails(self):
        with self.assertRaises(ValueError):
            report.read_lcov(self.lcov("DA:1,-1\n"), self.root)

    def test_zero_line_fails(self):
        with self.assertRaises(ValueError):
            report.read_lcov(self.lcov("DA:0,1\n"), self.root)

    def test_malformed_row_fails(self):
        with self.assertRaises(ValueError):
            report.read_lcov(self.lcov("DA:1,one\n"), self.root)

    def test_both_branch_outcomes_required(self):
        result = report.summarize(self.lcov(), self.export(), self.root, "native")
        self.assertEqual(result["source_branch_percent"], 50)

    def test_nonfinite_metric_fails(self):
        export = self.export()
        export["data"][0]["totals"]["lines"]["percent"] = float("nan")
        with self.assertRaises(ValueError):
            report.summarize(self.lcov(), export, self.root, "native")

    def test_multiple_exports_fail(self):
        export = self.export()
        export["data"] *= 2
        with self.assertRaises(ValueError):
            report.summarize(self.lcov(), export, self.root, "native")

    def test_metrics_serialize_without_special_objects(self):
        result = report.summarize(self.lcov(), self.export(), self.root, "portable")
        self.assertEqual(json.loads(json.dumps(result))["backend"], "portable")


if __name__ == "__main__":
    unittest.main()
