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

        while (true)
        {
        };
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

    uint16_t boot_number = boot_counter_instance.get_boot_number();

    // BBBBB- (6 chars)
    snprintf(&(sd_filename[0]), 5 + 1, "%05u", boot_number);
    sd_filename[5] = '-';
    // YYYY- (5 chars)
    snprintf(&(sd_filename[6]), 4 + 1, "%04u", crrt_calendar_time.year);
    sd_filename[10] = '-';
    // MM- (3 chars)
    snprintf(&(sd_filename[11]), 2 + 1, "%02u", crrt_calendar_time.month);
    sd_filename[13] = '-';
    // DDT (3 chars)
    snprintf(&(sd_filename[14]), 2 + 1, "%02u", crrt_calendar_time.day);
    sd_filename[16] = 'T';
    // HH- (3 chars)
    snprintf(&(sd_filename[17]), 2 + 1, "%02u", crrt_calendar_time.hour);
    sd_filename[19] = '-';
    // MM- (3 chars)
    snprintf(&(sd_filename[20]), 2 + 1, "%02u", crrt_calendar_time.minute);
    sd_filename[22] = '-';
    // SS. (3 chars)
    snprintf(&(sd_filename[23]), 2 + 1, "%02u", crrt_calendar_time.second);
    sd_filename[25] = '.';
    // .dat
    sd_filename[26] = 'd';
    sd_filename[27] = 'a';
    sd_filename[28] = 't';
    sd_filename[29] = '\0';

    SERIAL_USB->println(sd_filename);
}

void SD_Manager::log_boot(void)
{
    start();
    delay(100);
    wdt.restart();

    sd_file.print(F("\n\nBOOT\n\n"));
    delay(100);
    wdt.restart();
    sd_file.print(F("BOOT_done\n\n"));
    delay(100);
    wdt.restart();

    stop();
    delay(100);
    wdt.restart();
}

void SD_Manager::log_data(void)
{
    SERIAL_USB->println(F("start log_data..."));

    start();
    delay(100);
    wdt.restart();

    sd_file.print(F("\n\nDATA-start\n\n"));
    delay(100);
    wdt.restart();

    sd_file.print(F("THERMISTORS_START\n"));
    delay(100);
    wdt.restart();

    sd_file.print(F("THERMISTORS_POSIX_TIME_START: "));
    print_uint64_to_serial_print_buff(board_thermistors_manager.posix_time_start);
    sd_file.println(serial_print_buff);
    delay(100);
    wdt.restart();

    sd_file.println(F("READING_NBR,THERMISTOR_ID,CELCIUS,"));

    ThermistorReading crrt_reading;
    for (size_t i=0; i<board_thermistors_manager.vector_of_readings.size(); i++){
        sd_file.print(i);
        sd_file.print(",");
        crrt_reading = board_thermistors_manager.vector_of_readings[i];
        print_uint64_to_serial_print_buff(crrt_reading.id);
        sd_file.print(serial_print_buff);
        sd_file.print(",");
        sd_file.print(crrt_reading.reading);
        sd_file.println(",");
        delay(10);
        wdt.restart();
    }

    sd_file.print(F("THERMISTORS_STOP\n\n"));
    delay(100);
    wdt.restart();

    //
    sd_file.print(F("IRSENSOR_START\n"));
    delay(100);
    wdt.restart();

    sd_file.println(F("READING_NBR,POSIX_TIMESTAMP,IR_TEMP,SENSOR_TEMP,"));

    MLX_Information crrt_reading_mlx;
    for (size_t i=0; i<mlx90164_manager.crrt_accumulator_MLX.size(); i++){
        sd_file.print(i);
        sd_file.print(",");
        crrt_reading_mlx = mlx90164_manager.crrt_accumulator_MLX[i];
        sd_file.print(crrt_reading_mlx.posix_timestamp);
        sd_file.print(",");
        sd_file.print(crrt_reading_mlx.ir_temperature);
        sd_file.print(",");
        sd_file.print(crrt_reading_mlx.sensor_temperature);
        sd_file.println(",");
        delay(10);
        wdt.restart();
    }

    sd_file.print(F("IRSENSOR_STOP\n\n"));
    delay(100);
    wdt.restart();
    //

    sd_file.print(F("DATA-stop\n\n"));
    delay(100);
    wdt.restart();

    stop();
    delay(100);
    wdt.restart();

    SERIAL_USB->println("done log_data!");
    delay(100);
}