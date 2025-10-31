/**
 * @file main.cpp
 * @brief SD Card Speed and Latency Test for OpenLog Artemis
 * 
 * This program tests the write performance of the SD card by writing
 * multiple 512-byte buffers and measuring microsecond timestamps before
 * and after each write operation. Results include latency statistics
 * and throughput measurements.
 */

#include <Arduino.h>
#include "firmware_configuration.h"
#include "sd_card_manager.h"

// Test configuration constants
#define BUFFER_SIZE 512           ///< Size of write buffer (SD card sector size)
#define NUM_WRITES 10000          ///< Number of write operations to perform

// Timing constants
static constexpr uint32_t SERIAL_TIMEOUT_MS = 5000;      ///< Max wait for serial connection
static constexpr uint32_t ERROR_BLINK_DELAY_MS = 100;    ///< Delay for error blink pattern
static constexpr uint32_t SERIAL_PRINT_DELAY_US = 10;    ///< Delay between serial prints

// Buffer for writing
uint8_t buffer[BUFFER_SIZE];

// Timestamp arrays
uint32_t timestamps_before[NUM_WRITES];
uint32_t timestamps_after[NUM_WRITES];

void setup() {
  // Initialize serial
  SERIAL_USB->begin(BAUD_RATE_USB);
  while (!(*SERIAL_USB) && millis() < SERIAL_TIMEOUT_MS);
  
  pinMode(PIN_PWR_LED, OUTPUT);
  pinMode(PIN_STAT_LED, OUTPUT);
  
  SERIAL_USB->println(F("\n=== SDfat Speed & Latency Test ==="));
  SERIAL_USB->println(F("Testing with 512-byte buffers\n"));
  
  // Fill buffer with test pattern
  for (int i = 0; i < BUFFER_SIZE; i++) {
    buffer[i] = i & 0xFF;
  }
  
  // Initialize SD card
  digitalWrite(PIN_STAT_LED, HIGH);
  if (!sd_card_manager.start()) {
    digitalWrite(PIN_STAT_LED, LOW);
    while (1) {
      digitalWrite(PIN_PWR_LED, HIGH);
      delay(ERROR_BLINK_DELAY_MS);
      digitalWrite(PIN_PWR_LED, LOW);
      delay(ERROR_BLINK_DELAY_MS);
    }
  }
  digitalWrite(PIN_STAT_LED, LOW);
  
  // Calculate preallocation size with 10% margin
  uint32_t preallocSize = (uint32_t)NUM_WRITES * BUFFER_SIZE;
  uint32_t margin = preallocSize / 10;
  preallocSize += margin;
  
  // Open and preallocate test file
  if (!sd_card_manager.preallocate_and_open_file("LATENCY.BIN", preallocSize)) {
    while (1) {
      digitalWrite(PIN_PWR_LED, HIGH);
      delay(ERROR_BLINK_DELAY_MS);
      digitalWrite(PIN_PWR_LED, LOW);
      delay(ERROR_BLINK_DELAY_MS);
    }
  }
  
  SERIAL_USB->print(F("Writing "));
  SERIAL_USB->print(NUM_WRITES);
  SERIAL_USB->println(F(" buffers...\n"));
  
  // Perform write test
  digitalWrite(PIN_PWR_LED, HIGH);
  for (int i = 0; i < NUM_WRITES; i++) {
    timestamps_before[i] = micros();
    sd_card_manager.write_buffer(buffer, BUFFER_SIZE);
    timestamps_after[i] = micros();
  }
  
  // Sync and close file
  uint32_t sync_start = micros();
  sd_card_manager.close_and_sync_file();
  uint32_t sync_end = micros();
  
  // Stop SD card manager
  sd_card_manager.stop();
  
  digitalWrite(PIN_PWR_LED, LOW);
  
  // Calculate and display statistics
  SERIAL_USB->println(F("=== Write Latency Results ==="));
  SERIAL_USB->println(F("Write#\tBefore(us)\tAfter(us)\tLatency(us)"));
  
  uint32_t total_latency = 0;
  uint32_t min_latency = 0xFFFFFFFF;
  uint32_t max_latency = 0;
  
  for (int i = 0; i < NUM_WRITES; i++) {
    uint32_t latency = timestamps_after[i] - timestamps_before[i];
    total_latency += latency;
    
    if (latency < min_latency) min_latency = latency;
    if (latency > max_latency) max_latency = latency;
    
    SERIAL_USB->print(i);
    SERIAL_USB->print("\t");
    SERIAL_USB->print(timestamps_before[i]);
    SERIAL_USB->print("\t");
    SERIAL_USB->print(timestamps_after[i]);
    SERIAL_USB->print("\t");
    SERIAL_USB->println(latency);
    delayMicroseconds(SERIAL_PRINT_DELAY_US);
  }
  
  // Summary statistics
  uint32_t avg_latency = total_latency / NUM_WRITES;
  uint32_t total_bytes = BUFFER_SIZE * NUM_WRITES;
  uint32_t total_time = timestamps_after[NUM_WRITES-1] - timestamps_before[0];
  float throughput_kbps = (total_bytes * 1000.0) / total_time;
  
  SERIAL_USB->println(F("\n=== Summary ==="));
  SERIAL_USB->print(F("Total writes: "));
  SERIAL_USB->println(NUM_WRITES);
  SERIAL_USB->print(F("Buffer size: "));
  SERIAL_USB->print(BUFFER_SIZE);
  SERIAL_USB->println(F(" bytes"));
  SERIAL_USB->print(F("Total data: "));
  SERIAL_USB->print(total_bytes);
  SERIAL_USB->println(F(" bytes"));
  SERIAL_USB->print(F("Total time: "));
  SERIAL_USB->print(total_time);
  SERIAL_USB->println(F(" us"));
  SERIAL_USB->print(F("Sync time: "));
  SERIAL_USB->print(sync_end - sync_start);
  SERIAL_USB->println(F(" us"));
  SERIAL_USB->print(F("Average latency: "));
  SERIAL_USB->print(avg_latency);
  SERIAL_USB->println(F(" us"));
  SERIAL_USB->print(F("Min latency: "));
  SERIAL_USB->print(min_latency);
  SERIAL_USB->println(F(" us"));
  SERIAL_USB->print(F("Max latency: "));
  SERIAL_USB->print(max_latency);
  SERIAL_USB->println(F(" us"));
  SERIAL_USB->print(F("Throughput: "));
  SERIAL_USB->print(throughput_kbps);
  SERIAL_USB->println(F(" KB/s"));
  
  SERIAL_USB->println(F("\n=== All Timestamps (us) ==="));
  for (int i = 0; i < NUM_WRITES; i++) {
    SERIAL_USB->print(timestamps_before[i]);
    SERIAL_USB->print(",");
    SERIAL_USB->println(timestamps_after[i]);
    delayMicroseconds(SERIAL_PRINT_DELAY_US);
  }
  
  SERIAL_USB->println(F("\n=== Test Complete ==="));
}

void loop() {
  // Blink to indicate test is done
  digitalWrite(PIN_STAT_LED, HIGH);
  delay(1000);
  digitalWrite(PIN_STAT_LED, LOW);
  delay(1000);
}
