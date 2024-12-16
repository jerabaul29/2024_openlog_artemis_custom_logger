#include "sd_manager.h"

SD_Manager sd_manager_instance;

// for doing manipulations on strings
static constexpr size_t work_buffer_size{32};
char work_buffer[work_buffer_size];

void SD_Manager::start()
{
    // start the SD card
    SdSpiConfig sd_config{SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(SD_SPI_MHZ)};

    while (!sd_card.begin(sd_config))
    {
        SERIAL_USB->println(F("ERR Cannot start SD card"));

        // watchdog reboot
        while (true)
        {
        }
    }

    delay(100);
    wdt.restart();

    // open the file using the filename that is already set
    if (!sd_file.open(sd_filename, O_RDWR | O_CREAT))
    {
        Serial.println(F("ERR cannot open file"));

        while (true){};
    }

    delay(100);
    wdt.restart();

    // at this point, ready to write etc to file
}

void SD_Manager::stop()
{
    // flush to the SD card to make sure all is written
    sd_file.flush();
    delay(100);
    wdt.restart();

    // close the file
    if (!sd_file.close())
    {
        SERIAL_USB->println(F("ERR cannot close file"));
        while (true)
        {
        };
    }
    delay(100);
    wdt.restart();

    // stop the SD card, stop SPI etc so that ready to sleep, restart, etc
    sd_card.end();
    delay(100);
    wdt.restart();
}

void SD_Manager::update_filename()
{
    kiss_calendar_time crrt_calendar_time;
    posix_to_calendar(board_time_manager.get_posix_timestamp(), &crrt_calendar_time);

    // YYYY- (5 chars)
    snprintf(&(sd_filename[0]), 4 + 1, "%04u", crrt_calendar_time.year);
    sd_filename[4] = '-';
    // MM- (3 chars)
    snprintf(&(sd_filename[5]), 2 + 1, "%02u", crrt_calendar_time.month);
    sd_filename[7] = '-';
    // DDT (3 chars)
    snprintf(&(sd_filename[8]), 2 + 1, "%02u", crrt_calendar_time.day);
    sd_filename[10] = 'T';
    // HH- (3 chars)
    snprintf(&(sd_filename[11]), 2 + 1, "%02u", crrt_calendar_time.hour);
    sd_filename[13] = '-';
    // MM- (3 chars)
    snprintf(&(sd_filename[14]), 2 + 1, "%02u", crrt_calendar_time.minute);
    sd_filename[16] = '-';
    // SS. (3 chars)
    snprintf(&(sd_filename[17]), 2 + 1, "%02u", crrt_calendar_time.second);
    sd_filename[19] = '.';
    // .dat
    sd_filename[20] = 'd';
    sd_filename[21] = 'a';
    sd_filename[22] = 't';
    sd_filename[23] = '\0';
}

void SD_Manager::log_boot(void){
    start();
    delay(100);
    wdt.restart();

    sd_file.print(F("\n\nBOOT\n\nn"));
    delay(100);
    wdt.restart();
    sd_file.print(F("BOOT_done\n\n"));

    stop();
    delay(100);
    wdt.restart();
}

void SD_Manager::log_data(void){
    start();
    delay(100);
    wdt.restart();

    sd_file.print(F("\n\nDATA\n\nn"));

    sd_file.print(F("DATA-done\n\n"));

    stop();
    delay(100);
    wdt.restart();
}