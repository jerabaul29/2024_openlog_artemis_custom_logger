#include <Arduino.h>

#include "firmware_configuration.h"
#include "user_configuration.h"
#include "watchdog_manager.h"

void setup() {

  // TODO: wrap in wdt class or methods?
  wdt.configure(WDT_1HZ, 32, 32);
  wdt.start();

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

  pinMode(PIN_PWR_LED, OUTPUT);
}

void loop() {
  // we can just blink
  digitalWrite(PIN_PWR_LED, HIGH);
  delay(1000);
  wdt.restart();

  digitalWrite(PIN_PWR_LED, LOW);
  delay(1000);
  wdt.restart();
}
