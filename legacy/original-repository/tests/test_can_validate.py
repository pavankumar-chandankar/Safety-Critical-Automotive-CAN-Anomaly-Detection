"""Synthetic seeded regression tests for the offline classic-CAN validator."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class CANValidationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.exe = Path(cls.tmp.name) / "can_validate"
        compiler = shutil.which("g++") or shutil.which("clang++")
        if compiler is None:
            raise unittest.SkipTest("C++17 compiler not available")
        subprocess.run([
            compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
            str(ROOT / "cpp/can_validate.cpp"), "-o", str(cls.exe)
        ], check=True)

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, "tmp"):
            cls.tmp.cleanup()

    def invoke(self, text):
        file = Path(self.tmp.name) / "frames.csv"
        file.write_text(text, encoding="utf-8")
        return subprocess.run([str(self.exe), str(file)], text=True,
                              capture_output=True)

    def test_valid_standard_frames(self):
        proc = self.invoke("100,0x123,2,AA BB\n101,7FF,0,\n")
        self.assertEqual(proc.returncode, 0, proc.stderr)
        self.assertIn("valid=2 rejected=0", proc.stdout)

    def test_wrong_dlc(self):
        proc = self.invoke("100,0x123,3,AA BB\n")
        self.assertEqual(proc.returncode, 1)
        self.assertIn("rejected=1", proc.stdout)

    def test_out_of_range_identifier(self):
        self.assertEqual(self.invoke("100,0x800,1,AA\n").returncode, 1)

    def test_out_of_range_byte(self):
        self.assertEqual(self.invoke("100,0x123,1,1FF\n").returncode, 1)

    def test_malformed_timestamp(self):
        self.assertEqual(self.invoke("1oops,0x123,1,AA\n").returncode, 1)


if __name__ == "__main__":
    unittest.main()
