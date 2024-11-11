/**
 * @file piconfc_NTAG.h
 * @brief Header file for NTAG operations using the PN532 NFC module.
 *
 * This header defines functions and constants to facilitate reading from and writing to NTAG21X NFC tags (NTAG213, NTAG215, and NTAG216)
 * using the PN532 NFC module. These functions allow interaction with NTAG tags, including reading and writing specific pages,
 * determining tag model, and managing user-accessible data.
 *
 */

#ifndef NTAG_H
#define NTAG_H

#include "piconfc.h"

/**
 * @brief The size of an NTAG page in bytes.
 */
#define NTAG_PAGE_SIZE (0x04) // Bytes

/**
 * @enum NTAG21X
 * @brief Enumeration for the supported NTAG models.
 *
 * The NTAG21X series includes multiple models with different storage capacities and page limits.
 * This enum represents the NTAG models recognized by this library, each with a unique identifier
 * found on a specific tag page.
 *
 * @var MODEL_NTAG213 NTAG213 model, identified by value 0x12.
 * @var MODEL_NTAG215 NTAG215 model, identified by value 0x3E.
 * @var MODEL_NTAG216 NTAG216 model, identified by value 0x6D.
 */
enum NTAG21X {
    MODEL_NTAG213 = 0x12,
    MODEL_NTAG215 = 0x3E,
    MODEL_NTAG216 = 0x6D
};

/**
 * @brief Determines the NTAG model type (NTAG213, NTAG215, or NTAG216).
 *
 * This function reads a specific page (page 0x03) to identify the model of the NTAG tag,
 * which is essential for determining the correct number of pages available on the tag.
 *
 * @param config Pointer to the PicoNFCConfig structure containing configuration details.
 * @return Enum value of type NTAG21X indicating the tag model (NTAG213, NTAG215, or NTAG216),
 *         or 0 if the read operation fails.
 */
enum NTAG21X piconfc_NTAG_getModel(PicoNFCConfig *config);

/**
 * @brief Reads a single 4-byte page from the NTAG tag into the provided buffer.
 *
 * This function reads one page (4 bytes) from the specified page address on the NTAG tag
 * and stores it in the provided buffer. The function assumes that the buffer has enough
 * space to hold 4 bytes. It relies on `piconfc_NTAG_read4Pages` to read the data and
 * then copies only the first 4 bytes.
 *
 * @param config Pointer to the PicoNFCConfig structure containing configuration details.
 * @param page Page address to read from.
 * @param buffer Pointer to the buffer where the 4-byte page will be stored. Must be at least 4 bytes in size.
 * @return True if the page was read successfully; false otherwise.
 */
bool piconfc_NTAG_read1Page(PicoNFCConfig *config, uint8_t page, uint8_t *buffer);

/**
 * @brief Reads 4 consecutive pages (16 bytes) from the NTAG tag into the provided buffer.
 *
 * This function sends a command to read 4 sequential pages (16 bytes) starting from the specified
 * `startpage` on the NTAG tag. It uses `piconfc_PN532_initiatorDataExchange` to handle the data
 * exchange, and the function checks if the received data length matches the expected 16 bytes.
 *
 * @param config Pointer to the PicoNFCConfig structure containing configuration details.
 * @param startpage The starting page address for the read operation.
 * @param buffer Pointer to the buffer where the 16-byte data will be stored. Must be at least 16 bytes in size.
 * @return True if 16 bytes were read successfully; false otherwise.
 */
bool piconfc_NTAG_read4Pages(PicoNFCConfig *config, uint8_t startpage, uint8_t *buffer);

// NOT WORKING PENDING CRC Development. Can't send non NXP standard commands thru indataexchange. EVENTHOUGH NXP MADE THE STANDARD AND THE TAG!
// Reads all pages from startpage to stoppage inclusive and returns the number of bytes read into buffer. bufsize exists to prevent overflow.
int piconfc_NTAG_fastReadPages(PicoNFCConfig *config, uint8_t startpage, uint8_t stoppage, uint8_t *buffer, unsigned int bufsize);

/**
 * @brief Reads all user-accessible pages from an NTAG tag into the provided buffer, preventing overflow.
 *
 * This function reads user pages starting from page 4 up to the last user page, based on the detected NTAG model.
 * It retrieves pages in blocks of 4 (16 bytes) and stores them in the provided buffer. If the buffer size is
 * insufficient, the function stops reading to prevent overflow. A buffer size of at least 888 bytes is recommended
 * for NTAG216.
 *
 * @param config Pointer to the PicoNFCConfig structure containing I2C and buffer information.
 * @param buffer Pointer to the buffer where the data will be stored. Should be large enough for the expected data size.
 * @param bufsize Size of the buffer in bytes to prevent overflow.
 * @return The total number of bytes read and stored in the buffer. Returns 0 if the model is unknown or if no pages are read.
 */
int piconfc_NTAG_readUserPages(PicoNFCConfig *config, uint8_t *buffer, unsigned int bufsize);

/**
 * @brief Writes a 4-byte data block into a specified page of the NTAG.
 *
 * This function sends a command to write 4 bytes of data into a specific page of an NTAG chip.
 * The command format follows the NTAG protocol, with `NXP_CMD_WRITE` and the target page address.
 * It uses `piconfc_PN532_initiatorDataExchange` to transmit the data and does not check the response contents.
 * The function assumes the `buffer` contains at least 4 bytes of data to write.
 *
 * @param config Pointer to the PicoNFCConfig structure containing I2C and buffer information.
 * @param page Target page address on the NTAG chip to write data.
 * @param buffer Pointer to the 4-byte data buffer to be written to the NTAG page.
 * @return True if the data was successfully written to the specified page; false otherwise.
 */
bool piconfc_NTAG_writePage(PicoNFCConfig *config, uint8_t page, uint8_t *buffer);

/**
 * @brief Writes data from the buffer to the NTAG user pages, up to the buffer size or the max NTAG page limit.
 *
 * This function replaces the contents of the NTAG user pages starting from page 4 with the data provided in the buffer.
 * It iterates through each page, writing 4 bytes at a time, until it reaches the end of the buffer or the NTAG's maximum
 * user page limit, based on the detected NTAG model (NTAG213, NTAG215, or NTAG216). The function stops and returns false
 * if the buffer size is insufficient to write a full 4-byte page at any iteration.
 *
 * @param config Pointer to the `PicoNFCConfig` structure containing I2C and buffer information.
 * @param buffer Pointer to the data buffer to write to the NTAG user pages.
 * @param bufsize Size of the data buffer in bytes.
 * @return True if the entire buffer was written to the NTAG; false if there was insufficient data or a write error.
 */
bool piconfc_NTAG_writeUserData(PicoNFCConfig *config, uint8_t * buffer, unsigned int bufsize);

#endif /* NTAG_H */