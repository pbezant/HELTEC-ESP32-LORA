# SensorManager

A simple Arduino library for managing BME280 and INMP441 sensors. This library provides a clean interface for initializing and reading data from BME280 temperature, humidity, and pressure sensors, as well as INMP441 I2S microphones for sound level monitoring.

## Features

- Easy I2C setup and management for BME280 sensors
- Automatic detection of BME280 at common I2C addresses (0x76 and 0x77)
- Support for temperature, humidity, pressure, and altitude readings
- I2S configuration for INMP441 microphone
- Sound level monitoring and decibel (dB) calculation
- Efficient multi-parameter reading method
- I2C bus scanning for debugging

## Installation

### PlatformIO

Add to your `platformio.ini` file:

```ini
lib_deps =
  # Other dependencies
  SensorManager
```

### Arduino IDE

1. Download this repository as a ZIP file
2. In Arduino IDE, go to Sketch > Include Library > Add .ZIP Library
3. Select the downloaded ZIP file

## Dependencies

This library depends on:
- [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library)
- [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor)
- [ArduinoFFT](https://github.com/kosme/arduinoFFT) (for sound analysis)

## Usage

### Basic BME280 Example

```cpp
#include <Arduino.h>
#include <SensorManager.h>

SensorManager sensors;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("SensorManager Example");
  
  // Initialize with default I2C pins
  if (!sensors.begin()) {
    Serial.println("Could not find BME280 sensor!");
  }
}

void loop() {
  if (sensors.isBME280Available()) {
    Serial.print("Temperature: ");
    Serial.print(sensors.readTemperature());
    Serial.println(" °C");
    
    Serial.print("Humidity: ");
    Serial.print(sensors.readHumidity());
    Serial.println(" %");
    
    Serial.print("Pressure: ");
    Serial.print(sensors.readPressure());
    Serial.println(" hPa");
    
    Serial.print("Altitude: ");
    Serial.print(sensors.readAltitude());
    Serial.println(" m");
  }
  
  delay(2000);
}
```

### INMP441 Microphone Example

```cpp
#include <Arduino.h>
#include <SensorManager.h>

// I2S pins for INMP441
#define I2S_SCK 16  // Serial Clock
#define I2S_WS 15   // Word Select (Left/Right Clock)
#define I2S_SD 7    // Serial Data

SensorManager sensors;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("INMP441 Microphone Example");
  
  // Initialize the microphone
  if (!sensors.beginMicrophone(I2S_SCK, I2S_WS, I2S_SD)) {
    Serial.println("Failed to initialize INMP441 microphone!");
  } else {
    Serial.println("Microphone initialized successfully!");
  }
}

void loop() {
  if (sensors.isMicrophoneAvailable()) {
    // Read the sound level in decibels
    float dB = sensors.readDecibelLevel();
    
    Serial.print("Sound level: ");
    Serial.print(dB);
    Serial.println(" dB");
  }
  
  delay(1000);
}
```

### Combined Example

```cpp
#include <Arduino.h>
#include <SensorManager.h>

#define I2S_SCK 16
#define I2S_WS 15
#define I2S_SD 7

SensorManager sensors;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("SensorManager Combined Example");
  
  // Initialize BME280
  bool bmeInitialized = sensors.begin();
  Serial.println(bmeInitialized ? "BME280 initialized!" : "BME280 not found!");
  
  // Initialize INMP441
  bool micInitialized = sensors.beginMicrophone(I2S_SCK, I2S_WS, I2S_SD);
  Serial.println(micInitialized ? "INMP441 initialized!" : "INMP441 not found!");
}

void loop() {
  // Read environmental data
  if (sensors.isBME280Available()) {
    float temperature, humidity, pressure, altitude;
    sensors.readBME280(temperature, humidity, pressure, altitude);
    
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
  
  // Read sound level
  if (sensors.isMicrophoneAvailable()) {
    Serial.print("Sound level: ");
    Serial.print(sensors.readDecibelLevel());
    Serial.println(" dB");
  }
  
  delay(2000);
}
```

### Efficient Reading

For more efficient readings, use the `readBME280` method to get all values at once:

```cpp
float temperature, humidity, pressure, altitude;

// Get all values in a single call
sensors.readBME280(temperature, humidity, pressure, altitude);

Serial.print("Temperature: ");
Serial.print(temperature);
Serial.println(" °C");

// Use other values as needed
```

### Custom I2C and I2S Pins

You can specify custom I2C pins for BME280:

```cpp
// For ESP32, ESP8266, etc. with custom I2C pins
const int SDA_PIN = 16;
const int SCL_PIN = 17;

if (!sensors.begin(SDA_PIN, SCL_PIN)) {
  Serial.println("Could not find BME280 sensor!");
}
```

And custom I2S pins for INMP441:

```cpp
const int I2S_SCK_PIN = 16;  // Serial Clock
const int I2S_WS_PIN = 15;   // Word Select
const int I2S_SD_PIN = 7;    // Serial Data

if (!sensors.beginMicrophone(I2S_SCK_PIN, I2S_WS_PIN, I2S_SD_PIN)) {
  Serial.println("Could not initialize INMP441 microphone!");
}
```

## Configuration

You can customize the default settings by defining these before including the library:

```cpp
// Custom I2C pins (ESP32 default shown)
#define I2C_SDA 21
#define I2C_SCL 22

// Custom BME280 I2C address (default shown)
#define BME_ADDRESS 0x76

// Custom sea level pressure for altitude calculations
#define SEALEVELPRESSURE_HPA 1013.25

// Custom I2S pins for INMP441
#define I2S_SCK 16
#define I2S_WS 15
#define I2S_SD 7

// Custom sound level parameters
#define SAMPLE_RATE 16000      // Sample rate in Hz
#define SAMPLE_BITS 32         // Bits per sample
#define BUFFER_SIZE 512        // Buffer size for I2S and FFT
#define DB_SMOOTHING_FACTOR 0.8 // Smoothing factor for dB readings (0-1)
#define DB_OFFSET 35           // Calibration offset for dB calculation

#include <SensorManager.h>
```

## License

This library is released under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. 