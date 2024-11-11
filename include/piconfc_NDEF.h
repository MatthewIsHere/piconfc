/**
 * @file piconfc_NDEF.h
 * @brief Provides functions and structures for creating, parsing, and reading NDEF (NFC Data Exchange Format) records.
 *
 * This header file contains the public API for handling NDEF records, including encoding and
 * decoding TLV (Tag-Length-Value) structures, creating and parsing NDEF records, and reading
 * MIME and URI payloads. This API is intended for NFC applications and provides utilities
 * for managing various record types.
 */

#ifndef NDEF_H
#define NDEF_H

#include <stdint.h>
#include <stdbool.h>

/** @name NDEF URI Prefix Constants
 *  Prefixes for common URI schemes in NDEF records.
 */
///@{
#define NDEF_URIPREFIX_NONE (0x00)         ///< No prefix
#define NDEF_URIPREFIX_HTTP_WWWDOT (0x01)  ///< HTTP www. prefix
#define NDEF_URIPREFIX_HTTPS_WWWDOT (0x02) ///< HTTPS www. prefix
#define NDEF_URIPREFIX_HTTP (0x03)         ///< HTTP prefix
#define NDEF_URIPREFIX_HTTPS (0x04)        ///< HTTPS prefix
#define NDEF_URIPREFIX_TEL (0x05)          ///< Tel prefix
#define NDEF_URIPREFIX_MAILTO (0x06)       ///< Mailto prefix
#define NDEF_URIPREFIX_FTP_ANONAT (0x07)   ///< FTP
#define NDEF_URIPREFIX_FTP_FTPDOT (0x08)   ///< FTP dot
#define NDEF_URIPREFIX_FTPS (0x09)         ///< FTPS
#define NDEF_URIPREFIX_SFTP (0x0A)         ///< SFTP
#define NDEF_URIPREFIX_SMB (0x0B)          ///< SMB
#define NDEF_URIPREFIX_NFS (0x0C)          ///< NFS
#define NDEF_URIPREFIX_FTP (0x0D)          ///< FTP
#define NDEF_URIPREFIX_DAV (0x0E)          ///< DAV
#define NDEF_URIPREFIX_NEWS (0x0F)         ///< NEWS
#define NDEF_URIPREFIX_TELNET (0x10)       ///< Telnet prefix
#define NDEF_URIPREFIX_IMAP (0x11)         ///< IMAP prefix
#define NDEF_URIPREFIX_RTSP (0x12)         ///< RTSP
#define NDEF_URIPREFIX_URN (0x13)          ///< URN
#define NDEF_URIPREFIX_POP (0x14)          ///< POP
#define NDEF_URIPREFIX_SIP (0x15)          ///< SIP
#define NDEF_URIPREFIX_SIPS (0x16)         ///< SIPS
#define NDEF_URIPREFIX_TFTP (0x17)         ///< TFPT
#define NDEF_URIPREFIX_BTSPP (0x18)        ///< BTSPP
#define NDEF_URIPREFIX_BTL2CAP (0x19)      ///< BTL2CAP
#define NDEF_URIPREFIX_BTGOEP (0x1A)       ///< BTGOEP
#define NDEF_URIPREFIX_TCPOBEX (0x1B)      ///< TCPOBEX
#define NDEF_URIPREFIX_IRDAOBEX (0x1C)     ///< IRDAOBEX
#define NDEF_URIPREFIX_FILE (0x1D)         ///< File
#define NDEF_URIPREFIX_URN_EPC_ID (0x1E)   ///< URN EPC ID
#define NDEF_URIPREFIX_URN_EPC_TAG (0x1F)  ///< URN EPC tag
#define NDEF_URIPREFIX_URN_EPC_PAT (0x20)  ///< URN EPC pat
#define NDEF_URIPREFIX_URN_EPC_RAW (0x21)  ///< URN EPC raw
#define NDEF_URIPREFIX_URN_EPC (0x22)      ///< URN EPC
#define NDEF_URIPREFIX_URN_NFC (0x23)      ///< URN NFC
///@}

