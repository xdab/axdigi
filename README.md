# axdigi

AX.25 layer 2 digipeater for networked TNCs.

## TODO

- [x] Implement basic TCP client connecting to TNC (hardcoded IP/port)
  - [x] Use `tcp_client_init()` from libtnc
  - [x] Connect to 192.168.0.9:8144
  - [x] Handle connection errors
  - [x] Main event loop with select()

- [x] Implement KISS frame decoding pipeline
  - [x] Initialize `kiss_decoder_t`
  - [x] Process TCP bytes through decoder
  - [x] Handle complete KISS messages
  - [x] Parse AX.25 packet from KISS data

- [ ] Implement TNC2 output to stdout
  - [ ] Use `tnc2_packet_to_string()` from libtnc
  - [ ] Print each packet on new line
  - [ ] Flush stdout after each packet

- [ ] Create unit test framework (axdigi_test executable)
  - [ ] Create `src/test.c` with main entry point
  - [ ] Implement `assert_true()`, `assert_false()`, `assert_int_eq()` helpers
  - [ ] Track `assertions_passed` and `assertions_failed` as static int counters
  - [ ] Print test results (passed/failed counts)
  - [ ] Return exit code -1 if failures > 0, else 0
  - [ ] Update CMakeLists.txt to create `axdigi_test` executable
  - [ ] Add `make test` target to run tests
  - [ ] Link against libtnc and math library
  
- [ ] Implement basic digipeater logic (own callsign)
  - [ ] Detect own callsign in packet path
  - [ ] Set repeated flag on matching ax25_addr
  - [ ] Re-encode packet for transmission
  - [ ] Send back via TCP to TNC
  - [ ] Test: own callsign in path gets repeated flag set
  - [ ] Test: packet without own callsign is not modified
  - [ ] Test: multiple occurrences of callsign handled correctly

- [ ] Implement traced alias digipeating (WIDE2-2, TRACE3-1)
  - [ ] Parse alias format: NAME-HOPS (e.g., WIDE2-2)
  - [ ] Decrement hops remaining counter
  - [ ] Prepend own callsign (repeated=true) before alias
  - [ ] When hops reach 0, mark alias as repeated (NAME*)
  - [ ] Support multiple traced aliases in path
  - [ ] Test: WIDE2-2 becomes WIDE2-1 after first hop, callsign prepended
  - [ ] Test: WIDE2-1 becomes WIDE2* (repeated), callsign prepended
  - [ ] Test: TRACE3-1 handling and final repeated marking
  - [ ] Test: Multiple traced aliases in same path

- [ ] Implement untraced alias digipeating (SP3-1, AROS3-1)
  - [ ] Parse alias format: NAME-HOPS (e.g., SP3-1)
  - [ ] Decrement hops remaining counter
  - [ ] Mark alias as repeated when hops reach 0
  - [ ] Do NOT prepend callsign before alias
  - [ ] Support multiple untraced aliases in path
  - [ ] Test: SP3-1 becomes SP3* (repeated), NO callsign prepended
  - [ ] Test: AROS2-1 handling and final repeated marking
  - [ ] Test: Multiple untraced aliases in same path

- [ ] Add TNC2 protocol support (alternative to KISS)
  - [ ] Add command-line protocol selection
  - [ ] Implement TNC2 frame parsing
  - [ ] Handle both KISS and TNC2 input formats

- [ ] Create `options.c/h` with unified options struct
  - [ ] Define `options_t` struct
  - [ ] Add default values
  - [ ] Declare parsing functions

- [ ] Implement `options_args` for GNU argp CLI parsing
  - [ ] Define argp program version
  - [ ] Create option definitions
  - [ ] Parse handler function
  - [ ] Populate options_t from CLI

- [ ] Implement `options_file` for conf.c config file parsing
  - [ ] Load config file with `conf_load()`
  - [ ] Map config keys to options
  - [ ] Override defaults from file

- [ ] Define configuration options (TNC address/port, protocol, callsign, verbose)
  - [ ] `--tnc-host` / `tnc_host`
  - [ ] `--tnc-port` / `tnc_port`
  - [ ] `--protocol` / `protocol` (kiss/tnc2)
  - [ ] `--callsign` / `callsign`
  - [ ] `--verbose` / `verbose`
  - [ ] Test: CLI args override default values correctly
  - [ ] Test: Config file values override defaults
  - [ ] Test: CLI args override config file values

- [ ] Define digipeater configuration options (aliases)
  - [ ] `--traced-aliases` / `traced_aliases` (e.g., "WIDE,TRACE,QCARE")
  - [ ] `--untraced-aliases` / `untraced_aliases` (e.g., "SP,AROS,SR")
  - [ ] Parse comma-separated alias lists
  - [ ] Validate alias format (NAME-HOPS)
  - [ ] Test: traced_aliases parsed into array correctly
  - [ ] Test: untraced_aliases parsed into array correctly
  - [ ] Test: empty alias lists handled gracefully

## Overview

This is a pure layer-2 AX.25 digipeater. It connects to a networked TNC via TCP and handles AX.25 packet digipeating.

### Dependencies

- **libtnc**: AX.25 packet handling library (included as submodule)
  - TCP client/server for network connectivity
  - KISS and TNC2 frame encoding/decoding
  - AX.25 packet parsing and construction
  - Configuration file parsing

### Building

```bash
cmake -B build
cmake --build build
```

### Usage

```bash
axdigi --tnc-host 192.168.0.9 --tnc-port 8144 --callsign N0CALL-1
```

Or via configuration file:

```bash
axdigi -f axdigi.conf
```
