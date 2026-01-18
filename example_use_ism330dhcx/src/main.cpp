/*
   @file    ISM330DHCX_FIFO.ino
   @author  STMicroelectronics
   @brief   Example to use the ISM330DHCX 3D accelerometer and 3D gyroscope FIFO
 *******************************************************************************
   Copyright (c) 2022, STMicroelectronics
   All rights reserved.

   This software component is licensed by ST under BSD 3-Clause license,
   the "License"; You may not use this file except in compliance with the
   License. You may obtain a copy of the License at:
                          opensource.org/licenses/BSD-3-Clause

 *******************************************************************************
*/

// Includes
#include <Arduino.h>
#include <ISM330DHCXSensor.h>
#include "Wire.h"

TwoWire dev_interface = Wire1;

#define SENSOR_ODR 52.0f // In Hertz
#define ACC_FS 2 // In g
#define GYR_FS 2000 // In dps
#define MEASUREMENT_TIME_INTERVAL (1000.0f/SENSOR_ODR) // In ms
#define FIFO_SAMPLE_THRESHOLD 100  // Lower threshold to read before FIFO fills
#define FLASH_BUFF_LEN 32768

unsigned long timestamp_count = 0;
bool acc_available = false;
bool gyr_available = false;
int32_t acc_value[3];
int32_t gyr_value[3];
char buff[FLASH_BUFF_LEN];
uint32_t pos = 0;
ISM330DHCXStatusTypeDef retval;

uint8_t buffer_data[FLASH_BUFF_LEN];

void Read_FIFO_Data(uint16_t samples_to_read, ISM330DHCXSensor& AccGyr);

void setup() {
  Serial.begin(1000000);

  static constexpr int PIN_QWIIC_PWR = 18;
  pinMode(PIN_QWIIC_PWR, OUTPUT);
  digitalWrite(PIN_QWIIC_PWR, HIGH);
  delay(100);

  Wire1.begin();
  delay(100);
  Wire1.setClock(400000);

  ISM330DHCXSensor AccGyr(&Wire1, 0x6A);
  
  // Initialize IMU.

  retval = AccGyr.begin();
  Serial.print("ISM330DHCX initialization: ");
  Serial.println(retval == ISM330DHCX_OK ? "OK" : "FAIL");

  retval = AccGyr.ACC_Enable();
  Serial.print("ACC_Enable: ");
  Serial.println(retval == ISM330DHCX_OK ? "OK" : "FAIL");
  
  retval = AccGyr.GYRO_Enable();
  Serial.print("GYRO_Enable: ");
  Serial.println(retval == ISM330DHCX_OK ? "OK" : "FAIL");
  
  // Configure ODR and FS of the acc and gyro
  retval = AccGyr.ACC_SetOutputDataRate(SENSOR_ODR);
  Serial.print("ACC_SetOutputDataRate: ");
  Serial.println(retval == ISM330DHCX_OK ? "OK" : "FAIL");
  
  retval = AccGyr.ACC_SetFullScale(ACC_FS);
  Serial.print("ACC_SetFullScale: ");
  Serial.println(retval == ISM330DHCX_OK ? "OK" : "FAIL");
  
  retval = AccGyr.GYRO_SetOutputDataRate(SENSOR_ODR);
  Serial.print("GYRO_SetOutputDataRate: ");
  Serial.println(retval == ISM330DHCX_OK ? "OK" : "FAIL");
  
  retval = AccGyr.GYRO_SetFullScale(GYR_FS);
  Serial.print("GYRO_SetFullScale: ");
  Serial.println(retval == ISM330DHCX_OK ? "OK" : "FAIL");
  
  // Configure FIFO BDR for acc and gyro
  retval = AccGyr.FIFO_ACC_Set_BDR(SENSOR_ODR);
  Serial.print("FIFO_ACC_Set_BDR: ");
  Serial.println(retval == ISM330DHCX_OK ? "OK" : "FAIL");
  
  retval = AccGyr.FIFO_GYRO_Set_BDR(SENSOR_ODR);
  Serial.print("FIFO_GYRO_Set_BDR: ");
  Serial.println(retval == ISM330DHCX_OK ? "OK" : "FAIL");
  
  // Set FIFO in Continuous mode
  retval = AccGyr.FIFO_Set_Mode(ISM330DHCX_STREAM_MODE);
  Serial.print("FIFO_Set_Mode: ");
  Serial.println(retval == ISM330DHCX_OK ? "OK" : "FAIL");
  delay(100);
  
  Serial.println("ISM330DHCX FIFO Demo");
  Serial.println("Waiting for FIFO to fill...");


uint32_t loop_count {0};
unsigned long last_read_time = 0;

while(true) {
  uint16_t fifo_samples;

  // Check the number of samples inside FIFO
  retval = AccGyr.FIFO_Get_Num_Samples(&fifo_samples);
  if (retval != ISM330DHCX_OK) {
    Serial.println("Error reading FIFO samples");
  }
  
  // Only print occasionally to avoid flooding
  if (loop_count % 100 == 0) {
    Serial.print("Loop: ");
    Serial.print(loop_count);
    Serial.print(" Samples: ");
    Serial.println(fifo_samples);
  }

  // If we reach the threshold we can empty the FIFO
  if (fifo_samples > FIFO_SAMPLE_THRESHOLD) {
    unsigned long time_since_last = millis() - last_read_time;
    last_read_time = millis();
    
    /*
    Serial.print(">>> FIFO READ - Samples: ");
    Serial.print(fifo_samples);
    Serial.print(", Time since last read: ");
    Serial.print(time_since_last);
    Serial.println(" ms");
    */

    // Empty the FIFO
    Read_FIFO_Data(fifo_samples, AccGyr);
    // Print FIFO data
    Serial.print(buff);
  }

  delay(10);
  loop_count++;
}
}

void loop() {
  // Nothing to do here
}

void Read_FIFO_Data(uint16_t samples_to_read, ISM330DHCXSensor& AccGyr)
{
  uint16_t i;

  for (i = 0; i < samples_to_read; i++) {
    uint8_t tag;
    // Check the FIFO tag
    AccGyr.FIFO_Get_Tag(&tag);
    switch (tag) {
      // If we have a gyro tag, read the gyro data
      case ISM330DHCX_GYRO_NC_TAG: {
          AccGyr.FIFO_GYRO_Get_Axes(gyr_value);
          gyr_available = true;
          break;
        }
      // If we have an acc tag, read the acc data
      case ISM330DHCX_XL_NC_TAG: {
          AccGyr.FIFO_ACC_Get_Axes(acc_value);
          acc_available = true;
          break;
        }
      // We can discard other tags
      default: {
          break;
        }
    }
    // If we have the measurements of both acc and gyro, we can store them with timestamp
    if (acc_available && gyr_available) {
      int num_bytes;
      num_bytes = snprintf(&buff[pos], (FLASH_BUFF_LEN - pos), "%lu %d %d %d %d %d %d\r\n", (unsigned long)((float)timestamp_count * MEASUREMENT_TIME_INTERVAL), (int)acc_value[0], (int)acc_value[1], (int)acc_value[2], (int)gyr_value[0], (int)gyr_value[1], (int)gyr_value[2]);
      pos += num_bytes;
      timestamp_count++;
      acc_available = false;
      gyr_available = false;
    }
  }
  // We can add the termination character to the string, so we are ready to print it on hyper-terminal
  buff[pos] = '\0';
  pos = 0;
}