/**
 * @brief Represents a Tag-Length-Value (TLV) structure.
 *
 * The TLV structure is used in NDEF records for encoding length and value fields.
 */
struct TLV {
    uint8_t * buffer;
    int buffer_len;
    int value_offset;
    uint16_t value_length;
    uint8_t * value_ptr;
};

/**
 * @brief Enumeration for Type Name Format (TNF) values.
 *
 * The TNF values define the type of an NDEF record, indicating how its type should be interpreted.
 */
enum TNF {
    TNF_EMPTY = 0,
    TNF_WELLKNOWN,
    TNF_MIME,
    TNF_ABSURI,
    TNF_EXTERNAL,
    TNF_UNKNOWN,
    TNF_UNCHANGED,
    TNF_RESERVED
} __attribute__((packed));

/**
 * @brief Structure representing an NDEF record.
 *
 * An NDEF record contains metadata and a payload. This structure holds
 * pointers and offsets to each part of the record.
 */
typedef struct {
    uint8_t * buffer;
    enum TNF tnf;
    int data_offset;
    int data_length;
    uint8_t id_length;
    int id_offset;
    uint8_t type_length;
    int type_offset;
} NDEFRecord;

/**
 * @brief Parses a TLV (Tag-Length-Value) structure from an NTAG buffer.
 *
 * This function searches for a TLV structure within the provided buffer, starting from
 * the specified offset. If a valid TLV structure with a data tag (0x03) is found, it
 * populates the `empty_TLV` struct with the value's length and offset within the buffer.
 * The start parameter allows searching from a specific position in the buffer, with
 * `start = 0` being the default for a fresh search.
 *
 * @param empty_TLV Pointer to a TLV structure to be filled with parsed data.
 * @param buffer Pointer to the buffer containing the NTAG data.
 * @param bufsize Size of the buffer in bytes.
 * @param start Starting offset in the buffer for searching the TLV.
 * @return True if a valid TLV structure is found and parsed successfully, false otherwise.
 */
bool piconfc_NDEF_parseTLV(struct TLV *empty_TLV, uint8_t *buffer, int bufsize, int start);

/**
 * @brief Encodes data into a TLV (Tag-Length-Value) packet format.
 *
 * This function takes a data buffer and encodes it into a TLV packet in the provided result buffer.
 * The TLV format consists of a tag (0x03), followed by the length of the data, the data itself,
 * and a terminator (0xFE). The function supports both 1-byte and 3-byte length encodings depending
 * on the size of the data.
 *
 * @param data Pointer to the data buffer to encode.
 * @param datasize Size of the data buffer in bytes.
 * @param result Pointer to the result buffer where the TLV packet will be stored.
 *               This buffer must be large enough to hold `datasize + 5` bytes.
 * @param resultsize Size of the result buffer in bytes.
 * @return The length of the TLV packet in bytes if successful; returns 0 if the result buffer is too small.
 */
int piconfc_NDEF_encodeTLV(uint8_t *data, uint16_t datasize, uint8_t *result, int resultsize);

/**
 * @brief Parses an NDEF message from a buffer and loads an array of NDEF records.
 *
 * This function reads NDEF records from the provided buffer and stores them in a dynamically
 * allocated array of `NDEFRecord` structures. The caller is responsible for freeing this
 * array once done. The number of records parsed is returned by the function.
 *
 * @param buffer Pointer to the buffer containing the NDEF message.
 * @param bufsize Size of the buffer in bytes.
 * @param dest Pointer to an `NDEFRecord*` that will be set to the dynamically allocated
 *             array of parsed records.
 * @return The number of records parsed if successful; 0 if no records are found or in case of an error.
 */
int piconfc_NDEF_parseMessage(uint8_t *buffer, int bufsize, NDEFRecord **dest);

