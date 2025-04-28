#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>

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

// Default I2S configuration
#ifndef I2S_SCK
#define I2S_SCK 16
#endif

#ifndef I2S_WS
#define I2S_WS 15
#endif

#ifndef I2S_SD
#define I2S_SD 7
#endif

#ifndef I2S_PORT
#define I2S_PORT I2S_NUM_0
#endif

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 16000
#endif

#ifndef SAMPLE_BITS
#define SAMPLE_BITS 32
#endif

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512
#endif

#ifndef DB_SMOOTHING_FACTOR
#define DB_SMOOTHING_FACTOR 0.8
#endif

#ifndef DB_OFFSET
#define DB_OFFSET 35
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
    
    // INMP441 microphone readings
    bool beginMicrophone(int i2s_sck = I2S_SCK, int i2s_ws = I2S_WS, int i2s_sd = I2S_SD);
    float readDecibelLevel();
    float getLastDecibelLevel() const { return lastDecibelLevel; }
    
    // Get sensor status
    bool isBME280Available() { return bme280Available; }
    bool isMicrophoneAvailable() { return microphoneAvailable; }
    
    // Read all BME280 values at once (more efficient)
    void readBME280(float &temperature, float &humidity, float &pressure, float &altitude);
    
    // Direct sensor access (use carefully)
    Adafruit_BME280& getBME280() { return bme; }
    
private:
    Adafruit_BME280 bme;
    bool bme280Available;
    
    // I2S microphone variables
    bool microphoneAvailable;
    float lastDecibelLevel;
    float smoothedDecibelLevel;
    
    // FFT variables
    arduinoFFT fft;
    double fftReal[BUFFER_SIZE];
    double fftImag[BUFFER_SIZE];
    
    // I2S configuration
    i2s_config_t i2s_config;
    i2s_pin_config_t i2s_pins;
    
    // Internal methods
    void i2s_adc_data_scale(uint8_t* d_buff, uint8_t* s_buff, uint32_t len);
    double computeDecibelLevel(float* samples, uint32_t sampleCount);
}; 