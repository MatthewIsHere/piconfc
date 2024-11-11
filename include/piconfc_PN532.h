/**
 * @file piconfc_PN532.h
 * @brief Header file for interfacing with the PN532 NFC module via I2C.
 *
 * This file contains constants, macros, and function declarations for configuring
 * and interacting with the PN532 NFC module. It provides commands for performing
 * NFC-related tasks, such as detecting passive targets, data exchange, and reading
 * unique IDs from NFC tags.
 *
 * The functions defined in this header file enable communication with the PN532
 * over I2C, allowing initialization, command sending, and response parsing. Only I2C is supported.
 */

#ifndef PN532_H
#define PN532_H

#include "hardware/i2c.h"
#include "piconfc.h"

// PN532 Specific Definitions

#define PN532_PREAMBLE (0x00)   ///< Command sequence start, byte 1/3
#define PN532_STARTCODE1 (0x00) ///< Command sequence start, byte 2/3
#define PN532_STARTCODE2 (0xFF) ///< Command sequence start, byte 3/3
#define PN532_POSTAMBLE (0x00)  ///< EOD

#define PN532_HOSTTOPN532 (0xD4) ///< Host-to-PN532
#define PN532_PN532TOHOST (0xD5) ///< PN532-to-host

// PN532 Commands
#define PN532_COMMAND_DIAGNOSE (0x00)              ///< Diagnose
#define PN532_COMMAND_GETFIRMWAREVERSION (0x02)    ///< Get firmware version
#define PN532_COMMAND_GETGENERALSTATUS (0x04)      ///< Get general status
#define PN532_COMMAND_READREGISTER (0x06)          ///< Read register
#define PN532_COMMAND_WRITEREGISTER (0x08)         ///< Write register
#define PN532_COMMAND_READGPIO (0x0C)              ///< Read GPIO
#define PN532_COMMAND_WRITEGPIO (0x0E)             ///< Write GPIO
#define PN532_COMMAND_SETSERIALBAUDRATE (0x10)     ///< Set serial baud rate
#define PN532_COMMAND_SETPARAMETERS (0x12)         ///< Set parameters
#define PN532_COMMAND_SAMCONFIGURATION (0x14)      ///< SAM configuration
#define PN532_COMMAND_POWERDOWN (0x16)             ///< Power down
#define PN532_COMMAND_RFCONFIGURATION (0x32)       ///< RF config
#define PN532_COMMAND_RFREGULATIONTEST (0x58)      ///< RF regulation test
#define PN532_COMMAND_INJUMPFORDEP (0x56)          ///< Jump for DEP
#define PN532_COMMAND_INJUMPFORPSL (0x46)          ///< Jump for PSL
#define PN532_COMMAND_INLISTPASSIVETARGET (0x4A)   ///< List passive target
#define PN532_COMMAND_INATR (0x50)                 ///< ATR
#define PN532_COMMAND_INPSL (0x4E)                 ///< PSL
#define PN532_COMMAND_INDATAEXCHANGE (0x40)        ///< Data exchange
#define PN532_COMMAND_INCOMMUNICATETHRU (0x42)     ///< Communicate through
#define PN532_COMMAND_INDESELECT (0x44)            ///< Deselect
#define PN532_COMMAND_INRELEASE (0x52)             ///< Release
#define PN532_COMMAND_INSELECT (0x54)              ///< Select
#define PN532_COMMAND_INAUTOPOLL (0x60)            ///< Auto poll
#define PN532_COMMAND_TGINITASTARGET (0x8C)        ///< Init as target
#define PN532_COMMAND_TGSETGENERALBYTES (0x92)     ///< Set general bytes
#define PN532_COMMAND_TGGETDATA (0x86)             ///< Get data
#define PN532_COMMAND_TGSETDATA (0x8E)             ///< Set data
#define PN532_COMMAND_TGSETMETADATA (0x94)         ///< Set metadata
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88) ///< Get initiator command
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90) ///< Response to initiator
#define PN532_COMMAND_TGGETTARGETSTATUS (0x8A)     ///< Get target status

#define PN532_RESPONSE_INDATAEXCHANGE (0x41)      ///< Data exchange
#define PN532_RESPONSE_INLISTPASSIVETARGET (0x4B) ///< List passive target

#define PN532_WAKEUP (0x55) ///< Wake

#define PN532_I2C_ADDRESS (0x48 >> 1) ///< Default I2C address
#define PN532_I2C_READBIT (0x01)      ///< Read bit
#define PN532_I2C_BUSY (0x00)         ///< Busy
#define PN532_I2C_READY (0x01)        ///< Ready
#define PN532_I2C_READYTIMEOUT (20)   ///< Ready timeout

#define PN532_BAUD_ISO14443A (0x00) ///< Most common card rate in the US
#define PN532_BAUD_ISO14443B (0x03)

