# CAN trace input contract (version 1)

## Normalized CSV

Exactly **nine** columns, optional literal header `timestamp_us,id,format,bus,fd,brs,rtr,dlc,data`:

| Field | Accepted form | Rejection examples |
|---|---|---|
| `timestamp_us` | Unsigned decimal integer in microseconds | negative, float, overflow, missing |
| `id` | `0x` hex or unsigned decimal; standard ≤0x7FF, extended ≤0x1FFFFFFF | out of range, signed value |
| `format` | `std` or `ext` | guessed formats |
| `bus` | 0..15 | channel 16 |
| `fd` | `0` or `1` | boolean text `true` |
| `brs` | `0` or `1` | BRS=1 on classic CAN |
| `rtr` | `0` or `1` | RTR=1 with FD |
| `dlc` | 0..8 classic; 0..15 FD | 9 on classic |
| `data` | exactly DLC-mapped bytes as two uppercase/lowercase hex digits separated by spaces | `GG`, `A`, mismatch |

For classic CAN remote frames, `data` is empty and `dlc` denotes the requested length. Remote frames never support FD. The field has no quoting or escaping because payloads use no delimiters; the analyzer deliberately does not silently accept full RFC4180 features. Blank lines and `#` comment lines are ignored. Bounded line length is 4 KiB.

## CAN FD DLC mapping

| DLC | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Bytes | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 12 | 16 | 20 | 24 | 32 | 48 | 64 |

Source for contextual verification: [Kvaser CAN FD](https://kvaser.com/about-can/can-fd-2/).

## Linux candump subset

The `convert` subcommand accepts exact forms:

```text
(1700000000.123456) can0 123#AABB
(1700000000.123457) can1 18DAF110##1AABBCCDDEEFF001122334455
(1700000000.123458) can0 123#R8
```

Frame identifier uses exactly three hex digits for standard or eight for extended; this is how the converter preserves the frame type. `##1` sets BRS, `##0` clears BRS. ESI and extended text logging dialects are rejected. It is intentionally a subset: do not assume successful conversion of every real-world candump output.

## Policy format

`config/rules.csv` columns: `id,format,bus,min_period_us,max_period_us,max_bytes,allow_fd,counter_byte,counter_mask,checksum_byte`.

- Zero min/max period disables that boundary.
- `counter_byte` is zero-based and `counter_mask` must be contiguous low bits (e.g. 0x0F); use `-,-` to disable.
- `checksum_byte` is zero-based; use `-` to disable. The algorithm is a demonstration-only SUM8 checksum across all other bytes.
- `max_bytes` is bounded by protocol type and `allow_fd`. Duplicate rules are rejected.

## Signal catalog

`config/signals.csv` columns: `id,format,name,start_bit,length,endian,signed,factor,offset,min,max`.

- `endian` = `intel` or `motorola`; signed flag `0`/`1`.
- Signal footprint, uniqueness, nonoverlap and physical min/max are checked before analysis.
- A valid spec for a 64-byte theoretical payload can still be unavailable on a shorter frame, causing `SIGNAL_UNDECODABLE`.
- The decoder refuses unsigned 64-bit values above INT64_MAX to keep its raw field semantics explicit.

### Porting real data

Before interpreting real captures, document capture source, time origin, bus routing, CAN FD flags, recording tool versions, telemetry ownership/consent, actual manufacturer signal mapping, policy provenance, and a validation test against a trusted decoder. Never paste sensitive real vehicle traces into public issues.
