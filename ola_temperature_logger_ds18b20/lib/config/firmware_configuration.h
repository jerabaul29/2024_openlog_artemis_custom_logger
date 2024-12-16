#ifndef FIRMWARE_CONFIGURATION
#define FIRMWARE_CONFIGURATION

//////////////////////////////////////////////////////////////////////////////////////////
// the firmware configuration that the user should not touch
// this includes:
//   - hardware imposed choices (pins, ports numbers, etc)
//   - important conventions (baudrate, I2C frequencies, etc)
//////////////////////////////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include "macro_utils.h"
#include "print_utils.h"

//////////////////////////////////////////////////////////////////////////////////////////
// serial over USB related 

extern Uart * SERIAL_USB;
static constexpr int BAUD_RATE_USB {1000000};

//////////////////////////////////////////////////////////////////////////////////////////
// I2C over qwiic

// TODO: check that correct
static constexpr int PORT_I2C_QWIIC_NUMBER {2};

//TODO: check that correct
static constexpr byte PIN_QWIIC_SCL {8};
static constexpr byte PIN_QWIIC_SDA {9};

//////////////////////////////////////////////////////////////////////////////////////////
// pins on the PCB

// LEDs
static constexpr int PIN_PWR_LED {29};
static constexpr int PIN_STAT_LED {19};

// DS18B20
static constexpr int PIN_DS18B20_PWR {32};
static constexpr int PIN_DS18B20_DAT {11};

//////////////////////////////////////////////////////////////////////////////////////////
// SD card pins

static constexpr int SD_CS_PIN {23};
// static constexpr int DEDICATED_SPI {0};
static constexpr int SD_SPI_MHZ {24};  // try 12 if this does not work

//////////////////////////////////////////////////////////////////////////////////////////
// misc

static constexpr char commit_id[] {STRINGIFY_CONTENT(REPO_COMMIT_ID)};
static constexpr char git_branch[] {STRINGIFY_CONTENT(REPO_GIT_BRANCH)};

//////////////////////////////////////////////////////////////////////////////////////////
// functions

void print_firmware_config(void);

uint64_t read_chip_id(void);

#endif