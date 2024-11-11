#include "piconfc_PN532.h"
#include "piconfc_NDEF.h"
#include "piconfc_NTAG.h"

#ifndef PICONFC_H
#define PICONFC_H

typedef struct {
    i2c_inst_t *i2c_block;
    uint8_t scratch[1024];
} PicoNFCConfig;

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
bool piconfc_init(PicoNFCConfig *empty_config, i2c_inst_t *i2c_block, int sda_pin, int scl_pin);

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
bool piconfc_readNTAG(PicoNFCConfig *config, int timeout_ms, char **string_ptr);

/**
 * @brief Checks if an NFC tag is present within the read range.
 *
 * This function attempts to detect an NFC tag within range using ISO14443A protocol.
 * It reads the UID of the tag if present, but does not store it beyond the scope
 * of this check.
 *
 * @param config Pointer to the PicoNFCConfig structure containing the NFC configuration.
 * @param delay_ms Time to wait for tag. 500ms is recommended.
 * @return True if an NFC tag is detected, false otherwise.
 */
bool piconfc_tagPresent(PicoNFCConfig *config, int delay_ms);

// NFC tag read url
// NFC tag write url

#endif /* PICONFC_H */