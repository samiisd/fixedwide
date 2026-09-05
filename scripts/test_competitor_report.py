#!/usr/bin/env python3
"""Structural report tests; all numbers here are synthetic, never benchmark data."""
import copy
import unittest
import competitor_report as report


class ReportTests(unittest.TestCase):
    def setUp(self):
        self.meta = dict(schema="3", compiler="SYNTHETIC TEST DATA", iterations="8", repetitions="3",
                         dependencies="synthetic", decimal_contract="test", binary_contract="bounded test",
                         text_contract="test", validations="1", source_commit="0" * 40,
                         binary_sha256="0" * 64, cpu="synthetic", flags="synthetic", affinity="0", run_url="test")
        self.rows = [dict(zip(("library", "type", "semantic_class", "operation"), key),
                          iterations="8", repetitions="3", min_ns="1", median_ns="2", p95_ns="2",
                          max_ns="3", samples="1;2;3") for key in sorted(report.required_rows())]

    def test_complete(self):
        self.assertEqual(len(self.rows), 56)
        report.validate(self.meta, self.rows, True)

    def test_missing_row(self):
        with self.assertRaises(SystemExit):
            report.validate(self.meta, self.rows[:-1])

    def test_duplicate(self):
        with self.assertRaises(SystemExit):
            report.validate(self.meta, self.rows + self.rows[:1])

    def test_wrong_statistic(self):
        self.rows[0]["median_ns"] = "9"
        with self.assertRaises(SystemExit):
            report.validate(self.meta, self.rows)

    def test_nonfinite(self):
        self.rows[0]["samples"] = "1;nan;3"
        with self.assertRaises(SystemExit):
            report.validate(self.meta, self.rows)

    def test_sanitizer_data_not_publishable(self):
        self.meta["mode"] = "sanitized"
        with self.assertRaises(SystemExit):
            report.validate(self.meta, self.rows)

    def test_old_schema_not_publishable(self):
        self.meta["schema"] = "2"
        with self.assertRaises(SystemExit):
            report.validate(self.meta, self.rows)
        self.assertNotIn("|", report.summary(self.meta, self.rows))
        self.assertIn("withdrawn", report.summary(self.meta, self.rows))

    def test_missing_provenance(self):
        del self.meta["source_commit"]
        with self.assertRaises(SystemExit):
            report.validate(self.meta, self.rows, True)

    def test_readme_is_idempotent_and_data_driven(self):
        text = f"Intro\n{report.BEGIN}\nold\n{report.END}\nEnd\n"
        rendered = report.update_readme(text, report.summary(self.meta, self.rows))
        self.assertEqual(rendered, report.update_readme(rendered, report.summary(self.meta, self.rows)))
        changed = copy.deepcopy(self.rows)
        for r in changed:
            if r["library"] == "fixedwide" and r["type"] == "Fixed64<4>" and r["operation"] == "mul":
                r["median_ns"] = "7"
        self.assertNotEqual(rendered, report.update_readme(text, report.summary(self.meta, changed)))

    def test_missing_markers_rejected(self):
        with self.assertRaises(SystemExit):
            report.update_readme("README without markers", "summary")

    def test_unexpected_row(self):
        self.rows[0]["operation"] = "unregistered"
        with self.assertRaises(SystemExit):
            report.validate(self.meta, self.rows)

    def test_inconsistent_counts(self):
        self.rows[0]["iterations"] = "4"
        with self.assertRaises(SystemExit):
            report.validate(self.meta, self.rows)


if __name__ == "__main__":
    unittest.main()
