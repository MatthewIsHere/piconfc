#include "piconfc_NDEF.h"
#include <stdlib.h>
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>

#ifdef NDEF_DEBUG
    #include "piconfc_I2C.h"
#endif

const char *URIPrefixes[] = {
    "", "http://www.", "https://www.", "http://", "https://", "tel:",
    "mailto:", "ftp://anonymous:anonymous@", "ftp://ftp.", "ftps://",
    "sftp://", "smb://", "nfs://", "ftp://", "dav://", "news:",
    "telnet://", "imap:", "rtsp://", "urn:", "pop:", "sip:", "sips:",
    "tftp:", "btspp://", "btl2cap://", "btgoep://", "tcpobex://",
    "irdaobex://", "file://", "urn:epc:id:", "urn:epc:tag:",
    "urn:epc:pat:", "urn:epc:raw:", "urn:epc:", "urn:nfc:"
};

bool piconfc_NDEF_parseTLV(struct TLV *empty_TLV, uint8_t *buffer, int bufsize, int start) {
    empty_TLV->buffer = buffer;
    empty_TLV->buffer_len = bufsize;
    int head = -1;

    // Search for the TLV tag (0x03) starting from the specified offset
    for (int i = start; i < bufsize; i++) {
        if (buffer[i] == 0x03) { // Data tag for the NDEF message
            head = i;
            break;
        }
    }
    if (head == -1) // Tag not found
        return false;

    // Check if there’s enough space for the length field after the tag
    if (head + 2 > bufsize) return false;

    // Determine the length of the value based on the length encoding
    if (buffer[head + 1] == 0xFF) {
        // 3-byte length encoding (extended format)
        uint16_t len = buffer[head + 2];
        len <<= 8;
        len |= buffer[head + 3];
        empty_TLV->value_length = len;
        empty_TLV->value_offset = head + 4;

        // Validate that the value is correctly terminated with 0xFE
        if (buffer[empty_TLV->value_offset + empty_TLV->value_length] != 0xFE)
            return false;
    }
    else {
        // 1-byte length encoding (standard format)
        empty_TLV->value_length = buffer[head + 1];
        empty_TLV->value_offset = head + 2;

        // Validate that the value is correctly terminated with 0xFE
        if (buffer[empty_TLV->value_offset + empty_TLV->value_length] != 0xFE) {
            // If the TLV was invalid, retry parsing from the next byte after the tag
            return piconfc_NDEF_parseTLV(empty_TLV, buffer, bufsize, head + 1);
        }
    }

    // Set pointer to the start of the value within the buffer
    empty_TLV->value_ptr = empty_TLV->buffer + empty_TLV->value_offset;

    #ifdef NDEF_DEBUG
        printf("Parsed TLV: ");
        printhex(empty_TLV->value_ptr, empty_TLV->value_length); // Print parsed data if debugging
    #endif

    return true;
}

int piconfc_NDEF_encodeTLV(uint8_t *data, uint16_t datasize, uint8_t *result, int resultsize)
{
    // Ensure the result buffer is large enough to hold the encoded packet
    if (datasize + 5 > resultsize)
        return 0;

    int i = 0;
    result[i++] = 0x03; // Start with the TLV tag (0x03 for NDEF data)

    // Encode the length of the data
    if (datasize >= 0xFF) {
        // Use 3-byte length format if datasize exceeds 1 byte
        result[i++] = 0xFF;               // Indicates extended length format
        result[i++] = datasize >> 8;      // High byte of length
        result[i++] = datasize & 0x00FF;  // Low byte of length
    } else {
        // Use 1-byte length format for smaller data
        result[i++] = datasize;
    }

    // Copy the actual data into the result buffer
    for (int j = 0; j < datasize; j++) {
        result[i++] = data[j];
    }

    result[i++] = 0xFE; // Add terminator (0xFE)

    return i; // Return the length of the encoded TLV packet
}

int piconfc_NDEF_parseMessage(uint8_t *buffer, int bufsize, NDEFRecord **dest) {
    int expected_records = piconfc_NDEF_messageLen(buffer, bufsize);
    if (expected_records == 0) // Corrected condition to check if no records are found
        return 0;

    // Allocate memory for the array of NDEF records
    NDEFRecord *records = (NDEFRecord *)malloc(expected_records * sizeof(NDEFRecord));
    if (records == NULL) {
        return 0; // Return 0 if memory allocation fails
    }
    #ifdef NDEF_DEBUG
        printf("Allocated %d bytes for parseMessage\n", expected_records * sizeof(NDEFRecord));
    #endif

    // Parse each record and store it in the records array
    int records_parsed = 0;
    int offset = 0;
    while (offset <= bufsize && records_parsed < expected_records) {
        int new_offset = piconfc_NDEF_parseRecord(buffer, bufsize, offset, &records[records_parsed]);
        if (new_offset == -1) {
            break;
        }
        records_parsed++;
        offset = new_offset;
    }

    *dest = records; // Set the destination pointer to the records array
    #ifdef NDEF_DEBUG
        printf("made it out of parseMessage\n");
    #endif
    return records_parsed; // Return the number of records parsed
}

