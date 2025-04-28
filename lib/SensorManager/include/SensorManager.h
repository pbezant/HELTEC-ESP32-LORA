#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Default configuration values
#ifndef I2C_SDA
#define I2C_SDA 21
#endif

#ifndef I2C_SCL 
#define I2C_SCL 22
#endif

#ifndef BME_ADDRESS
#define BME_ADDRESS 0x76
#endif

#ifndef SEALEVELPRESSURE_HPA
#define SEALEVELPRESSURE_HPA 1013.25
#endif

class SensorManager {
public:
    SensorManager();
    
    // Initialization
    bool begin(int sda = I2C_SDA, int scl = I2C_SCL);
    
    // BME280 readings
    float readTemperature();
    float readHumidity();
    float readPressure();
    float readAltitude();
    
    // Get sensor status
    bool isBME280Available() { return bme280Available; }
    
    // Read all BME280 values at once (more efficient)
    void readBME280(float &temperature, float &humidity, float &pressure, float &altitude);
    
    // Direct sensor access (use carefully)
    Adafruit_BME280& getBME280() { return bme; }
    
private:
    Adafruit_BME280 bme;
    bool bme280Available;
}; 