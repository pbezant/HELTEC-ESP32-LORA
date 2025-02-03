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


// Pause between sends in seconds, so this is every 15 minutes. (Delay will be
// longer if regulatory or TTN Fair Use Policy requires it.)
#define MINIMUM_DELAY 90 


#include <heltec_unofficial.h>
#include <LoRaWAN_ESP32.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>

// Define pins
#define MOISTURE_PIN 36  // Analog pin for moisture sensor
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_ADDRESS 0x76 // Try 0x77 if 0x76 doesn't work

#define I2C_SDA 42
#define I2C_SCL 41
// #define BME_SCK 26
// #define BME_MISO 21
// #define BME_MOSI 20 
//#define BME_CS 42


// Create objects for sensors
Adafruit_BME280 bme; // I2C
BH1750 lightMeter;
LoRaWANNode* node;

// Structure to hold sensor readings
struct SensorData {
  float temperature;
  float humidity;
  float pressure;
  float light;
  int moisture;
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


// Function declarations
// void initHardware();
// void initRadio();
// void joinNetwork();
// void sendSensorData();
// void scanI2C();

// Initialize basic hardware components
void initHardware() {
    heltec_setup();
    //Wire.begin(I2C_SDA, I2C_SCL);
    bool wireStatus = Wire1.begin(I2C_SDA, I2C_SCL);
    delay(100); // Give I2C time to initialize
    scanI2C();
    //persist.begin();
}

// Initialize and check radio
void initRadio() {
    Serial.println("Radio init");
    int16_t state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        Serial.println("Radio did not initialize. We'll try again later.");
        goToSleep();
    }
    node = persist.manage(&radio);
}

// Join LoRaWAN network
void joinNetwork() {                         
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
    node->setDutyCycle(true, 1250);
}
// Function to read all sensors
void readSensors() {
    Serial.println("Reading Sensors");
    // Serial.println("Scanning I2C bus...");
    // // Read BME280
    // Serial.printf("Attempting to initialize BME280 at address 0x%02X\n", BME_ADDRESS);

    if (!bme.begin(BME_ADDRESS, &Wire1)) {
        Serial.println("Could not find BME280 sensor! Checking for common issues:");
        Wire.beginTransmission(BME_ADDRESS);
        byte error = Wire.endTransmission();
        if (error == 0) {
            Serial.println("I2C device found at address 0x76, but failed to initialize");
        } else {
            Serial.printf("I2C error: %d - No device found at address 0x%02X\n", error, BME_ADDRESS);
        }
        
    } else {
    
      Serial.println("BME280 initialized successfully!");
      sensorData.temperature = bme.readTemperature();
      sensorData.humidity = bme.readHumidity();
      sensorData.pressure = bme.readPressure() / 100.0F;
    }
    // Read BH1750
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        sensorData.light = lightMeter.readLightLevel();
    } else{
        Serial.println("Could not find BH1750 sensor!");
    }
    // Read moisture sensor
    sensorData.moisture = analogRead(MOISTURE_PIN);
    
    // Print readings for debugging
    Serial.printf("Temperature: %.2f°C\n", sensorData.temperature);
    Serial.printf("Humidity: %.2f%%\n", sensorData.humidity);
    Serial.printf("Pressure: %.2fhPa\n", sensorData.pressure);
    Serial.printf("Light: %.2flx\n", sensorData.light);
    Serial.printf("Moisture: %d\n", sensorData.moisture);
}
// Prepare and send sensor data
void sendSensorData() {
    // Prepare payload - 10 bytes total
    uint8_t uplinkData[10];
    
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
    
    // Light: 2 bytes
    uint16_t light = sensorData.light;
    uplinkData[5] = light >> 8;
    uplinkData[6] = light & 0xFF;
    
    // Moisture: 2 bytes
    uint16_t moist = sensorData.moisture;
    uplinkData[7] = moist >> 8;
    uplinkData[8] = moist & 0xFF;
    
    // Counter
    uplinkData[9] = count++;

    uint8_t downlinkData[256];
    size_t lenDown = sizeof(downlinkData);

    int16_t state = node->sendReceive(uplinkData, sizeof(uplinkData), 1, downlinkData, &lenDown);

    if(state == RADIOLIB_ERR_NONE) {
        Serial.println("Message sent, no downlink received.");
    } else if (state > 0) {
        Serial.println("Message sent, downlink received.");
    } else {
        Serial.printf("sendReceive returned error %d, we'll try again later.\n", state);
    }
}

// Refactored setup function
void setup() {
    initHardware();
    readSensors();
    initRadio();
    joinNetwork();
    sendSensorData();
    goToSleep();    // Does not return, program starts over next round
}



void goToSleep() {
  Serial.println("Going to deep sleep now");
  // allows recall of the session after deepsleep
  persist.saveSession(node);
  // Calculate minimum duty cycle delay (per FUP & law!)
  uint32_t interval = node->timeUntilUplink();
  // And then pick it or our MINIMUM_DELAY, whichever is greater
  uint32_t delayMs = max(interval, (uint32_t)MINIMUM_DELAY * 1000);
  Serial.printf("Next TX in %i mins\n", (delayMs/1000)/60);
  delay(100);  // So message prints
  // and off to bed we go
  heltec_deep_sleep(delayMs/1000);
}

void scanI2C() {
    Serial.println("Scanning I2C bus...");
    byte error, address;
    int nDevices = 0;
    
    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            nDevices++;
        }
    }
    
    if (nDevices == 0) {
        Serial.println("No I2C devices found\n");
    } else {
        Serial.println("I2C scan done\n");
    }
}

void loop() {
  // This is never called. There is no repetition: we always go back to
  // deep sleep one way or the other at the end of setup()
}