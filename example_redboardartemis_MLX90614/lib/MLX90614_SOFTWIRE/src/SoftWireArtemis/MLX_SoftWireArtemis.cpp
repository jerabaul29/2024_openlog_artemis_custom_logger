/**
 * SoftWireArtemis - Version 1.0 / paulvha / February 2019
 *
 * This is a bit-banging I2C implemenation that is taken from the ESP8266.
 * It has been adjusted to work on an ESP32 and support clock-stretching.
 *
 * The hardware I2C on an ESP32 is known for NOT supporting clock stretching.
 *
 * While it is aimed and tested for the SCD30, it should work for other
 * devices on the ESP32 as well.
 *
 * Below the original heading on the ESP8266
 */

/*
  TwoWireArtemis.cpp - TWI/I2C library for Arduino & Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
  Modified December 2014 by Ivan Grokhotkov (ivan@esp8266.com) - esp8266 support
  Modified April 2015 by Hrsto Gochkov (ficeto@ficeto.com) - alternative esp8266 support

 
* December 2024 / paulvha : Special port for MLX90614
* Tested on UNOR4 & Artemis ATP V 1.2.3
*/
#include <defWireArtemis.h>
#if defined(MLX_SOFTWireArtemis)

extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
}

#include "MLX_SoftWireArtemis.h"

// for potenial Debug
#define DPRINTLN(...)  Serial.println(__VA_ARGS__)
#define DPRINT(...)    Serial.print(__VA_ARGS__)


//Some boards don't have these pins available, and hence don't support WireArtemis.
//Check here for compile-time error.
//#if !defined(SDA) || !defined(SCL)
//#error WireArtemis library is not supported on this board
//#endif

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t TwoWireArtemis::rxBuffer[I2C_BUFFER_LENGTH];
uint8_t TwoWireArtemis::rxBufferIndex = 0;
uint8_t TwoWireArtemis::rxBufferLength = 0;
uint8_t TwoWireArtemis::txAddress = 0;
uint8_t TwoWireArtemis::txBuffer[I2C_BUFFER_LENGTH];
uint8_t TwoWireArtemis::txBufferIndex = 0;
uint8_t TwoWireArtemis::txBufferLength = 0;
uint8_t TwoWireArtemis::transmitting = 0;

// in case of ESP32 SDA 21 / SCL 22
static int default_sda_pin = SDA;
static int default_scl_pin = SCL;

// Constructors ////////////////////////////////////////////////////////////////

TwoWireArtemis::TwoWireArtemis(){}
TWI LTWI;

// Public Methods //////////////////////////////////////////////////////////////

void TwoWireArtemis::begin(int sda, int scl){
  default_sda_pin = sda;
  default_scl_pin = scl;
  LTWI.twi_init(sda, scl);
  flush();
}

uint8_t TwoWireArtemis::status(){
    return LTWI.twi_status();
}

void TwoWireArtemis::setClock(uint32_t frequency){
  LTWI.twi_setClock(frequency);
}

void TwoWireArtemis::setClockStretchLimit(uint32_t limit){
  LTWI.twi_setClockStretchLimit(limit);
}

size_t TwoWireArtemis::requestFrom(uint8_t address, size_t size, bool sendStop){
 // if(size > I2C_BUFFER_LENGTH)  size = I2C_BUFFER_LENGTH;
  size_t read = (LTWI.twi_readFrom(address, rxBuffer, size, sendStop) == 0)?size:0;
  rxBufferIndex = 0;
  rxBufferLength = read;
  return read;
}

uint8_t TwoWireArtemis::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop){
  return requestFrom(address, static_cast<size_t>(quantity), static_cast<bool>(sendStop));
}

uint8_t TwoWireArtemis::requestFrom(uint8_t address, uint8_t quantity){
  return requestFrom(address, static_cast<size_t>(quantity), true);
}

uint8_t TwoWireArtemis::requestFrom(int address, int quantity){
  return requestFrom(static_cast<uint8_t>(address), static_cast<size_t>(quantity), true);
}

uint8_t TwoWireArtemis::requestFrom(int address, int quantity, int sendStop){
  return requestFrom(static_cast<uint8_t>(address), static_cast<size_t>(quantity), static_cast<bool>(sendStop));
}

void TwoWireArtemis::beginTransmission(uint8_t address){
  transmitting = 1;
    txBufferIndex = 0;
  txBufferLength = 0;
  txAddress = address;
  flush();
}

void TwoWireArtemis::beginTransmission(int address){
  beginTransmission((uint8_t)address);
}

uint8_t TwoWireArtemis::endTransmission(uint8_t sendStop){
  int8_t ret = LTWI.twi_writeTo(txAddress, txBuffer, txBufferLength, sendStop);
  //txBufferIndex = 0;
  //txBufferLength = 0;
  transmitting = 0;
  return ret;
}

uint8_t TwoWireArtemis::endTransmission(void){
  return endTransmission(true);
}

size_t TwoWireArtemis::write(uint8_t data){
  if(transmitting){
    if(txBufferLength >= I2C_BUFFER_LENGTH){
      setWriteError();
      return 0;
    }
    txBuffer[txBufferIndex] = data;
    ++txBufferIndex;
    txBufferLength = txBufferIndex;
  } else {
    // i2c_slave_transmit(&data, 1);
  }
  return 1;
}

size_t TwoWireArtemis::write(const uint8_t *data, size_t quantity){
  if(transmitting){
    for(size_t i = 0; i < quantity; ++i){
      if(!write(data[i])) return i;
    }
  }else{
    // i2c_slave_transmit(data, quantity);
  }
  return quantity;
}

int TwoWireArtemis::available(void){
  int result = rxBufferLength - rxBufferIndex;

  if (!result) {
    // yielding here will not make more data "available",
    // but it will prevent the system from going into WDT reset
    yield();
  }

  return result;
}

int TwoWireArtemis::read(void){
  int value = -1;
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
    ++rxBufferIndex;
  }
  return value;
}

int TwoWireArtemis::peek(void){
  int value = -1;
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
  }
  return value;
}

void TwoWireArtemis::flush(void){
  twi_recovery();
  rxBufferIndex = 0;
  rxBufferLength = 0;
  txBufferIndex = 0;
  txBufferLength = 0;
}

// Preinstantiate Objects //////////////////////////////////////////////////////

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TWOWireArtemis)
TwoWireArtemis ArtemisWireArtemis;
#endif

#endif // SOFTWireArtemis
