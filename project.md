# Project Technical Blueprint

## Core Architecture

- **Hardware Platform**: ESP32-based Heltec WiFi LoRa 32 V3 board
- **Primary Framework**: Arduino framework on ESP32 (espressif32 platform)
- **Development Environment**: PlatformIO for project management, building, and deployment
- **Code Organization**: Modular architecture with separate library components for key functionalities
- **Project Structure**: Follows PlatformIO conventions with `/src`, `/include`, `/lib`, and `/test` directories

## Technology Stack

### Hardware
- **MCU**: ESP32 (Heltec WiFi LoRa 32 V3)
- **Communication**: LoRa radio module (SX126x series)
- **Sensors**: BME280 temperature/humidity/pressure sensor
- **Display**: OLED display (built into Heltec board)

### Software
- **Language**: C++ (GNU++14 standard)
- **Core Libraries**:
  - Heltec ESP32 Dev-Boards (v1.1.1): Base board support
  - RadioLib (v6.2.0): RF communication handling
  - Adafruit BME280: Environmental sensor interface
  - Adafruit Unified Sensor: Sensor abstraction layer
  - U8g2: OLED display control
  - Unity: Testing framework

### Custom Libraries
- **LoRaManager**: Abstracts LoRa communication details
- **DisplayManager**: Handles OLED display functionality
- **SensorManager**: Manages sensor initialization, reading, and data processing

## Communication Patterns

- **LoRa Communication**: Uses RadioLib for packet-based wireless communication
- **Serial Debugging**: 115200 baud with time, color, and logging filters
- **Sensor Data Flow**: Sensor → SensorManager → Main Application → DisplayManager/LoRaManager

## Build Configuration

- **Compiler Flags**: 
  - GNU++14 standard
  - Wall and Wextra for comprehensive warnings
  - Debug level 5 for verbose logging
  - RadioLib enabled via USE_RADIOLIB define

## Deployment Process
- **Upload Protocol**: esptool
- **Upload Speed**: 115200 baud
- **Reset Strategy**: Default reset before upload, hard reset after

## Testing Strategy
- **Framework**: Unity for unit testing
- **Test Location**: `/test` directory for test cases
- **Test Targets**: Individual components and integrated functionality 