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

// Create objects for sensors
Adafruit_BME280 bme;
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

// Function to read all sensors
void readSensors() {
    Serial.println("Reading Sensors");
  // Read BME280
  if (bme.begin(BME_ADDRESS)) {
  sensorData.temperature = bme.readTemperature();
  sensorData.humidity = bme.readHumidity();
  sensorData.pressure = bme.readPressure() / 100.0F;
  } else {
    Serial.println("Could not find BME280 sensor!");
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

RTC_DATA_ATTR uint8_t count = 0;

// Add these definitions after the other #define statements
#define BAND    915E6  // Set your region frequency (915MHz for US)
#define SF      DR_SF7  // Spreading Factor (DR_SF7 to DR_SF12)
#define TX_POWER    14  // Transmit power in dBm (max 20dBm)

// Add TTN credentials - Replace these with your values from TTN console
const char* devEui = "70B3D57ED8003DF4";  // Device EUI from TTN console
const char* appEui = "EABB82D8689A30D7";  // Application EUI from TTN console
const char* appKey = "C05BB00987036902C5AFBD3F6A55A3CF";  // App Key from TTN console

// Add this function before setup()
// void initLoRaWAN() {
//     Serial.println("Initializing LoRaWAN...");
    
//     // Set the device credentials
//     if (!persist.isProvisioned()) {
//         Serial.println("Setting device credentials...");
//         persist.setDevEui(devEui);
//         persist.setAppEui(appEui);
//         persist.setAppKey(appKey);
        
//         // Set region (US915 for United States)
//         persist.setRegion(REGION_US915);
        
//         // Set channel mask for US915 (using channels 8-15 and 65 for TTN)
//         uint16_t channelMask[] = {0x0000, 0xFF00, 0x0000, 0x0000, 0x0000, 0x0000};
//         persist.setChannelMask(channelMask);
//     }
// }

// Modify the setup() function to include LoRaWAN initialization
void setup() {
    heltec_setup();
    Wire.begin();
    //persist.begin();
    // Initialize LoRaWAN settings
    //initLoRaWAN();

    // Read sensors
    readSensors();

    // Initialize radio
    Serial.println("Radio init");
    int16_t state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        Serial.println("Radio did not initialize. We'll try again later.");
        goToSleep();
    }

    node = persist.manage(&radio);

    // Try to join network
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

    state = node->sendReceive(uplinkData, sizeof(uplinkData), 1, downlinkData, &lenDown);

    if(state == RADIOLIB_ERR_NONE) {
        Serial.println("Message sent, no downlink received.");
    } else if (state > 0) {
        Serial.println("Message sent, downlink received.");
    } else {
        Serial.printf("sendReceive returned error %d, we'll try again later.\n", state);
    }

    goToSleep();    // Does not return, program starts over next round
}

void loop() {
  // This is never called. There is no repetition: we always go back to
  // deep sleep one way or the other at the end of setup()
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

