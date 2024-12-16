#ifndef GNSS_MANAGER_H
#define GNSS_MANAGER_H

#include <SparkFun_I2C_GPS_Arduino_Library.h> //Use Library Manager or download here: https://github.com/sparkfun/SparkFun_I2C_GPS_Arduino_Library
extern I2CGPS myI2CGPS; //Hook object to the library

#include <TinyGPS++.h> //From: https://github.com/mikalhart/TinyGPSPlus
extern TinyGPSPlus gps; //Declare gps object

#include "time_manager.h"

#include "watchdog_manager.h"

#include "print_utils.h"

#include "Wire.h"

void set_time_from_gnss(void);

#endif