#include "thermistors_manager.h"

//--------------------------------------------------------------------------------
// helpers

void address_to_uint64_t(Address &addr_in, uint64_t &uint64_result)
{
    uint64_result = 0;
    for (int crrt_byte_index = 0; crrt_byte_index < 8; crrt_byte_index++)
    {
        uint64_result += static_cast<uint64_t>(addr_in[crrt_byte_index])
                         << (8 * (7 - crrt_byte_index));
    }
}

void uint64_t_to_address(uint64_t const &uint64_in, Address &addr_out)
{
    for (int crrt_byte_index = 0; crrt_byte_index < 8; crrt_byte_index++)
    {
        addr_out[crrt_byte_index] =
            static_cast<byte>(uint64_in >> (8 * (7 - crrt_byte_index)));
    }
}

void print_address(Address const &addr)
{
    for (int i = 0; i < 8; i++)
    {
        SERIAL_USB->print(addr[i], HEX);
        SERIAL_USB->write(' ');
    }
}

//--------------------------------------------------------------------------------
// class implementation

void Thermistors_Manager::start(void)
{
    SERIAL_USB->println(F("start thermistors"));
    // give power to the thermistors
    pinMode(PIN_DS18B20_PWR, OUTPUT);
    digitalWrite(PIN_DS18B20_PWR, HIGH);
    delay(500);

    // start time
    posix_time_start = board_time_manager.get_posix_timestamp();

    // get the list of active thermistors
    get_ordered_thermistors_ids();

    // ask for one conversion to start
    request_start_thermistors_conversion();

    vector_of_readings.clear();
}

void Thermistors_Manager::stop(void)
{
    SERIAL_USB->println(F("stop thermistors"));
    pinMode(PIN_DS18B20_PWR, INPUT);
    pinMode(PIN_DS18B20_DAT, INPUT);
    delay(100);
}

void Thermistors_Manager::get_ordered_thermistors_ids(void)
{
    SERIAL_USB->println(F("get list of ordered thermistors IDs"));

    // clear
    SERIAL_USB->println(F("clear list of thermistors IDs"));
    vector_of_ids.clear();

    // get all IDs
    Address crrt_addr;
    uint64_t crrt_id;

    while (true)
    {

        wdt.restart();

        if (!one_wire_thermistors.search(crrt_addr))
        {
            SERIAL_USB->println("No more addresses.");
            one_wire_thermistors.reset_search();
            delay(250);

            break;
        }

        SERIAL_USB->println(F("found new address..."));
        SERIAL_USB->print("ROM =");
        print_address(crrt_addr);
        SERIAL_USB->println();

        address_to_uint64_t(crrt_addr, crrt_id);
        SERIAL_USB->print(F("ID = "));
        print_uint64(crrt_id);
        SERIAL_USB->println();

        if (OneWire::crc8(crrt_addr, 7) != crrt_addr[7])
        {
            SERIAL_USB->println("WARNING: CRC is not valid!");
            continue;
        }
        else
        {
            SERIAL_USB->println(F("CRC is valid"));
        }

        // the first ROM byte indicates which chip
        switch (crrt_addr[0])
        {
        case 0x10:
            SERIAL_USB->println("  WARNING: Chip = DS18S20"); // or old DS1820
            break;
        case 0x28:
            SERIAL_USB->println("  CORRECT: Chip = DS18B20");
            vector_of_ids.push_back(crrt_id);
            pinMode(PIN_STAT_LED, OUTPUT);
            pinMode(PIN_PWR_LED, OUTPUT);
            digitalWrite(PIN_STAT_LED, HIGH);
            digitalWrite(PIN_PWR_LED, HIGH);
            delay(100);
            digitalWrite(PIN_STAT_LED, LOW);
            digitalWrite(PIN_PWR_LED, LOW);
            delay(300);
            pinMode(PIN_STAT_LED, INPUT);
            pinMode(PIN_PWR_LED, INPUT);
            break;
        case 0x22:
            SERIAL_USB->println("  WARNING: Chip = DS1822");
            break;
        default:
            SERIAL_USB->println("EROR: Device is not a DS18x20 family device.");
            continue;
        }
    }

    return;
}

