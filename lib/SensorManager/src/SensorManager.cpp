#include "SensorManager.h"
#include <driver/i2s.h>
#include <math.h>

SensorManager::SensorManager() : bme280Available(false), microphoneAvailable(false), lastDecibelLevel(0.0), smoothedDecibelLevel(0.0) {
    // Initialize FFT arrays
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fftReal[i] = 0.0;
        fftImag[i] = 0.0;
    }
}

bool SensorManager::begin(int sda, int scl) {
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

bool SensorManager::beginMicrophone(int i2s_sck, int i2s_ws, int i2s_sd) {
    Serial.println("Initializing I2S for INMP441 microphone...");
    
    // Configure I2S
    i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = (i2s_bits_per_sample_t)SAMPLE_BITS,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pins = {
        .bck_io_num = i2s_sck,
        .ws_io_num = i2s_ws,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = i2s_sd
    };
    
    // Install and start I2S driver
    esp_err_t result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (result != ESP_OK) {
        Serial.print("Error installing I2S driver: ");
        Serial.println(result);
        microphoneAvailable = false;
        return false;
    }
    
    result = i2s_set_pin(I2S_PORT, &i2s_pins);
    if (result != ESP_OK) {
        Serial.print("Error setting I2S pins: ");
        Serial.println(result);
        i2s_driver_uninstall(I2S_PORT);
        microphoneAvailable = false;
        return false;
    }
    
    Serial.println("I2S microphone initialized successfully!");
    microphoneAvailable = true;
    
    // Take an initial reading to calibrate
    readDecibelLevel();
    
    return true;
}

float SensorManager::readDecibelLevel() {
    if (!microphoneAvailable) {
        return -1.0; // Return -1 if microphone is not available
    }
    
    // Buffer for raw samples
    uint8_t buffer[BUFFER_SIZE * 4]; // 4 bytes per sample for 32-bit samples
    size_t bytes_read = 0;
    
    // Read samples from I2S
    esp_err_t result = i2s_read(I2S_PORT, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
    if (result != ESP_OK || bytes_read == 0) {
        Serial.println("Error reading from I2S");
        return lastDecibelLevel;
    }
    
    // Process samples
    for (int i = 0; i < BUFFER_SIZE; i++) {
        // Convert 32-bit sample to float (-1.0 to 1.0)
        int32_t sample = buffer[i*4] | (buffer[i*4+1] << 8) | (buffer[i*4+2] << 16) | (buffer[i*4+3] << 24);
        fftReal[i] = (float)sample / 2147483648.0; // Normalize to -1.0 to 1.0
        fftImag[i] = 0.0;
    }
    
    // Compute the decibel level
    double dbLevel = computeDecibelLevel(fftReal, BUFFER_SIZE);
    
    // Apply smoothing
    smoothedDecibelLevel = DB_SMOOTHING_FACTOR * smoothedDecibelLevel + (1.0 - DB_SMOOTHING_FACTOR) * dbLevel;
    lastDecibelLevel = smoothedDecibelLevel;
    
    Serial.print("Sound level: ");
    Serial.print(lastDecibelLevel);
    Serial.println(" dB");
    
    return lastDecibelLevel;
}

double SensorManager::computeDecibelLevel(double* samples, uint32_t sampleCount) {
    // Compute RMS (Root Mean Square)
    double sum = 0.0;
    for (uint32_t i = 0; i < sampleCount; i++) {
        sum += samples[i] * samples[i];
    }
    double rms = sqrt(sum / sampleCount);
    
    // Convert RMS to decibels
    // Reference: 0 dB is the threshold of human hearing
    // 20 * log10(rms / reference)
    double db = 20.0 * log10(rms + 1e-9); // Add a small value to prevent log(0)
    
    // Apply calibration offset
    db += DB_OFFSET;
    
    // Clamp to reasonable values (typically 30-120 dB for environmental sound)
    if (db < 30.0) db = 30.0;
    if (db > 120.0) db = 120.0;
    
    return db;
}

void SensorManager::i2s_adc_data_scale(uint8_t* d_buff, uint8_t* s_buff, uint32_t len) {
    uint32_t j = 0;
    uint32_t dac_value = 0;
    for (int i = 0; i < len; i += 4) {
        dac_value = ((((uint16_t)(s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
        d_buff[j++] = 0;
        d_buff[j++] = dac_value * 256 / 4096;
    }
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