/**
 * @file sd_card_manager.h
 * @brief SD Card Manager for OpenLog Artemis
 * 
 * This class provides a high-level interface for managing SD card operations
 * including initialization, file operations, and power management.
 */

#ifndef SD_CARD_MANAGER_H
#define SD_CARD_MANAGER_H

#include "firmware_configuration.h"
#include <SPI.h>
#include "SdFat.h"

/**
 * @class SD_Card_Manager
 * @brief Manages SD card initialization, power, and file operations
 * 
 * Provides methods to control SD card power, initialize the card,
 * open/close files with preallocation support, and perform write operations.
 */
class SD_Card_Manager {
public:
    /**
     * @brief Initialize and start the SD card
     * 
     * Powers on the SD card, initializes SPI communication, and reads card info.
     * If initialization fails, the SD card power is turned off.
     * 
     * @return true if initialization successful, false otherwise
     * @note Can be called multiple times; returns true immediately if already initialized
     */
    bool start();
    
    /**
     * @brief Stop the SD card and turn off power
     * 
     * Closes any open files, ends SD card communication, and turns off power.
     * Safe to call multiple times or before start() is called.
     */
    void stop();
    
    /**
     * @brief Open a file and optionally preallocate space
     * 
     * Removes existing file if present, opens a new file, truncates to zero,
     * and preallocates the specified size to improve write performance.
     * 
     * @param filename Name of file to create (8.3 format recommended)
     * @param size_bytes Number of bytes to preallocate (0 = no preallocation)
     * @return true if file opened successfully, false otherwise
     * @note Closes any previously open file before opening new one
     */
    bool preallocate_and_open_file(const char* filename, uint32_t size_bytes);
    
    /**
     * @brief Sync and close the currently open file
     * 
     * Flushes all pending writes to SD card and closes the file.
     * Safe to call even if no file is open.
     */
    void close_and_sync_file();
    
    /**
     * @brief Write a buffer to the currently open file
     * 
     * Writes data from buffer to file. For best performance, use 512-byte buffers
     * (SD card sector size) and ensure file is preallocated.
     * 
     * @param buffer Pointer to data to write (must not be NULL)
     * @param size Number of bytes to write
     * @return true if all bytes written successfully, false otherwise
     * @note Returns false if no file is open or if write fails
     */
    bool write_buffer(const uint8_t* buffer, size_t size);
    
    /**
     * @brief Get direct access to SD card object
     * 
     * For advanced operations not covered by this API.
     * Use with caution as it bypasses encapsulation.
     * 
     * @return Pointer to internal SdFat object
     */
    SdFat* get_sd() { return &sd_card; }
    
    /**
     * @brief Get direct access to file object
     * 
     * For advanced operations not covered by this API.
     * Use with caution as it bypasses encapsulation.
     * 
     * @return Pointer to internal FsFile object
     */
    FsFile* get_file() { return &sd_file; }

private:
    SdFat sd_card;              ///< SD card filesystem object
    FsFile sd_file;             ///< Currently open file object
    bool sd_initialized = false; ///< True if SD card successfully initialized
    bool file_open = false;     ///< True if a file is currently open
};

/// Global SD card manager instance
extern SD_Card_Manager sd_card_manager;

#endif

