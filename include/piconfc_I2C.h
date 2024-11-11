/**
 * @file piconfc_I2C.h
 * @brief I2C communication functions for interfacing with the PN532 NFC module.
 *
 * This header file provides functions for initializing and communicating with the PN532 NFC module
 * over I2C. It includes functions for setting up the I2C interface, sending commands, reading responses,
 * checking readiness, and validating responses from the PN532. The functions are designed to work
 * with the Raspberry Pi Pico's I2C API.
 *
 * The functions include:
 * - Initialization of the I2C pins and bus
 * - Checking if the PN532 is ready for communication
 * - Sending commands and waiting for acknowledgment
 * - Reading data and validating response packets
 *
 * These functions are meant to be used as a low-level API for higher-level NFC operations.
 */

#ifndef PICONFC_I2C_H
#define PICONFC_I2C_H

#include <stdbool.h>
#include "hardware/i2c.h"

#define PICONFC_I2C_FREQ (400 * 1000)

/**
 * @brief Initializes the I2C bus and configures the specified pins.
 *
 * This function initializes the specified I2C instance with a predefined frequency
 * and configures the given SDA and SCL pins for I2C functionality. It also enables
 * pull-up resistors on these pins, which is required for I2C communication.
 *
 * @param block Pointer to the I2C instance to initialize (e.g., i2c0 or i2c1).
 * @param sda_pin GPIO pin to use for the SDA (data) line.
 * @param scl_pin GPIO pin to use for the SCL (clock) line.
 */
void piconfc_I2C_init(i2c_inst_t *block, uint8_t sda_pin, uint8_t scl_pin);

/**
 * @brief Checks if the PN532 is ready for communication.
 *
 * This function reads a single byte from the PN532 and checks if it matches
 * the `PN532_I2C_READY` value, indicating that the device is ready.
 *
 * @param block Pointer to the I2C instance to use (e.g., i2c0 or i2c1).
 * @return True if the PN532 is ready; false otherwise.
 */
bool piconfc_I2C_isready(i2c_inst_t* block);

/**
 * @brief Waits until the PN532 is ready or a timeout occurs.
 *
 * This function repeatedly checks if the PN532 is ready by calling `piconfc_I2C_isready`.
 * If the device does not become ready within the specified timeout (in milliseconds),
 * the function returns false. If the timeout is set to 0, it will wait indefinitely.
 *
 * @param block Pointer to the I2C instance to use (e.g., i2c0 or i2c1).
 * @param timeout Maximum time to wait in milliseconds (0 for indefinite wait).
 * @return True if the PN532 is ready; false if the timeout is reached.
 */
bool piconfc_I2C_waitready(i2c_inst_t* block, int timeout);

/**
 * @brief Sends a command to the PN532 and waits for an acknowledgment.
 *
 * This function sends a command to the PN532 using `piconfc_I2C_writecommand` and then
 * waits for the device to be ready. It checks for an acknowledgment (ACK) within the specified
 * timeout period. If the ACK is received and the device becomes ready again, the function returns true.
 *
 * @param block Pointer to the I2C instance to use (e.g., i2c0 or i2c1).
 * @param cmd Pointer to the command data to send.
 * @param len Length of the command data in bytes.
 * @param timeout Maximum time to wait for an acknowledgment in milliseconds.
 * @return True if the command was acknowledged within the timeout; false otherwise.
 */
bool piconfc_I2C_sendcommand_andack(i2c_inst_t* block, uint8_t * cmd, uint8_t len, int timeout);

/**
 * @brief Reads data from the PN532 and checks for an acknowledgment (ACK).
 *
 * This function reads a predefined number of bytes from the PN532 and compares
 * the received data with the expected ACK pattern (`PN532_ACK`). If the received
 * data matches the ACK, the function returns true.
 *
 * @param block Pointer to the I2C instance to use (e.g., i2c0 or i2c1).
 * @return True if the received data matches the expected ACK; false otherwise.
 */
bool piconfc_I2C_readack(i2c_inst_t* block);

/**
 * @brief Reads a specified number of bytes from the PN532 into a buffer.
 *
 * This function reads `len + 1` bytes from the PN532. The first byte is a ready (RDY) byte,
 * which is ignored, and the remaining `len` bytes are copied into the provided buffer.
 *
 * @param block Pointer to the I2C instance to use (e.g., i2c0 or i2c1).
 * @param buffer Pointer to the buffer where the data will be stored.
 * @param len Number of data bytes to read from the PN532 (excluding the RDY byte).
 */
void piconfc_I2C_readdata(i2c_inst_t* block, uint8_t * buffer, uint8_t len);

/**
 * @brief Writes a command packet to the PN532 over I2C.
 *
 * This function constructs a command packet with the specified command data and length, 
 * following the PN532 protocol. It includes preamble, start codes, length, checksum, 
 * and postamble fields. The packet is then sent to the PN532.
 *
 * @param block Pointer to the I2C instance to use (e.g., i2c0 or i2c1).
 * @param cmd Pointer to the command data to send.
 * @param cmdlen Length of the command data in bytes.
 */
void piconfc_I2C_writecommand(i2c_inst_t* block, uint8_t * cmd, uint8_t cmdlen);

/**
 * @brief Parses a response from the PN532 and validates the packet structure.
 *
 * This function reads the response from the PN532 into the provided buffer, validates
 * the preamble, start codes, and checksums, and extracts the command ID and data length.
 * The buffer should be large enough to hold `expected_data_len + 8` bytes.
 *
 * @param block Pointer to the I2C instance to use (e.g., i2c0 or i2c1).
 * @param buffer Pointer to the buffer where the response will be stored.
 * @param expected_data_len Expected length of the response data, in bytes.
 * @return Length of the response data (excluding the PN532 indicator) if successful;
 *         0 if there was an error in validation.
 */
uint8_t piconfc_I2C_parseresponse(i2c_inst_t *block, uint8_t *buffer, uint8_t expected_data_len);

#endif /* PICONFC_I2C_H */