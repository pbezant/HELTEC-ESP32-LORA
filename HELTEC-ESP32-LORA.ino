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
// #include <esp32-hal-misc.h>
// #include <BH1750.h>

#define Serial Serial0  // Explicit serial port definition

// Define pins
// #define MOISTURE_PIN 36  // Analog pin for moisture sensor
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_ADDRESS 0x76 // Try 0x77 if 0x76 doesn't work

#define I2C_SDA 42
#define I2C_SCL 41

// Add this with other pin definitions
#define PIR_PIN 5  // Change this to match your PIR sensor connection
RTC_DATA_ATTR bool pir_wake = false;  // Keep track if PIR caused the wake

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



RTC_DATA_ATTR uint8_t count = 0;

// Add these definitions after the other #define statements
#define BAND    US915  // Set your region frequency (915MHz for US)
#define SF      DR_SF7  // Spreading Factor (DR_SF7 to DR_SF12)
#define TX_POWER    14  // Transmit power in dBm (max 20dBm)

// Add TTN credentials - Replace these with your values from TTN console
const char* devEui = "70B3D57ED8003DF4";  // Device EUI from TTN console
const char* appEui = "EABB82D8689A30D7";  // Application EUI from TTN console
const char* appKey = "C05BB00987036902C5AFBD3F6A55A3CF";  // App Key from TTN console

// Add this with other RTC variables near the top of the file
RTC_DATA_ATTR int16_t last_rssi = 0;  // Store previous transmission's RSSI

// Add this with other RTC variables
RTC_DATA_ATTR bool had_successful_transmission = false;  // Track if we've had a successful transmission
RTC_DATA_ATTR int consecutive_errors = 0;
RTC_DATA_ATTR uint32_t error_backoff_time = MINIMUM_DELAY;

// Function declarations
void initHardware();
void initRadio();
void joinNetwork();
void sendSensorData();
void scanI2C();


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
    pinMode(PIR_PIN, INPUT);
    bool wireStatus = Wire1.begin(I2C_SDA, I2C_SCL);
    delay(100);
  //  scanI2C();
    printWakeupReason();  // Print what woke us up
}

// Initialize and check radio
void initRadio() {
    SERIAL_LOG("Initializing radio");
    int16_t state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        Serial.println("Radio did not initialize. We'll try again later.");
        goToSleep();
    }
    node = persist.manage(&radio);
}

// Join LoRaWAN network
void joinNetwork() {                         
    SERIAL_LOG("Joining network");
    Serial.println("Attempting to join network...");
    int attempts = 0;
    while (!node->isActivated() && attempts < 5) {
        Serial.print("Join attempt ");
        Serial.println(attempts + 1);
        delay(5000);  // Wait between attempts
        attempts++;
    }

    if (!node->isActivated()) {
        Serial.println("Could not join network after 5 attempts. Going to sleep.");
        goToSleep();
    }

    Serial.println("Successfully joined network!");

    // Manages uplink intervals to the TTN Fair Use Policy
    node->setDutyCycle(true, 0);
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
// Prepare and send sensor data
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

    int16_t state = node->sendReceive(uplinkData, sizeof(uplinkData), 1, downlinkData, &lenDown);

    // Store current RSSI for next transmission
    last_rssi = radio.getRSSI();
    
    if(state == RADIOLIB_ERR_NONE || state > 0) {
        SERIAL_LOG(state == RADIOLIB_ERR_NONE ? 
            "Message sent, no downlink received. Status: %d" : 
            "Message sent, downlink received. Status: %d", 
            state);
        SERIAL_LOG("RSSI of this transmission: %d dBm (will be sent in next payload)", last_rssi);
        consecutive_errors = 0;
        error_backoff_time = MINIMUM_DELAY;
        had_successful_transmission = true;
        SERIAL_LOG("PIR triggers will now be enabled");
    } else {
        SERIAL_LOG("sendReceive returned error %d", state);
        consecutive_errors++;
        error_backoff_time = min(error_backoff_time, (uint32_t)MINIMUM_DELAY);
        SERIAL_LOG("Consecutive errors: %d, next retry in %d seconds", 
                  consecutive_errors, error_backoff_time);
        pir_wake = false;
    }
}

void goToSleep() {
    SERIAL_LOG("Preparing for deep sleep");
    uint32_t delayMs = node->timeUntilUplink();
    SERIAL_LOG("Sleeping for %d minutes", delayMs/6000);
    
    if (had_successful_transmission) {
        SERIAL_LOG("Enabling PIR wakeup");
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_5, HIGH);
    }
    
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
    heltec_setup();
    Serial.begin(115200);
    SERIAL_LOG("Initializing system");
    
    initHardware();
    
    if ((!had_successful_transmission || consecutive_errors > 0) && 
        esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        SERIAL_LOG("Ignoring PIR wake-up - waiting for successful transmission");
        goToSleep();
        return;
    }
    
    readSensors();
    initRadio();
    joinNetwork();
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