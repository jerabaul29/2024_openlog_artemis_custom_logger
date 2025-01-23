#include "boot_counter.h"

uint16_t Boot_Counter::get_boot_number(void)
{
    byte byte_times_001;
    byte byte_times_256;

    EEPROM.get(address_byte_times_001, byte_times_001);
    EEPROM.get(address_byte_times_256, byte_times_256);

    return (static_cast<uint16_t>(byte_times_001) * 1 + static_cast<uint16_t>(byte_times_256) * 256);
}

void Boot_Counter::set_boot_number(uint16_t value){
    EEPROM.put(address_byte_times_001, static_cast<byte>(value % 256));
    EEPROM.put(address_byte_times_256, static_cast<byte>(value >> 8));
}

void Boot_Counter::increment_boot_number(void)
{
    uint16_t previous = Boot_Counter::get_boot_number();
    Boot_Counter::set_boot_number(previous+1);
}

Boot_Counter boot_counter_instance;