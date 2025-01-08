#include "SparkFun_I2C_GPS_Arduino_Library.h"

I2CGPS myI2CGPS;

void setup()
{
  Serial.begin(1000000);
  delay(100);
  Serial.println("GTOP Read Example");

  if (myI2CGPS.begin() == false)
  {
    Serial.println("Module failed to respond. Please check wiring.");
    while (1);
  }
  Serial.println("GPS module found!");
}

void loop()
{
  while (myI2CGPS.available())
  {
    byte incoming = myI2CGPS.read();

    if(incoming == '$') Serial.println();
    Serial.write(incoming);
  }
}