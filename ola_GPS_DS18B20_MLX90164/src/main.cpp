#include <Arduino.h>

#include "firmware_configuration.h"
#include "user_configuration.h"
#include "watchdog_manager.h"
#include "time_manager.h"
#include "thermistors_manager.h"
#include "sd_manager.h"
#include "boot_counter.h"
#include "sleep_manager.h"
#include "gnss_manager.h"
#include "mlx90164_manager.h"

void setup()
{
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

  if (USE_SERIAL_PRINT)
  {
    SERIAL_USB->begin(BAUD_RATE_USB);
    delay(100);
  }

  // to set the boot number the first time if you want this to start at 0
  // if doing so, you MUST remember to put if to false being any real deployment!
  if (false){
    boot_counter_instance.set_boot_number(0);    
    SERIAL_USB->println(F("WARNING!! set_boot_number is active; only for setting to 0 or small test"));
  }
  delay(100);
  wdt.restart();

  // increment boot number
  boot_counter_instance.increment_boot_number();
  delay(100);
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
  mlx90164_manager.acquire_n_readings();
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

  pinMode(PIN_PWR_LED, OUTPUT);
  digitalWrite(PIN_PWR_LED, HIGH);
  board_time_manager.set_posix_timestamp(0);
  gnss_manager.get_a_fix();
  wdt.restart();
  pinMode(PIN_PWR_LED, INPUT);

  pinMode(PIN_STAT_LED, OUTPUT);
  pinMode(PIN_PWR_LED, OUTPUT);
  for (int i=0; i<7; i++){
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

  mlx90164_manager.acquire_n_readings();

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
