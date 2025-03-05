#ifndef LORAWAN_MANAGER_H
#define LORAWAN_MANAGER_H

#include <Arduino.h>
#include <RadioLib.h>
#define RADIO_BOARD_WIFI_LORA32_V3
#include <RadioBoards.h>
#include <LoRaWAN_ESP32.h>
//#include <heltec.h>  // Add this for Heltec board support
#include <SPI.h>     // Add this for SPI communication

// Define SERIAL_LOG macro
#define SERIAL_LOG(fmt, ...) Serial.printf(fmt "\n", ##__VA_ARGS__)

class LoRaWANManager {
public:
    // Constructor
    LoRaWANManager(const char* joinEui, const char* devEui, 
                   const char* nwkKey, const char* appKey,
                   LoRaWANBand_t region, uint8_t subBand);
    
    // Initialize LoRaWAN
    bool begin();
    
    // Join network
    bool joinNetwork();
    
    // Send data
    bool sendData(uint8_t* data, size_t len);
    
    // Get time until next allowed transmission
    uint32_t getTimeUntilNextTransmission() const;
    
    // Check if connected
    bool isConnected() const;

    // Receive downlink
    bool receiveDownlink(uint8_t* buffer, size_t* length, uint8_t* port = nullptr);

    ~LoRaWANManager() {
        delete node;
        delete radio;
        delete module;
    }

private:
    // Constants
    static const uint8_t MAX_RETRIES = 3;
    static const uint32_t RETRY_DELAY = 5000;  // 5 seconds
    
    // Member variables
    const char* _joinEui;
    const char* _devEui;
    const char* _nwkKey;
    const char* _appKey;
    LoRaWANBand_t _region;
    uint8_t _subBand;
    
    // LoRaWAN node instance
    LoRaWANNode* node;
    
    // Radio instance (using SX1262 for Heltec boards)
    Module* module;
    SX1262* radio;
    
    // Utility function to convert hex string to byte array
    uint8_t* toByteArray(const char* hexString, bool reverseOrder = false);
    
    // Track connection state
    bool _isConnected;
};

#endif // LORAWAN_MANAGER_H 