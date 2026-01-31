# axdigi

AX.25 layer 2 digipeater for networked TNCs.

### What it is

A pure layer-2 AX.25 digipeater software that connects to KISS TNCs via TCP or Unix socket. It handles:

- **Traced aliases** (WIDE2-2, TRACE3-1) — decrements hops, prepends own callsign
- **Untraced aliases** (SP3-1, AROS3-1) — just decrements hops
- **Own callsign digipeating** — marks itself in path
- **Basic packet deduplication** — prevents most duplicate transmissions

### What it isn't

This project is **not**:

- A TNC — just a digipeater, no modulation/demodulation
- APRS-aware — no filtering based on position, packet type, no beaconing, i-gating etc.
- Cross-platform — Linux-only (uses Unix sockets, signal handling)

## Build

```bash
make update # pulls and updates submobules
make clean
make build # build with debug symbols, no optimizations
make release # build with all optimizations, strip symbols
```

## Usage

```bash
# TCP TNC connection
axdigi -h 192.168.0.9 -p 8144 -C SR5DZ -s 0 -u SP,XR,ND -t WIDE,TRACE -U 2 -T 2

# Unix socket TNC
axdigi -x /run/tnc.sock -C SR5DZ -s 0

# Configuration file
axdigi -c axdigi.conf

# Dry run (no packets transmitted)
axdigi -n -C SR5DZ -s 0
```

## Command Line Arguments

| Short      | Long                    | Description                         |
| ---------- | ----------------------- | ----------------------------------- |
| `-c FILE`  | `--config=FILE`         | Configuration file                  |
| `-h ADDR`  | `--host=ADDR`           | TNC TCP address                     |
| `-p PORT`  | `--port=PORT`           | TNC TCP port                        |
| `-x SOCK`  | `--socket=SOCK`         | TNC Unix socket path                |
| `-C CALL`  | `--call=CALL`           | Digipeater callsign                 |
| `-s SSID`  | `--ssid=SSID`           | Digipeater SSID (0-15)              |
| `-t ALIAS` | `--traced=ALIAS`        | Traced aliases (comma-separated)    |
| `-u ALIAS` | `--untraced=ALIAS`      | Untraced aliases (comma-separated)  |
| `-T N`     | `--max-traced-hops=N`   | Max hops for traced aliases         |
| `-U N`     | `--max-untraced-hops=N` | Max hops for untraced aliases       |
| `-v LEVEL` | `--log-level=LEVEL`     | Log level: standard, verbose, debug |
| `-n`       | `--dry-run`             | Don't transmit packets              |

## Configuration File

Optionally, configuration can be read from a file using `-c FILE` or `--config=FILE`.

The file uses simple `key=value` syntax with `#` comments.

### Example

```ini
# axdigi.conf
host=192.168.0.9
port=8144
call=SR5DZ
ssid=0
aliases-traced=WIDE,TRACE
aliases-untraced=SP,XR,ND
max-traced-hops=2
max-untraced-hops=2
log-level=verbose
dry-run=false
```

## Alias behavior

### Traced aliases (e.g., WIDE, TRACE)

- Decrement hop counter on each digipeat
- Prepend own callsign (repeated=true) before the alias
- When hops reach 0, mark alias as repeated (NAME*)

For example:
`WIDE2-2` becomes `SR5DZ*,WIDE2-1` and if somehow digipeated again would become `SR5DZ*,SR5DZ*,WIDE2*`.

### Untraced aliases (e.g., SP, MZ)

- Decrement hop counter on each digipeat
- Do NOT prepend own callsign
- When hops reach 0, mark alias as repeated (NAME*)

For example: `SP3-2` becomes first `SP3-1` and then might become `SP3*` when bounced off another digi.



## Dependencies

- **libtnc**: included as a [git submodule](libs/libtnc/)
  - AX.25, KISS, CRC and networking routines


## Installation

There is a helper Make target `make install` which handles everything from compilation to asking for SU rights and installing files to the relevant directories.

Systemd service `axdigi.service` will be installed as well.
Please review the unit file and adjust for your needs.

## License

GNU General Public License v3.0 - see [LICENSE](LICENSE)

---

**Development notes:** See [.clinerules](.clinerules) for AI-friendly technical documentation.
