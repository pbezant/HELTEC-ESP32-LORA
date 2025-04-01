# DisplayManager Library

A comprehensive library for managing OLED displays on ESP32 boards, specifically designed for Heltec WiFi LoRa 32 devices. This library provides a simple interface for displaying information, creating UI elements, and logging data to both the display and serial output.

## Features

- Easy initialization and configuration of OLED displays
- Multiple screen management with different preset layouts
- Drawing utilities for text, shapes, and progress bars
- Built-in screens for:
  - Startup/initialization
  - LoRaWAN status
  - Sensor data
  - System logs
  - Error messages
- Integrated logging system with serial output support

## Installation

1. Place the `DisplayManager` folder in your PlatformIO project's `lib` directory
2. Include the library in your sketch with `#include <DisplayManager.h>`

## Hardware Requirements

- ESP32 board with an SSD1306 OLED display (default pins SDA=17, SCL=18, RST=21)
- Compatible with Heltec WiFi LoRa 32 V3 boards

## Dependencies

- U8g2 library (for OLED display)
- Arduino framework for ESP32

## Usage

### Basic Example

```cpp
#include <Arduino.h>
#include <DisplayManager.h>
#include <DisplayLogger.h>

// Create display manager instance
DisplayManager display;
DisplayLogger logger(display);

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  display.begin();
  
  // Show startup screen
  display.setScreen(1);
  
  // Update progress as your application initializes
  display.updateStartupProgress(25, "Connecting WiFi...");
  delay(1000);
  display.updateStartupProgress(50, "Joining LoRaWAN...");
  delay(1000);
  display.updateStartupProgress(75, "Reading sensors...");
  delay(1000);
  display.updateStartupProgress(100, "Ready!");
  delay(1000);
  
  // Switch to sensor data screen
  display.setScreen(3);
  
  // Log some messages
  logger.info("System initialized");
}

void loop() {
  // Update sensor data on the display (screen 3)
  display.updateSensorData(24.5, 65.0, 1013.2, 3.8);
  
  // Log a message
  logger.debug("Sensor data updated");
  
  delay(5000);
  
  // Switch to LoRaWAN status screen
  display.setScreen(2);
  
  // Update LoRaWAN status
  display.updateLoRaWANStatus(true, -105, 10, 2);
  
  // Log a message
  logger.info("LoRaWAN status updated");
  
  delay(5000);
  
  // Show log screen
  display.setScreen(4);
  
  delay(5000);
}
```

### Using the Logger

The `DisplayLogger` class provides a simple interface for logging messages to both the display and serial output:

```cpp
#include <Arduino.h>
#include <DisplayManager.h>
#include <DisplayLogger.h>

DisplayManager display;
DisplayLogger logger(display);

void setup() {
  Serial.begin(115200);
  display.begin();
  
  // Log messages with different levels
  logger.info("System starting up");
  logger.debug("Debug information");
  logger.warning("Warning message");
  logger.error("Error occurred");
  
  // Show the log screen
  logger.showLogScreen();
}

void loop() {
  // Your code here
}
```

### Integration with Heltec ESP32 LoRa Projects

To use this library in your Heltec ESP32 LoRa project:

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <DisplayManager.h>
#include <DisplayLogger.h>
#include <SensorManager.h>
#include <LoRaManager.h>

// Global instances
DisplayManager display;
DisplayLogger logger(display);
SensorManager sensors;
LoRaManager lora;

void setup() {
  Serial.begin(115200);
  
  // Initialize display with startup screen
  display.begin();
  display.setScreen(1); // Startup screen
  display.updateStartupProgress(10, "Initializing...");
  
  // Initialize other components
  Wire.begin(SDA_PIN, SCL_PIN);
  logger.info("I2C initialized");
  
  // Initialize sensors
  if (sensors.begin()) {
    logger.info("Sensors initialized");
  } else {
    logger.error("Sensor initialization failed");
  }
  
  // Initialize LoRa
  if (lora.begin()) {
    logger.info("LoRa initialized");
    display.updateStartupProgress(100, "Ready!");
  } else {
    logger.error("LoRa initialization failed");
    display.showErrorScreen("Hardware Error", "LoRa initialization failed");
  }
}

void loop() {
  // Read sensor data
  float temperature, humidity, pressure, battery;
  sensors.readSensorData(temperature, humidity, pressure);
  
  // Update display with current data
  display.updateSensorData(temperature, humidity, pressure, battery);
  
  // Send data via LoRa
  if (lora.sendData()) {
    logger.info("Data sent successfully");
  } else {
    logger.error("Failed to send data");
  }
  
  delay(5000);
}
```

## API Reference

### DisplayManager Class

#### Initialization

- `DisplayManager()` - Constructor
- `begin(int sda = -1, int scl = -1)` - Initialize the display with optional custom pins

#### Display Control

- `clear()` - Clear the display
- `refresh()` - Update the display with buffered content
- `sleep()` - Put the display in sleep mode
- `wakeup()` - Wake up the display from sleep mode
- `setContrast(uint8_t contrast)` - Set display contrast (0-255)
- `setNormalMode()` - Set display to normal mode

#### Drawing Functions

- `drawString(int x, int y, const String& text)` - Draw text at position
- `drawCenteredString(int y, const String& text)` - Draw horizontally centered text
- `drawRightAlignedString(int y, const String& text)` - Draw right-aligned text
- `drawProgressBar(int x, int y, int width, int height, uint8_t progress)` - Draw progress bar
- `drawRect(int x, int y, int width, int height)` - Draw rectangle outline
- `fillRect(int x, int y, int width, int height)` - Draw filled rectangle
- `drawLine(int x0, int y0, int x1, int y1)` - Draw line
- `setFont(const uint8_t* font)` - Set text font

#### Screen Management

- `setScreen(uint8_t screenIndex)` - Switch to a specific screen
  - 0: Main info screen (custom)
  - 1: Startup screen
  - 2: LoRaWAN status screen
  - 3: Sensor data screen
  - 4: Log screen

#### Screen Content Updates

- `updateStartupProgress(uint8_t progress, const String& statusText)` - Update startup screen
- `updateLoRaWANStatus(bool joined, int16_t rssi, uint32_t uplinks, uint32_t downlinks)` - Update LoRaWAN status screen
- `updateSensorData(float temperature, float humidity, float pressure, float battery)` - Update sensor data screen
- `showErrorScreen(const String& title, const String& errorMsg)` - Show error screen
- `showLoRaError(int errorCode)` - Show LoRa error with translated message

#### Logging

- `log(const String& message)` - Add message to log buffer
- `clearLog()` - Clear log buffer

### DisplayLogger Class

- `DisplayLogger(DisplayManager& displayManager, bool echoToSerial = true)` - Constructor
- `info(const String& message)` - Log information message
- `warning(const String& message)` - Log warning message
- `error(const String& message)` - Log error message
- `debug(const String& message)` - Log debug message
- `clear()` - Clear log buffer
- `setSerialEcho(bool echo)` - Enable/disable serial output
- `showLogScreen()` - Switch to log screen

## License

MIT License 