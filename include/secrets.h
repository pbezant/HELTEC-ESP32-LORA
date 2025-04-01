#pragma once

// LoRaWAN credentials
// Replace these with your actual device credentials from your LoRaWAN network server
// (e.g., The Things Network, Helium, etc.)

// Device EUI (8 bytes, hex format without spaces)
#define DEVEUI "EABB82D8689A30D7"

// Application EUI (8 bytes, hex format without spaces)
#define APPEUI "EABB82D8689A30D7"

// Application Key (16 bytes, hex format without spaces)
uint8_t appKey[] = {0xDD, 0x65, 0x87, 0xDD, 0x87, 0x0C, 0xC9, 0x56, 0xAC, 0xE7, 0x52, 0xE2, 0xDF, 0xAC, 0x2C, 0x7A};

// Network Key (16 bytes, hex format without spaces)
// For LoRaWAN 1.1, this should be a different key than APPKEY
// For LoRaWAN 1.0.x, this can be the same as APPKEY
uint8_t nwkKey[] = {0xDD, 0x65, 0x87, 0xDD, 0x87, 0x0C, 0xC9, 0x56, 0xAC, 0xE7, 0x52, 0xE2, 0xDF, 0xAC, 0x2C, 0x7A};

// Note: For security reasons, it's recommended to store these credentials
// in a separate file that is not committed to version control.
// You should replace these placeholder values with your actual credentials.

// Replace with your actual keys when using LoRaWAN
// For direct P2P LoRa communication, these are not used 