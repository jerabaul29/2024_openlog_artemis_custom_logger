#include <Arduino.h>

#include "firmware_configuration.h"
#include "user_configuration.h"
#include "watchdog_manager.h"
#include "time_manager.h"
#include "thermistors_manager.h"
#include "sd_manager.h"
#include "boot_counter.h"

void setup() {
  // TODO: wrap in wdt class or methods?
  wdt.configure(WDT_1HZ, 32, 32);
  wdt.start();

  // to set the boot number the first time
  // TODO: add to the standard USB stats
  // boot_counter_instance.set_boot_number(0);
  delay(100);
  wdt.restart();

  // increment boot number
  boot_counter_instance.increment_boot_number();
  delay(100);
  wdt.restart();

  if (USE_SERIAL_PRINT){
    SERIAL_USB->begin(BAUD_RATE_USB);
    delay(100);
  }

  // TODO: wrap in wdt class or method
  wdt.restart();

  print_firmware_config();
  wdt.restart();

  print_all_user_configs();
  wdt.restart();

  if (USE_SERIAL_PRINT){
    uint16_t crrt_boot_nbr = boot_counter_instance.get_boot_number();
    PRINTLN_VAR(crrt_boot_nbr);
  }

  pinMode(PIN_PWR_LED, OUTPUT);

  board_time_manager.set_posix_timestamp(1734344186);
  wdt.restart();

  sd_manager_instance.update_filename();
  sd_manager_instance.log_boot();

  board_thermistors_manager.start();
  board_thermistors_manager.perform_time_acquisition();
  board_thermistors_manager.stop();
}

void loop() {
  // we can just blink
  digitalWrite(PIN_PWR_LED, HIGH);
  delay(1000);
  wdt.restart();

  digitalWrite(PIN_PWR_LED, LOW);
  delay(1000);
  wdt.restart();

  board_time_manager.print_status();
}
