"""Exercise measurement decisions that could silently invalidate a hardware run."""

import json
from pathlib import Path
import tempfile
import unittest

import zen5_bench as bench


class TopologyTests(unittest.TestCase):
    def make_sysfs(self, root):
        # Deliberately noncontiguous IDs, unusual SMT partners and index7 L3.
        (root / "online").write_text("2,5,9,11,17,21,25,30\n")
        siblings = {2: [2, 25], 5: [5, 17], 9: [9, 30], 11: [11, 21],
                    17: [5, 17], 21: [11, 21], 25: [2, 25], 30: [9, 30]}
        for cpu, pair in siblings.items():
            directory = root / f"cpu{cpu}"
            topology = directory / "topology"
            topology.mkdir(parents=True)
            for key, value in {"physical_package_id": "0", "die_id": "0", "core_id": str(pair[0]),
                               "thread_siblings_list": ",".join(map(str, pair))}.items():
                (topology / key).write_text(value)
            for index, level, size, shared in ((0, 1, "32K", pair),
                    (7, 3, "96M" if cpu in (2, 5, 17, 25) else "32M",
                     [2, 5, 17, 25] if cpu in (2, 5, 17, 25) else [9, 11, 21, 30])):
                cache = directory / "cache" / f"index{index}"
                cache.mkdir(parents=True)
                for key, value in {"level": str(level), "size": size, "id": str(shared[0]),
                                   "type": "Unified", "shared_cpu_list": ",".join(map(str, shared))}.items():
                    (cache / key).write_text(value)

    def test_cache_discovery_and_single_physical_core(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.make_sysfs(root)
            info = bench.topology(root, {2, 5, 9, 11, 17, 21, 25, 30})
            self.assertEqual(bench.select_cpu(info, "vcache"), 5)
            self.assertEqual(bench.select_cpu(info, "frequency"), 11)
            self.assertIsNone(bench.select_cpu(info, "all"))
            self.assertEqual(info["cpus"][0]["caches"][1]["index"], "index7")

    def test_allowed_cpus_and_smt_are_not_assumed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self.make_sysfs(root)
            info = bench.topology(root, {17, 21, 25, 30})
            self.assertEqual(bench.select_cpu(info, "vcache"), 25)
            self.assertEqual(bench.select_cpu(info, "frequency"), 30)
            restricted = bench.topology(root, {17, 25})
            self.assertEqual(bench.select_cpu(restricted, "vcache"), 25)
            with self.assertRaises(ValueError):
                bench.select_cpu(restricted, "frequency")
            with self.assertRaises(ValueError):
                bench.topology(root, {99})

    def test_cpu_and_cache_parsing(self):
        self.assertEqual(bench.cpu_list("0-2,7,10-11"), {0, 1, 2, 7, 10, 11})
        self.assertEqual(bench.cache_bytes("98304K"), 96 * 1024 * 1024)
        for value in ("", "2-0", "1;2", "1,", "-1"):
            with self.assertRaises(ValueError):
                bench.cpu_list(value)


class InputAndResultTests(unittest.TestCase):
    def test_validation_and_strict_fp(self):
        self.assertEqual(bench.settings({})["repetitions"], 5)
        for field, values in {
            "ZEN5_REPETITIONS": ("0", "31", "1;true", "1.5"),
            "ZEN5_MIN_TIME": ("0s", "60.1s", "2", "NaNs"),
            "ZEN5_BASELINE_SHA": ("main", "refs/pull/1/head", "0" * 39),
            "ZEN5_FLAGS": ("-O3;true", "$(command)", "-O3\n`command`", "@response"),
            "ZEN5_FILTER": ("x\ny",), "ZEN5_CCD": ("1-7",),
        }.items():
            for value in values:
                with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                    bench.settings({field: value})
        config = bench.settings({"ZEN5_FLAGS": "-O3 -march=znver5 -flto -ffast-math"})
        self.assertTrue(config["flags"].endswith("-ffp-contract=off -fno-fast-math"))

    def test_empty_failed_and_nonfinite_results_are_not_measurements(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "result.json"
            good = {"name": "case", "cpu_time": 10, "real_time": 11, "iterations": 100, "time_unit": "ns"}
            for rows in ([], [{**good, "error_occurred": True}], [{**good, "iterations": 0}],
                         [{**good, "cpu_time": float("nan")}], [{**good, "run_type": "aggregate"}]):
                path.write_text(json.dumps({"benchmarks": rows}))
                with self.assertRaises(ValueError):
                    bench.benchmark_rows(path)
            path.write_text(json.dumps({"benchmarks": [good]}))
            self.assertEqual(bench.benchmark_rows(path), [good])

    def test_refused_prefix_denominator_and_paired_differences(self):
        with tempfile.TemporaryDirectory() as directory:
            results = []
            for pair, baseline, candidate in ((0, 100, 80), (1, 120, 90)):
                for revision, cpu in (("baseline", baseline), ("candidate", candidate)):
                    path = Path(directory) / f"{revision}-{pair}.json"
                    path.write_text(json.dumps({"benchmarks": [{"name": "refusal", "cpu_time": cpu,
                        "real_time": cpu * 2, "iterations": 10, "time_unit": "ns",
                        "completed_rows": 2, "input_rows": 125000}]}))
                    results.append((revision, pair, path))
            config = bench.settings({"ZEN5_BASELINE_SHA": "a" * 40})
            report = bench.summary(results, config, {"candidate_sha": "b" * 40, "selected_cpu": 5,
                                                    "run_id": "1", "run_attempt": "1"})
            self.assertIn("| candidate | refusal | ns/completed row | 42.5000 |", report)
            self.assertIn("| refusal | -12.5000 | -15.0000–-10.0000 |", report)


if __name__ == "__main__":
    unittest.main()