void Thermistors_Manager::request_start_thermistors_conversion(void)
{
    wdt.restart();

    start_last_conversion_ms = millis();

    Address crrt_address;

    // ask each sensor to start new measurement
    SERIAL_USB->println(F("ask to start conversion..."));
    for (auto &crrt_id : vector_of_ids)
    {
        uint64_t_to_address(crrt_id, crrt_address);
        print_address(crrt_address);
        SERIAL_USB->println();

        one_wire_thermistors.reset();
        one_wire_thermistors.select(crrt_address);
        one_wire_thermistors.write(0x44); // start conversion, with parasite power on at the end
    }

    return;
}

void Thermistors_Manager::collect_thermistors_conversions(void)
{
    wdt.restart();
    Address crrt_address;
    byte present = 0;
    byte data[12];
    float celsius;
    byte crc;

    // wait until conversion is ready
    delay(remaining_conversion_time());

    // collect the output of each sensor
    SERIAL_USB->println(F("collect results..."));
    for (auto &crrt_id : vector_of_ids)
    {
        uint64_t_to_address(crrt_id, crrt_address);
        print_address(crrt_address);
        SERIAL_USB->println();

        present = one_wire_thermistors.reset();
        one_wire_thermistors.select(crrt_address);
        one_wire_thermistors.write(0xBE); // Read Scratchpad

        SERIAL_USB->print("  Data = ");
        SERIAL_USB->print(present, HEX);
        SERIAL_USB->print(" ");
        for (int i = 0; i < 9; i++)
        { // we need 9 bytes
            data[i] = one_wire_thermistors.read();
            SERIAL_USB->print(data[i], HEX);
            SERIAL_USB->print(" ");
        }
        SERIAL_USB->print(" CRC=");
        crc = OneWire::crc8(data, 8);
        SERIAL_USB->print(crc, HEX);
        if (crc != data[8])
        {
            SERIAL_USB->println(F(" ERROR: non matching CRC"));
        }
        else
        {
            SERIAL_USB->println(F(" CRC OK"));
        }

        // Convert the data to actual temperature
        // because the result is a 16 bit signed integer, it should
        // be stored to an "int16_t" type, which is always 16 bits
        // even when compiled on a 32 bit processor.
        int16_t raw = (data[1] << 8) | data[0];
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
        {
            raw = raw & ~7;
            SERIAL_USB->println(F("  WARNING: 9 bit res"));
        } // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
        {
            raw = raw & ~3;
            SERIAL_USB->println(F("  WARNING: 10 bit res"));
        } // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
        {
            raw = raw & ~1;
            SERIAL_USB->println(F("  WARNING: 11 bit res"));
        } // 11 bit res, 375 ms
        else if (cfg == 0x60)
        {
            SERIAL_USB->println(F("  OK: 12 bits resolution"));
        }
        else
        {
            SERIAL_USB->println(F("  ERROR: unknown resolution config!"));
        }

        celsius = (float)raw / 16.0;
        SERIAL_USB->print("  Temperature = ");
        SERIAL_USB->print(celsius);
        SERIAL_USB->print(" Celsius");
        SERIAL_USB->println();

        vector_of_readings.push_back(ThermistorReading{crrt_id, celsius});
    }
}

unsigned long Thermistors_Manager::remaining_conversion_time(void)
{
    if (millis() - start_last_conversion_ms < duration_conversion_thermistor_ms)
    {
        unsigned long result = duration_conversion_thermistor_ms - (millis() - start_last_conversion_ms);
        SERIAL_USB->print(F("conversion not over yet; there are "));
        SERIAL_USB->print(result);
        SERIAL_USB->println(F(" ms left"));
        return result;
    }
    else
    {
        return 0UL;
    }
}

void Thermistors_Manager::perform_time_acquisition(void)
{
    unsigned long time_start_acquisition = millis();

    int number_of_readings = 0;

    while (millis() - time_start_acquisition <
           duration_thermistor_acquisition_ms)
    {
        collect_thermistors_conversions();
        request_start_thermistors_conversion();
    }
}

OneWire one_wire_thermistors(PIN_DS18B20_DAT);
Thermistors_Manager board_thermistors_manager;