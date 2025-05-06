#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Config.h"

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

#ifndef I2C_CLOCK_SPEED
#define I2C_CLOCK_SPEED 100000
#endif

class SensorManager {
public:
    SensorManager();
    
    /**
     * @brief Initialize the sensor manager
     * 
     * @param sda SDA pin for I2C
     * @param scl SCL pin for I2C
     * @return true if BME280 was detected and initialized, false otherwise
     */
    bool begin(int sda, int scl);
    
    /**
     * @brief Perform a complete reset of the I2C bus
     * 
     * @param sda SDA pin for I2C
     * @param scl SCL pin for I2C
     */
    void resetI2C(int sda, int scl);
    
    /**
     * @brief Scan the I2C bus for connected devices
     * 
     * @param wire TwoWire instance to use
     * @param sda SDA pin for I2C
     * @param scl SCL pin for I2C
     */
    void scanI2CBus(TwoWire *wire, int sda, int scl);
    
    /**
     * @brief Power cycle the BME280 if VEXT pin is available
     */
    void powerCycleBME280();
    
    /**
     * @brief Read temperature from BME280
     * 
     * @return float Temperature in Celsius, or -273.15 if sensor not available
     */
    float readTemperature();
    
    /**
     * @brief Read humidity from BME280
     * 
     * @return float Humidity in %, or 0.0 if sensor not available
     */
    float readHumidity();
    
    /**
     * @brief Read pressure from BME280
     * 
     * @return float Pressure in hPa, or 0.0 if sensor not available
     */
    float readPressure();
    
    /**
     * @brief Read altitude from BME280
     * 
     * @return float Altitude in meters, or 0.0 if sensor not available
     */
    float readAltitude();
    
    /**
     * @brief Read all BME280 values at once
     * 
     * @param temperature Reference to store temperature
     * @param humidity Reference to store humidity
     * @param pressure Reference to store pressure
     * @param altitude Reference to store altitude
     */
    void readBME280(float &temperature, float &humidity, float &pressure, float &altitude);
    
    // Get sensor status
    bool isBME280Available() { return bme280Available; }
    
    // Direct sensor access (use carefully)
    Adafruit_BME280& getBME280() { return bme; }
    
private:
    Adafruit_BME280 bme;
    bool bme280Available;
    TwoWire *wire; // Store which Wire instance we're using
}; 