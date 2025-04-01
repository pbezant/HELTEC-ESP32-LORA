# Heltec ESP32 LoRa Sensor Node

This project implements a sensor node using the Heltec ESP32 LoRa V3 board. It includes support for:

- OLED display with multiple screens and logging (using the DisplayManager library)
- BME280 temperature, humidity, and pressure sensor
- LoRaWAN communication using RadioLib
- Deep sleep for power saving
- PIR motion sensor wake-up (optional)

## Project Structure

The project is organized in a modular way to make it easy to reuse components:

- `include/Config.h` - Central configuration file
- `include/SensorManager.h` - BME280 sensor handling
- `include/secrets.h` - Credentials template for LoRaWAN
- `src/main.cpp` - Main application
- `lib/DisplayManager` - OLED display and logging management
- `lib/LoRaManager` - LoRaWAN communication

## Libraries Used

This project uses the following libraries:

- **DisplayManager**: Custom library for OLED display and logging
- **U8g2**: Modern, feature-rich display library
- **RadioLib**: Comprehensive library for LoRa communication
- **Adafruit BME280**: Sensor library for temperature, humidity and pressure
- **Heltec ESP32 Dev-Boards**: Support library for the Heltec ESP32 board

## Setup

1. Clone this repository
2. Open the project in PlatformIO
3. Optionally edit `include/Config.h` to adjust LoRa parameters
4. Connect your Heltec ESP32 LoRa V3 board
5. Build and upload the firmware

## Hardware Connections

- BME280 sensor: Connected to I2C pins (SDA: 46, SCL: 45)
- PIR motion sensor (optional): Connected to pin 5
- OLED display: Connected to SDA: 17, SCL: 18, RST: 21 (handled internally by the library)

## Configuration

You can customize the behavior by editing `include/Config.h`:

- Change I2C pins
- Adjust LoRa parameters (frequency, bandwidth, SF, power)
- Modify sleep duration
- Enable/disable features
- Configure LoRa module pins

## Power Saving

The device uses deep sleep to conserve power:

- The OLED display turns off after a configurable timeout
- The device goes to deep sleep after sending data
- It can be woken up by a timer or a PIR motion sensor

## DisplayManager Library

The project now includes a custom DisplayManager library for managing the OLED display:

- Located in the `lib/DisplayManager` directory
- Provides a reusable interface for OLED display management
- Includes a logging system with different severity levels
- Features multiple screen types (startup, LoRaWAN status, sensor data, logs)
- Easy to integrate into other ESP32 projects

### Using DisplayManager

```cpp
#include <DisplayManager.h>
#include <DisplayLogger.h>

// Create instances
DisplayManager display;
DisplayLogger logger(display);

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  display.begin();
  
  // Show startup screen with progress
  display.setScreen(1);
  display.updateStartupProgress(50, "Initializing...");
  
  // Log messages at different levels
  logger.info("System starting");
  logger.warning("Warning message");
  logger.error("Error message");
  logger.debug("Debug info");
}

void loop() {
  // Show sensor data
  display.setScreen(3);
  display.updateSensorData(23.5, 65.0, 1013.2, 3.7);
  
  // Show LoRaWAN status
  display.setScreen(2);
  display.updateLoRaWANStatus(true, -105, 10, 2);
  
  // Show log screen
  display.setScreen(4);
  
  delay(5000);
}
```

## Extending the Project

The modular design makes it easy to add new features:

- Add new sensors by extending the SensorManager class
- Create new display screens by adding cases to the setScreen method
- Implement additional LoRa functionality as needed

## License

This project is released under the MIT License. 