int piconfc_NDEF_messageLen(uint8_t *buffer, int bufsize) {
    int totalRecords = 0;

    // Check if buffer is empty or if the first record has the ME (Message End) flag set
    if (bufsize < 1 || (buffer[0] & 0x40)) {
        #ifdef NDEF_DEBUG
            printf("Found message end at first record. Returning 1\n");
        #endif
        return 1; // Return 1 if there's only one record with ME flag set
    }

    int i = 0;
    // Iterate through the buffer to count the records
    while (i < bufsize) {
        // Stop if the current record has the ME (Message End) flag set
        if (buffer[i] & 0x40) {
            break;
        }

        uint8_t flags = buffer[i++];
        bool short_record = flags & 0x10; // Check if it's a short record
        bool has_id = flags & 0x08;       // Check if the record has an ID field
        uint8_t type_len = buffer[i++];   // Length of the type field

        uint32_t payload_len = 0;
        // Determine payload length based on record format
        if (short_record) {
            payload_len = buffer[i++]; // 1-byte payload length for short records
        } else {
            // 4-byte payload length for standard records
            uint32_t len1 = buffer[i++];
            uint32_t len2 = buffer[i++];
            uint32_t len3 = buffer[i++];
            uint32_t len4 = buffer[i++];
            payload_len = (len1 << 24) | (len2 << 16) | (len3 << 8) | len4;
        }

        int id_len = 0;
        // Check if the record has an ID field and get its length
        if (has_id) {
            id_len = buffer[i++];
        }

        // Skip over the type field
        i += type_len;

        // Skip over the ID field if present
        i += id_len;

        // Skip over the payload
        i += payload_len;

        // Increment the total record count
            #ifdef NDEF_DEBUG
                    printf("Incremented total records to %d\n", totalRecords + 1);
            #endif
            totalRecords++;
    }
    return totalRecords;
}

int piconfc_NDEF_parseRecord(uint8_t *buffer, int bufsize, int offset, NDEFRecord *empty_record) {
    // Check if there is enough space in the buffer for the minimum record length
    if (offset + 4 >= bufsize) return -1;

    int ptr = offset;

    // Flags and TNF (Type Name Format) byte
    bool sr = buffer[ptr] & 0x10; // Short Record flag
    bool il = buffer[ptr] & 0x08; // ID Length flag
    enum TNF tnf = buffer[ptr] & 7; // Extract TNF (Type Name Format) value
    ptr += 1;

    // Type length field
    uint8_t payload_type_len = buffer[ptr];
    ptr += 1;

    // Determine payload length based on the Short Record (SR) flag
    int payload_data_len = 0;
    if (sr) {
        // 1-byte payload length for short records
        payload_data_len = buffer[ptr];
        ptr += 1;
    } else {
        // 4-byte payload length for standard records
        payload_data_len = (buffer[ptr] << 24) | (buffer[ptr + 1] << 16) | (buffer[ptr + 2] << 8) | buffer[ptr + 3];
        ptr += 4;
    }

    // If the IL (ID Length) flag is set, a 1-byte ID length field is present
    uint8_t payload_id_len = 0;
    if (il) {
        payload_id_len = buffer[ptr];
        ptr += 1;
    }

    // Set the type offset if type length is greater than zero
    int payload_type_offset = 0;
    if (payload_type_len > 0) {
        payload_type_offset = ptr;
        ptr += payload_type_len;
        // Check if buffer has enough space for the type field
        if (payload_type_offset + payload_type_len > bufsize) return -1;
    }

    // Set the ID offset if ID length is greater than zero
    int payload_id_offset = 0;
    if (payload_id_len > 0) {
        payload_id_offset = ptr;
        ptr += payload_id_len;
        // Check if buffer has enough space for the ID field
        if (payload_id_offset + payload_id_len > bufsize) return -1;
    }

    // Set the data offset if payload length is greater than zero
    int payload_data_offset = 0;
    if (payload_data_len > 0) {
        payload_data_offset = ptr;
        ptr += payload_data_len;
        // Check if buffer has enough space for the payload data
        if (payload_data_offset + payload_data_len > bufsize) return -1;
    }

    // Populate the NDEFRecord structure with parsed data and offsets
    empty_record->tnf = tnf;
    empty_record->buffer = buffer;
    empty_record->data_offset = payload_data_offset;
    empty_record->data_length = payload_data_len;
    empty_record->id_offset = payload_id_offset;
    empty_record->id_length = payload_id_len;
    empty_record->type_offset = payload_type_offset;
    empty_record->type_length = payload_type_len;

    // Return the updated offset after parsing this record
    return ptr;
}

