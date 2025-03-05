#include "LoRaWANManager.h"

#define RADIO_BOARD_AUTO
// Pin definitions for Heltec WiFi LoRa 32 V3
#define LORA_CS   8     // NSS pin
#define LORA_DIO1 14    // DIO1 pin
#define LORA_RST  12    // RESET pin
#define LORA_BUSY 13    // BUSY pin

LoRaWANManager::LoRaWANManager(const char* joinEui, const char* devEui, 
                             const char* nwkKey, const char* appKey,
                             LoRaWANBand_t region, uint8_t subBand)
    : _joinEui(joinEui), _devEui(devEui), _nwkKey(nwkKey), _appKey(appKey),
      _region(region), _subBand(subBand), _isConnected(false) {
    
    // Initialize the Module with the correct pins
    module = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);
    radio = new SX1262(module);
    node = nullptr;
}

bool LoRaWANManager::begin() {
    if (!radio || !module) return false;
    
    SERIAL_LOG("Initializing radio");
    int16_t state = radio->begin();
    if (state != RADIOLIB_ERR_NONE) {
        SERIAL_LOG("Radio initialization failed: %d", state);
        return false;
    }
    SERIAL_LOG("Radio initialized successfully");
    
    node = new LoRaWANNode(radio, &_region, _subBand);
    return (node != nullptr);
}

bool LoRaWANManager::joinNetwork() {
    if (!node) {
        SERIAL_LOG("Error: LoRaWAN node not initialized");
        return false;
    }
    
    SERIAL_LOG("Starting OTAA activation process...");
    SERIAL_LOG("Join EUI: %s", _joinEui);
    SERIAL_LOG("Dev EUI: %s", _devEui);

    // Convert hex strings to byte arrays
    uint8_t* joinEuiBytes = toByteArray(_joinEui, true);  // Reverse for TTN
    uint8_t* devEuiBytes = toByteArray(_devEui, true);    // Reverse for TTN
    uint8_t* nwkKeyBytes = toByteArray(_nwkKey);          // No reverse for keys
    uint8_t* appKeyBytes = toByteArray(_appKey);          // No reverse for keys
    
    if (!joinEuiBytes || !devEuiBytes || !nwkKeyBytes || !appKeyBytes) {
        SERIAL_LOG("Error: Key conversion failed. Check key formats");
        return false;
    }

    SERIAL_LOG("JoinEui: %d", joinEuiBytes);
    SERIAL_LOG("DevEui: %d", devEuiBytes);
    SERIAL_LOG("nwkKeyBytes: %d", nwkKeyBytes);
    SERIAL_LOG("appKey: %d", appKeyBytes);
    // Convert byte arrays to uint64_t for EUIs
    uint64_t joinEUI = 0;
    uint64_t devEUI = 0;
    memcpy(&joinEUI, joinEuiBytes, 8);
    memcpy(&devEUI, devEuiBytes, 8);
    
    SERIAL_LOG("joinEUI: %d", joinEUI);
    SERIAL_LOG("DevEUI: %d", devEUI);
    uint8_t retries = 0;
    
 
    // Setup the OTAA session
    SERIAL_LOG("Initializing OTAA...");
    int16_t state = node->beginOTAA(joinEUI, devEUI, nwkKeyBytes, appKeyBytes);
    
    // Clean up the byte arrays
    delete[] joinEuiBytes;
    delete[] devEuiBytes;
    delete[] nwkKeyBytes;
    delete[] appKeyBytes;
    
    if (state != RADIOLIB_ERR_NONE) {
        SERIAL_LOG("OTAA initialization failed with error code: %d", state);
        return false;
    }
    SERIAL_LOG("OTAA initialization successful");
    
    while (retries < MAX_RETRIES) {
        state = node->activateOTAA();
        
        if (state == RADIOLIB_ERR_NONE) {
            _isConnected = true;
            SERIAL_LOG("Joined successfully!");
            return true;
        }
        
        retries++;
        if (retries < MAX_RETRIES) {
            SERIAL_LOG("Join failed, retrying in %d ms: %d", RETRY_DELAY, state);
            delay(RETRY_DELAY);
        }
    }
    
    SERIAL_LOG("Join failed after %d attempts", MAX_RETRIES);
    return false;
}

bool LoRaWANManager::sendData(uint8_t* data, size_t len) {
    if (!node || !_isConnected) return false;
    
    uint8_t downlinkData[256];
    size_t lenDown = sizeof(downlinkData);
    
    int16_t state = node->sendReceive(data, len, 1, downlinkData, &lenDown);
    
    if (state == RADIOLIB_ERR_NONE || state > 0) {
        SERIAL_LOG("Message sent successfully");
        if (state > 0) {
            SERIAL_LOG("Received %d bytes of downlink data", state);
        }
        return true;
    }
    
    SERIAL_LOG("Failed to send message: %d", state);
    return false;
}

uint32_t LoRaWANManager::getTimeUntilNextTransmission() const {
    if (!node) return RETRY_DELAY;
    return node->timeUntilUplink();
}

bool LoRaWANManager::isConnected() const {
    return _isConnected;
}

bool LoRaWANManager::receiveDownlink(uint8_t* buffer, size_t* length, uint8_t* port) {
    if (!node || !_isConnected || !buffer || !length) {
        return false;
    }

    // Buffer for downlink data
    uint8_t downlinkPort = 0;
    size_t downlinkLen = *length;  // Use the provided buffer size

    SERIAL_LOG("Waiting for downlink...");
    
    // Try to receive downlink
    uint8_t emptyPayload = 0;
    int16_t state = node->sendReceive(&emptyPayload, 1, 1, buffer, &downlinkLen);
    
    
    if (state == RADIOLIB_ERR_NONE) {
        SERIAL_LOG("Received downlink on port %d with %d bytes", downlinkPort, downlinkLen);
        
        // Update the length with actual received bytes
        *length = downlinkLen;
        
        // If port pointer provided, update it
        if (port) {
            *port = downlinkPort;
        }

        // Print received data for debugging
        SERIAL_LOG("Downlink data:");
        for (size_t i = 0; i < downlinkLen; i++) {
            Serial.printf("%02X ", buffer[i]);
        }
        Serial.println();
        
        return true;
    } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
        SERIAL_LOG("No downlink received (timeout)");
    } else {
        SERIAL_LOG("Failed to receive downlink: %d", state);
    }

    return false;
}

uint8_t* LoRaWANManager::toByteArray(const char* hexString, bool reverseOrder) {
    size_t len = strlen(hexString) / 2;  // Two hex chars per byte
    uint8_t* byteArray = new uint8_t[len];  // Dynamic array allocation
    
    SERIAL_LOG("Converting hex string: %s (length: %d)", hexString, len);
    
    for(size_t i = 0; i < len; i++) {
        char byteStr[3] = {hexString[i*2], hexString[i*2 + 1], '\0'};
        if (reverseOrder) {
            byteArray[len - 1 - i] = (uint8_t)strtol(byteStr, NULL, 16);  // Reversed
        } else {
            byteArray[i] = (uint8_t)strtol(byteStr, NULL, 16);  // Normal
        }
    }
    
    // Print full array in hex
    Serial.print("Full byte array: ");
    for(size_t i = 0; i < len; i++) {
        Serial.printf("%02X ", byteArray[i]);
    }
    Serial.println();
    
    return byteArray;
} 