#ifndef LORA_MANAGER_H
#define LORA_MANAGER_H

#include <Arduino.h>
#include <RadioLib.h>

class LoRaManager {
public:
    // Constructor and destructor
    LoRaManager();
    ~LoRaManager();
    
    // Singleton instance
    static LoRaManager* instance;
    
    // Initialize the LoRa module
    bool begin(int8_t pinCS, int8_t pinDIO1, int8_t pinReset, int8_t pinBusy);
    
    // Set the LoRaWAN credentials
    void setCredentials(uint64_t joinEUI, uint64_t devEUI, uint8_t* appKey, uint8_t* nwkKey);
    
    // Join the LoRaWAN network
    bool joinNetwork();
    
    // Send data to the LoRaWAN network
    bool sendData(uint8_t* data, size_t len, uint8_t port = 1, bool confirmed = false);
    
    // Helper method to send a string
    bool sendString(const String& data, uint8_t port = 1);
    
    // Get the last RSSI value
    float getLastRssi();
    
    // Get the last SNR value
    float getLastSnr();
    
    // Check if the device is joined to the network
    bool isNetworkJoined();
    
    // Handle events (should be called in the loop)
    void handleEvents();
    
    // Get the last error from LoRaWAN operations
    int getLastErrorCode();
    
private:
    // Radio module and LoRaWAN node
    SX1262* radio;
    LoRaWANNode* node;
    const LoRaWANBand_t* lorawanBand;
    
    // LoRaWAN credentials
    uint64_t joinEUI;
    uint64_t devEUI;
    uint8_t appKey[16];
    uint8_t nwkKey[16];
    
    // Status variables
    bool isJoined;
    float lastRssi;
    float lastSnr;
    
    // Receive buffer
    uint8_t receivedData[256];
    size_t receivedBytes;
    
    // Error handling
    int lastErrorCode;
};

#endif // LORA_MANAGER_H