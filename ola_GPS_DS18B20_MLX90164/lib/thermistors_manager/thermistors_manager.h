#ifndef THERMISTORS_MANAGER_H
#define THERMISTORS_MANAGER_H

/*
A class to use thermistors of model DS18B20 as a thermistor string, and log some info
about both temperature reported by these, as well as inclination / motion as reported by
the IMU.

The thermistors are using the 1-wire interface. Data on pin 35, power on pin 4.
*/

#include "Arduino.h"

#include "user_configuration.h"

#include "etl/vector.h"

#include "print_utils.h"

#include "watchdog_manager.h"
#include "time_manager.h"

#include <OneWire.h>

/*
Brainstorming
- need to send the ID of the thermistors, or no way to know anything if one of them dies...
- get all the IDs in advance, and find ways to have LSBs of IDs not overlapping when putting thermistors
- thermistors resolution is 12 bits (though a bit less, but 11 is not enough, need to use 12)

- could send, per thermistor, 3 bytes i.e. 24 bits: 6 bits "lowest significant bits of ID; 12 bits averaged reading; 6 bits "RMS"; " 
- regarding the "RMS": may be enough to send the "max-min" of the binary reading? ...
- regarding the 12 bits averaged reading: could compute on the fly, all in int?
- 6 thermistors would mean: 18 bytes per packet + 1 header byte = 19 bytes per packet in the message
- i.e., could easily send 20 packets at a time in a message if needed (let us say 10 max)
- HOWEVER, need a couple more bytes about the IMU positioning (see under)

- need to report the inclination of the sensor? add 2 bytes in each packet for pitch and roll discretized over their ranges? Averaged over the duration of the measurement? With 2 more bytes (1 byte for each) indicating the max pitch and roll deviation from mean observed during the measurement?
- pitch is -90 degrees to +90 degrees
- roll is -180 degrees to + 180 degrees

// we collect teh IMU data at the start of the class; otherwise, delays due to the communication with the
// DS18B20 slows too much the IMU stuff, and we get NaNs and corrupted data

TESTING:
- need to test at - degrees
- need to test at + degrees
- test the "orientation measurement" stuff
*/

// TODO: move to user config
// TODO: print as part of config
// how many thermistors to use on the thermistor string, at most
constexpr int number_of_thermistors {10};
// duration over which sample thermistor data
constexpr int duration_thermistor_acquisition_ms {6000};

struct ThermistorReading{
    uint64_t id;
    float reading;
};

using Address = byte[8];

class Thermistors_Manager{
    public:

        void start(void);
        void stop(void);

        void get_ordered_thermistors_ids(void);

        bool time_to_measure_thermistors(void) const;

        void request_start_thermistors_conversion(void);

        void collect_thermistors_conversions(void);

        // >0 is the number of ms needed to wait for all conversions to be over
        // == 0 the conversions are ready to query
        unsigned long remaining_conversion_time(void);

        // perform a time acquisition of temperature; i.e., for each thermistor,
        // sample for several seconds and compute mean and RMS values of the temperature
        void perform_time_acquisition(void);

        static constexpr unsigned long duration_conversion_thermistor_ms {1000UL};
        static constexpr int vector_of_readings_length = number_of_thermistors * duration_thermistor_acquisition_ms / duration_conversion_thermistor_ms;

        etl::vector<uint64_t, number_of_thermistors> vector_of_ids;
        etl::vector<ThermistorReading, vector_of_readings_length + 20> vector_of_readings;
        uint64_t posix_time_start {0};

        unsigned long start_last_conversion_ms;

        uint8_t reading_number {0};
};

extern OneWire one_wire_thermistors;
extern Thermistors_Manager board_thermistors_manager;

#endif