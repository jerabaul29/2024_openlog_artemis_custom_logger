/**
 * @file sd_card_manager.cpp
 * @brief Implementation of SD Card Manager
 */

#include "sd_card_manager.h"

// Global instance
SD_Card_Manager sd_card_manager;

/**
 * @brief Turn on SD card power
 * 
 * Sets the SD_PWR pin LOW (active low) to enable SD card power,
 * then waits for the card to stabilize.
 */
void microSDPowerOn()
{
    delay(10);
    SERIAL_USB->println(F("Turning SD card power ON..."));
    pinMode(SD_PWR, OUTPUT);
    digitalWrite(SD_PWR, LOW);
    delay(250);  // Wait for SD card to power up
}

/**
 * @brief Turn off SD card power
 * 
 * Sets the SD_PWR pin HIGH to disable SD card power.
 */
void microSDPowerOff()
{
    delay(10);
    SERIAL_USB->println(F("Turning SD card power OFF..."));
    pinMode(SD_PWR, OUTPUT);
    digitalWrite(SD_PWR, HIGH);
    delay(10);
}

bool SD_Card_Manager::start() {
    if (sd_initialized) {
        return true;
    }
    
    microSDPowerOn();
    
    SERIAL_USB->println(F("Initializing SD card..."));
    
    SdSpiConfig sd_config{SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(SD_SPI_MHZ)};
    
    if (!sd_card.begin(sd_config)) {
        SERIAL_USB->println(F("ERROR: SD card initialization failed!"));
        SERIAL_USB->println(F("Check that SD card is inserted properly"));
        SERIAL_USB->print(F("Error code: "));
        SERIAL_USB->println(sd_card.card()->errorCode(), HEX);
        SERIAL_USB->print(F("Error data: "));
        SERIAL_USB->println(sd_card.card()->errorData(), HEX);
        
        // Turn off power on failure
        microSDPowerOff();
        return false;
    }
    
    sd_initialized = true;
    SERIAL_USB->println(F("SD card initialized successfully"));
    
    // Print card info
    SERIAL_USB->print(F("Card type: "));
    switch (sd_card.card()->type()) {
        case SD_CARD_TYPE_SD1:
            SERIAL_USB->println(F("SD1"));
            break;
        case SD_CARD_TYPE_SD2:
            SERIAL_USB->println(F("SD2"));
            break;
        case SD_CARD_TYPE_SDHC:
            SERIAL_USB->println(F("SDHC"));
            break;
        default:
            SERIAL_USB->println(F("Unknown"));
    }
    
    SERIAL_USB->print(F("Volume type: FAT"));
    SERIAL_USB->println(sd_card.vol()->fatType());
    
    return true;
}

void SD_Card_Manager::stop() {
    if (file_open) {
        close_and_sync_file();
    }
    
    if (sd_initialized) {
        sd_card.end();
        sd_initialized = false;
        SERIAL_USB->println(F("SD card stopped"));
    }
    
    microSDPowerOff();
}

bool SD_Card_Manager::preallocate_and_open_file(const char* filename, uint32_t size_bytes) {
    if (!sd_initialized) {
        SERIAL_USB->println(F("ERROR: SD card not initialized"));
        return false;
    }
    
    if (file_open) {
        close_and_sync_file();
    }
    
    SERIAL_USB->print(F("Opening file: "));
    SERIAL_USB->println(filename);
    
    // Try to remove old file first
    if (sd_card.exists(filename)) {
        SERIAL_USB->println(F("WARNING: Removing old file..."));
        sd_card.remove(filename);
    }
    
    if (!sd_file.open(filename, O_RDWR | O_CREAT)) {
        SERIAL_USB->println(F("ERROR: Failed to open test file!"));
        SERIAL_USB->print(F("Error code: "));
        SERIAL_USB->println(sd_card.card()->errorCode(), HEX);
        SERIAL_USB->print(F("Error data: "));
        SERIAL_USB->println(sd_card.card()->errorData(), HEX);
        return false;
    }
    
    file_open = true;
    SERIAL_USB->println(F("File opened successfully"));
    
    // Truncate file to zero before preallocation
    if (!sd_file.truncate(0)) {
        SERIAL_USB->println(F("WARNING: Failed to truncate file"));
    }
    
    // Preallocate if size specified
    if (size_bytes > 0) {
        SERIAL_USB->print(F("Preallocating "));
        SERIAL_USB->print(size_bytes);
        SERIAL_USB->println(F(" bytes..."));
        
        if (!sd_file.preAllocate(size_bytes)) {
            SERIAL_USB->println(F("WARNING: Failed to preallocate file!"));
            SERIAL_USB->println(F("Continuing without preallocation..."));
        } else {
            SERIAL_USB->println(F("File preallocated successfully"));
            sd_file.sync();  // Ensure FAT is updated
        }
    }
    
    return true;
}

void SD_Card_Manager::close_and_sync_file() {
    if (!file_open) {
        return;
    }
    
    SERIAL_USB->println(F("Syncing and closing file..."));
    sd_file.sync();
    sd_file.close();
    file_open = false;
    SERIAL_USB->println(F("File closed"));
}

bool SD_Card_Manager::write_buffer(const uint8_t* buffer, size_t size) {
    if (!file_open) {
        SERIAL_USB->println(F("ERROR: No file open for writing"));
        return false;
    }
    
    size_t written = sd_file.write(buffer, size);
    return (written == size);
}
