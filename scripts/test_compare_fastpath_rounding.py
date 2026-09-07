"""Reject incomplete or incomparable supplementary performance evidence."""
import csv
import io
import unittest

from compare_fastpath_rounding import compare_matrix, matrix_keys, validate_matrix


def fixtures():
    return [dict(digits=digits, workload=name, mode=mode, repeat=repeat,
                 iterations=4096, cpu_ns_per_op=3.5, checksum=123)
            for digits, name, mode in sorted(matrix_keys()) for repeat in range(3)]


def csv_text(rows):
    stream = io.StringIO()
    writer = csv.DictWriter(stream, fieldnames=list(fixtures()[0]))
    writer.writeheader()
    writer.writerows(rows)
    return stream.getvalue()


class MatrixEvidenceTests(unittest.TestCase):
    def test_valid_matrix(self):
        self.assertEqual(len(matrix_keys()), 48)
        self.assertEqual(len(validate_matrix(csv_text(fixtures()), 4096, 3)), 144)

    def test_missing_sample(self):
        with self.assertRaises(ValueError):
            validate_matrix(csv_text(fixtures()[:-1]), 4096, 3)

    def test_duplicate_sample(self):
        rows = fixtures()
        rows[-1] = dict(rows[0])
        with self.assertRaises(ValueError):
            validate_matrix(csv_text(rows), 4096, 3)

    def test_unexpected_workload(self):
        rows = fixtures()
        rows[0]["workload"] = "unrecognized"
        with self.assertRaises(ValueError):
            validate_matrix(csv_text(rows), 4096, 3)

    def test_bad_measurements(self):
        for value in ("nan", "inf", "-inf", 0, -1):
            with self.subTest(value=value):
                rows = fixtures()
                rows[0]["cpu_ns_per_op"] = value
                with self.assertRaises(ValueError):
                    validate_matrix(csv_text(rows), 4096, 3)

    def test_different_iterations(self):
        with self.assertRaises(ValueError):
            validate_matrix(csv_text(fixtures()), 8192, 3)

    def test_invalid_checksum(self):
        for value in (-1, 1 << 64):
            rows = fixtures()
            rows[0]["checksum"] = value
            with self.assertRaises(ValueError):
                validate_matrix(csv_text(rows), 4096, 3)

    def paired(self):
        return [dict(row, variant=variant, batch=0) for variant in ("base", "head") for row in fixtures()]

    def test_matching_results(self):
        self.assertEqual(len(compare_matrix(self.paired())), 48)

    def test_different_result(self):
        rows = self.paired()
        rows[-1]["checksum"] = 456
        with self.assertRaisesRegex(ValueError, "checksum"):
            compare_matrix(rows)

    def test_unbalanced(self):
        with self.assertRaisesRegex(ValueError, "unbalanced"):
            compare_matrix(self.paired()[:-1])

    def test_missing_workload_pair(self):
        rows = self.paired()
        excluded = rows[0]["workload"]
        with self.assertRaisesRegex(ValueError, "incomplete"):
            compare_matrix([row for row in rows if row["workload"] != excluded])


if __name__ == "__main__":
    unittest.main()
