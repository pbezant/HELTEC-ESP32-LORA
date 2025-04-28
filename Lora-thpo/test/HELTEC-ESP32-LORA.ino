// Join LoRaWAN network
void joinNetwork() {
    // node->clearSession();
    SERIAL_LOG("Activating OTAA");

    uint8_t retries = 0;
    
    // Setup the OTAA session information
    int16_t state = node->beginOTAA(joinEui, devEui, toByteArray(NWKKEY), toByteArray(APPKEY));
    
    if (state != RADIOLIB_ERR_NONE) {
        SERIAL_LOG("Failed to initialize OTAA: %d", state);
        return;
    } else{
        SERIAL_LOG("OTAA initialized successfully: %d", state);
    }

    // ... rest of existing code ...
} 