// NXP Commands
#define NXP_CMD_AUTH_A (0x60)      ///< Auth A
#define NXP_CMD_GET_VERSION (0x60) /// Also GET_VERSION
#define NXP_CMD_AUTH_B (0x61)      ///< Auth B
#define NXP_CMD_READ (0x30)        ///< Read
#define NXP_CMD_FASTREAD (0x3A)
#define NXP_CMD_READ_CNT (0x39)
#define NXP_CMD_WRITE (0xA0)            ///< Write
#define NXP_CMD_TRANSFER (0xB0)         ///< Transfer
#define NXP_CMD_DECREMENT (0xC0)        ///< Decrement
#define NXP_CMD_INCREMENT (0xC1)        ///< Increment
#define NXP_CMD_STORE (0xC2)            ///< Store
#define NXP_ULTRALIGHT_CMD_WRITE (0xA2) ///< Write (NXP Ultralight)

// PN532 Underlying commands
/**
 * @brief Retrieves the firmware version of the PN532 in Major.minor format.
 *
 * This function sends a command to the PN532 to request the firmware version.
 * It expects the response to contain version information, which is returned as
 * a float representing the Major.minor version (e.g., 1.6).
 *
 * @param config Pointer to the `PicoNFCConfig` structure containing I2C and buffer information.
 * @return Firmware version as a float (e.g., 1.6) if successful; returns -1.0 if there is an error.
 */
float piconfc_PN532_firmwareVersion(PicoNFCConfig *config);

/**
 * @brief Initiates a self-test of the PN532 RF transceiver.
 *
 * This function sends the RF Regulation Test command to the PN532, which performs
 * a self-test of the RF transceiver. The test runs continuously until another command
 * is sent to the PN532 to halt it. It waits for the device to be ready and checks for
 * an acknowledgment (ACK) from the PN532.
 *
 * @param config Pointer to the `PicoNFCConfig` structure containing I2C and buffer information.
 * @return True if the self-test was successfully initiated; false if there was a communication error.
 */
bool piconfc_PN532_RFRegulationTest(PicoNFCConfig *config);

/**
 * @brief Configures the Secure Access Module (SAM) in the PN532.
 *
 * This function sends the SAM Configuration command to the PN532, which is required
 * for reliable operation of the antenna. It sets the SAM to normal mode, with a timeout
 * of 1 second, and disables the IRQ pin. The function then waits for the device to respond
 * with an acknowledgment and verifies the response to confirm successful configuration.
 *
 * @param config Pointer to the `PicoNFCConfig` structure containing I2C and buffer information.
 * @return True if the SAM configuration was successful; false if there was a communication error
 *         or if the response was incorrect.
 */
bool piconfc_PN532_SAMConfiguration(PicoNFCConfig *config);

/**
 * @brief Sets the number of passive activation retries for the PN532.
 *
 * This function configures the PN532's passive activation retry settings, which define
 * the number of attempts the module makes to activate a card in the detection field.
 * Setting the retries to 0xFF makes the PN532 retry indefinitely until a card is found
 * or the operation is aborted. The maximum configurable retries value is 0x10.
 *
 * @param config Pointer to the `PicoNFCConfig` structure containing I2C and buffer information.
 * @param retries Number of retries for passive activation (0xFF for unlimited retries).
 * @return True if the command was acknowledged and the response length is valid; false otherwise.
 */
bool piconfc_PN532_setPassiveActivationRetries(PicoNFCConfig *config, uint8_t retries);

/**
 * @brief Waits for an NFC card to enter the detection field and reads its unique ID (UID).
 *
 * This function sends the InListPassiveTarget command to the PN532, which waits for an NFC
 * card to enter the detection field. It then reads the UID of the card, which is required
 * for further communication. The function blocks until a card is detected or the specified
 * timeout period is reached.
 *
 * @param config Pointer to the `PicoNFCConfig` structure containing I2C and buffer information.
 * @param baudrate Baud rate selector for passive target detection (e.g., 0x00 for 106 kbps).
 * @param uid Pointer to a buffer where the card's UID will be stored.
 * @param uid_len Pointer to a variable where the length of the UID will be stored.
 * @param timeout Maximum time to wait for a card in milliseconds.
 * @return True if a card was successfully detected and the UID was read; false if no card was
 *         detected or if there was a communication error.
 */
bool piconfc_PN532_readPassiveTargetID(PicoNFCConfig *config, uint8_t baudrate, uint8_t *uid, uint8_t *uid_len, uint16_t timeout);

/**
 * @brief Sends data to an already discovered NFC card and receives its response.
 *
 * This function sends data to an NFC card that has already been activated and is in the communication field.
 * It uses the PN532's InDataExchange command to perform data exchange with the card. The function waits
 * for an acknowledgment, then retrieves the response and checks for any errors. The response data is copied
 * into the `receive` buffer, and the length of the received data is stored in `received_length`.
 *
 * @param config Pointer to the `PicoNFCConfig` structure containing I2C and buffer information.
 * @param send Pointer to the buffer containing data to send to the NFC card.
 * @param sendlen Length of the data to send in bytes.
 * @param receive Pointer to the buffer where the received data will be stored.
 * @param received_length Pointer to a variable where the length of the received data will be stored.
 * @param rbuf_size Size of the receive buffer in bytes.
 * @return True if data exchange was successful and the response was received; false if there was a communication error or if the response indicated an error.
 */
bool piconfc_PN532_initiatorDataExchange(PicoNFCConfig *config, uint8_t *send, uint8_t sendlen, uint8_t *receive, uint8_t *received_length, uint8_t rbuf_size);

#endif /* PN532_H */