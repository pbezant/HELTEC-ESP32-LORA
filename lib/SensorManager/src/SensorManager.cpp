#include "SensorManager.h"

SensorManager::SensorManager() : bme280Available(false) {
}

bool SensorManager::begin(int sda, int scl) {
    // Add a delay to ensure any previous I2C operations have completed
    // This helps prevent conflicts with upload process
    delay(100);
    
    // Avoid warnings about re-initialization
    // The Wire library will handle multiple calls to begin() gracefully
    Wire.begin(sda, scl);
    
    // Scan I2C bus for devices (for debugging)
    Serial.println("Scanning I2C bus...");
    byte foundDevices = 0;
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        byte error = Wire.endTransmission();
        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (addr < 16) Serial.print("0");
            Serial.print(addr, HEX);
            Serial.println();
            foundDevices++;
        }
    }
    if (foundDevices == 0) {
        Serial.println("No I2C devices found");
    } else {
        Serial.print("Found ");
        Serial.print(foundDevices);
        Serial.println(" I2C device(s)");
    }
    
    // Try to initialize BME280 at primary address
    Serial.print("Trying BME280 at address 0x");
    Serial.print(BME_ADDRESS, HEX);
    Serial.print("... ");
    bme280Available = bme.begin(BME_ADDRESS, &Wire);
    
    if (bme280Available) {
        Serial.println("Success!");
    } else {
        Serial.println("Failed");
        
        // Try alternative I2C address if the first one fails
        Serial.print("Trying BME280 at address 0x77... ");
        bme280Available = bme.begin(0x77, &Wire);
        if (bme280Available) {
            Serial.println("Success!");
        } else {
            Serial.println("Failed");
        }
    }
    
    if (bme280Available) {
        // Configure the BME280 sensor
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating Mode
                        Adafruit_BME280::SAMPLING_X2,     // Temperature Oversampling
                        Adafruit_BME280::SAMPLING_X16,    // Pressure Oversampling
                        Adafruit_BME280::SAMPLING_X1,     // Humidity Oversampling
                        Adafruit_BME280::FILTER_X16,      // Filtering
                        Adafruit_BME280::STANDBY_MS_500); // Standby Time
    }
    
    return bme280Available;
}

float SensorManager::readTemperature() {
    if (!bme280Available) return -273.15; // Absolute zero if sensor not available
    
    return bme.readTemperature();
}

float SensorManager::readHumidity() {
    if (!bme280Available) return 0.0;
    
    return bme.readHumidity();
}

float SensorManager::readPressure() {
    if (!bme280Available) return 0.0;
    
    return bme.readPressure() / 100.0F; // Convert Pa to hPa
}

float SensorManager::readAltitude() {
    if (!bme280Available) return 0.0;
    
    return bme.readAltitude(SEALEVELPRESSURE_HPA);
}

void SensorManager::readBME280(float &temperature, float &humidity, float &pressure, float &altitude) {
    if (bme280Available) {
        temperature = bme.readTemperature();
        humidity = bme.readHumidity();
        pressure = bme.readPressure() / 100.0F; // Convert Pa to hPa
        altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    } else {
        temperature = -273.15;
        humidity = 0.0;
        pressure = 0.0;
        altitude = 0.0;
    }
} 