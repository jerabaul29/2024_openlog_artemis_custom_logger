#include <Arduino.h>
#include "firmware_configuration.h"
#include "sd_card_manager.h"

// Test configuration
#define BUFFER_SIZE 512
#define NUM_WRITES 10000

// Buffer for writing
uint8_t buffer[BUFFER_SIZE];

// Timestamp arrays
uint32_t timestamps_before[NUM_WRITES];
uint32_t timestamps_after[NUM_WRITES];

void setup() {
  // Initialize serial
  Serial.begin(1000000);
  while (!Serial && millis() < 5000);
  
  pinMode(PIN_PWR_LED, OUTPUT);
  pinMode(PIN_STAT_LED, OUTPUT);
  
  Serial.println("\n=== SDfat Speed & Latency Test ===");
  Serial.println("Testing with 512-byte buffers\n");
  
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
      delay(100);
      digitalWrite(PIN_PWR_LED, LOW);
      delay(100);
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
      delay(100);
      digitalWrite(PIN_PWR_LED, LOW);
      delay(100);
    }
  }
  
  Serial.print("Writing ");
  Serial.print(NUM_WRITES);
  Serial.println(" buffers...\n");
  
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
  Serial.println("=== Write Latency Results ===");
  Serial.println("Write#\tBefore(us)\tAfter(us)\tLatency(us)");
  
  uint32_t total_latency = 0;
  uint32_t min_latency = 0xFFFFFFFF;
  uint32_t max_latency = 0;
  
  for (int i = 0; i < NUM_WRITES; i++) {
    uint32_t latency = timestamps_after[i] - timestamps_before[i];
    total_latency += latency;
    
    if (latency < min_latency) min_latency = latency;
    if (latency > max_latency) max_latency = latency;
    
    Serial.print(i);
    Serial.print("\t");
    Serial.print(timestamps_before[i]);
    Serial.print("\t");
    Serial.print(timestamps_after[i]);
    Serial.print("\t");
    Serial.println(latency);
    delayMicroseconds(10);
  }
  
  // Summary statistics
  uint32_t avg_latency = total_latency / NUM_WRITES;
  uint32_t total_bytes = BUFFER_SIZE * NUM_WRITES;
  uint32_t total_time = timestamps_after[NUM_WRITES-1] - timestamps_before[0];
  float throughput_kbps = (total_bytes * 1000.0) / total_time;
  
  Serial.println("\n=== Summary ===");
  Serial.print("Total writes: ");
  Serial.println(NUM_WRITES);
  Serial.print("Buffer size: ");
  Serial.print(BUFFER_SIZE);
  Serial.println(" bytes");
  Serial.print("Total data: ");
  Serial.print(total_bytes);
  Serial.println(" bytes");
  Serial.print("Total time: ");
  Serial.print(total_time);
  Serial.println(" us");
  Serial.print("Sync time: ");
  Serial.print(sync_end - sync_start);
  Serial.println(" us");
  Serial.print("Average latency: ");
  Serial.print(avg_latency);
  Serial.println(" us");
  Serial.print("Min latency: ");
  Serial.print(min_latency);
  Serial.println(" us");
  Serial.print("Max latency: ");
  Serial.print(max_latency);
  Serial.println(" us");
  Serial.print("Throughput: ");
  Serial.print(throughput_kbps);
  Serial.println(" KB/s");
  
  Serial.println("\n=== All Timestamps (us) ===");
  for (int i = 0; i < NUM_WRITES; i++) {
    Serial.print(timestamps_before[i]);
    Serial.print(",");
    Serial.println(timestamps_after[i]);
    delayMicroseconds(10);
  }
  
  Serial.println("\n=== Test Complete ===");
}

void loop() {
  // Blink to indicate test is done
  digitalWrite(PIN_STAT_LED, HIGH);
  delay(1000);
  digitalWrite(PIN_STAT_LED, LOW);
  delay(1000);
}
