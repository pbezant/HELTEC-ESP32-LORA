# SensorManager

A simple Arduino library for managing BME280 and other sensors. This library provides a clean interface for initializing and reading data from BME280 temperature, humidity, and pressure sensors.

## Features

- Easy I2C setup and management for BME280 sensors
- Automatic detection of BME280 at common I2C addresses (0x76 and 0x77)
- Support for temperature, humidity, pressure, and altitude readings
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

## Usage

### Basic Example

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

### Custom I2C Pins

You can specify custom I2C pins when initializing:

```cpp
// For ESP32, ESP8266, etc. with custom I2C pins
const int SDA_PIN = 16;
const int SCL_PIN = 17;

if (!sensors.begin(SDA_PIN, SCL_PIN)) {
  Serial.println("Could not find BME280 sensor!");
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

#include <SensorManager.h>
```

## License

This library is released under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. 