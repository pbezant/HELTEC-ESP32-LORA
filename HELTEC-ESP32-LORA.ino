/**
 * 
 * FOR THIS EXAMPLE TO WORK, YOU MUST INSTALL THE "LoRaWAN_ESP32" LIBRARY USING
 * THE LIBRARY MANAGER IN THE ARDUINO IDE.
 * 
 * This code will send a two-byte LoRaWAN message every 15 minutes. The first
 * byte is a simple 8-bit counter, the second is the ESP32 chip temperature
 * directly after waking up from its 15 minute sleep in degrees celsius + 100.
 *
 * If your NVS partition does not have stored TTN / LoRaWAN provisioning
 * information in it yet, you will be prompted for them on the serial port and
 * they will be stored for subsequent use.
 *
 * See https://github.com/ropg/LoRaWAN_ESP32
*/


// ============= Configuration =============
#define MINIMUM_DELAY 120


#include <heltec_unofficial.h>
#include <LoRaWAN_ESP32.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "secrets.h"
// #include "DisplayLogger.h"

// #include <esp32-hal-misc.h>
// #include <BH1750.h>

#define Serial Serial0  // Explicit serial port definition

// Define pins
// #define MOISTURE_PIN 36  // Analog pin for moisture sensor
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_ADDRESS 0x76 // Try 0x77 if 0x76 doesn't work

#define I2C_SCL 45
#define I2C_SDA 46

// Add this with other pin definitions
#define PIR_PIN 5
#define PIR_WAKE_LEVEL HIGH  // HIGH for active-high PIR, LOW for active-low
RTC_DATA_ATTR bool pir_wake;  // Keep track if PIR caused the wake


// Add these definitions after the other #define statements
#define SF      DR_SF7  // Spreading Factor (DR_SF7 to DR_SF12)
#define TX_POWER    14  // Transmit power in dBm (max 20dBm)
const LoRaWANBand_t Region = US915;
const uint8_t subBand = 2;

RTC_DATA_ATTR uint8_t count;
RTC_DATA_ATTR int16_t last_rssi = 0;  // Store previous transmission's RSSI
RTC_DATA_ATTR bool had_successful_transmission = false;  // Track if we've had a successful transmission
RTC_DATA_ATTR int consecutive_errors = 0;
RTC_DATA_ATTR uint32_t error_backoff_time = MINIMUM_DELAY;

// Add these with other RTC variables
#define MAX_RETRIES 3
#define RETRY_DELAY 5000  // 5 seconds between retries
RTC_DATA_ATTR uint32_t join_attempts = 0;
RTC_DATA_ATTR uint32_t max_join_attempts = 5;

// Add these definitions with other RTC variables
#define DOWNLINK_CONFIG 0x01
#define DOWNLINK_COMMAND 0x02
RTC_DATA_ATTR uint32_t custom_sleep_interval = MINIMUM_DELAY;  // In seconds

// Replace existing SERIAL_LOG macro with:
#define SERIAL_LOG(fmt, ...) do { \
    Serial.printf("[%s:%d] ", pathToFileName(__FILE__), __LINE__); \
    Serial.printf(fmt __VA_OPT__(,) __VA_ARGS__); \
    Serial.println(); \
} while(0)
// Modify setup() to respect the transmission success flag

// Create objects for sensors
Adafruit_BME280 bme; // I2C 3.3v
LoRaWANNode* node = new LoRaWANNode(&radio, &Region, subBand);

// Modify the struct to include PIR status and RSSI
struct SensorData {
    float temperature;
    float humidity;
    float pressure;
    bool pir_triggered;
    int16_t rssi;
} sensorData;

// Add these at the top with other includes
#include <Preferences.h>
Preferences store;

// Add these with other RTC variables
RTC_DATA_ATTR uint8_t LWsession[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];
RTC_DATA_ATTR uint16_t bootCount = 0;
RTC_DATA_ATTR uint16_t bootCountSinceUnsuccessfulJoin = 0;

void setup() {
    heltec_setup();
    SERIAL_LOG("System initialized");
    
    initHardware();
    initRadio();
    joinNetwork();
    // Only try to join if we're not already activated
    // if (!node->isActivated()) {
    //     joinNetwork();
    // } else {
    //     SERIAL_LOG("Node already activated, resuming session");
    // }
// }
// void loop() {
    // heltec_loop();
    readSensors();
    sendSensorData();
    if(custom_sleep_interval == 0) {
       
        SERIAL_LOG("delaying for %d ms", node->timeUntilUplink());
        delay(node->timeUntilUplink()*1000);
        
    }else{

        goToSleep();
    }
}
void loop() {
}
// Add this function to check wake-up cause
void printWakeupReason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    switch(wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            SERIAL_LOG("Wakeup caused by external signal using RTC_IO (PIR sensor)");
            pir_wake = true;
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            SERIAL_LOG("Wakeup caused by timer");
            pir_wake = false;
            break;
        default:
            SERIAL_LOG("Wakeup was not caused by deep sleep: %d", wakeup_reason);
            pir_wake = false;
            break;
    }
}

