#ifndef BOOT_COUNTER_H
#define BOOT_COUNTER_H

#include <EEPROM.h>

class Boot_Counter{
    public:
        uint16_t get_boot_number(void);
        void increment_boot_number(void);
        void set_boot_number(uint16_t);

    private:
        static constexpr int address_byte_times_001 {0};
        static constexpr int address_byte_times_256 {1};
};

extern Boot_Counter boot_counter_instance;

#endif