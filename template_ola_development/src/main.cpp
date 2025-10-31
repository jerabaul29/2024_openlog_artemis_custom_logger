#include <Arduino.h>

#include "firmware_configuration.h"
#include "user_configuration.h"
#include "watchdog_manager.h"
#include "boot_counter.h"
#include "time_manager.h"
#include "sleep_manager.h"
#include "sd_card_manager.h"
#include "print_utils.h"
#include "kiss_posix_time_utils.hpp"
#include "kiss_posix_time_extras.hpp"

static constexpr uint32_t BLOCK_SIZE = 512;
static constexpr uint32_t NUM_BLOCKS = 100;
static constexpr uint32_t TOTAL_FILE_SIZE = BLOCK_SIZE * NUM_BLOCKS;
static constexpr unsigned long SLEEP_DURATION_SECONDS = 5UL;
static constexpr uint32_t SERIAL_TIMEOUT_MS = 5000;

void setup() {
  // ii) Start serial using firmware configuration definitions
  SERIAL_USB->begin(BAUD_RATE_USB);
  while (!(*SERIAL_USB) && millis() < SERIAL_TIMEOUT_MS);
  
  SERIAL_USB->println(F("\n=== OpenLog Artemis Utilities Demo ==="));
  SERIAL_USB->print(F("Serial initialized at "));
  SERIAL_USB->print(BAUD_RATE_USB);
  SERIAL_USB->println(F(" baud"));

  // i) Start watchdog
  SERIAL_USB->println(F("\n=== Starting Watchdog ==="));
  wdt.configure(WDT_1HZ, 32, 32);
  wdt.start();
  SERIAL_USB->println(F("Watchdog started with 32 second timeout"));
  wdt.restart();

  // iii) Print firmware information and boot count
  SERIAL_USB->println(F("\n=== Firmware Information ==="));
  print_firmware_config();
  
  uint16_t boot_number = boot_counter_instance.get_boot_number();
  boot_counter_instance.increment_boot_number();
  SERIAL_USB->print(F("Boot count: "));
  SERIAL_USB->println(boot_number);
  wdt.restart();

  // iv) Set up the clock using dummy time 2025-10-31T12:01:02Z
  SERIAL_USB->println(F("\n=== Setting Up Clock ==="));
  board_time_manager.setup_RTC();
  
  kiss_time_t dummy_time = posix_timestamp_from_YMDHMS(2025, 10, 31, 12, 1, 2);
  board_time_manager.set_posix_timestamp(dummy_time);
  
  SERIAL_USB->println(F("Clock configured with time: 2025-10-31T12:01:02Z"));
  board_time_manager.print_status();
  wdt.restart();

  // v) Sleep for a few seconds
  SERIAL_USB->println(F("\n=== Sleeping ==="));
  SERIAL_USB->print(F("Sleeping for "));
  SERIAL_USB->print(SLEEP_DURATION_SECONDS);
  SERIAL_USB->println(F(" seconds..."));
  delay(100);
  
  sleep_for_seconds(SLEEP_DURATION_SECONDS);
  
  SERIAL_USB->println(F("Woke up from sleep"));
  board_time_manager.print_status();
  wdt.restart();

  // vi) Write 100 blocks of 512 bytes to a file and measure performance
  SERIAL_USB->println(F("\n=== SD Card Write Test ==="));
  
  if (!sd_card_manager.start()) {
    SERIAL_USB->println(F("ERROR: Failed to initialize SD card"));
    return;
  }
  SERIAL_USB->println(F("SD card initialized successfully"));
  wdt.restart();

  const char* test_filename = "test.bin";
  if (!sd_card_manager.preallocate_and_open_file(test_filename, TOTAL_FILE_SIZE)) {
    SERIAL_USB->println(F("ERROR: Failed to open and preallocate file"));
    sd_card_manager.stop();
    return;
  }
  SERIAL_USB->print(F("File opened and preallocated: "));
  SERIAL_USB->print(test_filename);
  SERIAL_USB->print(F(" ("));
  SERIAL_USB->print(TOTAL_FILE_SIZE);
  SERIAL_USB->println(F(" bytes)"));
  wdt.restart();

  uint8_t write_buffer[BLOCK_SIZE];
  for (uint32_t i = 0; i < BLOCK_SIZE; i++) {
    write_buffer[i] = i % 256;
  }

  unsigned long start_time = millis();
  unsigned long min_latency = 999999UL;
  unsigned long max_latency = 0UL;
  unsigned long total_latency = 0UL;

  SERIAL_USB->print(F("Writing "));
  SERIAL_USB->print(NUM_BLOCKS);
  SERIAL_USB->print(F(" blocks of "));
  SERIAL_USB->print(BLOCK_SIZE);
  SERIAL_USB->println(F(" bytes..."));

  for (uint32_t block = 0; block < NUM_BLOCKS; block++) {
    unsigned long block_start = millis();
    
    if (!sd_card_manager.write_buffer(write_buffer, BLOCK_SIZE)) {
      SERIAL_USB->print(F("ERROR: Write failed at block "));
      SERIAL_USB->println(block);
      break;
    }
    
    unsigned long block_latency = millis() - block_start;
    min_latency = min(min_latency, block_latency);
    max_latency = max(max_latency, block_latency);
    total_latency += block_latency;

    if ((block + 1) % 25 == 0) {
      SERIAL_USB->print(F("Progress: "));
      SERIAL_USB->print(block + 1);
      SERIAL_USB->print(F("/"));
      SERIAL_USB->println(NUM_BLOCKS);
      wdt.restart();
    }
  }

  unsigned long total_time = millis() - start_time;

  sd_card_manager.close_and_sync_file();
  SERIAL_USB->println(F("File closed and synced"));
  wdt.restart();

  SERIAL_USB->println(F("\n=== Write Performance Statistics ==="));
  SERIAL_USB->print(F("Total blocks written: "));
  SERIAL_USB->println(NUM_BLOCKS);
  SERIAL_USB->print(F("Total bytes written: "));
  SERIAL_USB->println(TOTAL_FILE_SIZE);
  SERIAL_USB->print(F("Total time: "));
  SERIAL_USB->print(total_time);
  SERIAL_USB->println(F(" ms"));
  
  SERIAL_USB->print(F("Average latency per block: "));
  SERIAL_USB->print(total_latency / NUM_BLOCKS);
  SERIAL_USB->println(F(" ms"));
  
  SERIAL_USB->print(F("Min latency: "));
  SERIAL_USB->print(min_latency);
  SERIAL_USB->println(F(" ms"));
  
  SERIAL_USB->print(F("Max latency: "));
  SERIAL_USB->print(max_latency);
  SERIAL_USB->println(F(" ms"));
  
  unsigned long throughput_kbps = (TOTAL_FILE_SIZE * 1000UL) / total_time / 1024UL;
  SERIAL_USB->print(F("Throughput: "));
  SERIAL_USB->print(throughput_kbps);
  SERIAL_USB->println(F(" KB/s"));

  sd_card_manager.stop();
  SERIAL_USB->println(F("SD card stopped"));
  wdt.restart();

  SERIAL_USB->println(F("\n=== Setup Complete ==="));
  SERIAL_USB->println(F("All utilities demonstrated successfully!"));
}

void loop() {
  wdt.restart();
  digitalWrite(PIN_STAT_LED, HIGH);
  delay(1000);
  wdt.restart();
  digitalWrite(PIN_STAT_LED, LOW);
  delay(1000);
}
