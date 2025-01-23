#include "print_utils.h"

// common printing buffer
char serial_print_buff[serial_print_max_buffer];

void print_uint64(uint64_t to_print){
    // I have tried with snprintf and associates, seem to not work with uint64_t
    // even using the special macros etc. Write my simple implementation by hand
    // then!

    static constexpr size_t utils_char_buffer_size {24};
    char utils_char_buffer[utils_char_buffer_size] {'\0'};

    size_t crrt_index {0};

    while (crrt_index < utils_char_buffer_size){
        utils_char_buffer[crrt_index] = static_cast<char>(to_print % 10 + 48);
        to_print /= 10;
        if (to_print == 0){
            break;
        }
        else {
            crrt_index += 1;
        }
    }

    while (true){
        SERIAL_USB->print(utils_char_buffer[crrt_index]);
        if (crrt_index == 0){
            break;
        }
        else {
            crrt_index -= 1;
        }
    }
}

void print_uint64_to_serial_print_buff(uint64_t to_print){
    for (size_t i=0; i<serial_print_max_buffer; i++){
        serial_print_buff[i] = '\0';
    }

    size_t crrt_index {128};

    while (crrt_index < serial_print_max_buffer){
        serial_print_buff[crrt_index] = static_cast<char>(to_print % 10 + 48);
        to_print /= 10;
        if (to_print == 0){
            break;
        }
        else {
            crrt_index += 1;
        }
    }

    size_t crrt_index_reverse {0};
    while(true){
        serial_print_buff[crrt_index_reverse] = serial_print_buff[crrt_index];

        if (crrt_index == 128){
            break;
        }
        else {
            crrt_index -= 1;
            crrt_index_reverse += 1;
        }
    }
}

// for the print_hex family of functins:
// 1 byte is represented by 2 hex chars
// so u8 ie 1 byte is 0x + 1*2 + 1 hex chars ie 5 chars
// so u16 ie 2 bytes is 0x + 2*2 + 1 hex chars ie 7 chars
// so u32 ie 4 bytes is 0x + 2*4 + 1 hex chars ie 11 chars
// so u64 ie 8 bytes is 0x + 2*8 + 1 hex chars ie 19 chars
// we round to closest number of bytes that is a multiple of 4 as we are on either 32 or 64 bits MCU

// print arrays of uints

void print_hex_u8s(uint8_t *data, size_t length)
{
    constexpr size_t tmp_size {8};
    char tmp[tmp_size];
    for (int i = 0; i < length; i++)
    {
        snprintf(tmp, tmp_size, "0x%.2X", data[i]);
        SERIAL_USB->print(tmp);
        SERIAL_USB->print(" ");
    }
}

void print_hex_u16s(uint16_t *data, size_t length)
{
    constexpr size_t tmp_size {8};
    char tmp[tmp_size];
    for (int i = 0; i < length; i++)
    {
        snprintf(tmp, tmp_size, "0x%.4X", data[i]);
        SERIAL_USB->print(tmp);
        SERIAL_USB->print(" ");
    }
}

void print_hex_u32s(uint32_t *data, size_t length)
{
    constexpr size_t tmp_size {12};
    char tmp[tmp_size];
    for (int i = 0; i < length; i++)
    {
        snprintf(tmp, tmp_size, "0x%.8X", data[i]);
        SERIAL_USB->print(tmp);
        SERIAL_USB->print(" ");
    }
}

void print_hex_u64s(uint64_t *data, size_t length)
{
    constexpr size_t tmp_size {20};
    char tmp[tmp_size];
    for (int i = 0; i < length; i++)
    {
        // workaround uint64_t issues with the snprintf provided
        snprintf(tmp, tmp_size, "0x%.8X", static_cast<uint32_t>(data[i] >> 32));
        SERIAL_USB->print(tmp);
        snprintf(tmp, tmp_size, "%.8X", static_cast<uint32_t>(data[i]));
        SERIAL_USB->print(tmp);
        SERIAL_USB->print(" ");
    }
}

// printing a single uint

void print_hex_u8(uint8_t data)
{
    constexpr size_t tmp_size {8};
    char tmp[tmp_size];
    snprintf(tmp, tmp_size, "0x%.2X", data);
    SERIAL_USB->print(tmp);
}

void print_hex_u16(uint16_t data)
{
    constexpr size_t tmp_size {8};
    char tmp[tmp_size];
    snprintf(tmp, tmp_size, "0x%.4X", data);
    SERIAL_USB->print(tmp);
}

void print_hex_u32(uint32_t data)
{
    constexpr size_t tmp_size {12};
    char tmp[tmp_size];
    snprintf(tmp, tmp_size, "0x%.8X", data);
    SERIAL_USB->print(tmp);
}

void print_hex_u64(uint64_t data)
{
    constexpr size_t tmp_size {20};
    char tmp[tmp_size];

    // workaround uint64_t issues with the snprintf provided
    snprintf(tmp, tmp_size, "0x%.8X", static_cast<uint32_t>(data >> 32));
    SERIAL_USB->print(tmp);
    snprintf(tmp, tmp_size, "%.8X", static_cast<uint32_t>(data));
    SERIAL_USB->print(tmp);
    SERIAL_USB->print(" ");
}

void serialPrintf(const char *fmt, ...) {
  /* pointer to the variable arguments list */
  va_list pargs;
  /* Initialise pargs to point to the first optional argument */
  va_start(pargs, fmt);
  /* create the formatted data and store in buff */
  vsnprintf(serial_print_buff, serial_print_max_buffer, fmt, pargs);
  va_end(pargs);
  SERIAL_USB->print(serial_print_buff);
}

void print_vector_uc(const etl::ivector<unsigned char>& vector){
    SERIAL_USB->println(F("--------------------"));
    SERIAL_USB->print(F("vector of unsigned char, size "));
    SERIAL_USB->print(vector.size());
    SERIAL_USB->print(F(" max size "));
    SERIAL_USB->println(vector.max_size());

    SERIAL_USB->print(F("content: "));

    for (const unsigned char crrt_char : vector){
        SERIAL_USB->print(F("0x"));
        SERIAL_USB->print(crrt_char, HEX);
        SERIAL_USB->print(F(" | "));
    }
    SERIAL_USB->println();

    SERIAL_USB->print(F("Rock7 format content: "));
    for (const unsigned char crrt_char : vector){
      snprintf(serial_print_buff, serial_print_max_buffer, "%02X", crrt_char);
      SERIAL_USB->print(serial_print_buff);
    }
    SERIAL_USB->println();

    SERIAL_USB->println(F("--------------------"));
}