#include "SensorManager.h"
#include "Config.h"

SensorManager::SensorManager() : bme280Available(false), wire(&Wire) {
    // Use primary Wire by default (more compatible with sensors)
}

// Power cycle the BME280 to reset it if possible
void SensorManager::powerCycleBME280() {
    #if defined(VEXT_PIN)
    Serial.println("Power cycling BME280 via VEXT...");
    pinMode(VEXT_PIN, OUTPUT);
    
    // Power off
    digitalWrite(VEXT_PIN, HIGH); // HIGH turns power off on VEXT
    delay(500);
    
    // Power on
    digitalWrite(VEXT_PIN, LOW); // LOW turns power on on VEXT
    delay(500);
    
    Serial.println("BME280 power cycle complete");
    #else
    Serial.println("VEXT pin not defined, cannot power cycle BME280");
    #endif
}

// Reset the I2C bus entirely
void SensorManager::resetI2C(int sda, int scl) {
    Serial.println("Performing complete I2C bus reset on pins SDA=" + String(sda) + ", SCL=" + String(scl));
    
    // End any existing bus first
    Wire.end();
    delay(100);
    
    // Configure SDA and SCL for bit-banging
    pinMode(sda, OUTPUT);
    pinMode(scl, OUTPUT);
    
    // First make sure SCL is high
    digitalWrite(scl, HIGH);
    delayMicroseconds(10);
    
    // Toggle SDA to generate STOP condition
    digitalWrite(sda, HIGH);
    delayMicroseconds(10);
    digitalWrite(sda, LOW);
    delayMicroseconds(10);
    digitalWrite(sda, HIGH);
    delayMicroseconds(10);
    
    // Clock out any stuck transaction (9 clock cycles)
    for (int i = 0; i < 9; i++) {
        digitalWrite(scl, LOW);
        delayMicroseconds(10);
        digitalWrite(scl, HIGH);
        delayMicroseconds(10);
    }
    
    // Final STOP condition (SDA low->high while SCL is high)
    digitalWrite(sda, LOW);
    delayMicroseconds(10);
    digitalWrite(sda, HIGH);
    delayMicroseconds(10);
    
    // Release the pins
    pinMode(sda, INPUT);
    pinMode(scl, INPUT);
    
    Serial.println("I2C bus reset complete");
}

// Scan the I2C bus for devices
void SensorManager::scanI2CBus(TwoWire *wire, int sda, int scl) {
    Serial.println("Performing detailed I2C bus scan on pins SDA=" + String(sda) + ", SCL=" + String(scl));
    
    byte foundDevices = 0;
    byte error, address;
    
    for (address = 1; address < 127; address++) {
        wire->beginTransmission(address);
        error = wire->endTransmission();
        
        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            
            // Try to identify common I2C devices
            if (address == 0x3C || address == 0x3D) {
                Serial.println(" - Likely OLED Display");
            } else if (address == 0x76 || address == 0x77) {
                Serial.println(" - Likely BME280/BMP280 Sensor"); 
            } else {
                Serial.println();
            }
            
            foundDevices++;
        } else if (error == 4) {
            Serial.print("Unknown error at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    
    if (foundDevices == 0) {
        Serial.println("No I2C devices found - check wiring connections");
    } else {
        Serial.print("Found ");
        Serial.print(foundDevices);
        Serial.println(" I2C device(s)");
    }
}

bool SensorManager::begin(int sda, int scl) {
    // Log the I2C pins being used for debugging
    Serial.println("\n=== BME280 Sensor Initialization ===");
    Serial.println("Initializing BME280 with primary Wire on pins SDA=" + String(sda) + ", SCL=" + String(scl));
    
    // Complete reset of I2C infrastructure
    resetI2C(sda, scl);
    delay(100);
    
    // Terminate any existing Wire connection
    Wire.end();
    delay(100);
    
    // Initialize with our pins
    Serial.println("Setting up primary Wire bus for BME280...");
    if (Wire.begin(sda, scl)) {
        Serial.println("Primary I2C bus initialized successfully");
    } else {
        Serial.println("Primary I2C bus initialization failed");
        return false;
    }
    
    // Set clock to 100kHz for better reliability (standard I2C speed)
    Wire.setClock(100000); // 100kHz
    delay(100);
    
    // Scan I2C bus to see what's connected
    scanI2CBus(wire, sda, scl);
    
    // Try to initialize BME280 at primary address
    Serial.print("Trying BME280 at address 0x");
    Serial.print(BME_ADDRESS, HEX);
    Serial.print(" with primary Wire... ");
    
    // Multiple attempts to initialize BME280
    for (int attempt = 0; attempt < 3; attempt++) {
        bme280Available = bme.begin(BME_ADDRESS, wire);
        if (bme280Available) {
            Serial.println("Success on attempt " + String(attempt+1) + "!");
            break;
        } else {
            if (attempt < 2) {
                Serial.println("Failed attempt " + String(attempt+1) + ", retrying...");
                delay(200); // Longer delay before retry
            } else {
                Serial.println("Failed after multiple attempts");
            }
        }
    }
    
    if (!bme280Available) {
        // Try alternative I2C address
        Serial.print("Trying BME280 at alternative address 0x77... ");
        bme280Available = bme.begin(0x77, wire);
        if (bme280Available) {
            Serial.println("Success at address 0x77!");
        } else {
            Serial.println("Failed. BME280 not detected at either address.");
        }
    }
    
    if (bme280Available) {
        // Configure the BME280 sensor for weather monitoring mode
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating Mode
                        Adafruit_BME280::SAMPLING_X2,     // Temperature Oversampling
                        Adafruit_BME280::SAMPLING_X16,    // Pressure Oversampling
                        Adafruit_BME280::SAMPLING_X1,     // Humidity Oversampling
                        Adafruit_BME280::FILTER_X16,      // Filtering
                        Adafruit_BME280::STANDBY_MS_500); // Standby Time
        
        // Give BME280 time to stabilize after initialization
        delay(200);
                        
        // Verify we can read from the sensor
        float test_temp = bme.readTemperature();
        float test_hum = bme.readHumidity();
        float test_pressure = bme.readPressure() / 100.0F;
        
        Serial.println("Verification readings:");
        Serial.println("  Temperature: " + String(test_temp) + "°C");
        Serial.println("  Humidity: " + String(test_hum) + "%");
        Serial.println("  Pressure: " + String(test_pressure) + " hPa");
        
        if (isnan(test_temp) || isnan(test_hum) || isnan(test_pressure) || 
            (test_temp == 0.0 && test_hum == 0.0 && test_pressure == 0.0)) {
            Serial.println("WARNING: BME280 returning zeros or NaN values.");
            Serial.println("May not be working correctly despite successful initialization.");
            bme280Available = false;
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