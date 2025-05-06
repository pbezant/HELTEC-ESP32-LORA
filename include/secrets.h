#pragma once

// LoRaWAN credentials
// Replace these with your actual device credentials from your LoRaWAN network server
// (e.g., The Things Network, Helium, etc.)

// Device EUI (8 bytes, hex format without spaces)
#define DEVEUI "dcb753528f2e2352"

// Application EUI (8 bytes, hex format without spaces)
#define APPEUI "1ce2885bcf0b8cc2"

// Application Key (16 bytes, hex format without spaces)
#define APPKEY "3ad55cc4db7ae1326b04663e5d8f11af"

// Network Key (16 bytes, hex format without spaces)
// For LoRaWAN 1.1, this should be a different key than APPKEY
// For LoRaWAN 1.0.x, this can be the same as APPKEY
#define NWKKEY "3ad55cc4db7ae1326b04663e5d8f11af"

// Note: For security reasons, it's recommended to store these credentials
// in a separate file that is not committed to version control.
// You should replace these placeholder values with your actual credentials.

// Replace with your actual keys when using LoRaWAN
// For direct P2P LoRa communication, these are not used 