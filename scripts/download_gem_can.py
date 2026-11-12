#!/usr/bin/env python3
"""Retrieve or import the cited GEM-CAN archive without redistributing the recordings."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import tempfile
import urllib.request
import zipfile

ZENODO_RECORD = "https://zenodo.org/records/19161139"
ARCHIVE_URL = ZENODO_RECORD + "/files/GEM_CAN_Dataset_R2.zip?download=1"
PUBLISHED_MD5 = "03f7a86ed05da9ab282f2886bf9b3ca3"
FILES = ("GEM_Normal_Driving_Raw_Labeled.csv", "GEM_Attack_Scenario_Raw_Labeled.csv")
MAX_COMPRESSED_BYTES = 120 * 1024 * 1024
MAX_MEMBER_BYTES = 120 * 1024 * 1024


def extract_archive(source: Path, destination: Path) -> dict:
    if source.stat().st_size > MAX_COMPRESSED_BYTES:
        raise ValueError("archive exceeds size limit")
    actual_md5 = hashlib.md5(source.read_bytes()).hexdigest()  # nosec: checksum matches publisher's md5, not used for security
    if actual_md5 != PUBLISHED_MD5:
        raise ValueError(f"archive checksum differs from Zenodo v3: {actual_md5}")
    destination.mkdir(parents=True, exist_ok=True)
    manifest = {"source": ZENODO_RECORD, "original_archive_md5": actual_md5, "files": {}}
    with zipfile.ZipFile(source) as zf:
        members = [info for info in zf.infolist() if not info.is_dir()]
        for expected in FILES:
            matches = [item for item in members if Path(item.filename).name == expected]
            if len(matches) != 1:
                raise ValueError(f"archive must have exactly one {expected}, found {len(matches)}")
            item = matches[0]
            if item.file_size > MAX_MEMBER_BYTES or item.flag_bits & 0x1:
                raise ValueError("unexpected file size or encrypted archive member")
            target = destination / expected
            with zf.open(item) as infile, target.open("wb") as outfile:
                shutil.copyfileobj(infile, outfile)
            sha = hashlib.sha256(target.read_bytes()).hexdigest()
            manifest["files"][expected] = {"sha256": sha, "bytes": target.stat().st_size}
    manifest_path = destination / "provenance.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return manifest


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--archive", type=Path, help="Use an archive already downloaded from the Zenodo record")
    ap.add_argument("--dest", type=Path, default=Path("data/gem_can"))
    args = ap.parse_args()
    if args.archive:
        archive = args.archive
        if not archive.is_file():
            ap.error(f"archive not found: {archive}")
        manifest = extract_archive(archive, args.dest)
    else:
        with tempfile.TemporaryDirectory(prefix="gemcan_") as tmp:
            archive = Path(tmp) / "GEM_CAN_Dataset_R2.zip"
            req = urllib.request.Request(ARCHIVE_URL, headers={"User-Agent": "CAN-Sentinel-Research/1.0"})
            # Download via bounded streaming; never commit raw recordings.
            with urllib.request.urlopen(req, timeout=45) as response, archive.open("wb") as out:
                total = 0
                while chunk := response.read(1024 * 1024):
                    total += len(chunk)
                    if total > MAX_COMPRESSED_BYTES:
                        raise ValueError("remote archive exceeds size limit")
                    out.write(chunk)
            manifest = extract_archive(archive, args.dest)
    for file, details in manifest["files"].items():
        print(f"{args.dest / file}  sha256={details['sha256']}")
    print(f"Provenance: {args.dest / 'provenance.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
