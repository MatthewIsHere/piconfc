#include "piconfc_NTAG.h"
#include "piconfc_PN532.h"
#include "piconfc.h"
#include <string.h>

enum NTAG21X piconfc_NTAG_getModel(PicoNFCConfig *config) {
    uint8_t rbuf[4];
    bool success = piconfc_NTAG_read1Page(config, 0x03, rbuf);
    if (!success) return 0; // Return 0 if unable to read the page, indicating an unknown model.
    return rbuf[2]; // Return the value in rbuf[2] which identifies the NTAG model.
}

bool piconfc_NTAG_read1Page(PicoNFCConfig *config, uint8_t page, uint8_t *buffer) {
    bool success = piconfc_NTAG_read4Pages(config, page, config->scratch);
    if (success) {
        memcpy(buffer, config->scratch, 4); // Copy only the first 4 bytes from scratch to buffer
        return true;
    }
    return false; // Return false if the read operation fails
}

bool piconfc_NTAG_read4Pages(PicoNFCConfig *config, uint8_t startpage, uint8_t *buffer) {
    uint8_t cmdbuf[] = {
        NXP_CMD_READ,   // Command to initiate read
        startpage       // Starting page to read from
    };
    uint8_t rlen = 0;
    bool success = piconfc_PN532_initiatorDataExchange(config, cmdbuf, sizeof(cmdbuf), buffer, &rlen, 16);
    bool retval = success && rlen == 16; // Confirm 16 bytes were read

    #ifdef NTAG_DEBUG
        printf("read4pages retval: %d. rlen: %d\n", retval, rlen); // Debug output
    #endif
    
    return retval;
}


// NOT WORKING PENDING CRC Development. Can't send non NXP standard commands thru indataexchange. EVENTHOUGH NXP MADE THE STANDARD AND THE TAG!
// Reads all pages from startpage to stoppage inclusive and returns the number of bytes read into buffer. bufsize exists to prevent overflow.
int piconfc_NTAG_fastReadPages(PicoNFCConfig *config, uint8_t startpage, uint8_t stoppage, uint8_t *buffer, unsigned int bufsize) {
    if (stoppage <= startpage) return 0;

    uint8_t cmdbuf[] = {
        NXP_CMD_FASTREAD,
        startpage, 
        stoppage
    };
    int pages = stoppage - startpage + 1;

    uint8_t rlen = 0;
    bool success = piconfc_PN532_initiatorDataExchange(config, cmdbuf, sizeof(cmdbuf), config->scratch, &rlen, pages * NTAG_PAGE_SIZE );

    if (success) {
        for (int i = 0; i < rlen; i++) {
            if (i == bufsize) break;
            buffer[i] = config->scratch[i];
        }
        return rlen;
    } else {
        return 0;
    }
}

int piconfc_NTAG_readUserPages(PicoNFCConfig *config, uint8_t *buffer, unsigned int bufsize) {
    enum NTAG21X model = piconfc_NTAG_getModel(config); // Identify the NTAG model to set read limits
    int head = 0;

    #ifdef NTAG_DEBUG
        printf("readingUserpages. model: %d\n", model);
    #endif
    
    uint8_t end_userpages = 0;
    switch (model) {
        case MODEL_NTAG213:
            end_userpages = 0x27; // End page for NTAG213
            break;
        case MODEL_NTAG215:
            end_userpages = 0x81; // End page for NTAG215
            break;
        case MODEL_NTAG216:
            end_userpages = 0xE1; // End page for NTAG216
            break;
        default:
            return 0; // Unknown model, exit with 0 bytes read
    }

    for (int page = 4; page < end_userpages; page += 4) {
        if (head + 16 >= bufsize) break; // Prevent buffer overflow
        bool result = piconfc_NTAG_read4Pages(config, page, buffer + head); // Read 4 pages (16 bytes)
        if (!result) break; // Stop reading if read fails
        head += 16; // Increment head position in buffer by 16 bytes
    }
    return head; // Total bytes read
}

bool piconfc_NTAG_writePage(PicoNFCConfig *config, uint8_t page, uint8_t *buffer) {
    uint8_t cmdbuf[] = {
        NXP_CMD_WRITE,   // NTAG write command
        page,            // Target page address
        buffer[0],       // First byte of data
        buffer[1],       // Second byte of data
        buffer[2],       // Third byte of data
        buffer[3]        // Fourth byte of data
    };
    
    uint8_t retbuf[8]; // Temporary buffer for response (likely unnecessary)
    uint8_t rlen = 0;
    bool success = piconfc_PN532_initiatorDataExchange(config, cmdbuf, sizeof(cmdbuf), retbuf, &rlen, 0);
    return success; // Return success status of the write operation
}

bool piconfc_NTAG_writeUserData(PicoNFCConfig *config, uint8_t * buffer, unsigned int bufsize) {
    enum NTAG21X model = piconfc_NTAG_getModel(config);
    int head = 0;

    if (model == 0x00) model = MODEL_NTAG213; // Default to NTAG213 if model detection fails
    uint8_t end_userpages = 0;
    switch (model) {
        case MODEL_NTAG213:
            end_userpages = 0x27; // End page for NTAG213
            break;
        case MODEL_NTAG215:
            end_userpages = 0x81; // End page for NTAG215
            break;
        case MODEL_NTAG216:
            end_userpages = 0xE1; // End page for NTAG216
            break;
    }
    
    // Write data starting from page 4 up to the end_userpages limit
    for (int i = 4; i < end_userpages; i++) {
        if (head + 4 > bufsize) return false; // Insufficient buffer data to write 4 bytes
        bool result = piconfc_NTAG_writePage(config, i, buffer+head); // Write 4 bytes to the current page
        if (!result) return false; // Return false if write fails
        head += 4; // Move to the next 4 bytes in the buffer
    }
    return true; // Return true if all pages were written successfully
}