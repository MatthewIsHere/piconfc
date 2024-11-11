#include "piconfc_PN532.h"
#include "piconfc_NDEF.h"
#include "piconfc_NTAG.h"

#ifndef PICONFC_H
#define PICONFC_H

typedef struct {
    i2c_inst_t *i2c_block;
    uint8_t scratch[1024];
} PicoNFCConfig;

bool piconfc_init(PicoNFCConfig* empty_config, i2c_inst_t *block, int sda_pin, int scl_pin);
// Wait timeout (ms) for NFC tag.
// retval -1: error reading, -2: not NTAG21X, 0: no card found before timeout, 1: successful
bool piconfc_readNTAG(PicoNFCConfig *config, int timeout_ms, char ** string_ptr);
bool piconfc_tagPresent(PicoNFCConfig *config);

// NFC tag read url
// NFC tag write url

#endif /* PICONFC_H */