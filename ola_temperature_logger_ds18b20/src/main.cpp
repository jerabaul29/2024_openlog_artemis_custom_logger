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

  uint16_t crrt_boot_nbr = boot_counter_instance.get_boot_number();
  PRINTLN_VAR(crrt_boot_nbr);

  // TODO: use GPS instead!!
  board_time_manager.set_posix_timestamp(0);
  wdt.restart();
  // seems like this GNSS does not work...
  // set_time_from_gnss();

  sd_manager_instance.update_filename();
  sd_manager_instance.log_boot();

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
