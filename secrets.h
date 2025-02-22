#include "LoRaCredentials.h"
//dev
// extern const uint64_t appEui = 0xEABB82D8689A30D7LL;  // Application Join EUI from TTN console
// extern const uint64_t joinEui = appEui;
// extern const uint64_t devEui = 0x70B3D57ED8003DF4LL;  // Device EUI from TTN console

// // The keys need to be byte arrays
// extern const uint8_t appKey[16] = { 0xC0, 0x5B, 0xB0, 0x09, 0x87, 0x03, 0x69, 0x02, 
//                                  0xC5, 0xAF, 0xBD, 0x3F, 0x6A, 0x55, 0xA3, 0xCF };
// extern const uint8_t nwkKey[16] = { 0xC0, 0x5B, 0xB0, 0x09, 0x87, 0x03, 0x69, 0x02, 
//                                  0xC5, 0xAF, 0xBD, 0x3F, 0x6A, 0x55, 0xA3, 0xCF };
//master
// extern const uint64_t appEui = "EABB82D8689A30D7";  // Application Join EUI from TTN console
// extern const uint64_t joinEui = appEui;
// extern const uint64_t devEui = "70B3D57ED8003FD8";  // Device EUI from TTN console
// extern const uint8_t appKey[] = { 0xC0, 0x5B, 0xB0, 0x09, 0x87, 0x03, 0x69, 0x02, 
//                                  0xC5, 0xAF, 0xBD, 0x3F, 0x6A, 0x55, 0xA3, 0xCF };
// extern const uint8_t nwkKey[] = { 0xC0, 0x5B, 0xB0, 0x09, 0x87, 0x03, 0x69, 0x02, 
//                                  0xC5, 0xAF, 0xBD, 0x3F, 0x6A, 0x55, 0xA3, 0xCF };

//test
extern const char* appEui = EABB82D8689A30D7;  // Application Join EUI from TTN console
extern const char* joinEui = appEui;
extern const char* devEui = 70B3D57ED8003DF4;  // Device EUI from TTN console
extern const char* appKey = 893C390D8EA8C6E69BCAA5843548B2C6;
extern const char* nwkKey = appKey;

// // Define credentials as string literals
// #define APP_EUI "EABB82D8689A30D7"
// #define DEV_EUI "70B3D57ED8003DF4"
// #define APP_KEY "893C390D8EA8C6E69BCAA5843548B2C6"
// #define NWK_KEY "893C390D8EA8C6E69BCAA5843548B2C6"
// // Create credentials object
// LoRaCredentials credentials(APP_EUI, DEV_EUI, APP_KEY, NWK_KEY);