#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "firmware_configuration.h"
#include "user_configuration.h"
#include "print_utils.h"

#include <SPI.h>
#include "SdFat.h"

#include "watchdog_manager.h"
#include "boot_counter.h"

#include "time_manager.h"
#include "kiss_posix_time_utils.hpp"

#include "thermistors_manager.h"

// which kind of card format is used
// this is what works on my 32 GB SD card
typedef SdFs sd_t;
typedef FsFile file_t;
// may need to use ExFat so that can have large SD cards
// typedef SdExFat sd_t;
// typedef ExFile file_t;

class SD_Manager{
    public:
        // update filename based on the UTC clock value
        void update_filename();

        // write a boot log message
        void log_boot(void);

        // write a full data file
        void log_data(void);

    private:
        // make everything ready to use
        // start the SD card, SPI etc
        // open file
        void start();

        // stop the SD logging, to be ready to sleep etc
        // close file
        // stop SD card, SPI etc
        void stop();

        char sd_filename[32];  // 28 should be enough, but a bit of margin and alignment

        file_t sd_file;
        sd_t sd_card;
};

extern SD_Manager sd_manager_instance;

#endif