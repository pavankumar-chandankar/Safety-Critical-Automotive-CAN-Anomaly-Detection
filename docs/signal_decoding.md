# Bit-exact signal decoding and testing

This project implements a deliberately limited **DBC-inspired signal layer**; it does not parse complete DBC files. Signals are loaded from CSV with the same essential coordinates used in CAN databases.

## Intel (`intel`) bit-order convention

At signal start bit `s`, for each bit `i` in `[0, length)`, the bit is read from `payload[(s+i)/8]` at position `(s+i)%8` (LSB numbered zero), accumulated into raw `raw |= bit << i`. Example: bytes `34 12`, start=0, length=16 -> raw `0x1234`.

## Motorola (`motorola`) sawtooth numbering

Start bit identifies the MSB. At each step, read `payload[bit/8]` at bit position `bit%8`, accumulate `raw = (raw << 1) | bit`. Move to `bit-1` unless `bit%8==0`, then jump to `bit+15`. Example bytes `12 34`, start=7, length=16 -> raw `0x1234`. Physical value uses `raw * factor + offset` after sign extension.

The implementation checks the effective footprint against payload length at **every step**, avoiding silent cross-frame reads or shifts ≥64. Signed 64-bit minimum decoding is explicitly tested. Overlapping fields and duplicate names are rejected at catalog load time.

## Regression strategy

- Intel and Motorola extraction at byte boundaries and across byte boundaries.
- Signed negative values and 64-bit edge case.
- Physical scaling, offset and range evaluation.
- Out-of-payload start and length, duplicate or overlapping signals.
- Real-world interoperability is not established; verify against a trusted third-party decoder before claiming DBC compatibility.

Reference explanation of the DBC bit layout: [cantools documentation](https://cantools.readthedocs.io/en/latest/).
