#include "gnss_manager.h"

TwoWire ArtemisWire(PORT_I2C_QWIIC_NUMBER);
I2CGPS myI2CGPS;
TinyGPSPlus gps;

void set_time_from_gnss(void)
{
    unsigned long timeout_ms{5 * 60 * 1000};
    unsigned long millis_start = millis();

    // start the GNSS
    SERIAL_USB->println(F("start gnss"));

    if (!myI2CGPS.begin(ArtemisWire, 100000))
    {
        SERIAL_USB->println("Module failed to respond. Please check wiring.");
        delay(100);
        while (1)
            ; // Freeze!
    }
    SERIAL_USB->println(F("gnss started"));

    // myI2CGPS.enableDebugging(*SERIAL_USB);

    while (millis() - millis_start < timeout_ms)
    {
        delay(500);
        SERIAL_USB->print("L");
        wdt.restart();

        char crrt_char;
        while (myI2CGPS.available()) // available() returns the number of new bytes available from the GPS module
        {
            crrt_char = myI2CGPS.read();
            SERIAL_USB->print(crrt_char);
            gps.encode(crrt_char); // Feed the GPS parser
        }

        if (gps.time.isUpdated()) // Check to see if new GPS info is available
        {
            if (gps.time.isValid())
            {
                SERIAL_USB->print(F("Date: "));
                SERIAL_USB->print(gps.date.month());
                SERIAL_USB->print(F("/"));
                SERIAL_USB->print(gps.date.day());
                SERIAL_USB->print(F("/"));
                SERIAL_USB->print(gps.date.year());

                SERIAL_USB->print((" Time: "));
                if (gps.time.hour() < 10)
                    SERIAL_USB->print(F("0"));
                SERIAL_USB->print(gps.time.hour());
                SERIAL_USB->print(F(":"));
                if (gps.time.minute() < 10)
                    SERIAL_USB->print(F("0"));
                SERIAL_USB->print(gps.time.minute());
                SERIAL_USB->print(F(":"));
                if (gps.time.second() < 10)
                    SERIAL_USB->print(F("0"));
                SERIAL_USB->print(gps.time.second());

                SERIAL_USB->println(); // Done printing time

                break;
            }
            else
            {
                SERIAL_USB->println(F("Time not yet valid"));
            }
        }
    }

    // stop the GNSS
    SERIAL_USB->println(F("stop gnss"));
}
