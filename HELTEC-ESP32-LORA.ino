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
// #include <esp32-hal-misc.h>
// #include <BH1750.h>

#define Serial Serial0  // Explicit serial port definition

// Define pins
// #define MOISTURE_PIN 36  // Analog pin for moisture sensor
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_ADDRESS 0x76 // Try 0x77 if 0x76 doesn't work

#define I2C_SDA 45
#define I2C_SCL 46

// Add this with other pin definitions
#define PIR_PIN 5  // Change this to match your PIR sensor connection
#define pir_gpio GPIO_NUM_5
RTC_DATA_ATTR bool pir_wake;  // Keep track if PIR caused the wake

// Create objects for sensors
Adafruit_BME280 bme; // I2C 3.3v
// BH1750 lightMeter;
LoRaWANNode* node;

// Modify the struct to include PIR status and RSSI
struct SensorData {
    float temperature;
    float humidity;
    float pressure;
    bool pir_triggered;
    int16_t rssi;
} sensorData;



RTC_DATA_ATTR uint8_t count;

// Add these definitions after the other #define statements
#define BAND    US915  // Set your region frequency (915MHz for US)
#define SF      DR_SF7  // Spreading Factor (DR_SF7 to DR_SF12)
#define TX_POWER    14  // Transmit power in dBm (max 20dBm)

// Add this with other RTC variables near the top of the file
RTC_DATA_ATTR int16_t last_rssi = 0;  // Store previous transmission's RSSI

// Add this with other RTC variables
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

// Function declarations
void initHardware();
void initRadio();
void joinNetwork();
void sendSensorData();
void goToSleep();
void handleDownlink(uint8_t* data, size_t len);


// Replace existing SERIAL_LOG macro with:
#define SERIAL_LOG(fmt, ...) do { \
    Serial.printf("[%s:%d] ", pathToFileName(__FILE__), __LINE__); \
    Serial.printf(fmt __VA_OPT__(,) __VA_ARGS__); \
    Serial.println(); \
} while(0)

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
    heltec_setup();
    printWakeupReason();  // Print what woke us up
    pinMode(PIR_PIN, INPUT);
    bool wireStatus = Wire1.begin(I2C_SDA, I2C_SCL);
    delay(100);
   
}
int16_t state;
// Initialize and check radio
void initRadio() {
    if(esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
        SERIAL_LOG("Resuming from deep sleep");
        // Reset radio hardware
        radio.reset();
        delay(50);
       // persist.loadSession(node); //the red button
    }
    SERIAL_LOG("Initializing radio");
    state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("Radio did not initialize. We'll try again later. Reason %d\n", state);
        goToSleep();
    }
    node = persist.manage(&radio);
}

// Join LoRaWAN network
void joinNetwork() {
    node->clearSession();
    SERIAL_LOG("Activating OTAA");
    int joinResult;
    uint8_t retries = 0;
    
    while (retries < MAX_RETRIES) {
        joinResult = node->activateOTAA();
        SERIAL_LOG("Join attempt %d result: %d", retries + 1, joinResult);
        
        if (joinResult == RADIOLIB_ERR_NONE) {
            SERIAL_LOG("Joined successfully!");
            persist.saveSession(node);
            node->setDutyCycle(true, 0);
            join_attempts = 0;  // Reset join attempts on success
            return;
        }
        
        retries++;
        join_attempts++;
        
        if (join_attempts >= max_join_attempts) {
            SERIAL_LOG("Max join attempts reached. Increasing sleep time.");
            error_backoff_time *= 2;  // Double the sleep time
            goToSleep();
        }
        
        if (retries < MAX_RETRIES) {
            SERIAL_LOG("Join failed, retrying in %d ms", RETRY_DELAY);
            delay(RETRY_DELAY);
        }
    }
    
    SERIAL_LOG("Join failed after %d attempts", MAX_RETRIES);
}

// Function to read all sensors
void readSensors() {
    Serial.println("Reading Sensors");
    sensorData.pir_triggered = pir_wake;
    
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
      sensorData.temperature = bme.readTemperature();
      sensorData.humidity = bme.readHumidity();
      sensorData.pressure = bme.readPressure() / 100.0F;
    }
    
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
    uint8_t uplinkData[9];
    
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
    
    // Counter (byte 5)
    uplinkData[5] = count++;
    
    // PIR status (byte 6)
    uplinkData[6] = sensorData.pir_triggered ? 1 : 0;
    
    // RSSI: 2 bytes (signed int16) (bytes 7-8)
    uplinkData[7] = last_rssi >> 8;
    uplinkData[8] = last_rssi & 0xFF;

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

// Modify goToSleep() to use custom_sleep_interval
void goToSleep() {
    SERIAL_LOG("Preparing for deep sleep");
    uint32_t delayMs = max(node->timeUntilUplink(), custom_sleep_interval * 1000);
    // SERIAL_LOG("Sleeping for %d minutes", delayMs/60000);
    SERIAL_LOG("Sleeping for %f", delayMs);
    
    if (had_successful_transmission) {
        SERIAL_LOG("Enabling PIR wakeup");
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_5, HIGH);
    }
    SERIAL_LOG("ACTUALLY only sleeping for 120 seconds");
    esp_sleep_enable_timer_wakeup(120*1000000);
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

// Modify setup() to respect the transmission success flag
void setup() {
    delay(5000);
    heltec_setup();
    Serial.begin(115200);
    SERIAL_LOG("Initializing system");
    
    initHardware();
    readSensors();
    if ((!had_successful_transmission || consecutive_errors > 0) && 
        esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        SERIAL_LOG("Ignoring PIR wake-up - waiting for successful transmission");
        goToSleep();
        return;
    }
    initRadio();
   
  // if(!node->isActivated()) {
    joinNetwork();
  //}
    sendSensorData();
    goToSleep();
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

void loop() {
  // This is never called. There is no repetition: we always go back to
  // deep sleep one way or the other at the end of setup()
}