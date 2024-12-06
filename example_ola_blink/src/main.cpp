#include <Arduino.h>

#include "pins.h"
#include "ports.h"

void setup() {
  pinMode(PIN_PWR_LED, OUTPUT);
}

void loop() {
  // we can just blink
  digitalWrite(PIN_PWR_LED, HIGH);
  delay(500);
  digitalWrite(PIN_PWR_LED, LOW);
  delay(500);
}
