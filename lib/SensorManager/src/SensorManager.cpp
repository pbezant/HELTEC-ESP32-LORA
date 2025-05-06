#include "SensorManager.h"

SensorManager::SensorManager() : bme280Available(false) {
}

bool SensorManager::begin(int sda, int scl) {
    // Log the I2C pins being used for debugging
    Serial.println("Initializing BME280 with I2C pins - SDA: " + String(sda) + ", SCL: " + String(scl));
    
    // Ensure wire is properly initialized
    if(Wire.begin(sda, scl)) {
        Serial.println("I2C initialized successfully");
    } else {
        Serial.println("I2C initialization may have issues");
    }
    
    // Increase I2C clock for faster response
    Wire.setClock(100000); // Standard 100kHz I2C clock
    
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
        Serial.println("No I2C devices found - check wiring connections");
    } else {
        Serial.print("Found ");
        Serial.print(foundDevices);
        Serial.println(" I2C device(s)");
    }
    
    // Try to initialize BME280 at primary address
    Serial.print("Trying BME280 at address 0x");
    Serial.print(BME_ADDRESS, HEX);
    Serial.print("... ");
    
    // Multiple attempts to initialize BME280
    for (int attempt = 0; attempt < 3; attempt++) {
        bme280Available = bme.begin(BME_ADDRESS, &Wire);
        if (bme280Available) {
            Serial.println("Success on attempt " + String(attempt+1) + "!");
            break;
        } else {
            if (attempt < 2) {
                Serial.println("Failed attempt " + String(attempt+1) + ", retrying...");
                delay(100); // Short delay before retry
            } else {
                Serial.println("Failed after multiple attempts");
            }
        }
    }
    
    if (!bme280Available) {
        // Try alternative I2C address if the first one fails
        Serial.print("Trying BME280 at address 0x77... ");
        bme280Available = bme.begin(0x77, &Wire);
        if (bme280Available) {
            Serial.println("Success!");
        } else {
            Serial.println("Failed");
            
            // One last attempt with the default Wire instance
            Serial.print("Final attempt with default Wire... ");
            bme280Available = bme.begin();
            if (bme280Available) {
                Serial.println("Success!");
            } else {
                Serial.println("Failed. BME280 not detected.");
            }
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
                        
        // Verify we can read from the sensor
        float test_temp = bme.readTemperature();
        float test_hum = bme.readHumidity();
        Serial.println("Verification readings - Temp: " + String(test_temp) + "°C, Humidity: " + String(test_hum) + "%");
        
        if (test_temp == 0.0 && test_hum == 0.0) {
            Serial.println("Warning: BME280 returning zeros. May not be working correctly despite successful initialization.");
        }
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