// Modify initHardware() to configure PIR pin
void initHardware() {
    printWakeupReason();  // Print what woke us up
    pinMode(PIR_PIN, INPUT_PULLDOWN);  // PULLDOWN if PIR is active-high
    bool wireStatus = Wire1.begin(I2C_SDA, I2C_SCL);
    delay(100);
    
    // Add BME280 sleep mode configuration
    if (bme.begin(BME_ADDRESS, &Wire1)) {
        // Set BME280 to forced mode (takes one measurement then sleeps)
        bme.setSampling(Adafruit_BME280::MODE_FORCED,
                       Adafruit_BME280::SAMPLING_X1,  // temperature
                       Adafruit_BME280::SAMPLING_X1,  // pressure
                       Adafruit_BME280::SAMPLING_X1,  // humidity
                       Adafruit_BME280::FILTER_OFF);
        SERIAL_LOG("BME280 configured for forced mode");
    }
}

// Initialize and check radio
void initRadio() {
    SERIAL_LOG("Initializing radio");
    int16_t state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        SERIAL_LOG("Radio initialization failed: %d", state);
        goToSleep();
        return;
    }
    //    node = persist.manage(&radio);
    // SERIAL_LOG("%d", node);
    SERIAL_LOG("Radio initialized successfully");
}

