#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Config.h"

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