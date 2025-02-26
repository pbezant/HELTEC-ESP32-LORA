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
    if (!node) return false;
    
    SERIAL_LOG("Activating OTAA");

    // Convert hex strings to byte arrays
    uint8_t* joinEuiBytes = toByteArray(_joinEui);
    uint8_t* devEuiBytes = toByteArray(_devEui);
    uint8_t* nwkKeyBytes = toByteArray(_nwkKey);
    uint8_t* appKeyBytes = toByteArray(_appKey);
    
    if (!joinEuiBytes || !devEuiBytes || !nwkKeyBytes || !appKeyBytes) {
        SERIAL_LOG("Failed to convert keys to bytes");
        return false;
    }

    // Convert byte arrays to uint64_t for EUIs
    uint64_t joinEUI = 0;
    uint64_t devEUI = 0;
    memcpy(&joinEUI, joinEuiBytes, 8);
    memcpy(&devEUI, devEuiBytes, 8);

    uint8_t retries = 0;
    
    // Setup the OTAA session information with correct types
    int16_t state = node->beginOTAA(joinEUI, devEUI, nwkKeyBytes, appKeyBytes);
    
    // Clean up the byte arrays
    delete[] joinEuiBytes;
    delete[] devEuiBytes;
    delete[] nwkKeyBytes;
    delete[] appKeyBytes;
    
    if (state != RADIOLIB_ERR_NONE) {
        SERIAL_LOG("Failed to initialize OTAA: %d", state);
        return false;
    }
    
    while (retries < MAX_RETRIES) {
        state = node->activateOTAA();
        
        if (state == RADIOLIB_ERR_NONE) {
            _isConnected = true;
            SERIAL_LOG("Joined successfully!");
            return true;
        }
        
        retries++;
        if (retries < MAX_RETRIES) {
            SERIAL_LOG("Join failed, retrying in %d ms", RETRY_DELAY);
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

uint8_t* LoRaWANManager::toByteArray(const char* hexString) {
    size_t len = strlen(hexString);
    if (len % 2 != 0) return nullptr;
    
    size_t byteLen = len / 2;
    uint8_t* bytes = new uint8_t[byteLen];
    
    for (size_t i = 0; i < byteLen; i++) {
        char highNibble = hexString[i * 2];
        char lowNibble = hexString[i * 2 + 1];
        
        // Convert hex chars to integer values
        highNibble = (highNibble >= 'A') ? (highNibble - 'A' + 10) : (highNibble - '0');
        lowNibble = (lowNibble >= 'A') ? (lowNibble - 'A' + 10) : (lowNibble - '0');
        
        bytes[i] = (highNibble << 4) | lowNibble;
    }
    
    return bytes;
} 