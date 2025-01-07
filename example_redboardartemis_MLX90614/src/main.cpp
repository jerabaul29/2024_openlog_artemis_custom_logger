
/****************************************************************************** 
MLX90614_Get_ID.ino
Print ID register values stored in the MLX90614

This example reads from the MLX90614's ID registers, and 
prints out the 64-byte value to the serial monitor.

Hardware Hookup (if you're not using the eval board):
MLX90614 ------------- Arduino
  VDD ------------------ 3.3V
  VSS ------------------ GND
  SDA ------------------ SDA (A4 on older boards)
  SCL ------------------ SCL (A5 on older boards)
  
Jim Lindblom @ SparkFun Electronics
October 23, 2015
https://github.com/sparkfun/SparkFun_MLX90614_Arduino_Library

Development environment specifics:
Arduino 1.6.5
SparkFun IR Thermometer Evaluation Board - MLX90614
******************************************************************************/
/**************************************************************************
 * Special port with software WireArtemis : December 2024 / paulvha 
 *************************************************************************/
// detect which WireArtemis library to use
#include <defWireArtemis.h>

#ifndef MLX_SOFTWireArtemis
#include <WireArtemis.h> // I2C library, required for MLX90614
#else
#warning USING MLX_SOFTWireArtemis
#endif
/*************************************************************************/

#include <SparkFunMLX90614.h>//Click here to get the library: http://librarymanager/All#Qwiic_IR_Thermometer by SparkFun

IRTherm therm; // Create an IRTherm object to interact with throughout

#include <SoftWireArtemis/MLX_SoftWireArtemis.h>
TwoWireArtemis WireArtemis;

#define LED_BUILTIN 5

void setup() 
{
  Serial.begin(1000000); // Initialize Serial to log output
  WireArtemis.begin(); //Join I2C bus
  
  if (therm.begin() == false){ // Initialize the MLX90614
    Serial.println("Qwiic IR thermometer did not acknowledge! Freezing!");
    while(1);
  }
  Serial.println("Qwiic IR thermometer acknowledged.");
  
  if (therm.readID()) // Read from the ID registers
  { // If the read succeeded, print the ID:
    Serial.println("ID: 0x" + 
                   String(therm.getIDH(), HEX) +
                   String(therm.getIDL(), HEX));
  }
  Serial.println(String(therm.readEmissivity()));

  therm.setUnit(TEMP_C); // Set the library's units to Farenheit
  // Alternatively, TEMP_F can be replaced with TEMP_C for Celsius or
  // TEMP_K for Kelvin.

  pinMode(LED_BUILTIN, OUTPUT); // LED pin as output
}

void loop() 
{
  digitalWrite(LED_BUILTIN, HIGH);
    
  // Call therm.read() to read object and ambient temperatures from the sensor.
  if (therm.read()) // On success, read() will return 1, on fail 0.
  {
    // Use the object() and ambient() functions to grab the object and ambient
	// temperatures.
	// They'll be floats, calculated out to the unit you set with setUnit().
    Serial.print("Object: " + String(therm.object(), 2));
    Serial.println("C");
    Serial.print("Ambient: " + String(therm.ambient(), 2));
    Serial.println("C");
    Serial.println();
  }
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
