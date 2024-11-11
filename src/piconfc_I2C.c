#include <stdio.h>
#include <string.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "piconfc_I2C.h"
#include "piconfc_PN532.h"

const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

void piconfc_I2C_init(i2c_inst_t *block, uint8_t sda_pin, uint8_t scl_pin) {
    // Initialize the I2C instance with the specified frequency
    i2c_init(block, PICONFC_I2C_FREQ);

    // Configure SDA and SCL pins for I2C functionality
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    // Enable pull-up resistors on SDA and SCL, required for I2C
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

bool piconfc_I2C_isready(i2c_inst_t* block) {
    uint8_t rdy;
    // Read one byte from the PN532 to check if it's ready
    i2c_read_blocking(block, PN532_I2C_ADDRESS, &rdy, 1, false);
    
    // Return true if the byte matches the ready indicator
    return rdy == PN532_I2C_READY;
}

bool piconfc_I2C_waitready(i2c_inst_t* block, int timeout) {
    int timer = 0;

    // Loop until PN532 is ready or the timeout is reached
    while (!piconfc_I2C_isready(block)) {
        // Check if a timeout is specified
        if (timeout != 0) {
            timer += 1;
            // Exit if the timer exceeds the timeout value
            if (timer > timeout) {
                #ifdef I2C_DEBUG
                    printf("TIMEOUT after %d ms\n", timer);
                #endif
                return false;
            }
        }
        // Wait for 1 ms before checking again
        sleep_ms(1);
    }
    return true;
}

bool piconfc_I2C_sendcommand_andack(i2c_inst_t* block, uint8_t * cmd, uint8_t len, int timeout) {

    // Send command packet to the PN532
    piconfc_I2C_writecommand(block, cmd, len);

    // Wait for the device to be ready before reading the ACK
    if (!piconfc_I2C_waitready(block, timeout)) return false;

    // Brief delay to allow the device to process the command
    sleep_ms(1);

    // Check if the ACK was received
    if (!piconfc_I2C_readack(block)) {
        return false;
    }

    // Brief delay to allow the device to process the command
    sleep_ms(1);

    // Wait for the device to be ready again after the ACK
    if (!piconfc_I2C_waitready(block, timeout)) {
        return false;
    }

    return true;
}

bool piconfc_I2C_readack(i2c_inst_t* block) {
    uint8_t ackbuf[sizeof(PN532_ACK)];
    
    // Read data into ackbuf and check if it matches the ACK pattern
    piconfc_I2C_readdata(block, ackbuf, sizeof(ackbuf));
    return (memcmp((char *)ackbuf, (char *)PN532_ACK, sizeof(PN532_ACK)) == 0);
}

void piconfc_I2C_readdata(i2c_inst_t* block, uint8_t * buffer, uint8_t len) {
    uint8_t rbuff[len + 1]; // +1 for leading RDY byte

    // Read len + 1 bytes from PN532 (first byte is RDY, remaining are data)
    i2c_read_blocking(block, PN532_I2C_ADDRESS, rbuff, len + 1, true);

    // Copy data from rbuff, skipping the first byte
    for (uint8_t i = 0; i < len; i++) {
        buffer[i] = rbuff[i + 1];
    }

    #ifdef I2C_DEBUG
        printf("readdata: ");
        printhex(rbuff, len + 1);
    #endif
}

void piconfc_I2C_writecommand(i2c_inst_t* block, uint8_t * cmd, uint8_t cmdlen) {
    uint8_t packet[8 + cmdlen];
    uint8_t data_len = cmdlen + 1;

    // Construct the packet header
    packet[0] = PN532_PREAMBLE;
    packet[1] = PN532_STARTCODE1;
    packet[2] = PN532_STARTCODE2;
    packet[3] = data_len;                     // Data length (command length + 1)
    packet[4] = ~data_len + 1;                // Length checksum, must sum to 0x00
    packet[5] = PN532_HOSTTOPN532;            // Direction byte (Host to PN532)

    uint8_t checksum_val = 0;

    // Copy command data and calculate checksum
    for (int i = 0; i < cmdlen; i++) {
        packet[6 + i] = cmd[i];
        checksum_val += cmd[i];
    }

    // Finalize the packet with data checksum and postamble
    packet[6 + cmdlen] = ~(PN532_HOSTTOPN532 + checksum_val) + 1; // Checksum for direction and data
    packet[7 + cmdlen] = PN532_POSTAMBLE; // Postamble byte

    // Send the packet over I2C
    i2c_write_blocking(block, PN532_I2C_ADDRESS, packet, 8 + cmdlen, false);

    #ifdef I2C_DEBUG
        printf("wrote: ");
        printhex(packet, 8 + cmdlen);
    #endif
}

uint8_t piconfc_I2C_parseresponse(i2c_inst_t *block, uint8_t *buffer, uint8_t expected_data_len) {
    // Read the response from the PN532 into the buffer
    piconfc_I2C_readdata(block, buffer, 8 + expected_data_len);

    // Validate the preamble and start codes
    if (buffer[0] != PN532_PREAMBLE || buffer[1] != PN532_STARTCODE1 || buffer[2] != PN532_STARTCODE2) {
        #ifdef I2C_DEBUG
            printf("Validate failed Preamble check!\n");
        #endif
        return 0;
    }

    // Validate length and length checksum
    uint8_t len = buffer[3];
    uint8_t checksum = len + buffer[4];
    if (checksum != 0) {
        #ifdef I2C_DEBUG
            printf("Validate failed length checksum!\n");
        #endif
        return 0;
    }

    // Validate data direction and calculate data checksum
    uint8_t direction = buffer[5];
    uint8_t sum = 0 + direction;
    for (int i = 1; i < len; i++) {
        buffer[i - 1] = buffer[5 + i];
        sum += buffer[5 + i];
    }
    sum += buffer[5 + len];
    if (sum != 0) {
        #ifdef I2C_DEBUG
            printf("Validate failed data checksum!\n");
        #endif
        return 0;
    }

    // Return the length of the data, excluding the PN532 indicator
    return len - 1;
}

// Does what it says
void printhex(uint8_t* buff, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02X ", buff[i]);
    }
    printf("\n");
}