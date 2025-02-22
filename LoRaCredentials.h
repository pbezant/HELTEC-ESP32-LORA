#pragma once

class LoRaCredentials {
private:
    uint64_t appEui;
    uint64_t devEui;
    uint8_t appKey[16];
    uint8_t nwkKey[16];

    // Helper to convert hex string to byte array
    void hexStringToByteArray(const char* hexString, uint8_t* byteArray, size_t length) {
        for (size_t i = 0; i < length; i++) {
            sscanf(hexString + 2*i, "%2hhx", &byteArray[i]);
        }
    }

    // Helper to convert hex string to uint64_t
    uint64_t hexStringToUint64(const char* hexString) {
        uint64_t result = 0;
        sscanf(hexString, "%lx", &result);
        return result;
    }

public:
    LoRaCredentials(const char* appEuiStr, const char* devEuiStr, const char* keyStr, const char* nwkKeyStr) {
        // Convert EUIs from hex strings to uint64_t
        appEui = hexStringToUint64(appEuiStr);
        devEui = hexStringToUint64(devEuiStr);
        
        // Convert key from hex string to byte array
        hexStringToByteArray(keyStr, appKey, 16);
        hexStringToByteArray(nwkKeyStr, nwkKey, 16);
    }

    uint64_t getAppEui() const { return appEui; }
    uint64_t getDevEui() const { return devEui; }
    uint64_t getJoinEui() const { return appEui; } // joinEui is same as appEui
    const uint8_t* getAppKey() const { return appKey; }
    const uint8_t* getNwkKey() const { return nwkKey; }

    // Debug method to print credentials
    void printCredentials() {
        Serial.printf("AppEUI: %016llX\n", appEui);
        Serial.printf("DevEUI: %016llX\n", devEui);
        Serial.print("AppKey: ");
        for(int i = 0; i < 16; i++) {
            if(appKey[i] < 0x10) Serial.print('0');
            Serial.print(appKey[i], HEX);
        }
        Serial.print("NWKKey: ");
        for(int i = 0; i < 16; i++) {
            if(nwkKey[i] < 0x10) Serial.print('0');
            Serial.print(nwkKey[i], HEX);
        }
        Serial.println();
    }
}; 