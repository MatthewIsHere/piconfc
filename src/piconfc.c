#include "pico/stdlib.h"
#include <stdlib.h>
#include "piconfc.h"
#include "piconfc_I2C.h"

bool piconfc_init(PicoNFCConfig *empty_config, i2c_inst_t *i2c_block, int sda_pin, int scl_pin) {
    empty_config->i2c_block = i2c_block;                 // Set the I2C instance in the config structure
    piconfc_I2C_init(i2c_block, sda_pin, scl_pin);       // Initialize I2C with specified pins
    return piconfc_PN532_SAMConfiguration(empty_config); // Configure the NFC module's SAM
}

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
    int num_records = piconfc_NDEF_parseMessage(tlv.value_ptr, tlv.value_length, &records);
    if (num_records < 1) {
        free(records); // Free records array if parsing failed
        return false;
    }

    // Read the payload of the first NDEF record into the output string
    bool success = piconfc_NDEF_readPayloadString(&records[0], string_ptr);
    free(records); // Free the records array after use

    return success;
}

bool piconfc_tagPresent(PicoNFCConfig *config, int delay_ms) {
    uint8_t uid[7] = { 0 };       // Array to hold the UID if a tag is found
    uint8_t uid_len = 0;          // Variable to store the length of the UID
    // Attempt to read the UID of a tag within range using the specified configuration
    return piconfc_PN532_readPassiveTargetID(config, PN532_BAUD_ISO14443A, uid, &uid_len, delay_ms);
}
