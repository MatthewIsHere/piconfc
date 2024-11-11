#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "piconfc.h"
#include "piconfc_PN532.h"
#include "piconfc_I2C.h"

#define DEFAULT_TIMEOUT 5000

// Hoist
void printhex(uint8_t* buff, int len);

float piconfc_PN532_firmwareVersion(PicoNFCConfig *config) {
    uint8_t command = PN532_COMMAND_GETFIRMWAREVERSION;
    
    // Send the GETFIRMWAREVERSION command and check for acknowledgment
    if (!piconfc_I2C_sendcommand_andack(config->i2c_block, &command, 1, DEFAULT_TIMEOUT)) {
        return -1.0;  // Return -1.0 if there was an error sending the command
    }

    // Parse the response and check for success
    int success = piconfc_I2C_parseresponse(config->i2c_block, config->scratch, 5);
    if (!success) {
        return -1.0;  // Return -1.0 if parsing the response failed
    }
    
    // Calculate the firmware version as Major.minor (e.g., 1.6)
    return (config->scratch[2] * 10 + config->scratch[3]) / 10.0;
}

bool piconfc_PN532_RFRegulationTest(PicoNFCConfig *config) {
    uint8_t buffer[2] = { PN532_COMMAND_RFREGULATIONTEST, 0 };

    // Send the RF Regulation Test command to the PN532
    piconfc_I2C_writecommand(config->i2c_block, buffer, 2);

    // Short delay to allow the command to process
    sleep_ms(1);

    // Wait for the PN532 to become ready
    if (!piconfc_I2C_waitready(config->i2c_block, DEFAULT_TIMEOUT)) {
        return false;
    }

    // Check for acknowledgment from the PN532
    if (!piconfc_I2C_readack(config->i2c_block)) {
        return false;
    }

    // Additional delay to ensure the test is running
    sleep_ms(1);

    return true;
}

bool piconfc_PN532_SAMConfiguration(PicoNFCConfig *config) {
    uint8_t buffer[] = {
        PN532_COMMAND_SAMCONFIGURATION,
        0x01, // Normal mode
        0x14, // Timeout of 50ms * 20 (0x14) = 1 second
        0x00  // Disable IRQ pin; not implemented but works reliably without it
    };

    // Send the SAM Configuration command and wait for acknowledgment
    if (!piconfc_I2C_sendcommand_andack(config->i2c_block, buffer, sizeof(buffer), DEFAULT_TIMEOUT)) {
        return false;
    }
    
    // Parse the response and check the return command value
    uint8_t len = piconfc_I2C_parseresponse(config->i2c_block, config->scratch, 1);

    return config->scratch[0] == 0x15; // Expected response indicating success
}

bool piconfc_PN532_setPassiveActivationRetries(PicoNFCConfig *config, uint8_t retries) {
    uint8_t buffer[] = {
        PN532_COMMAND_RFCONFIGURATION,
        0x05,   // Retries section
        0xFF,   // Default MxRtyATR value
        0x01,   // Default MxRtyPSL value
        retries // Set MxPassiveRty value based on input
    };

    // Send the RF Configuration command and wait for acknowledgment
    if (!piconfc_I2C_sendcommand_andack(config->i2c_block, buffer, sizeof(buffer), DEFAULT_TIMEOUT)) {
        return false;
    }

    // Parse the response and check for a valid response length
    uint8_t len = piconfc_I2C_parseresponse(config->i2c_block, config->scratch, 1);
    return len == 1;
}

bool piconfc_PN532_readPassiveTargetID(PicoNFCConfig *config, uint8_t baudrate, uint8_t *uid, uint8_t *uid_len, uint16_t timeout) {
    uint8_t buffer[] = {
        PN532_COMMAND_INLISTPASSIVETARGET,
        1, // Max cards = 1
        baudrate // Baud rate selector
    };

    // Send the InListPassiveTarget command and wait for acknowledgment
    if (!piconfc_I2C_sendcommand_andack(config->i2c_block, buffer, sizeof(buffer), timeout)) {
        return false;
    }

    // Parse the response and check for valid response length and card detection status
    uint8_t len = piconfc_I2C_parseresponse(config->i2c_block, config->scratch, 20);
    if (len == 0 || config->scratch[1] == 0) {
        return false;  // Return false if no card was detected
    }

    // Parse ATQA and SAK values for debugging or future use
    uint16_t ATQA = config->scratch[3];
    ATQA <<= 8;
    ATQA |= config->scratch[4];

    uint8_t SAK = config->scratch[5];

    // Store UID length and UID in provided buffers
    *uid_len = config->scratch[6];
  
    #if DEBUG
        printf("ATQA: %04X\n", ATQA);
        printf("SAK: %02X\n", SAK);
        printf("UID: ");
        printhex(config->scratch + 7, *uid_len);
    #endif

    // Copy UID from scratch buffer to the user-provided UID buffer
    for (uint8_t i = 0; i < *uid_len; i++) {
        uid[i] = config->scratch[7 + i];
    }

    return true;
}

bool piconfc_PN532_initiatorDataExchange(PicoNFCConfig *config, uint8_t *send, uint8_t sendlen, uint8_t *receive, uint8_t *received_length, uint8_t rbuf_size) {
    uint8_t cmdbuf[2 + sendlen];
    cmdbuf[0] = PN532_COMMAND_INDATAEXCHANGE;
    cmdbuf[1] = 1; // Card slot (only slot 1 is supported)
    memcpy(cmdbuf + 2, send, sendlen);

    // Send the InDataExchange command and wait for acknowledgment
    if (!piconfc_I2C_sendcommand_andack(config->i2c_block, cmdbuf, sizeof(cmdbuf), DEFAULT_TIMEOUT)) {
        return false;
    }

    // Parse the response, expecting rbuf_size + 2 bytes (command ID + status byte + data)
    uint8_t len = piconfc_I2C_parseresponse(config->i2c_block, config->scratch, rbuf_size + 2);

    // Check for valid command ID and status byte
    if (config->scratch[0] != 0x41 || config->scratch[1] != 0) {
        return false;
    }
    #if DEBUG
        printf("Successfully indataexchanged\n");
    #endif

    // Copy the response data (excluding command ID and status byte) into the receive buffer
    for (int i = 2; i < len; i++) {
        receive[i - 2] = config->scratch[i];
    }

    // Set the length of the received data
    *received_length = len - 2;

    return true;
}

// Does what it says
void printhex(uint8_t* buff, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02X ", buff[i]);
    }
    printf("\n");
}