/**
 * @brief Calculates the number of NDEF records in a given buffer.
 *
 * This function iterates through the buffer to count the number of NDEF records present.
 * It supports both short and standard length formats for the payload and handles records
 * with optional ID fields. The function stops when it reaches the end of the message or
 * the end of the buffer.
 *
 * @param buffer Pointer to the buffer containing the NDEF message.
 * @param bufsize Size of the buffer in bytes.
 * @return The total number of NDEF records in the buffer.
 */
int piconfc_NDEF_messageLen(uint8_t *buffer, int bufsize);

/**
 * @brief Parses an individual NDEF record from a buffer.
 *
 * This function reads an NDEF record starting from a specified offset within a buffer.
 * It decodes the record's header, including flags, type length, payload length, and
 * optional ID length. It populates an `NDEFRecord` structure with the parsed details,
 * including offsets and lengths for each section of the record.
 *
 * @param buffer Pointer to the buffer containing the NDEF message data.
 * @param bufsize Size of the buffer in bytes.
 * @param offset Offset within the buffer where the record begins.
 * @param empty_record Pointer to an `NDEFRecord` structure to populate with the parsed data.
 * @return The updated offset after parsing the record if successful; -1 if an error occurs (e.g., buffer size is insufficient).
 */
int piconfc_NDEF_parseRecord(uint8_t *buffer, int bufsize, int offset, NDEFRecord *empty_record);

/**
 * @brief Creates an NDEF record and stores it in a dynamically allocated buffer.
 *
 * This function constructs an NDEF record with specified parameters, including the
 * Type Name Format (TNF), type field, ID field, and payload data. It dynamically allocates
 * memory for the record, which the caller is responsible for freeing. The function supports
 * both short and standard length formats for the payload, and includes optional ID fields.
 *
 * @param dest Pointer to a buffer pointer where the created record will be stored.
 *             The caller is responsible for freeing this buffer.
 * @param recordlen Pointer to an unsigned int where the length of the created record will be stored.
 * @param tnf Type Name Format (TNF) for the NDEF record.
 * @param type Pointer to the type field data.
 * @param typelen Length of the type field data in bytes.
 * @param id Pointer to the ID field data (optional).
 * @param idlen Length of the ID field data in bytes (0 if not used).
 * @param payload Pointer to the payload data.
 * @param payloadlen Length of the payload data in bytes.
 * @return True if the record was successfully created; false if memory allocation failed.
 */
bool piconfc_NDEF_createRecord(uint8_t **dest, unsigned int *recordlen, enum TNF tnf, uint8_t *type, uint8_t typelen, uint8_t *id, uint8_t idlen, uint8_t *payload, unsigned int payloadlen);

/**
 * @brief Reads a MIME type string from an NDEF record.
 *
 * This function extracts the MIME type string from an NDEF record if the Type Name Format (TNF)
 * is set to `TNF_MIME`. It allocates memory for the string and appends a null terminator.
 * The caller is responsible for freeing the allocated string once done.
 *
 * @param record Pointer to the NDEFRecord structure containing the MIME type data.
 * @param string Pointer to a char pointer where the resulting MIME type string will be stored.
 *               The caller is responsible for freeing this string if the function succeeds.
 * @return True if the MIME type string was successfully read; false if the record is not of MIME type or if memory allocation fails.
 */
bool piconfc_NDEF_readMIMEString(NDEFRecord *record, char **string);

/**
 * @brief Reads a URI payload string from an NDEF record.
 *
 * This function extracts the URI payload from an NDEF record if the Type Name Format (TNF)
 * is set to `TNF_WELLKNOWN` and the type is `U`, indicating a URI with a prefix.
 * It dynamically allocates memory for the complete URI, appending the appropriate prefix
 * (e.g., "http://") if present. The caller is responsible for freeing the allocated string.
 *
 * @param record Pointer to the NDEFRecord structure containing the URI data.
 * @param string Pointer to a char pointer where the resulting URI string will be stored.
 *               The caller is responsible for freeing this string if the function succeeds.
 * @return True if the URI payload string was successfully read; false if the record does
 *         not contain a URI or if memory allocation fails.
 */
bool piconfc_NDEF_readPayloadString(NDEFRecord *record, char **string);

#endif /* NDEF_H */