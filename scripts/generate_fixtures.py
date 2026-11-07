#!/usr/bin/env python3
"""Seeded, deterministic, wholly synthetic classical CAN and CAN FD traces.

Never described as captured vehicle telemetry. No external Python dependencies.
"""
import argparse
import csv
import random
from pathlib import Path

HEADER = ["timestamp_us", "id", "format", "bus", "fd", "brs", "rtr", "dlc", "data"]


def frames(seed=42, cycles=40):
    rng = random.Random(seed)
    for i in range(cycles):
        base = 1000000 + i * 20000
        for j in range(2):
            counter = (2 * i + j) & 15
            rpm_raw = 2800 + rng.randrange(-30, 30)
            data = [counter, rpm_raw & 255, rpm_raw >> 8, 75, 0, 0, 0, 0]
            data[-1] = sum(data[:-1]) & 255
            yield [base + j * 10000, "0x120", "std", 0, 0, 0, 0, 8, data]
        speed = 12000 + rng.randrange(-50, 50)
        yield [base + 3000, "0x220", "std", 0, 0, 0, 0, 8,
               [speed & 255, speed >> 8, 0, 0, 0, 0, 0, 0]]
        # 12-byte CAN FD payload: DLC 9, ID 29-bit, BRS enabled.
        torque = rng.randrange(50, 150)
        yield [base + 6000, "0x18DAF110", "ext", 1, 1, 1, 0, 9,
               [torque, 0] + [0] * 10]
        yield [base + 16000, "0x18DAF110", "ext", 1, 1, 1, 0, 9,
               [torque, 0] + [0] * 10]


def inject(rows, fault):
    rows = [[t, identifier, fmt, bus, fd, brs, rtr, dlc, list(data)]
            for t, identifier, fmt, bus, fd, brs, rtr, dlc, data in rows]
    if fault == "none":
        return rows
    # Stable selected message in a known frame stream.
    index = 13
    if fault == "unknown-id":
        rows[index][1] = "0x777"
    elif fault == "counter-jump":
        target = next(i for i in range(15, len(rows)) if rows[i][1] == "0x120")
        rows[target][8][0] = 15
        rows[target][8][7] = sum(rows[target][8][:-1]) & 255
    elif fault == "checksum":
        target = next(i for i in range(15, len(rows)) if rows[i][1] == "0x120")
        rows[target][8][7] ^= 255
    elif fault == "signal-range":
        target = next(i for i in range(15, len(rows)) if rows[i][1] == "0x220")
        rows[target][8][:2] = [255, 255]
    elif fault == "timing-burst":
        target = next(i for i in range(15, len(rows)) if rows[i][1] == "0x120")
        rows[target][0] = rows[target - 1][0]
    elif fault == "wrong-bus":
        rows[index][3] = 2
    elif fault == "fd-length":
        rows[index][7] = 9
        rows[index][4] = 1
        rows[index][8] = [0] * 12
    elif fault == "malformed":
        rows[index][8] = ["GG"]
    else:
        raise ValueError(f"Unknown fault kind: {fault}")
    return rows


def save(path, rows):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(HEADER)
        for time, identifier, fmt, bus, fd, brs, rtr, dlc, payload in rows:
            writer.writerow([time, identifier, fmt, bus, fd, brs, rtr, dlc,
                             " ".join(f"{x:02X}" if isinstance(x, int) else x for x in payload)])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--cycles", type=int, default=40)
    parser.add_argument("--fault", choices=["none", "unknown-id", "counter-jump", "checksum",
                       "signal-range", "timing-burst", "wrong-bus", "fd-length", "malformed"], default="none")
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    save(args.output, inject(sorted(list(frames(args.seed, args.cycles)), key=lambda row: row[0]), args.fault))


if __name__ == "__main__":
    main()