bool piconfc_NDEF_createRecord(uint8_t **dest, unsigned int *recordlen, enum TNF tnf, uint8_t *type, uint8_t typelen, uint8_t *id, uint8_t idlen, uint8_t *payload, unsigned int payloadlen) {
    uint32_t len = 0;
    uint8_t flags = tnf; 
    len += sizeof(flags); // Account for flags byte
    len += sizeof(typelen); // Account for type length byte

    // Set SR (Short Record) flag if payload length fits in one byte, and add length accordingly
    if (payloadlen < 256) {
        flags |= 0x10; // SR flag
        len += sizeof(uint8_t); // 1-byte payload length
    } else {
        len += sizeof(uint32_t); // 4-byte payload length
    }

    // Set IL (ID Length) flag if ID is provided, and account for ID length byte
    if (idlen > 0) {
        flags |= 0x08; // IL flag
        len += sizeof(idlen); // 1-byte ID length
    }

    // Add the lengths of type, ID, and payload fields
    len += typelen;
    len += idlen;
    len += payloadlen;

    // Allocate memory for the record
    uint8_t* record = (uint8_t *)malloc(len);
    if (record == NULL) 
        return false; // Return false if memory allocation fails

    int i = 0;
    record[i++] = flags; // Set flags and TNF
    record[i++] = typelen; // Set type length

    // Set payload length based on short or standard format
    if (payloadlen < 256) {
        record[i++] = (uint8_t) payloadlen;
    } else {
        // Use 4 bytes to represent payload length for larger payloads
        record[i++] = (uint8_t)((payloadlen >> 24) & 0xFF);
        record[i++] = (uint8_t)((payloadlen >> 16) & 0xFF);
        record[i++] = (uint8_t)((payloadlen >> 8) & 0xFF);
        record[i++] = (uint8_t)(payloadlen & 0xFF);
    }

    // Set ID length if provided
    if (idlen > 0) {
        record[i++] = idlen;
    }

    // Copy type field data into the record
    if (typelen > 0) {
        memcpy(record + i, type, typelen);
        i += typelen;
    }

    // Copy ID field data into the record if present
    if (idlen > 0) {
        memcpy(record + i, id, idlen);
        i += idlen;
    }

    // Copy payload data into the record
    memcpy(record + i, payload, payloadlen);
    i += payloadlen;

    *dest = record;       // Set the destination pointer to the created record
    *recordlen = len;     // Set the record length

    return true;
}

bool piconfc_NDEF_readMIMEString(NDEFRecord *record, char **string) {
    // Check if the record's TNF (Type Name Format) indicates a MIME type
    if (record->tnf != TNF_MIME) return false;

    // Allocate memory for the MIME type string (+1 for the null terminator)
    *string = malloc(record->type_length + 1);
    if (*string == NULL) return false; // Return false if memory allocation fails

    // Copy the MIME type data from the record buffer to the allocated string
    memcpy(*string, record->buffer + record->type_offset, record->type_length);
    (*string)[record->type_length] = 0x00; // Null-terminate the string

    return true;
}

bool piconfc_NDEF_readPayloadString(NDEFRecord *record, char **string) {
    uint8_t prefix_id = 0x00;
    int prefix_len = 0x00;
    const char *prefix = NULL;

    // Check if the record's TNF indicates a URI and its type is 'U' (URI identifier code)
    if (record->tnf == TNF_WELLKNOWN && record->type_length == 1 && record->buffer[record->type_offset] == 'U') {
        // Verify that the prefix ID is within a valid range
        if (record->buffer[record->data_offset] >= 36) return false; // Invalid prefix ID
        prefix_id = record->buffer[record->data_offset];
        prefix = URIPrefixes[prefix_id];
        prefix_len = strlen(prefix);
    }

    // Calculate the total length needed for the final URI string, including prefix (if present)
    int total_length = prefix_len + record->data_length;
    if (prefix_len > 0) {
        total_length -= 1; // Adjust length to exclude the prefix byte in data
    }

    // Allocate memory for the complete URI plus null terminator
    char *final = malloc(total_length + 1);
    if (final == NULL) return false; // Return false if memory allocation fails

    int head = 0;
    int data_head = record->data_offset;
    int data_max = record->data_length;

    // Copy the URI prefix if it exists
    if (prefix_len > 0) {
        for (int i = 0; i < prefix_len; i++) {
            final[head] = prefix[i];
            head++;
        }
        // Skip the prefix byte in the payload data and adjust data length
        data_head += 1;
        data_max -= 1;
    }

    // Copy the actual payload data after the prefix
    for (int j = 0; j < data_max; j++) {
        final[head] = record->buffer[data_head + j];
        head++;
    }

    final[head] = 0x00; // Null-terminate the final URI string

    *string = final; // Set the output string pointer to the allocated URI
    return true;
}