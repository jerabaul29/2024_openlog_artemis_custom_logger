#ifndef SD_CARD_MANAGER_H
#define SD_CARD_MANAGER_H

#include "firmware_configuration.h"
#include <SPI.h>
#include "SdFat.h"

class SD_Card_Manager {
public:
    // Start the SD card
    bool start();
    
    // Stop the SD card
    void stop();
    
    // Pre-allocate a new file and open it
    bool preallocate_and_open_file(const char* filename, uint32_t size_bytes);
    
    // Close and sync the open file
    void close_and_sync_file();
    
    // Write one byte array to the file
    // If you want this to go fast, use an array of 512 bytes, which is a SD card memory page
    bool write_buffer(const uint8_t* buffer, size_t size);
    
    // Get the SD card object (for advanced operations)
    SdFat* get_sd() { return &sd_card; }
    
    // Get the file object (for advanced operations)
    FsFile* get_file() { return &sd_file; }

private:
    SdFat sd_card;
    FsFile sd_file;
    bool sd_initialized = false;
    bool file_open = false;
};

extern SD_Card_Manager sd_card_manager;

#endif
