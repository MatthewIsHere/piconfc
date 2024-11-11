#include "pico/stdlib.h"
#include "piconfc.h"
#include "piconfc_I2C.h"

/**
 * @brief Initializes the Pico NFC configuration with the specified I2C block and pins.
 *
 * This function initializes the Pico NFC configuration by setting the I2C block,
 * configuring the specified SDA and SCL pins for I2C communication, and performing
 * the necessary SAM (Secure Access Module) configuration for the NFC module.
 *
 * @param empty_config Pointer to an empty PicoNFCConfig structure to be initialized.
 * @param i2c_block Pointer to the I2C instance (e.g., `i2c0` or `i2c1`).
 * @param sda_pin GPIO pin number for the I2C SDA line.
 * @param scl_pin GPIO pin number for the I2C SCL line.
 * @return True if the SAM configuration was successful; false otherwise.
 */
bool piconfc_init(PicoNFCConfig *empty_config, i2c_inst_t *i2c_block, int sda_pin, int scl_pin) {
    empty_config->i2c_block = i2c_block;                 // Set the I2C instance in the config structure
    piconfc_I2C_init(i2c_block, sda_pin, scl_pin);       // Initialize I2C with specified pins
    return piconfc_PN532_SAMConfiguration(empty_config); // Configure the NFC module's SAM
}

// static bool last = false;
/**
 * @brief Reads an NTAG and retrieves its payload as a string.
 *
 * This function reads data from an NTAG NFC tag, parses the NDEF message, and retrieves the
 * payload from the first NDEF record as a dynamically allocated string. The caller is 
 * responsible for freeing the allocated string once it is no longer needed.
 *
 * @param config Pointer to the PicoNFCConfig structure containing the NFC configuration.
 * @param timeout_ms Timeout in milliseconds to wait for the tag.
 * @param string_ptr Pointer to a char pointer where the resulting string will be stored.
 *                   The caller is responsible for freeing this string if the function succeeds.
 * @return True if a valid NDEF message was read and the payload was successfully retrieved;
 *         false otherwise.
 */
bool piconfc_readNTAG(PicoNFCConfig *config, int timeout_ms, char **string_ptr) {
    uint8_t uid[7] = { 0 };
    uint8_t uid_len = 0;

    // Attempt to detect an NFC tag within range
    bool found = piconfc_PN532_readPassiveTargetID(config, PN532_BAUD_ISO14443A, uid, &uid_len, timeout_ms);
    if (!found) return false;

    static uint8_t readbuf[888];
    // Read user pages from the detected tag into a static buffer
    int memlen = piconfc_NTAG_readUserPages(config, readbuf, sizeof(readbuf));
    if (memlen == 0) return false;

    struct TLV tlv;
    // Parse the TLV structure from the data read
    bool valid = piconfc_NDEF_parseTLV(&tlv, readbuf, memlen, 0);
    if (!valid) return false;

    NDEFRecord *records = NULL;
    // Parse the NDEF message into individual records
    int num_records = piconfc_NDEF_parseMessage(tlv.buffer, tlv.buffer_len, &records);
    if (num_records < 1) {
        free(records); // Free records array if parsing failed
        return false;
    }

    // Read the payload of the first NDEF record into the output string
    bool success = piconfc_NDEF_readPayloadString(&records[0], string_ptr);
    free(records); // Free the records array after use

    return success;
}

/**
 * @brief Checks if an NFC tag is present within the read range.
 *
 * This function attempts to detect an NFC tag within range using ISO14443A protocol.
 * It reads the UID of the tag if present, but does not store it beyond the scope
 * of this check.
 *
 * @param config Pointer to the PicoNFCConfig structure containing the NFC configuration.
 * @return True if an NFC tag is detected, false otherwise.
 */
bool piconfc_tagPresent(PicoNFCConfig *config) {
    uint8_t uid[7] = { 0 };       // Array to hold the UID if a tag is found
    uint8_t uid_len = 0;          // Variable to store the length of the UID
    // Attempt to read the UID of a tag within range using the specified configuration
    return piconfc_PN532_readPassiveTargetID(config, PN532_BAUD_ISO14443A, uid, &uid_len, 500);
}
