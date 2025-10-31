Copied from the base template 2025-10-31.

## SDfat Speed & Latency Test

This program tests the write latency and throughput of the SDfat library on the OpenLog Artemis board.

### Test Configuration

- **Buffer size**: 512 bytes
- **Number of writes**: 100
- **Measurement**: Microsecond timestamps before and after each write operation

### Features

- Writes 512-byte buffers in quick succession
- Captures microsecond timestamps for each write
- Calculates min, max, and average latency
- Measures overall throughput
- Outputs detailed per-write statistics and summary

### To compile:

`pio run`

Note that `pio` is an alias in the `~/.bashrc`; if it is not loaded, the full path is:

`alias pio="/home/jeanr/.platformio/penv/bin/pio"`

### To upload and monitor:

`pio run -t upload && pio device monitor`
