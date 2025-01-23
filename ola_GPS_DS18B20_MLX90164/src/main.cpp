#include <Arduino.h>

#include "firmware_configuration.h"
#include "user_configuration.h"
#include "watchdog_manager.h"
#include "time_manager.h"
#include "thermistors_manager.h"
#include "sd_manager.h"
#include "boot_counter.h"
//#include "gnss_manager.h"
#include "sleep_manager.h"
#include "gnss_manager.h"


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



void setup()
{
  Serial.begin(1000000); // Initialize Serial to log output

  pinMode(PIN_QWIIC_PWR, OUTPUT);
  digitalWrite(PIN_QWIIC_PWR, HIGH);
  delay(500);
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


  while (1){}

  // TODO: wrap in wdt class or methods?
  wdt.configure(WDT_1HZ, 32, 32);
  wdt.start();

  pinMode(PIN_PWR_LED, OUTPUT);
  for (int i=0; i<6; i++){
    digitalWrite(PIN_PWR_LED, HIGH);
    delay(200);
    digitalWrite(PIN_PWR_LED, LOW);
    delay(200);
  }
  pinMode(PIN_PWR_LED, INPUT);

  // to set the boot number the first time
  // TODO: add to the standard USB stats
  // boot_counter_instance.set_boot_number(0);
  delay(100);
  wdt.restart();

  // increment boot number
  boot_counter_instance.increment_boot_number();
  delay(100);
  wdt.restart();

  if (USE_SERIAL_PRINT)
  {
    SERIAL_USB->begin(BAUD_RATE_USB);
    delay(100);
  }

  // TODO: wrap in wdt class or method
  wdt.restart();

  print_firmware_config();
  wdt.restart();

  print_all_user_configs();
  wdt.restart();

  analogReadResolution(14);
  delay(100);
  int read_PIN_PWR_O_3 = analogRead(PIN_PWR_O_3);
  PRINTLN_VAR(read_PIN_PWR_O_3);
  float read_input_voltage = ((float) read_PIN_PWR_O_3) * 3.0 * 1.8 / 16348. * 1.58;
  PRINTLN_VAR(read_input_voltage);

  pinMode(PIN_QWIIC_PWR, OUTPUT);
  digitalWrite(PIN_QWIIC_PWR, LOW);

  pinMode(PIN_ICM_PWR, OUTPUT);
  digitalWrite(PIN_ICM_PWR, LOW);

  uint16_t crrt_boot_nbr = boot_counter_instance.get_boot_number();
  PRINTLN_VAR(crrt_boot_nbr);

  pinMode(PIN_STAT_LED, OUTPUT);
  pinMode(PIN_PWR_LED, OUTPUT);
  for (int i=0; i<2; i++){
    digitalWrite(PIN_STAT_LED, HIGH);
    digitalWrite(PIN_PWR_LED, HIGH);
    delay(200);
    digitalWrite(PIN_STAT_LED, LOW);
    digitalWrite(PIN_PWR_LED, LOW);
    delay(200);
  }
  pinMode(PIN_STAT_LED, INPUT);
  pinMode(PIN_PWR_LED, INPUT);

  pinMode(PIN_PWR_LED, OUTPUT);
  digitalWrite(PIN_PWR_LED, HIGH);
  sd_manager_instance.update_filename();
  sd_manager_instance.log_boot();
  pinMode(PIN_PWR_LED, INPUT);

  pinMode(PIN_STAT_LED, OUTPUT);
  pinMode(PIN_PWR_LED, OUTPUT);
  for (int i=0; i<3; i++){
    digitalWrite(PIN_STAT_LED, HIGH);
    digitalWrite(PIN_PWR_LED, HIGH);
    delay(200);
    digitalWrite(PIN_STAT_LED, LOW);
    digitalWrite(PIN_PWR_LED, LOW);
    delay(200);
  }
  pinMode(PIN_STAT_LED, INPUT);
  pinMode(PIN_PWR_LED, INPUT);

  pinMode(PIN_PWR_LED, OUTPUT);
  digitalWrite(PIN_PWR_LED, HIGH);
  board_thermistors_manager.start();
  board_thermistors_manager.perform_time_acquisition();
  board_thermistors_manager.stop();
  pinMode(PIN_PWR_LED, INPUT);

  pinMode(PIN_STAT_LED, OUTPUT);
  pinMode(PIN_PWR_LED, OUTPUT);
  for (int i=0; i<4; i++){
    digitalWrite(PIN_STAT_LED, HIGH);
    digitalWrite(PIN_PWR_LED, HIGH);
    delay(200);
    digitalWrite(PIN_STAT_LED, LOW);
    digitalWrite(PIN_PWR_LED, LOW);
    delay(200);
  }
  pinMode(PIN_STAT_LED, INPUT);
  pinMode(PIN_PWR_LED, INPUT);

  pinMode(PIN_PWR_LED, OUTPUT);
  digitalWrite(PIN_PWR_LED, HIGH);
  board_time_manager.set_posix_timestamp(0);
  gnss_manager.get_a_fix();
  wdt.restart();
  pinMode(PIN_PWR_LED, INPUT);

  pinMode(PIN_STAT_LED, OUTPUT);
  pinMode(PIN_PWR_LED, OUTPUT);
  for (int i=0; i<6; i++){
    digitalWrite(PIN_STAT_LED, HIGH);
    digitalWrite(PIN_PWR_LED, HIGH);
    delay(200);
    digitalWrite(PIN_STAT_LED, LOW);
    digitalWrite(PIN_PWR_LED, LOW);
    delay(200);
  }
  pinMode(PIN_STAT_LED, INPUT);
  pinMode(PIN_PWR_LED, INPUT);

  sleep_for_seconds(20);
}

void loop()
{
  pinMode(PIN_STAT_LED, OUTPUT);
  for (int i=0; i<3; i++){
    digitalWrite(PIN_STAT_LED, HIGH);
    delay(200);
    digitalWrite(PIN_STAT_LED, LOW);
    delay(200);
  }
  pinMode(PIN_STAT_LED, INPUT);

  gnss_manager.get_a_fix();

  board_thermistors_manager.start();
  board_thermistors_manager.perform_time_acquisition();
  board_thermistors_manager.stop();

  sd_manager_instance.update_filename();
  sd_manager_instance.log_data();

    pinMode(PIN_STAT_LED, OUTPUT);
  for (int i=0; i<4; i++){
    digitalWrite(PIN_STAT_LED, HIGH);
    delay(200);
    digitalWrite(PIN_STAT_LED, LOW);
    delay(200);
  }
  pinMode(PIN_STAT_LED, INPUT);

  sleep_for_seconds(15 * 60);
}
