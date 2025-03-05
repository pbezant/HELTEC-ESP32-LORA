# Heltec ESP32 LoRa Sensor Node

This project implements a sensor node using the Heltec ESP32 LoRa V3 board. It includes support for:

- OLED display with multiple screens and logging
- BME280 temperature, humidity, and pressure sensor
- Direct LoRa communication using RadioLib
- Deep sleep for power saving
- PIR motion sensor wake-up (optional)

## Project Structure

The project is organized in a modular way to make it easy to reuse components:

- `include/Config.h` - Central configuration file
- `include/DisplayManager.h` - OLED display management
- `include/SensorManager.h` - BME280 sensor handling
- `include/LoRaManager.h` - LoRa communication
- `include/secrets.h` - Credentials template (for API compatibility)
- `src/main.cpp` - Main application

## Libraries Used

This project uses the following libraries:

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

## Direct LoRa vs LoRaWAN

This project uses direct point-to-point LoRa communication rather than LoRaWAN:

- **Simpler Implementation**: No need for network servers or gateways
- **Lower Overhead**: Direct communication reduces protocol overhead
- **Longer Range**: Can use higher spreading factors without duty cycle limits
- **More Control**: Full control over frequency, bandwidth, and other parameters
- **Peer-to-Peer**: Can communicate directly with other nodes

If you need LoRaWAN functionality, you may need to modify the `LoRaManager` class to use a LoRaWAN library.

## Extending the Project

The modular design makes it easy to add new features:

- Add new sensors by extending the SensorManager class
- Create new display screens by adding cases to the setScreen method
- Implement additional LoRa functionality as needed

## License

This project is released under the MIT License. 