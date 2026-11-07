"""Executable-level contract checks with stable fixtures. No third-party dependencies."""
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BINARY = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT / "build/cansec"
if len(sys.argv) > 1:
    sys.argv.pop(1)


class TestCLI(unittest.TestCase):
    def analyze(self, fixture, *options):
        args = [str(BINARY), "analyze", "--trace", str(ROOT / "fixtures" / fixture),
                "--rules", str(ROOT / "config/rules.csv"), "--signals", str(ROOT / "config/signals.csv"), *options]
        out = subprocess.run(args, text=True, capture_output=True)
        return out, json.loads(out.stdout) if out.stdout.lstrip().startswith("{") else None

    def test_nominal_has_zero_findings(self):
        r, data = self.analyze("nominal.csv")
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual((data["frames"], data["findings_total"], data["parse_errors_total"]), (200, 0, 0))

    def test_detect_counter_jump(self):
        _, data = self.analyze("fault_counter.csv")
        self.assertGreaterEqual(data["by_code"].get("COUNTER_JUMP", 0), 1)

    def test_detect_checksum(self):
        _, data = self.analyze("fault_checksum.csv")
        self.assertEqual(data["by_code"].get("CHECKSUM_MISMATCH"), 1)

    def test_detect_signal_range(self):
        _, data = self.analyze("fault_signal.csv")
        self.assertEqual(data["by_code"].get("SIGNAL_RANGE"), 1)

    def test_detect_unknown_identifier(self):
        _, data = self.analyze("fault_unknown.csv")
        self.assertEqual(data["by_code"].get("UNKNOWN_ID"), 1)

    def test_malformed_returns_code_two(self):
        r, data = self.analyze("fault_malformed.csv")
        self.assertEqual(r.returncode, 2)
        self.assertEqual(data["parse_errors_total"], 1)

    def test_fail_on_finding_returns_three(self):
        r, _ = self.analyze("fault_checksum.csv", "--fail-on-finding")
        self.assertEqual(r.returncode, 3)

    def test_nonfail_mode_reports_findings_but_succeeds(self):
        r, data = self.analyze("fault_checksum.csv")
        self.assertEqual(r.returncode, 0)
        self.assertGreater(data["findings_total"], 0)

    def test_invalid_rules_cause_config_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            rules = Path(tmp) / "bad_rules.csv"
            rules.write_text("0x120,std,0,20000,10000,8,0,-,-,-\n", encoding="utf-8")
            r = subprocess.run([str(BINARY), "analyze", "--trace", str(ROOT / "fixtures/nominal.csv"),
                                "--rules", str(rules)], text=True, capture_output=True)
            self.assertEqual(r.returncode, 2)
            self.assertIn("invalid", r.stderr)

    def test_text_format(self):
        r, data = self.analyze("fault_counter.csv", "--format", "text")
        self.assertIsNone(data)
        self.assertEqual(r.returncode, 0)
        self.assertIn("COUNTER_JUMP", r.stdout)

    def test_max_frames_is_reported_as_parse_error(self):
        r, data = self.analyze("nominal.csv", "--max-frames", "5")
        self.assertEqual(r.returncode, 2)
        self.assertEqual(data["frames"], 5)
        self.assertEqual(data["parse_errors"][0]["code"], "FRAME_LIMIT")

    def test_output_json_file(self):
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / "report.json"
            r, _ = self.analyze("nominal.csv", "--output", str(output))
            self.assertEqual(r.returncode, 0)
            self.assertEqual(json.loads(output.read_text())["frames"], 200)

    def test_convert_candump_then_analyze(self):
        with tempfile.TemporaryDirectory() as tmp:
            log = Path(tmp) / "input.log"
            csv = Path(tmp) / "converted.csv"
            log.write_text("(1700000000.123456) can0 123#AABB\n", encoding="utf-8")
            r = subprocess.run([str(BINARY), "convert", "--input", str(log), "--output", str(csv)], text=True, capture_output=True)
            self.assertEqual(r.returncode, 0, r.stderr)
            self.assertIn("timestamp_us,id,format", csv.read_text())
            self.assertIn("0x123", csv.read_text())

    def test_converter_does_not_create_partial_csv_if_invalid(self):
        with tempfile.TemporaryDirectory() as tmp:
            log = Path(tmp) / "bad.log"
            out = Path(tmp) / "out.csv"
            log.write_text("garbage\n", encoding="utf-8")
            r = subprocess.run([str(BINARY), "convert", "--input", str(log), "--output", str(out)], text=True, capture_output=True)
            self.assertEqual(r.returncode, 2)
            self.assertFalse(out.exists())

    def test_safe_cli_rejects_unknown_subcommand(self):
        r = subprocess.run([str(BINARY), "flash-ecu"], text=True, capture_output=True)
        self.assertEqual(r.returncode, 2)


if __name__ == "__main__":
    unittest.main(verbosity=2)
