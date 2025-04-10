#pragma once

// LoRaWAN credentials
// Replace these with your actual device credentials from your LoRaWAN network server
// (e.g., The Things Network, Helium, etc.)

// Device EUI (8 bytes, hex format without spaces)
#define DEVEUI "70B3D57ED8004389"

// Application EUI (8 bytes, hex format without spaces)
#define APPEUI "0000000000000000"

// Application Key (16 bytes, hex format without spaces)
uint8_t appKey[] = {0x56, 0x6C, 0x2F, 0xA7, 0xF7, 0xFE, 0xCF, 0xDF, 0x99, 0xDE, 0x9B, 0x1B, 0x87, 0xF0, 0x3B, 0xAC};

// Network Key (16 bytes, hex format without spaces)
// For LoRaWAN 1.1, this should be a different key than APPKEY
// For LoRaWAN 1.0.x, this can be the same as APPKEY
uint8_t nwkKey[] = {0x56, 0x6C, 0x2F, 0xA7, 0xF7, 0xFE, 0xCF, 0xDF, 0x99, 0xDE, 0x9B, 0x1B, 0x87, 0xF0, 0x3B, 0xAC};

// Note: For security reasons, it's recommended to store these credentials
// in a separate file that is not committed to version control.
// You should replace these placeholder values with your actual credentials.

// Replace with your actual keys when using LoRaWAN
// For direct P2P LoRa communication, these are not used 