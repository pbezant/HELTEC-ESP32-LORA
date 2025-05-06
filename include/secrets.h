#pragma once

// LoRaWAN credentials
// Replace these with your actual device credentials from your LoRaWAN network server
// (e.g., The Things Network, Helium, etc.)

// Device EUI (8 bytes, hex format without spaces)
#define DEVEUI "70B3D57ED8004069"

// Application EUI (8 bytes, hex format without spaces)
#define APPEUI "70B3D57ED8004069"

// Application Key (16 bytes, hex format without spaces)
#define APPKEY "0EF9A3826DD27B1F46D82048EE598998"

// Network Key (16 bytes, hex format without spaces)
// For LoRaWAN 1.1, this should be a different key than APPKEY
// For LoRaWAN 1.0.x, this can be the same as APPKEY
#define NWKKEY "0EF9A3826DD27B1F46D82048EE598998"

// Note: For security reasons, it's recommended to store these credentials
// in a separate file that is not committed to version control.
// You should replace these placeholder values with your actual credentials.

// Replace with your actual keys when using LoRaWAN
// For direct P2P LoRa communication, these are not used 