// Join LoRaWAN network
void joinNetwork() {
    SERIAL_LOG("Activating OTAA");
    int16_t state = RADIOLIB_ERR_UNKNOWN;
    
    // Setup the OTAA session information
    state = node->beginOTAA(joinEui, devEui, toByteArray(nwkKey), toByteArray(appKey));
    if (state != RADIOLIB_ERR_NONE) {
        SERIAL_LOG("Failed to initialize OTAA: %d", state);
        return;
    } else{
        SERIAL_LOG("OTAA initialized successfully: %d", state);
    }
    
    SERIAL_LOG("Recalling LoRaWAN nonces & session");
    store.begin("radiolib");

    // If we have previously saved nonces, restore them and try to restore session
    if (store.isKey("nonces")) {
        uint8_t buffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE+1];
        store.getBytes("nonces", buffer, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
        state = node->setBufferNonces(buffer);
        
        if (state != RADIOLIB_ERR_NONE) {
            SERIAL_LOG("Restoring nonces buffer failed: %d", state);
        }

        // Recall session from RTC deep-sleep preserved variable
        state = node->setBufferSession(LWsession);
        if (state == RADIOLIB_ERR_NONE) {
            SERIAL_LOG("Successfully restored session - now activating");
            state = node->activateOTAA();
            if (state == RADIOLIB_LORAWAN_SESSION_RESTORED) {
                store.end();
                return;
            }
        }
    } else {
        SERIAL_LOG("No Nonces saved - starting fresh");
    }

    // If we got here, there was no session to restore, so start trying to join
    uint8_t retries = 0;
    while (retries < MAX_RETRIES) {
        state = node->activateOTAA();
        SERIAL_LOG("Join attempt %d result: %d", retries + 1, state);
        
        if (state == RADIOLIB_LORAWAN_NEW_SESSION) {
            SERIAL_LOG("Joined successfully!");
            
            // Save the join counters (nonces) to permanent store
            SERIAL_LOG("Saving nonces to flash");
            uint8_t buffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
            uint8_t *persist = node->getBufferNonces();
            memcpy(buffer, persist, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
            store.putBytes("nonces", buffer, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
            
            // Save session to RTC memory
            persist = node->getBufferSession();
            memcpy(LWsession, persist, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
            
            bootCountSinceUnsuccessfulJoin = 0;
            store.end();
            return;
        }
        
        retries++;
        join_attempts++;
        
        if (join_attempts >= max_join_attempts) {
            SERIAL_LOG("Max join attempts reached. Increasing sleep time.");
            error_backoff_time *= 2;
            store.end();
            goToSleep();
        }
        
        if (retries < MAX_RETRIES) {
            SERIAL_LOG("Join failed, retrying in %d ms", RETRY_DELAY);
            delay(RETRY_DELAY);
        }
    }
    
    SERIAL_LOG("Join failed after %d attempts", MAX_RETRIES);
    store.end();
}

// Function to read all sensors
void readSensors() {
    Serial.println("Reading Sensors");

    
    if (!bme.begin(BME_ADDRESS, &Wire1)) {
        Serial.println("Could not find BME280 sensor! Checking for common issues:");
        Wire.beginTransmission(BME_ADDRESS);
        byte error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n, but failed to initialize", BME_ADDRESS);
        } else {
            Serial.printf("I2C error: %d - No device found at address 0x%02X\n", error, BME_ADDRESS);
        }
        
    } else {
        Serial.println("BME280 initialized successfully!");
        
        // Force a measurement
        bme.takeForcedMeasurement();
        
        // Read the values
        sensorData.temperature = bme.readTemperature();
        sensorData.humidity = bme.readHumidity();
        sensorData.pressure = bme.readPressure() / 100.0F;
        
        // Put BME280 into sleep mode
        bme.setSampling(Adafruit_BME280::MODE_SLEEP);
        SERIAL_LOG("BME280 put into sleep mode");
    }
    sensorData.pir_triggered = pir_wake;
    // Print readings for debugging
    Serial.printf("Temperature: %.2f°C\n", sensorData.temperature);
    Serial.printf("Humidity: %.2f%%\n", sensorData.humidity);
    Serial.printf("Pressure: %.2fhPa\n", sensorData.pressure);
    Serial.printf("PIR Triggered: %s\n", sensorData.pir_triggered ? "Yes" : "No");
}

// Downlink Message Format Documentation
/*
 * The downlink messages follow this format:
 * 
 * Configuration Messages (Type 0x01):
 * 1. Set Sleep Interval:
 *    [0x01][0x01][seconds_msb][seconds_mid][seconds_lsb]
 *    Example (300 seconds): 0x01 0x01 0x00 0x01 0x2C
 * 
 * 2. Set Max Join Attempts:
 *    [0x01][0x02][attempts]
 *    Example (10 attempts): 0x01 0x02 0x0A
 * 
 * Command Messages (Type 0x02):
 * 1. Reset Device:
 *    [0x02][0x01]
 *    Example: 0x02 0x01
 * 
 * 2. Force Sensor Reading:
 *    [0x02][0x02]
 *    Example: 0x02 0x02
 */

void handleDownlink(uint8_t* data, size_t len) {
    if (len < 2) {
        SERIAL_LOG("Downlink too short to be valid");
        return;
    }

    uint8_t messageType = data[0];
    
    switch(messageType) {
        case DOWNLINK_CONFIG: {
            // Configuration messages
            if (len >= 5) {  // Minimum length for config message
                switch(data[1]) {
                    case 0x01: {  // Sleep interval configuration
                        uint32_t new_interval = (data[2] << 16) | (data[3] << 8) | data[4];
                        if (new_interval >= MINIMUM_DELAY) {
                            custom_sleep_interval = new_interval;
                            SERIAL_LOG("New sleep interval set: %d seconds", custom_sleep_interval);
                        }
                        break;
                    }
                    case 0x02: {  // Max join attempts configuration
                        if (data[2] > 0) {
                            max_join_attempts = data[2];
                            SERIAL_LOG("New max join attempts set: %d", max_join_attempts);
                        }
                        break;
                    }
                    default:
                        SERIAL_LOG("Unknown config type: 0x%02X", data[1]);
                }
            }
            break;
        }
        case DOWNLINK_COMMAND: {
            // Command messages
            if (len >= 2) {
                switch(data[1]) {
                    case 0x01: {  // Reset device
                        SERIAL_LOG("Reset command received");
                        resetRTCVariables();
                        ESP.restart();
                        break;
                    }
                    case 0x02: {  // Force immediate sensor reading
                        SERIAL_LOG("Immediate sensor reading requested");
                        readSensors();
                        break;
                    }
                    default:
                        SERIAL_LOG("Unknown command: 0x%02X", data[1]);
                }
            }
            break;
        }
        default:
            SERIAL_LOG("Unknown message type: 0x%02X", messageType);
    }
}

// Modify sendSensorData() to use the new downlink handler
void sendSensorData() {
    // Prepare payload - now 9 bytes total
    uint8_t uplinkData[8];
    
    // Temperature: 2 bytes (multiplied by 100 to preserve 2 decimal places)
    int16_t temp = sensorData.temperature * 100;
    uplinkData[0] = temp >> 8;
    uplinkData[1] = temp & 0xFF;
    
    // Humidity: 1 byte (multiplied by 2 to preserve 1 decimal place)
    uplinkData[2] = (uint8_t)(sensorData.humidity * 2);
    
    // Pressure: 2 bytes (subtract 900 to fit in 2 bytes)
    uint16_t press = (sensorData.pressure - 900) * 10;
    uplinkData[3] = press >> 8;
    uplinkData[4] = press & 0xFF;
    
    // PIR status (byte 5)
    uplinkData[5] = sensorData.pir_triggered ? 1 : 0;

    // RSSI: 2 bytes (signed int16) (bytes 6-7)
    uplinkData[6] = last_rssi >> 8;
    uplinkData[7] = last_rssi & 0xFF;

    uint8_t downlinkData[256];
    size_t lenDown = sizeof(downlinkData);
    uint8_t retries = 0;

    while (retries < MAX_RETRIES) {
        int16_t state = node->sendReceive(uplinkData, sizeof(uplinkData), 1, downlinkData, &lenDown);
        last_rssi = radio.getRSSI();

        if (state == RADIOLIB_ERR_NONE || state > 0) {
            SERIAL_LOG("Message sent successfully on attempt %d", retries + 1);
            SERIAL_LOG("RSSI: %d dBm", last_rssi);
            
            if (state > 0) {
                SERIAL_LOG("Received %d bytes of downlink data", state);
                // Process downlink data
                handleDownlink(downlinkData, state);
            }
            
            consecutive_errors = 0;
            error_backoff_time = MINIMUM_DELAY;
            had_successful_transmission = true;
            return;
        }

        retries++;
        SERIAL_LOG("Transmission attempt %d failed with error %d", retries, state);
        
        if (retries < MAX_RETRIES) {
            SERIAL_LOG("Retrying in %d ms", RETRY_DELAY);
            delay(RETRY_DELAY);
        }
    }

    SERIAL_LOG("All transmission attempts failed");
    consecutive_errors++;
    error_backoff_time *= 2;  // Double the backoff time
    error_backoff_time = min(error_backoff_time, (uint32_t)(3600));  // Cap at 1 hour
    had_successful_transmission = false;
}

uint32_t calculateDelay(uint32_t timeUntilUplink, uint32_t customInterval){
      // Get time until next uplink and custom interval
    //uint32_t timeUntilNext = node->timeUntilUplink();
    uint32_t customDelay = customInterval * 1000;
    SERIAL_LOG("Time until next uplink: %d ms", timeUntilUplink);
    SERIAL_LOG("Custom sleep interval: %d ms", customDelay);
    
    uint32_t delayMs = max(timeUntilUplink, customDelay);
    SERIAL_LOG("Final sleep duration: %d ms (%d minutes)", delayMs, delayMs/60000);
    return delayMs;
}

// Modify goToSleep() to use custom_sleep_interval
void goToSleep() {
    SERIAL_LOG("Preparing for deep sleep");
    
    uint32_t delayMs = calculateDelay(node->timeUntilUplink(), custom_sleep_interval);
    
    // Debug PIR wake-up configuration
    SERIAL_LOG("PIR wake-up enabled on GPIO %d, trigger level: %s", 
               PIR_PIN, 
               PIR_WAKE_LEVEL == HIGH ? "HIGH" : "LOW");
    
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_5, PIR_WAKE_LEVEL);
    esp_sleep_enable_timer_wakeup(delayMs * 1000ULL); // Convert to microseconds
    
    // Log enabled wake-up sources
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    SERIAL_LOG("Current wake-up cause: %d", cause);
    SERIAL_LOG("Enabled wake-up sources:");
    SERIAL_LOG("- Timer: %d minutes", delayMs/60000);
    SERIAL_LOG("- PIR sensor on GPIO %d", PIR_PIN);
    
    // Ensure BME280 is in sleep mode
    if (bme.begin(BME_ADDRESS, &Wire1)) {
        bme.setSampling(Adafruit_BME280::MODE_SLEEP);
        SERIAL_LOG("BME280 confirmed in sleep mode before deep sleep");
    }
    
    // Power down I2C bus
    Wire1.end();
    SERIAL_LOG("I2C bus powered down");
    
    SERIAL_LOG("Entering deep sleep now...");
    esp_deep_sleep_start();
}

void scanI2C() {
    SERIAL_LOG("Scanning I2C bus...");
    byte error, address;
    int nDevices = 0;
    
    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            SERIAL_LOG("I2C device found at address 0x%02X", address);
            nDevices++;
        }
    }
    
    if (nDevices == 0) {
        SERIAL_LOG("No I2C devices found");
    } else {
        SERIAL_LOG("I2C scan done");
    }
}



// Add this function to reset all RTC variables if needed
void resetRTCVariables() {
    had_successful_transmission = false;
    consecutive_errors = 0;
    error_backoff_time = MINIMUM_DELAY;
    count = 0;
    last_rssi = 0;
    pir_wake = false;
}

uint8_t* toByteArray(const char* hexString) {
    static uint8_t byteArray[16];  // Static array to hold the result
    
    // Convert each pair of hex chars to a byte
    for(int i = 0; i < 16; i++) {
        char byteStr[3] = {hexString[i*2], hexString[i*2 + 1], '\0'};
        byteArray[i] = strtol(byteStr, NULL, 16);
    }
    SERIAL_LOG("Converted hex string to byte array: %s", hexString);
    return byteArray;
}

