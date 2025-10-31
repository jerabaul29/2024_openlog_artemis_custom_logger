Copied from the base template 2025-10-31.

## SDfat Speed & Latency Test

This program tests the write latency and throughput of the SDfat library on the OpenLog Artemis board.

Hardware: the OpenLogArtemis, and a SD card I had lying around (SP Superior PRO micro SD HC I |3| (10) 16GB, probably some older Silicon Power SD card). The exact SD card used may have quite a bit to say!

Results of the tests, with pre allocation:

```
=== Summary ===
Total writes: 10000
Buffer size: 512 bytes
Total data: 5120000 bytes
Total time: 79771942 us
Sync time: 118 us
Average latency: 7973 us
Min latency: 7971 us
Max latency: 9196 us
Throughput: 64.18 KB/s
```

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
